#include "tiger/liveness/liveness.h"
#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/x64frame.h"
#include "tiger/output/logger.h"
#include <iostream>
#include <string>

extern frame::RegManager *reg_manager;

namespace live {

bool MoveList::Contain(INodePtr src, INodePtr dst) {
  return std::any_of(move_list_.cbegin(), move_list_.cend(),
                     [src, dst](std::pair<INodePtr, INodePtr> move) {
                       return move.first == src && move.second == dst;
                     });
}

void MoveList::Delete(INodePtr src, INodePtr dst) {
  assert(src && dst);
  auto move_it = move_list_.begin();
  for (; move_it != move_list_.end(); move_it++) {
    if (move_it->first == src && move_it->second == dst) {
      break;
    }
  }
  move_list_.erase(move_it);
}

MoveList *MoveList::Union(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : move_list_) {
    res->move_list_.push_back(move);
  }
  for (auto move : list->GetList()) {
    if (!res->Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

MoveList *MoveList::Intersect(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : list->GetList()) {
    if (Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

void LiveGraphFactory::LiveMap() {
  /* TODO: Put your lab6 code here */

  /// KH-note: calculate in and out based on input flowgraph_
  /// (Constant Propagation is not included in this lab)
  /// first, initialize the "map": set all L(x, p) to false
  auto & nodes = this->flowgraph_->Nodes()->GetList();
  for (auto instr_node : nodes) {
    this->in_->Enter(instr_node, new temp::TempList());
    this->out_->Enter(instr_node, new temp::TempList());
  }
  /// KH-note: we have return_sink in ProcEntryExit2 that will add %rax to out_

  /// KH-note: parse by step, until no more changes can be done
  bool maybe_changed = true;
  while (maybe_changed) {
    /// KH-note: any change will set this to true
    maybe_changed = false;

    /// KH-note: check by node with Liveness Set equations
    for (auto instr_node : nodes) {
      auto ins = instr_node->NodeInfo();
      auto ori_in = this->in_->Look(instr_node);
      auto ori_out = this->out_->Look(instr_node);

      /// KH-note: in[s] = use[s] <union> (out[s] – def[s])
      /// %rsp shouldn't participate here
      auto new_in = ins->Use()->Union(ori_out->Diff(ins->Def()));
      new_in->Delete(reg_manager->GetRegister(frame::X64RegManager::RSP));
      if (!new_in->Equal(ori_in)) {
        maybe_changed = true;
      }
      /// KH-note: out[s] = <UNION forall t in s.succ[]> in[t]
      auto new_out = new temp::TempList();
      for (auto succ_node : instr_node->Succ()->GetList()) {
        new_out = new_out->Union(this->in_->Look(succ_node));
      }
      new_out->Delete(reg_manager->GetRegister(frame::X64RegManager::RSP));
      if (!new_out->Equal(ori_out)) {
        maybe_changed = true;
      }
      
      this->in_->Set(instr_node, new_in);
      this->out_->Set(instr_node, new_out);
    }
  }

  /*
  TigerLog("KH liveness start\n");
  temp::Map *color = temp::Map::LayerMap(reg_manager->temp_map_, temp::Map::Name());
  for (auto n : nodes) {
    assem::InstrList lis;
    lis.Append(n->NodeInfo());
    cg::AssemInstr asi(&lis);
    TigerLog(&asi, color);
    TigerLog("in_: ");
    for (auto nb : this->in_->Look(n)->GetList()) {
      TigerLog(" \"");
      TigerLog(*(color->Look(nb)));
      TigerLog("\"");
    }
    TigerLog("\n");
    TigerLog("ou_: ");
    for (auto nb : this->out_->Look(n)->GetList()) {
      TigerLog(" \"");
      TigerLog(*(color->Look(nb)));
      TigerLog("\"");
    }
    TigerLog("\n");
  }
  TigerLog("KH liveness end\n");
  */
}

void LiveGraphFactory::InterfGraph() {
  /* TODO: Put your lab6 code here */

  /// KH-note: build interfere graph with in_ and out_
  /// step1. precolor all registers. Registers() doesn't include %rsp.
  auto & reg_list = reg_manager->Registers()->GetList();
  for (auto reg : reg_list) {
    auto reg_node = this->live_graph_.interf_graph->NewNode(reg);
    this->temp_node_map_->Enter(reg, reg_node);
  }
  for (auto itr = reg_list.begin(); itr != reg_list.end(); itr ++) {
    auto succ_itr = itr;
    succ_itr ++;
    for (; succ_itr != reg_list.end(); succ_itr ++) {
      /// KH-note: interfere graph is not directional, 
      /// But I still create <-> edges for convenience
      auto node1 = this->temp_node_map_->Look(*itr);
      auto node2 = this->temp_node_map_->Look(*succ_itr);
      this->live_graph_.interf_graph->AddEdge(node1, node2);
      this->live_graph_.interf_graph->AddEdge(node2, node1);
    }
  }

  /// KH-note: step2. add all **living** temps to graph
  /// use in_ and out_ to see whether a temp is living.
  /// dead temp won't exist in temp_node_map_
  auto add_temp = [this](temp::Temp * t) -> void {
    if (t == reg_manager->GetRegister(frame::X64RegManager::RSP)) {
      return;
    }
    if (this->temp_node_map_->Look(t)) {
      return;
    }
    auto temp_node = this->live_graph_.interf_graph->NewNode(t);
    this->temp_node_map_->Enter(t, temp_node);
  };
  for (auto node : this->flowgraph_->Nodes()->GetList()) {
    for (auto ins : this->in_->Look(node)->GetList()) {
      add_temp(ins);
    }
    for (auto outs : this->out_->Look(node)->GetList()) {
      add_temp(outs);
    }
  }

  /// KH-note: actually fill the graph.
  /// 2 temps interfere if they appear in out[n] for any n
  for (auto nd : this->flowgraph_->Nodes()->GetList()) {
    /// KH-note: this won't include %rsp (see in LiveMap())
    auto outs = this->out_->Look(nd)->GetList();
    for (auto out1 = outs.begin(); out1 != outs.end(); out1 ++) {
      auto node1 = this->temp_node_map_->Look(*out1);
      auto out2 = out1;
      out2 ++;
      for (; out2 != outs.end(); out2 ++) {
        auto node2 = this->temp_node_map_->Look(*out2);
        if (node1->GoesTo(node2)) {
          /// KH-note: already matched
          continue;
        }
        this->live_graph_.interf_graph->AddEdge(node1, node2);
        this->live_graph_.interf_graph->AddEdge(node2, node1);
      }
    }

    /// KH-note: additionaly, set the movemap.
    /// Bcuz of my bad impl in lab5, I must also check OperInstr and find all 'movq t1, t2'...
    /// there won't be an edge between src and dst if they are temps, bcuz src dies after the move.
    auto instr = nd->NodeInfo();
    auto uses = instr->Use();
    auto defs = instr->Def();
    if (uses->GetList().size() != 1 || defs->GetList().size() != 1) {
      continue;
    }
    std::string assem = "";
    if (auto OI = dynamic_cast<assem::OperInstr *>(instr)) {
      assem = OI->assem_;
    }
    if (auto MI = dynamic_cast<assem::MoveInstr *>(instr)) {
      assem = MI->assem_;
    }
    if (assem.substr(0, 4) != "movq" || assem.find("(") != std::string::npos) {
      continue;
    }
    auto src_node = this->temp_node_map_->Look(uses->NthTemp(0));
    auto dst_node = this->temp_node_map_->Look(defs->NthTemp(0));
    if (src_node && dst_node && src_node != dst_node) {
      /// KH-note: src or dst might be dead
      this->live_graph_.moves->Append(src_node, dst_node);
    }
  }
}

void LiveGraphFactory::Liveness() {
  LiveMap();
  InterfGraph();
}

} // namespace live
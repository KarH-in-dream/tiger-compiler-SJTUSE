#include "tiger/liveness/flowgraph.h"
#include "tiger/codegen/assem.h"
#include "tiger/output/logger.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
namespace fg {

void FlowGraphFactory::AssemFlowGraph() {
  /* TODO: Put your lab6 code here */

  /// KH-note: build flow-graph based on given instr_list.
  /// Note that reg allocation is in scope of a single function.
  /// Each "BasicBlock" still ends with a jmp (excluding conditional jumps) or retq?
  /// - NO! for example the first block before block "a_a" in function "a"
  
  /// KH-note: step1. construct basis of the map, including:
  /// - create node for all instrs and insert to graph
  /// - record label nodes
  /// - create edge from a node's pred to itself, excluding cases when its pred is jmp or ret
  ///   emmm actually (is successor of jmp) equals (is label instr)
  FNodePtr last_node = nullptr;
  for (auto ins : this->instr_list_->GetList()) {
    auto node = this->flowgraph_->NewNode(ins);
    if (auto label = dynamic_cast<assem::LabelInstr *>(ins)) {
      (*this->label_map_)[label->label_->Name()] = node;
    }
    if (last_node) {
      /// KH-note: add if last node is not pure jmp
      bool link_to_prev = true;
      if (auto oper = dynamic_cast<assem::OperInstr *>(last_node->NodeInfo())) {
        auto code = oper->assem_.substr(0, 3);
        if (code == "jmp" || code == "ret") {
          link_to_prev = false;
        }
      }
      if (link_to_prev) {
        this->flowgraph_->AddEdge(last_node, node);
      }
    }
    last_node = node;
  }

  /// KH-note: step2. add jump edges.
  /// Only OperInstr has jump target.
  for (auto ins : this->instr_list_->GetList()) {
    auto oper = dynamic_cast<assem::OperInstr *>(ins);
    if (oper == nullptr || oper->jumps_ == nullptr) {
      continue;
    }

    auto & nodes = this->flowgraph_->Nodes()->GetList();
    auto node_itr = std::find_if(nodes.begin(), nodes.end(),
      [ins](FNodePtr n) -> bool { return n->NodeInfo() == ins; });
    if (node_itr == nodes.end()) {
      TigerLog("node not found in graph\n");
      continue;
    }

    for (auto target : (*oper->jumps_->labels_)) {
      auto target_node = this->label_map_->find(target->Name());
      if (target_node == this->label_map_->end()) {
        TigerLog("target not found in map\n");
        continue;
      }
      this->flowgraph_->AddEdge(*node_itr, target_node->second);
    }
  }
}

} // namespace fg

namespace assem {

temp::TempList *LabelInstr::Def() const { return new temp::TempList(); }

temp::TempList *MoveInstr::Def() const { return dst_; }

temp::TempList *OperInstr::Def() const { return dst_; }

temp::TempList *LabelInstr::Use() const { return new temp::TempList(); }

temp::TempList *MoveInstr::Use() const {
  return (src_) ? src_ : new temp::TempList();
}

temp::TempList *OperInstr::Use() const {
  return (src_) ? src_ : new temp::TempList();
}
} // namespace assem

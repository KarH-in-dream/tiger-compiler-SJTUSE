#include "tiger/regalloc/regalloc.h"

#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/x64frame.h"
#include "tiger/liveness/liveness.h"
#include "tiger/output/logger.h"
#include "tiger/regalloc/color.h"
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

extern frame::RegManager *reg_manager;
extern std::map<std::string, std::pair<int, int>> frame_info_map;

namespace ra {
/* TODO: Put your lab6 code here */

void RegAllocator::RegAlloc() {
  // std::cerr << "reg alloc for " << this->function_name << std::endl;
  while (true) {
    fg::FlowGraphFactory fgf(this->instr_list);
    fgf.AssemFlowGraph();
    auto flowgraph = fgf.GetFlowGraph();

    live::LiveGraphFactory lgf(flowgraph);
    lgf.Liveness();
    
    this->temp_node_map_ = lgf.GetTempNodeMap();
    auto col = col::Color(lgf.GetLiveGraph(), this->temp_node_map_);
    auto col_res = col.DoColoring();

    /// KH-note: remove dead temps (and instrs) in instr_list
    this->ir_clean();

    if (col_res.spills == nullptr || col_res.spills->GetList().size() == 0) {
      /// KH-note: valid coloring, no need to rewrite
      this->coloring_ = col_res.coloring;
      this->dupmove_clean();
      this->setxx_fix();
      break;
    }
    else {
      this->rewrite(col_res.spills);
    }
  }
}

std::unique_ptr<Result> RegAllocator::TransferResult() {
  return std::make_unique<Result>(this->coloring_, this->instr_list);
}

void RegAllocator::ir_clean() {
  /// KH-note: remove instrs that includes dead temps
  /// dead temps only lives in Def()
  std::list<assem::Instr *> targets;
  for (auto instr : this->instr_list->GetList()) {
    int dead_cnt = 0;
    for (auto used : instr->Def()->GetList()) {
      if (reg_manager->Registers()->Contain(used) ||
        used == reg_manager->GetRegister(frame::X64RegManager::RSP)) {
        continue;
      }
      if (this->temp_node_map_->Look(used) == nullptr) {
        dead_cnt ++;
      }
    }
    if (dead_cnt == 0) {
      continue;
    }
    if (dead_cnt != instr->Def()->GetList().size()) {
      TigerLog("Only part of instr's Def() are dead");
      continue;
    }
    targets.push_back(instr);
  }
  for (auto instr : targets) {
    this->instr_list->Remove(instr);
  }
}

void RegAllocator::dupmove_clean() {
  /// KH-note: remove "movq %rxx, %rxx" in OperInstr
  std::list<assem::Instr *> targets;
  for (auto instr : this->instr_list->GetList()) {
    auto operi = dynamic_cast<assem::OperInstr *>(instr);
    if (!operi) {
      continue;
    }
    if (operi->assem_.substr(0, 4) != "movq") {
      continue;
    }
    if (operi->assem_.find("(") != std::string::npos) {
      continue;
    }
    if (operi->src_->GetList().size() == 1 && operi->dst_->GetList().size() == 1) {
      auto src = operi->src_->NthTemp(0);
      auto dst = operi->dst_->NthTemp(0);
      if (this->coloring_->Look(src) == this->coloring_->Look(dst)) {
        targets.push_back(instr);
      }
    }
  }
  for (auto instr : targets) {
    this->instr_list->Remove(instr);
  }
}

void RegAllocator::setxx_fix() {
  std::list<std::list<assem::Instr *>::const_iterator> targets;
  auto & list = this->instr_list->GetList();
  for (auto itr = list.begin(); itr != list.end(); itr ++) {
    auto operi = dynamic_cast<assem::OperInstr *>(*itr);
    if (!operi) {
      continue;
    }
    if (operi->assem_.substr(0, 3) != "set") {
      continue;
    }
    if (operi->src_->GetList().size() == 0 && operi->dst_->GetList().size() == 1) {
      targets.push_back(itr);
    }
  }
  for (auto itr : targets) {
    auto dst = new assem::OperInstr(
      "movq $0, `d0",
      new temp::TempList((*itr)->Def()->NthTemp(0)),
      new temp::TempList(),
      nullptr
    );
    this->instr_list->Insert(itr, dst);
  }
}

void RegAllocator::rewrite(live::INodeListPtr spills) {
  /// KH-note: for instr that includes a spilled temp
  /// if USE:
  /// - replace the old temp with a new one
  /// - insert code to read from the old temp's memory location
  /// if DEF:
  /// - replace the old temp with a new one
  /// - append code to store to the old temp's memory location

  /// KH-note: step1 - allocate space for spilled temps in memory
  /// access frame_info_map with function name to get the global framesize
  /// (the pair represents <frame.offset_, frame.calculateActualFramesize()>)
  std::unordered_map<temp::Temp *, int> offsets;
  auto & f_info = frame_info_map.at(this->function_name);
  for (auto tnode : spills->GetList()) {
    f_info.first -= 8;
    f_info.second += 8;
    /// KH-note: offset decrease first, then used (see previous code in x64frame.cc)
    offsets[tnode->NodeInfo()] = f_info.first;
  }

  /// KH-note: step2 - remember instrs that should be modified
  /// That's bcuz InstrList class is not friendly enough
  std::list<std::pair<std::list<assem::Instr *>::const_iterator, temp::Temp *>> target_instrs = {};
  auto & insl = this->instr_list->GetList();
  for (auto itr = insl.begin(); itr != insl.end(); itr ++) {
    for (auto tnode : spills->GetList()) {
      auto temp = tnode->NodeInfo();
      if ((*itr)->Use()->Contain(temp) || (*itr)->Def()->Contain(temp)) {
        target_instrs.push_back({ itr, temp });
      }
    }
  }

  /// KH-note: step3 - actually rewrite the code
  auto insert_getval = 
    [this, &offsets](std::list<assem::Instr *>::const_iterator pos, 
      temp::Temp * tmp_old, temp::Temp * tmp_new, bool load) -> void {
      auto addr_tmp = temp::TempFactory::NewTemp();
      auto load_global_ins = new assem::OperInstr(
        "movq " + this->function_name + "_framesize_global(%rip), `d0",
        new temp::TempList(addr_tmp),
        new temp::TempList(),
        nullptr);
      auto rsp_tmp = temp::TempFactory::NewTemp();
      auto load_rsp_ins = new assem::OperInstr(
        "movq %rsp, `d0",
        new temp::TempList(rsp_tmp),
        new temp::TempList(),
        nullptr);
      auto calc_fp_ins = new assem::OperInstr(
        "addq `s0, `d0",
        new temp::TempList(addr_tmp),
        new temp::TempList({ rsp_tmp, addr_tmp }),
        nullptr);
      auto calc_addr_ins = new assem::OperInstr(
        "addq $" + std::to_string(offsets[tmp_old]) + ", `d0",
        new temp::TempList(addr_tmp),
        new temp::TempList(addr_tmp),
        nullptr);
      auto operation_ins = new assem::OperInstr(
        load ? "movq (`s0), `d0" : "movq `s0, (`s1)",
        load ? new temp::TempList(tmp_new) : new temp::TempList(),
        load ? new temp::TempList(addr_tmp) : new temp::TempList({ tmp_new, addr_tmp }),
        nullptr);
      this->instr_list->Insert(pos, load_global_ins);
      this->instr_list->Insert(pos, load_rsp_ins);
      this->instr_list->Insert(pos, calc_fp_ins);
      this->instr_list->Insert(pos, calc_addr_ins);
      this->instr_list->Insert(pos, operation_ins);
    };
  for (auto [itr, temp] : target_instrs) {
    auto ntmp = temp::TempFactory::NewTemp();
    if ((*itr)->Use()->Contain(temp)) {
      (*itr)->Use()->Replace(temp, ntmp);
      insert_getval(itr, temp, ntmp, true);
    }
    if ((*itr)->Def()->Contain(temp)) {
      (*itr)->Def()->Replace(temp, ntmp);
      auto nitr = itr;
      nitr ++;
      insert_getval(nitr, temp, ntmp, false);
    }
  }
}

} // namespace ra
#include "tiger/codegen/codegen.h"
#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/output/logger.h"

#include <cassert>
#include <iostream>
#include <llvm-14/llvm/IR/Constants.h>
#include <llvm-14/llvm/IR/GlobalVariable.h>
#include <llvm-14/llvm/IR/InstrTypes.h>
#include <llvm-14/llvm/IR/Instruction.h>
#include <llvm-14/llvm/IR/Instructions.h>
#include <llvm-14/llvm/IR/Value.h>
#include <llvm-14/llvm/Support/Casting.h>
#include <sstream>
#include <string>

extern frame::RegManager *reg_manager;
extern frame::Frags *frags;

namespace {

constexpr int maxlen = 1024;

} // namespace

namespace cg {

void CodeGen::Codegen() {
  temp_map_ = new std::unordered_map<llvm::Value *, temp::Temp *>();
  bb_map_ = new std::unordered_map<llvm::BasicBlock *, int>();
  auto *list = new assem::InstrList();

  // firstly get all global string's location
  for (auto &&frag : frags->GetList()) {
    if (auto *str_frag = dynamic_cast<frame::StringFrag *>(frag)) {
      auto tmp = temp::TempFactory::NewTemp();
      list->Append(new assem::OperInstr(
          "leaq " + std::string(str_frag->str_val_->getName()) + "(%rip),`d0",
          new temp::TempList(tmp), new temp::TempList(), nullptr));
      temp_map_->insert({str_frag->str_val_, tmp});
    }
  }

  // move arguments to temp
  auto arg_iter = traces_->GetBody()->arg_begin();
  auto regs = reg_manager->ArgRegs();
  auto tmp_iter = regs->GetList().begin();

  // first arguement is rsp, we need to skip it
  ++arg_iter;

  /// KH-note: move all params except the first to temp value (can be optimized in lab6)
  for (; arg_iter != traces_->GetBody()->arg_end() &&
         tmp_iter != regs->GetList().end();
       ++arg_iter, ++tmp_iter) {
    auto tmp = temp::TempFactory::NewTemp();
    list->Append(new assem::OperInstr("movq `s0,`d0", new temp::TempList(tmp),
                                      new temp::TempList(*tmp_iter), nullptr));
    temp_map_->insert({static_cast<llvm::Value *>(arg_iter), tmp});
  }

  // pass-by-stack parameters
  if (arg_iter != traces_->GetBody()->arg_end()) {
    /// KH-note: a temp used to store the input %rsp
    auto last_sp = temp::TempFactory::NewTemp();
    list->Append(
        new assem::OperInstr("movq %rsp,`d0", new temp::TempList(last_sp),
                             new temp::TempList(reg_manager->GetRegister(
                                 frame::X64RegManager::Reg::RSP)),
                             nullptr));
    /// KH-note: calc fp, similiar to SimpleVar
    list->Append(new assem::OperInstr(
        "addq $" + std::string(traces_->GetFunctionName()) +
            "_framesize_local,`s0",
        new temp::TempList(last_sp),
        new temp::TempList({last_sp, reg_manager->GetRegister(
                                         frame::X64RegManager::Reg::RSP)}),
        nullptr));
    
    /// KH-note: load extra params from stack.
    /// The kth (k start from 1, the 0th is %rsp) param is at fp+8k in outgo area
    /// (bcuz in tiger we copy all args to outgo area inside the function)
    while (arg_iter != traces_->GetBody()->arg_end()) {
      auto tmp = temp::TempFactory::NewTemp();
      list->Append(new assem::OperInstr(
          "movq " +
              std::to_string(8 * (arg_iter - traces_->GetBody()->arg_begin())) +
              "(`s0),`d0",
          new temp::TempList(tmp), new temp::TempList(last_sp), nullptr));
      temp_map_->insert({static_cast<llvm::Value *>(arg_iter), tmp});
      ++arg_iter;
    }
  }

  // construct bb_map
  int bb_index = 0;
  for (auto &&bb : traces_->GetBasicBlockList()->GetList()) {
    bb_map_->insert({bb, bb_index++});
  }

  for (auto &&bb : traces_->GetBasicBlockList()->GetList()) {
    // record every return value from llvm instruction
    for (auto &&inst : bb->getInstList())
      temp_map_->insert({&inst, temp::TempFactory::NewTemp()});
  }

  /// KH-note: this is part of the PHI translation of mine.
  /// I choose to  move the ‘move’ instruction before the jmp instruction of the basic block
  /// But instr_list is a 1d list, which means the PHI incoming value may be translated after the PHI
  /// So I have to record the relations before all those InstrSels.
  this->phi_map.clear();
  for (auto &&bb : traces_->GetBasicBlockList()->GetList()) {
    for (auto & inst : bb->getInstList()) {
      if (auto * PHI = llvm::dyn_cast<llvm::PHINode>(&inst)) {
        for (auto i = 0; i < PHI->getNumIncomingValues(); i ++) {
          auto blo = PHI->getIncomingBlock(i);
          auto val = PHI->getIncomingValue(i);
          this->phi_map[blo][bb] = val;
        }
      }
    }
  }

  for (auto &&bb : traces_->GetBasicBlockList()->GetList()) {
    // Generate label for basic block
    list->Append(new assem::LabelInstr(std::string(bb->getName())));

    // Generate instructions for basic block
    for (auto &&inst : bb->getInstList())
      InstrSel(list, inst, traces_->GetFunctionName(), bb);
  }

  assem_instr_ = std::make_unique<AssemInstr>(frame::ProcEntryExit2(
      frame::ProcEntryExit1(traces_->GetFunctionName(), list)));
}

void AssemInstr::Print(FILE *out, temp::Map *map) const {
  for (auto instr : instr_list_->GetList())
    instr->Print(out, map);
  fprintf(out, "\n");
}

temp::Temp * CodeGen::llvm2temp(llvm::Value * input, std::string_view func_name) {
  auto tmp = reg_manager->GetRegister(frame::X64RegManager::Reg::RSP);  
  if (!this->IsRsp(input, func_name)) {
    auto find_res = this->temp_map_->find(input);
    if (find_res == this->temp_map_->end()) {
      TigerLog("can't find " + std::string(input->getName()) +
        " in function " + std::string(func_name) + "\n");
      /// KH-note: used to show the error space. Code in lab5 won't generate %r8
      ///   so if any errors, I can search %r8 in generated code to get its location
      // tmp = reg_manager->GetRegister(frame::X64RegManager::Reg::R8);
    }
    else {
      tmp = find_res->second;
    }
  }
  return tmp;
}
void CodeGen::createPhiJump(llvm::BasicBlock * bfrom, llvm::BasicBlock * bto,
  assem::InstrList *instr_list, std::string jmp_oper, std::string_view func_name){
  auto mpi = this->phi_map.find(bfrom);
  if (mpi != this->phi_map.end()) {
    auto smpi = mpi->second.find(bto);
    if (smpi != mpi->second.end()) {
      auto move_instr = 
        InstrBuilder("movq", func_name, this)
        .addValue(smpi->second, InstrBuilder::AS_SRC)
        ->addTemp(this->phi_temp_, InstrBuilder::AS_DST)
        ->build();
      instr_list->Append(move_instr);
    }
  }

  auto j_instr = new assem::OperInstr(
    jmp_oper + " `j0",
    new temp::TempList(),
    new temp::TempList(),
    new assem::Targets(new std::vector<temp::Label *> { 
      temp::LabelFactory::NamedLabel(std::string(bto->getName())) }));
  instr_list->Append(j_instr);
}

InstrBuilder * InstrBuilder::addValue(llvm::Value * val, int CFG) {
  
  this->addParam();

  if (auto * GV = llvm::dyn_cast<llvm::GlobalVariable>(val)) {
    auto gvn = std::string(GV->getName());
    this->i->assem_ += gvn + "(%rip)";
    /// KH-note: assem 'name(%rip)' represents its data, not address.
    /// Thus if not USE_MEM, the operation must be changed from movq to leaq.
    if (!(CFG & USE_MEM)) {
      if (this->i->assem_.substr(0, 4) != "movq" || (CFG & AS_DST)) {
        /// KH-note: otherwise it's invalid operation
        TigerLog("Call global variable's addr directly: " + gvn + "\n");
      }
      else {
        this->i->assem_.replace(0, 4, "leaq");
      }
    }
  }
  else if (auto CI = llvm::dyn_cast<llvm::ConstantInt>(val)) {
    auto extv = CI->getSExtValue();
    if (CI->getBitWidth() == 1) {
      /// KH-note: true (with type i1) will be signed extended to -1 (wtf...)
      /// I translate conditional br with cmpe $1, so I have to let true as 1 here.
      /// Maybe use cmpne $0 is better...
      extv = CI->getZExtValue();
    }
    this->i->assem_ += "$" + std::to_string(extv);
  }
  else if (auto NI = llvm::dyn_cast<llvm::ConstantPointerNull>(val)) {
    /// KH-note: yes, null is not int type.
    this->i->assem_ += "$0";
  }
  else {
    auto tmp = this->cg->llvm2temp(val, this->func);
    this->need_separator = false;
    return this->addTemp(tmp, CFG);
  }

  return this;
}
InstrBuilder * InstrBuilder::addTemp(temp::Temp * temp, int CFG) {

  this->addParam();

  if (!(CFG & AS_SRC) && !(CFG & AS_DST)) {
    TigerLog("unused temp!!!\n");
    return this;
  }
  std::string to_append = "";
  if (CFG & AS_SRC) {
    to_append = "`s" + std::to_string(this->i->src_->GetList().size());
    this->i->src_->Append(temp);
  }
  if (CFG & AS_DST) {
    to_append = "`d" + std::to_string(this->i->dst_->GetList().size());
    this->i->dst_->Append(temp);
  }
  if (CFG & USE_MEM) {
    to_append = "(" + to_append + ")";
  }
  this->i->assem_ += to_append;
  return this;
}

void CodeGen::InstrSel(assem::InstrList *instr_list, llvm::Instruction &inst,
                       std::string_view function_name, llvm::BasicBlock *bb) {
  // TASK: your lab5 code here

  /// KH-note: Instruction Selection for an Instr in current block
  /// instr_list is where the new instr should be appended to
  /// use llvm::dyn_cast for type checking. If can't cast, it will return nullptr like dynamic_cast<>()

  {
    /// KH-note: skip instructions to param %0 in llvm IR.
    /// SP calculation is now implemented by ProcEntryExit3
    /// Note in llvm IR we didn't recover %0 as %rsp
    auto param0 = this->traces_->GetBody()->getArg(0);
    for (auto op : inst.operand_values()) {
      if (op == param0) {
        return;
      }
    }
  }

  /// KH-note: 1. llvm::Instruction::Load
  if (auto * LI = llvm::dyn_cast<llvm::LoadInst>(&inst)) {
    /// KH-note: %res = load t, t* %src, align x
    /// %src is the first operand (use getOperand(0) is ok); %res is the inst itself
    auto src_addr = LI->getPointerOperand();
    auto load_instr = 
      InstrBuilder("movq", function_name, this)
      .addValue(src_addr, InstrBuilder::AS_SRC | InstrBuilder::USE_MEM)
      ->addValue(LI, InstrBuilder::AS_DST)
      ->build();
    instr_list->Append(load_instr);
    return;
  }

  /// KH-note: 2. llvm::Instruction::Add/Sub/Mul/SDiv
  /// These binary operators are likely to have type llvm::BinaryOperator,
  ///   but that's not true when LHS and RHS are all constant values (...)
  if (auto * BO = llvm::dyn_cast<llvm::BinaryOperator>(&inst)) {
    /// KH-note: translate instr to addq, subq, imulq and idivq.
    /// addq and subq are like 'oper %val, %dest' -> '%dest = oper i64 %dest, %val'
    ///   So we need to move LHS to a temp and then do the oper.
    /// idivq is different with form 'oper %val';
    ///   idivq: R[%rax] <- R[%rdx]:R[%rax] / %val, R[%rdx] <- R[%rdx]:R[%rax] % %val
    ///     bcuz idivq also use %rdx, a "cqto" should be inserted before idivq to set %rdx.
    /// imulq has 2 forms, the binary form is similiar to addq. the unary form:
    ///   imulq: R[%rdx]:R[%rax] <- R[%rax] * %val
    /// Here I have to use the binary form imulq bcuz tiger interpreter doesn't support unary form.
    /// In conclusion we need to: move src to %rax, (cqto), do the oper, move %rax to result temp.
    auto bo_code = BO->getOpcode();
    auto LHS = BO->getOperand(0);
    auto RHS = BO->getOperand(1);

    if (bo_code == llvm::Instruction::Add ||
        bo_code == llvm::Instruction::Sub || 
        bo_code == llvm::Instruction::Mul) {

      /// KH-note: normal BinOp, do the extra move
      auto move_instr = 
        InstrBuilder("movq", function_name, this)
        .addValue(LHS, InstrBuilder::AS_SRC)
        ->addValue(BO, InstrBuilder::AS_DST)
        ->build();
      instr_list->Append(move_instr);

      std::string oper_type = 
        (bo_code == llvm::Instruction::Add) ? "addq" : 
        (bo_code == llvm::Instruction::Sub) ? "subq" : "imulq";
      auto oper_instr = 
        InstrBuilder(oper_type, function_name, this)
        .addValue(RHS, InstrBuilder::AS_SRC)
        ->addValue(BO, InstrBuilder::AS_SRC | InstrBuilder::AS_DST)
        ->build();
      instr_list->Append(oper_instr);
    }
    else if (bo_code == llvm::Instruction::SDiv) {
      /// KH-note: abnormal Unary Op, do the %rax move
      auto rax_ = reg_manager->GetRegister(frame::X64RegManager::Reg::RAX);
      auto rdx_ = reg_manager->GetRegister(frame::X64RegManager::Reg::RDX);

      auto move_rax_instr = 
        InstrBuilder("movq", function_name, this)
        .addValue(LHS, InstrBuilder::AS_SRC)
        ->addTemp(rax_, InstrBuilder::AS_DST)
        ->build();
      instr_list->Append(move_rax_instr);

      auto cqto_instr = new assem::OperInstr(
        "cqto", 
        new temp::TempList(rdx_), 
        new temp::TempList(), 
        nullptr);
      instr_list->Append(cqto_instr);
      
      auto oper_instr = 
        InstrBuilder("idivq", function_name, this)
        .addValue(RHS, InstrBuilder::AS_SRC)
        ->build();
      oper_instr->src_->Append(rax_);
      oper_instr->dst_->Append(rax_);
      oper_instr->src_->Append(rdx_);
      instr_list->Append(oper_instr);

      auto move_res_instr = 
        InstrBuilder("movq", function_name, this)
        .addTemp(rax_, InstrBuilder::AS_SRC)
        ->addValue(BO, InstrBuilder::AS_DST)
        ->build();
      instr_list->Append(move_res_instr);
    }
    else {
      TigerLog("unknown llvm binary op " + std::to_string(bo_code) + "\n");
    }
    return;
  }
  
  /// KH-note: 3. llvm::Instruction::PtrToInt/IntToPtr
  if (auto * CI = llvm::dyn_cast<llvm::CastInst>(&inst)) {
    /// KH-note: %dst = ptrtoint pointerty %src to intty
    /// A simple move is ok bcuz we only use i64 %rxx registers
    auto src = CI->getOperand(0);
    auto move_instr = 
      InstrBuilder("movq", function_name, this)
      .addValue(src, InstrBuilder::AS_SRC)
      ->addValue(CI, InstrBuilder::AS_DST)
      ->build();
    instr_list->Append(move_instr);
    return;
  }

  /// KH-note: 4. llvm::Instruction::GetElementPtr
  if (auto * GEP = llvm::dyn_cast<llvm::GetElementPtrInst>(&inst)) {
    /// KH-note:
    /// In tiger record, each attr takes a wordsize (see in translate.cc, RecordExp)
    /// In tiger array, each elem takes sizeof(long) (see in runtime_llvm.cc, init_array)
    /// Thus everything in tiger IR accessed by GEP is 8byte!
    /// With this assumption I can make things a lot more easier (and much more tricky...)
    auto src = GEP->getPointerOperand();
    std::vector<llvm::Value*> indexList(GEP->idx_begin(), GEP->idx_end());

    /// KH-note: too tricky so I add extra check here
    if (indexList.empty()) {
      TigerLog("GEP index list is empty???\n");
    }
    llvm::Value * last_index = indexList.back();
    indexList.pop_back();
    for (auto v : indexList) {
      auto IDC = llvm::dyn_cast<llvm::ConstantInt>(v);
      if ((!IDC) || (IDC->getSExtValue() != 0)) {
        TigerLog("GEP normal index not 0 !!!\n");
        return;
      }
    }

    if (auto LIDC = llvm::dyn_cast<llvm::ConstantInt>(last_index)) {
      auto lea_instr = new assem::OperInstr(
        "leaq " + std::to_string(8 * LIDC->getSExtValue()) + "(`s0), `d0",
        new temp::TempList(this->llvm2temp(GEP, function_name)),
        new temp::TempList(this->llvm2temp(src, function_name)),
        nullptr
      );
      instr_list->Append(lea_instr);
    }
    else {
      /// KH-note: calc src+8*last_index.
      /// It's a pity: our interpreter only accepts leaq instrs like 'leaq const(temp), temp'
      instr_list->Append(InstrBuilder("movq", function_name, this)
        .addValue(last_index, InstrBuilder::AS_SRC)
        ->addValue(GEP, InstrBuilder::AS_DST)
        ->build());
      instr_list->Append(InstrBuilder("imulq $8,", function_name, this)
        .addValue(GEP, InstrBuilder::AS_SRC | InstrBuilder::AS_DST)
        ->build());
      instr_list->Append(InstrBuilder("addq", function_name, this)
        .addValue(src, InstrBuilder::AS_SRC)
        ->addValue(GEP, InstrBuilder::AS_SRC | InstrBuilder::AS_DST)
        ->build());
    }
    return;
  }

  /// KH-note: 5. llvm::Instruction::Store
  if (auto * SI = llvm::dyn_cast<llvm::StoreInst>(&inst)) {
    /// KH-note: store t %val, t* %dst, align x (no result)
    auto src_val = SI->getValueOperand();
    auto dst_addr = SI->getPointerOperand();
    /// KH-note: store instr doesn't define any temp as return value
    auto move_instr = 
      InstrBuilder("movq", function_name, this)
      .addValue(src_val, InstrBuilder::AS_SRC)
      ->addValue(dst_addr, InstrBuilder::AS_SRC | InstrBuilder::USE_MEM)
      ->build();
    instr_list->Append(move_instr);
    return;
  }

  /// KH-note: 6. llvm::Instruction::BitCast/ZExt
  if (auto * CI = llvm::dyn_cast<llvm::CastInst>(&inst)) {
    /// KH-note: %res = zext intty_s %src to intty_l
    /// I decide to simply ignore int type differences bcuz 1-bit condition is stored
    ///   in i64 %rxx in this lab, not buffered in cond reg.
    /// I will create cmp on my own when I need an i1 as condition in br.
    auto src = CI->getOperand(0);
    auto move_instr = 
      InstrBuilder("movq", function_name, this)
      .addValue(src, InstrBuilder::AS_SRC)
      ->addValue(CI, InstrBuilder::AS_DST)
      ->build();
    instr_list->Append(move_instr);
    return;
  }

  /// KH-note: 7. llvm::Instruction::Call
  if (auto * CI = llvm::dyn_cast<llvm::CallInst>(&inst)) {
    /// KH-note: call retty @func([argty args])
    /// The first arg for tiger's func may be current %rsp, which should be ignored.
    /// Here's 3 steps:
    /// - Pass arg values, some of them may be passed through stack
    /// - 'call function_name'
    /// - move %rax to temp
    auto rax_ = reg_manager->GetRegister(frame::X64RegManager::Reg::RAX);
    
    std::vector<llvm::Value*> args(CI->arg_begin(), CI->arg_end());
    auto & argregs = reg_manager->ArgRegs()->GetList();
    auto itr_arg = args.begin();
    auto itr_reg = argregs.begin();
    /// KH-note: if the called function is tiger-function, then the %rsp arg must be
    ///   current frame's rsp (and not passed by reg)
    /// Caution some built-in functions have no args
    bool skip_rsp = itr_arg != args.end() && this->IsRsp(*itr_arg, function_name);
    if (skip_rsp) {
      itr_arg ++;
    }
    for (; itr_arg != args.end(); itr_arg ++) {
      if (itr_reg == argregs.end()) {
        /// KH-note: extra args need to be passed by stack.
        /// The offset may not be correct for built-in functions that don't have rsp as the first arg
        /// But these functions won't have more than 6 args LOL
        auto instr = 
          InstrBuilder("movq", function_name, this)
          .addValue(*itr_arg, InstrBuilder::AS_SRC)
          ->build();
        instr->assem_ += ", " + std::to_string(8 * (itr_arg - args.begin())) + "(%rsp)";
        instr_list->Append(instr);
      }
      else {
        /// KH-note: pass by reg
        auto instr = 
          InstrBuilder("movq", function_name, this)
          .addValue(*itr_arg, InstrBuilder::AS_SRC)
          ->addTemp(*itr_reg, InstrBuilder::AS_DST)
          ->build();
        itr_reg ++;
        instr_list->Append(instr);
      }
    }

    /// KH-note: not "getFunction()"
    auto func = CI->getCalledFunction();
    auto call_instr = new assem::OperInstr(
      "call " + func->getName().str(),
      new temp::TempList(rax_),
      new temp::TempList(),
      nullptr
    );
    instr_list->Append(call_instr);

    auto res_instr = 
      InstrBuilder("movq", function_name, this)
      .addTemp(rax_, InstrBuilder::AS_SRC)
      ->addValue(CI, InstrBuilder::AS_DST)
      ->build();
    instr_list->Append(res_instr);
    return;
  }

  /// KH-note: 8. llvm::Instruction::Ret
  if (auto * RI = llvm::dyn_cast<llvm::ReturnInst>(&inst)) {
    /// KH-note: ret type %val
    /// The real ret is in ProcEntryExit(1? or 3?), just move result to %rax and jump there.
    auto rax_ = reg_manager->GetRegister(frame::X64RegManager::Reg::RAX);
    if (auto val = RI->getReturnValue()) {
      /// KH-note: function may return void
      auto instr = 
        InstrBuilder("movq", function_name, this)
        .addValue(RI->getOperand(0), InstrBuilder::AS_SRC)
        ->addTemp(rax_, InstrBuilder::AS_DST)
        ->build();
      instr_list->Append(instr);
    }

    /// KH-note: use named label to access a unique label. It's similiar to sym::Symbol
    auto ret_instr = new assem::OperInstr(
      "jmp `j0",
      new temp::TempList(),
      new temp::TempList(),
      new assem::Targets(new std::vector<temp::Label *> { 
        temp::LabelFactory::NamedLabel(std::string(function_name) + "_return") }));
    instr_list->Append(ret_instr);
    return;
  }

  /// KH-note: 9. llvm::Instruction::Br
  if (auto * RI = llvm::dyn_cast<llvm::BranchInst>(&inst)) {
    /// KH-note: to impl the PHI instr, we need to use phi_temp to record
    ///   the result of current branch before jmps.
    /// It assumes PHI is called instantly after jmps.
    /// This should be done even if the br is conditional bcuz of translation of AND and OR.

    if (RI->isConditional()) {
      auto cond = RI->getCondition();
      /// KH-note: maybe "cmpneq $0" is better here?
      auto cmp_instr = 
        InstrBuilder("cmpq $1,", function_name, this)
        .addValue(cond, InstrBuilder::AS_SRC)
        ->build();
      instr_list->Append(cmp_instr);

      auto dest_true = RI->getSuccessor(0);
      this->createPhiJump(bb, dest_true, instr_list, "je", function_name);
      auto dest_false = RI->getSuccessor(1);
      this->createPhiJump(bb, dest_false, instr_list, "jmp", function_name);
    }
    else {
      auto dest_ = RI->getSuccessor(0);
      this->createPhiJump(bb, dest_, instr_list, "jmp", function_name);
    }
    return;
  }

  /// KH-note: 10. llvm::Instruction::Icmp
  if (auto * ICI = llvm::dyn_cast<llvm::ICmpInst>(&inst)) {
    /// KH-note: %res = icmp oper intty LHS, RHS
    /// The %res should be stored to %rxx register, so use cmpq first.
    /// Notice 'cmpq LHS, RHS' is based on (RHS - LHS), reversed from llvm IR code.

    std::string set_oper = "";
    switch (auto prd =ICI->getPredicate()) {
      case llvm::CmpInst::ICMP_EQ:
        set_oper = "sete"; break;
      case llvm::CmpInst::ICMP_NE:
        set_oper = "setne"; break;
      case llvm::CmpInst::ICMP_SLT:
        set_oper = "setl"; break;
      case llvm::CmpInst::ICMP_SLE:
        set_oper = "setle"; break;
      case llvm::CmpInst::ICMP_SGT:
        set_oper = "setg"; break;
      case llvm::CmpInst::ICMP_SGE:
        set_oper = "setge"; break;
      default:
        TigerLog("Unsupported ICMP operation " + std::to_string(prd) + "\n");
    }

    auto LHS = ICI->getOperand(0);
    auto RHS = ICI->getOperand(1);
    auto cmp_instr = 
      InstrBuilder("cmpq", function_name, this)
      .addValue(RHS, InstrBuilder::AS_SRC)
      ->addValue(LHS, InstrBuilder::AS_SRC)
      ->build();
    instr_list->Append(cmp_instr);

    auto set_instr = 
      InstrBuilder(set_oper, function_name, this)
      .addValue(ICI, InstrBuilder::AS_DST)
      ->build();
    instr_list->Append(set_instr);
    return;
  }

  /// KH-note: 11. llvm::Instruction::Phi
  if (auto * PHI = llvm::dyn_cast<llvm::PHINode>(&inst)) {
    /// KH-note: the main part is done in br and pre InstrSels.
    auto instr = 
      InstrBuilder("movq", function_name, this)
      .addTemp(this->phi_temp_, InstrBuilder::AS_SRC)
      ->addValue(PHI, InstrBuilder::AS_DST)
      ->build();
    instr_list->Append(instr);
    return;
  }

  throw std::runtime_error(std::string("Unknown instruction: ") +
                           inst.getOpcodeName());
}

} // namespace cg
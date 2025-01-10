#include "tiger/frame/x64frame.h"
#include "tiger/codegen/assem.h"
#include "tiger/env/env.h"
#include "tiger/frame/temp.h"

#include <iostream>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

extern frame::RegManager *reg_manager;
extern llvm::IRBuilder<> *ir_builder;
extern llvm::Module *ir_module;

namespace frame {

X64RegManager::X64RegManager() : RegManager() {
  for (int i = 0; i < REG_COUNT; i++)
    regs_.push_back(temp::TempFactory::NewTemp());

  // Note: no frame pointer in tiger compiler
  std::array<std::string_view, REG_COUNT> reg_name{
      "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%rbp", "%rsp",
      "%r8",  "%r9",  "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
  int reg = RAX;
  for (auto &name : reg_name) {
    temp_map_->Enter(regs_[reg], new std::string(name));
    reg++;
  }
}

temp::TempList *X64RegManager::Registers() {
  const std::array reg_array{
      RAX, RBX, RCX, RDX, RSI, RDI, RBP, R8, R9, R10, R11, R12, R13, R14, R15,
  };
  auto *temp_list = new temp::TempList();
  for (auto &reg : reg_array)
    temp_list->Append(regs_[reg]);
  return temp_list;
}

temp::TempList *X64RegManager::ArgRegs() {
  const std::array reg_array{RDI, RSI, RDX, RCX, R8, R9};
  auto *temp_list = new temp::TempList();
  ;
  for (auto &reg : reg_array)
    temp_list->Append(regs_[reg]);
  return temp_list;
}

temp::TempList *X64RegManager::CallerSaves() {
  std::array reg_array{RAX, RDI, RSI, RDX, RCX, R8, R9, R10, R11};
  auto *temp_list = new temp::TempList();
  ;
  for (auto &reg : reg_array)
    temp_list->Append(regs_[reg]);
  return temp_list;
}

temp::TempList *X64RegManager::CalleeSaves() {
  std::array reg_array{RBP, RBX, R12, R13, R14, R15};
  auto *temp_list = new temp::TempList();
  ;
  for (auto &reg : reg_array)
    temp_list->Append(regs_[reg]);
  return temp_list;
}

temp::TempList *X64RegManager::ReturnSink() {
  temp::TempList *temp_list = CalleeSaves();
  temp_list->Append(regs_[SP]);
  temp_list->Append(regs_[RV]);
  return temp_list;
}

int X64RegManager::WordSize() { return 8; }

temp::Temp *X64RegManager::FramePointer() { return regs_[FP]; }

/// KH-note:
/// InFrameAccess: value is stored in some frame with given offset.
///   the accessed local is stored in (%fp + offset), which equals to (%sp + offset + framesize).
///   %fp doesn't exist in tiger in practice, so we use the last one.
/// InRegAccess is no longer supported in llvm version tiger lab5-part1.
class InFrameAccess : public Access {
public:
  int offset;
  frame::Frame *parent_frame;

  explicit InFrameAccess(int offset, frame::Frame *parent)
      : offset(offset), parent_frame(parent) {}

  /* TASK: Put your lab5-part1 code here */

  /// KH-note:
  /// Why frame_addr_ptr when we already have parent_frame->sp?
  /// Because for escape values, parent_frame may not be the current frame.
  /// In that case, parent_frame->sp is not a valid value.
  llvm::Value *ToLLVMVal(llvm::Value *frame_addr_ptr) const override {
    /// KH-note: 
    /// Handling static links is not frame::Access's job, so I do plain calc here.
    /// frame_addr_ptr should have type i64 (not i64*), which can be converted to
    ///   pointers of all type.

    auto type_i64 = llvm::Type::getInt64Ty(ir_module->getContext());

    auto load_global_framesize = ir_builder->CreateLoad(
      type_i64,
      this->parent_frame->framesize_global
    );
    auto offset_sp_to_val = ir_builder->CreateAdd(
      load_global_framesize, 
      llvm::ConstantInt::get(type_i64, this->offset)
    );
    auto target_addr_val = ir_builder->CreateAdd(
      frame_addr_ptr,
      offset_sp_to_val
    );

    return target_addr_val;
  }
};

class X64Frame : public Frame {
public:
  X64Frame(temp::Label *name, std::list<frame::Access *> *formals)
      : Frame(8, 0, name, formals) {}

  [[nodiscard]] std::string GetLabel() const override { return name_->Name(); }
  [[nodiscard]] temp::Label *Name() const override { return name_; }
  [[nodiscard]] std::list<frame::Access *> *Formals() const override {
    return formals_;
  }
  /// KH-note:
  /// In ancient times, some values are accessed by InRegAccess.
  ///   But now all values are stored on stack - and the arg 'escape' is useless here...
  frame::Access *AllocLocal(bool escape) override {
    frame::Access *access;

    offset_ -= reg_manager->WordSize();
    access = new InFrameAccess(offset_, this);

    return access;
  }
  void AllocOutgoSpace(int size) override {
    if (size > outgo_size_)
      outgo_size_ = size;
  }
};

frame::Frame *NewFrame(temp::Label *name, std::list<bool> formals) {
  /* TASK: Put your lab5-part1 code here */

  /// KH-note: 
  /// We don't need to add static link as formal here.
  /// - Frames should not handle static link; it shall be regarded as a normal formal.
  /// - Those frames which has no caller don't need to handle static link, eg. tigermain.
  /// That's translation Level's job.
  /// You may see i64 %0 and i64 %1 in .ll files; %0 is caller's %sp, which is not saved
  ///   on stack. %1 is static link that points to the function's parent level.
  /// They are not included in formals here.

  auto formals_access = new std::list<frame::Access *>{};
  auto new_frame = new frame::X64Frame(name, formals_access);

  auto offset_start = 0;
  for (auto itr = formals.begin(); itr != formals.end(); itr ++) {
    /// KH-note:
    /// (Instructions from PPT) For each formal parameter, NewFrame() must calculate:
    /// - How the parameter will be seen from inside the function
    ///   In a register or in a frame location
    /// - What instructions must be produced to implement the "view shift"
    /// Here's my answer:
    /// - The kth parameter (excluding %sp) is stored in caller's outgo area, 
    ///   never in register. In short it's %fp+8k (k=1 for the first formal)
    /// - Calculate the offset from callee's frame top to caller's outgo area
    ///   Callee's frame size is unknown here, that's why we use frame top.
    offset_start += reg_manager->WordSize();
    auto arg_access = new InFrameAccess(offset_start, new_frame);
    new_frame->Formals()->push_back(arg_access);
  }

  return new_frame;
}

/**
 * Moving incoming formal parameters, the saving and restoring of callee-save
 * Registers
 * @param frame curruent frame
 * @param stm statements
 * @return statements with saving, restoring and view shift
 */
assem::InstrList *ProcEntryExit1(std::string_view function_name,
                                 assem::InstrList *body) {
  // TODO: your lab5 code here

  /// KH-note: This function is called when body is already filled with function body assembly code.
  /// I can't be sure which callee_saved regs are used in the function, so I save all of them.

  /// KH-note:
  /// Most of these regs are finally saved to stack, and other temp values may be allocated on stack,
  ///   not in registers. That won't conflict with our stackframe design.
  /// Static link saves the last %sp, so there can be any number of temp values between two frames.
  ///   After all temp values won't escape, they won't be accessed through other frames.
  
  std::vector<std::pair<temp::Temp *, temp::Temp *>> save_rec;

  for (auto callee_saved : reg_manager->CalleeSaves()->GetList()) {
    auto save_temp = temp::TempFactory::NewTemp();
    auto instr = new assem::OperInstr(
      "movq `s0, `d0", 
      new temp::TempList(save_temp), 
      new temp::TempList(callee_saved),
      nullptr);
    body->Insert(body->GetList().begin(), instr);
    save_rec.push_back({callee_saved, save_temp});
  }
  
  /// KH-note: different from PPT, I create the return label here
  ///   because I think these reg restoring instrs must not be skipped.
  auto ret_label = new assem::LabelInstr(
    std::string(function_name) + "_return");
  body->Append(ret_label);

  for (auto & [callee_saved, save_temp] : save_rec) {
    auto instr = new assem::OperInstr(
      "movq `s0, `d0", 
      new temp::TempList(callee_saved), 
      new temp::TempList(save_temp),
      nullptr);
    body->Append(instr);
  }

  return body;
}

/**
 * Appends a “sink” instruction to the function body to tell the register
 * allocator that certain registers are live at procedure exit
 * @param body function body
 * @return instructions with sink instruction
 */
assem::InstrList *ProcEntryExit2(assem::InstrList *body) {
  body->Append(new assem::OperInstr("", new temp::TempList(),
                                    reg_manager->ReturnSink(), nullptr));
  return body;
}

/**
 * The procedure entry/exit sequences
 * @param frame the frame of current func
 * @param body current function body
 * @return whole instruction list with prolog_ end epilog_
 */
assem::Proc *ProcEntryExit3(std::string_view function_name,
                            assem::InstrList *body) {
  std::string prologue = "";
  std::string epilogue = "";

  // TODO: your lab5 code here

  /// KH-notes: 4+2 instructions.
  /// - define function name label + update %sp
  /// (original function body)
  /// - reload %sp + retq
  /// and pseudo-code before and after body 
  ///   (I simply ignore it; the lab has added everything needed in output.cc)

  auto rsp_ = reg_manager->GetRegister(frame::X64RegManager::Reg::RSP);
  auto rax_ = reg_manager->GetRegister(frame::X64RegManager::Reg::RAX);
  auto rdi_ = reg_manager->GetRegister(frame::X64RegManager::Reg::RDI);

  /// KH-note: an label that signs beginning of the function
  auto instr_label = new assem::LabelInstr(std::string(function_name));
  auto instr_movdsp = new assem::OperInstr(
    "movq " + std::string(function_name) + "_framesize_global(%rip), `d0", 
    new temp::TempList(rax_),
    new temp::TempList(), 
    nullptr);
  auto instr_subsp = new assem::OperInstr(
    "subq `s0, %rsp", 
    new temp::TempList({ rsp_ }),
    new temp::TempList({ rax_, rsp_ }), 
    nullptr);
  body->Insert(body->GetList().begin(), instr_subsp);
  body->Insert(body->GetList().begin(), instr_movdsp);
  body->Insert(body->GetList().begin(), instr_label);

  auto instr_movdsp2 = new assem::OperInstr(
    "movq " + std::string(function_name) + "_framesize_global(%rip),`d0", 
    new temp::TempList(rdi_),
    new temp::TempList(), 
    nullptr);
  auto instr_addsp = new assem::OperInstr(
    "addq `s0, %rsp", 
    new temp::TempList({ rsp_ }),
    new temp::TempList({ rdi_, rsp_ }), 
    nullptr);
  auto instr_ret = new assem::OperInstr(
    "retq", nullptr, nullptr, nullptr);
  body->Append(instr_movdsp2);
  body->Append(instr_addsp);
  body->Append(instr_ret);

  return new assem::Proc(prologue, body, epilogue);
}

void Frags::PushBack(Frag *frag) { frags_.emplace_back(frag); }

} // namespace frame
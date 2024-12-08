#include "tiger/frame/x64frame.h"
#include "tiger/env/env.h"

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


} // namespace frame
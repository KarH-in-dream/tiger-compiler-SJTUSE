#ifndef TIGER_FRAME_FRAME_H_
#define TIGER_FRAME_FRAME_H_

#include <list>
#include <memory>
#include <string>

#include "tiger/frame/temp.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace frame {

class RegManager {
public:
  RegManager() : temp_map_(temp::Map::Empty()) {}

  temp::Temp *GetRegister(int regno) { return regs_[regno]; }

  /**
   * Get general-purpose registers except RSI
   * NOTE: returned temp list should be in the order of calling convention
   * @return general-purpose registers
   */
  [[nodiscard]] virtual temp::TempList *Registers() = 0;

  /**
   * Get registers which can be used to hold arguments
   * NOTE: returned temp list must be in the order of calling convention
   * @return argument registers
   */
  [[nodiscard]] virtual temp::TempList *ArgRegs() = 0;

  /**
   * Get caller-saved registers
   * NOTE: returned registers must be in the order of calling convention
   * @return caller-saved registers
   */
  [[nodiscard]] virtual temp::TempList *CallerSaves() = 0;

  /**
   * Get callee-saved registers
   * NOTE: returned registers must be in the order of calling convention
   * @return callee-saved registers
   */
  [[nodiscard]] virtual temp::TempList *CalleeSaves() = 0;

  /**
   * Get return-sink registers
   * @return return-sink registers
   */
  [[nodiscard]] virtual temp::TempList *ReturnSink() = 0;

  /**
   * Get word size
   */
  [[nodiscard]] virtual int WordSize() = 0;

  [[nodiscard]] virtual temp::Temp *FramePointer() = 0;

  temp::Map *temp_map_;

protected:
  std::vector<temp::Temp *> regs_;
};

class Access {
public:
  /* TASK: Put your lab5-part1 code here */

  /// KH-note: defined in PPT
  /// %x_offset = add i64 %func_framesize, -k
  /// %x_addr = add i64 %func_sp, %x_offset 
  /// Its return value should be pure i64 value, not any type ptr
  ///   bcuz Access knows nothing about types.
  virtual llvm::Value *ToLLVMVal(llvm::Value *frame_addr_ptr) const = 0;
  
  virtual ~Access() = default;
};

/// KH-note:
/// Abstract Frame structure in tiger (From address higher to lower):
/// - Frame local values. size = -offset_
///   Note all local vars in lab5-part1 are stored in Frame.
/// - Outgo Area. size = outgo_size_
///   This area is used to store function params (including static link, 
///     though Frame only regards static link as an extra formal).
/// - Return addr.
/// In translation, frame's size is a constant value. An example is in qsort.tig:
///   Inside dosort() when calling init(), it passes (%0 - @dosort_framesize_global)
///     as the first formal (as sp) to init().
///   So although @dosort_framesize_global is unknown at this point, the addr will be correct.
class Frame {
public:
  int outgo_size_;
  int offset_;
  temp::Label *name_;
  std::list<frame::Access *> *formals_;
  llvm::GlobalVariable *framesize_global;
  /// KH-note:
  /// %sp points to return addr of current frame.
  /// %fp(though actually not exists) points to caller of current frame's sp
  ///   and is passed to Tiger Language's function as the first formal.
  /// Thus %fp+8 points to caller's static link (also passed to func as second formal).
  /// You should caution that this is only a valid value in current frame.
  llvm::Value *sp;

  Frame(int outgo_size, int offset, temp::Label *name,
        std::list<frame::Access *> *formals)
      : outgo_size_(outgo_size), offset_(offset), name_(name),
        formals_(formals) {}

  [[nodiscard]] virtual std::string GetLabel() const = 0;
  [[nodiscard]] virtual temp::Label *Name() const = 0;
  [[nodiscard]] virtual std::list<frame::Access *> *Formals() const = 0;
  virtual frame::Access *AllocLocal(bool escape) = 0;
  virtual void AllocOutgoSpace(int size) = 0;
  int calculateActualFramesize() {
    return (-offset_ + outgo_size_) + 8;
  }
};

/**
 * Fragments
 */

class Frag {
public:
  virtual ~Frag() = default;

  enum OutputPhase {
    Proc,
    String,
    FrameSize,
  };

  /**
   *Generate assembly for main program
   * @param out FILE object for output assembly file
   */
  virtual void OutputAssem(FILE *out, OutputPhase phase,
                           bool need_ra) const = 0;
};

class StringFrag : public Frag {
public:
  llvm::Value *str_val_;
  std::string str_;

  StringFrag(llvm::Value *str_val, std::string str)
      : str_val_(str_val), str_(std::move(str)) {}

  void OutputAssem(FILE *out, OutputPhase phase, bool need_ra) const override;
};

class FrameSizeFrag : public Frag {
public:
  llvm::Value *framesize_val_;
  int framesize_;

  FrameSizeFrag(llvm::Value *framesize_val, int framesize)
      : framesize_val_(framesize_val), framesize_(framesize) {}

  void OutputAssem(FILE *out, OutputPhase phase, bool need_ra) const override;
};

class ProcFrag : public Frag {
public:
  llvm::Function *body_;

  ProcFrag(llvm::Function *body) : body_(body) {}

  void OutputAssem(FILE *out, OutputPhase phase, bool need_ra) const override;
};

class Frags {
public:
  Frags() = default;
  void PushBack(Frag *frag);
  const std::list<Frag *> &GetList() { return frags_; }

private:
  std::list<Frag *> frags_;
};

/// KH-note:
/// formals is used for args, each function arg corresponds with one bool.
///   It signs whether an arg escapes.
/// Escape args (and all other args in lab5-part1) should be stored at a fixed location on stack.
frame::Frame *NewFrame(temp::Label *name, std::list<bool> formals);

} // namespace frame

#endif
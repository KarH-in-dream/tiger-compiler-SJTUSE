#ifndef TIGER_CODEGEN_CODEGEN_H_
#define TIGER_CODEGEN_CODEGEN_H_

#include "tiger/canon/canon.h"
#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/x64frame.h"
#include <llvm-14/llvm/IR/BasicBlock.h>
#include <llvm-14/llvm/IR/Instructions.h>
#include <llvm-14/llvm/IR/Value.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// Forward Declarations
namespace frame {
class RegManager;
class Frame;
} // namespace frame

namespace assem {
class Instr;
class InstrList;
} // namespace assem

namespace canon {
class Traces;
} // namespace canon

namespace cg {

class InstrBuilder;

class AssemInstr {
public:
  AssemInstr() = delete;
  explicit AssemInstr(assem::InstrList *instr_list) : instr_list_(instr_list) {}

  void Print(FILE *out, temp::Map *map) const;

  [[nodiscard]] assem::InstrList *GetInstrList() const { return instr_list_; }

private:
  assem::InstrList *instr_list_;
};

class CodeGen {
public:
  /// KH-note: canonical and trace
  /// In LLVM IR, two blocks in different functions may have a same name.
  /// In assembly, all functions are actually labels, so labels in different
  ///   functions must have different names.
  /// That's canonical.
  CodeGen(std::unique_ptr<canon::Traces> traces)
      : phi_temp_(temp::TempFactory::NewTemp()), traces_(std::move(traces)) {}

  void Codegen();

  // check if the value is %sp in llvm
  bool IsRsp(llvm::Value *val, std::string_view function_name) const {
    // TODO: your lab5 code here
    /// KH-note: nothing special but I name %sp as function_sp in lab5-1
    /// And param %0 of tiger func is also sp
    if (val == this->traces_->GetBody()->getArg(0)) {
      return true;
    }
    if (val->getName() == std::string(function_name) + "_sp") {
      return true;
    }
    return false;
  }

  // bb is to add move instruction to record which block it jumps from
  // function_name can be used to construct return or exit label
  void InstrSel(assem::InstrList *instr_list, llvm::Instruction &inst,
                std::string_view function_name, llvm::BasicBlock *bb);
  std::unique_ptr<AssemInstr> TransferAssemInstr() {
    return std::move(assem_instr_);
  }

private:
  // record mapping from llvm value to temp
  std::unordered_map<llvm::Value *, temp::Temp *> *temp_map_;
  // for phi node, record mapping from llvm basic block to index of the block,
  // to check which block it jumps from
  std::unordered_map<llvm::BasicBlock *, int> *bb_map_;
  // for phi node, use a temp to record which block it jumps from
  temp::Temp *phi_temp_;
  std::unique_ptr<canon::Traces> traces_;
  std::unique_ptr<AssemInstr> assem_instr_;

  /// KH-note: used before jmp, map[jmp_source][jmp_target] = move src->dst
  std::unordered_map<llvm::BasicBlock *, std::unordered_map<llvm::BasicBlock *, llvm::Value *>> phi_map;
  /// KH-note: If there's any value needed by PHI, it will create the move instr along with the jmp.
  void createPhiJump(llvm::BasicBlock * bfrom, llvm::BasicBlock * bto, 
    assem::InstrList *instr_list, std::string jmp_oper, std::string_view func_name);

  /// KH-note: convert a llvm temp value to temp::Temp
  temp::Temp * llvm2temp(llvm::Value * input, std::string_view func_name);
  friend class InstrBuilder;
};

/// KH-note: my tool class to construct an instr with any kind of llvm::Value *
class InstrBuilder {
  public:
    explicit InstrBuilder(std::string instr, std::string_view func_name, CodeGen * codegen) {
      this->func = func_name;
      this->cg = codegen;
      this->need_separator = false;
      this->i = new assem::OperInstr (
        instr + ' ', new temp::TempList(), new temp::TempList(), nullptr);
    }

    InstrBuilder * addValue(llvm::Value * val, int CFG);
    InstrBuilder * addTemp(temp::Temp * temp, int CFG);

    static const int AS_SRC = 0b0001; /// KH-note: the added value is used as src
    static const int AS_DST = 0b0010; /// KH-note: the added value is used as dest
    static const int USE_MEM = 0b0100; /// KH-note: the added value will access memory

    assem::OperInstr * build() { return this->i; }
  private:
    assem::OperInstr * i;
    CodeGen * cg;
    std::string_view func;
    bool need_separator;

    void addParam() { 
      if (!this->need_separator) {
        this->need_separator = true;
        return;
      }
      this->i->assem_ += ", ";
    };
};

} // namespace cg
#endif
#ifndef TIGER_REGALLOC_REGALLOC_H_
#define TIGER_REGALLOC_REGALLOC_H_

#include "tiger/codegen/assem.h"
#include "tiger/codegen/codegen.h"
#include "tiger/frame/frame.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/regalloc/color.h"
#include "tiger/util/graph.h"
#include <string>

namespace ra {

class Result {
public:
  temp::Map *coloring_;
  assem::InstrList *il_;

  Result() : coloring_(nullptr), il_(nullptr) {}
  Result(temp::Map *coloring, assem::InstrList *il)
      : coloring_(coloring), il_(il) {}
  Result(const Result &result) = delete;
  Result(Result &&result) = delete;
  Result &operator=(const Result &result) = delete;
  Result &operator=(Result &&result) = delete;
  ~Result() {}
};

/// KH-note: a builder class to do the overall reg alloc.
class RegAllocator {
  /* TODO: Put your lab6 code here */
  public:
    RegAllocator(std::string func_name, std::unique_ptr<cg::AssemInstr> assem_instr)
      : function_name(func_name), instr_list(assem_instr->GetInstrList()) {}
    void RegAlloc();
    std::unique_ptr<Result> TransferResult();

  private:
    std::string function_name;
    assem::InstrList * instr_list;
    temp::Map * coloring_;

    void rewrite(live::INodeListPtr spills);
    void ir_clean();
    void dupmove_clean();
    void setxx_fix();

    tab::Table<temp::Temp, live::INode> * temp_node_map_;
};

} // namespace ra

#endif
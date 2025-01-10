#include "tiger/escape/escape.h"
#include "tiger/absyn/absyn.h"

namespace esc {
void EscFinder::FindEscape() { absyn_tree_->Traverse(env_.get()); }
} // namespace esc

namespace absyn {

void AbsynTree::Traverse(esc::EscEnvPtr env) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: begin from root.
  this->root_->Traverse(env, 0);
}

void SimpleVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  auto def = env->Look(this->sym_);
  if (def == nullptr) {
    return;
  }
  /// KH-note: 
  /// A value escapes when it's used in other scope.
  /// Passed by reference / Addr taken / Called in nested scope
  /// Only the last situation may occur in tiger.
  if (def->depth_ < depth) {
    *(def->escape_) = true;
  }
}

void FieldVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note:
  /// Review - FieldVar == var_.sym_
  /// We only concern about the var_ itself; there's no way that
  ///   var_ doesn't escape but field escapes.
  this->var_->Traverse(env, depth);
}

void SubscriptVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  this->var_->Traverse(env, depth);
  this->subscript_->Traverse(env, depth);
}

void VarExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: obviously an exp can't escape
  this->var_->Traverse(env, depth);
}

void NilExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: do nothing...
}

void IntExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: do nothing...
}

void StringExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: do nothing...
}

void CallExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note:
  /// Args are exps, not vars. So check each exp.
  /// Functions in tiger can't be dynamically constructed,
  ///   So functions are all static and thus can't escape.
  for (auto arg : this->args_->GetList()) {
    arg->Traverse(env, depth);
  } 
}

void OpExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  this->left_->Traverse(env, depth);
  this->right_->Traverse(env, depth);
}

void RecordExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  for (auto ef : this->fields_->GetList()) {
    ef->exp_->Traverse(env, depth);
  }
}

void SeqExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  for (auto exp : this->seq_->GetList()) {
    exp->Traverse(env, depth);
  }
}

void AssignExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  this->exp_->Traverse(env, depth);
  this->var_->Traverse(env, depth);
}

void IfExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: 
  /// It's still in the same scope! 
  /// In tiger, vars can't be suddenly defined in normal exp/stm.
  this->test_->Traverse(env, depth);
  this->then_->Traverse(env, depth);
  if (this->elsee_) {
    this->elsee_->Traverse(env, depth);
  }
}

void WhileExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  this->test_->Traverse(env, depth);
  this->body_->Traverse(env, depth);
}

void ForExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note:
  /// for body is a new scope that contains loop variable.
  /// lo and hi are exps that belongs to outer scope.
  this->lo_->Traverse(env, depth);
  this->hi_->Traverse(env, depth);
  env->BeginScope();
  {
    /// KH-note: 
    /// assume loop variable doesn't escape first.
    /// &escape should never access local variiable; they will be trashed when function returns.
    this->escape_ = false;
    env->Enter(
      this->var_, 
      new esc::EscapeEntry(
        /// KH-note: 
        /// Still the same depth!!!
        /// Escape is not like scope; it's for program.
        /// In machine code layout, function may be far from its caller,
        ///   so escape has its value.
        /// But loop is located in the block like any other code.
        depth, 
        &this->escape_
      )
    );
    this->body_->Traverse(env, depth);
  }
  env->EndScope();
}

void BreakExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: do nothing...
}

void LetExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  env->BeginScope();
  {
    for (auto dec : this->decs_->GetList()) {
      dec->Traverse(env, depth);
    }
    this->body_->Traverse(env, depth);
  }
  env->EndScope();
}

void ArrayExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  this->init_->Traverse(env, depth);
  this->size_->Traverse(env, depth);
}

void VoidExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: do nothing...
}

void FunctionDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  for (auto fdec : this->functions_->GetList()) {
    env->BeginScope();
    {
      /// KH-note: the only case that depth could +1
      auto func_depth = depth + 1;
      for (auto field : fdec->params_->GetList()) {
        field->escape_ = false;
        env->Enter(
          field->name_, 
          new esc::EscapeEntry(
            func_depth, 
            &field->escape_
          )
        );
      }
      fdec->body_->Traverse(env, func_depth);
    }
    env->EndScope();
  }
}

void VarDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  this->init_->Traverse(env, depth);
  /// KH-note: init_ should not see the newly inserted var_ (though we have semant check...)
  this->escape_ = false;
  env->Enter(
    this->var_, 
    new esc::EscapeEntry(
      depth, 
      &this->escape_
    )
  );
}

void TypeDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: do nothing...
}

} // namespace absyn

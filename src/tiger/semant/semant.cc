#include "tiger/semant/semant.h"
#include "tiger/absyn/absyn.h"
#include "tiger/env/env.h"
#include "tiger/semant/types.h"
#include "tiger/symbol/symbol.h"
#include <algorithm>
#include <cassert>
#include <list>
#include <unordered_map>
#include <utility>

namespace absyn {

/// KH-note: a constant for error type.
static type::VoidTy * const error_ty_ = nullptr;

void AbsynTree::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                           err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// AbsynTree is short of "abstract syntax tree". Start from its root exp.
  /// @Param venv      : "value environment". Store variables and functions in current scope.
  /// @Param tenv      : "type environment". Store user-defined types in current scope.
  /// @Param labelcount: used to match BREAK and LOOP.
  this->root_->SemAnalyze(venv, tenv, 0, errormsg);
}

type::Ty *SimpleVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// SimpleVar: a symbol representing a var.
  /// Possible failures:
  /// - symbol undefined (testcase 19, 20)
  /// - symbol is a function (not included)
  assert(this->sym_);
  auto entry = venv->Look(this->sym_);
  if (entry == nullptr || typeid(*entry) != typeid(env::VarEntry)) {
    errormsg->Error(this->pos_, "undefined variable %s", this->sym_->Name().data());
    return error_ty_;
  }
  else {
    /// KH-note: Call ActualTy when actually needs it.
    return static_cast<env::VarEntry *>(entry)->ty_;
  }
}

type::Ty *FieldVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// FieldVar: a symbol representing field of a record.
  /// Possible failures:
  /// - field undefined (testcase 22)
  /// - parent is not a record (testcase 25)
  assert(this->var_);
  assert(this->sym_);
  auto var_t = this->var_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (var_t == error_ty_) {
    return error_ty_;
  }
  auto var_act = var_t->ActualTy();
  /// KH-note: IsSameType regards NIL as RECORD, but NIL has no field.
  if (typeid(*var_act) != typeid(type::RecordTy)) {
      errormsg->Error(var_->pos_, "not a record type");
      return error_ty_;
  }
  auto v_fields = static_cast<type::RecordTy *>(var_act)->fields_;
  assert(v_fields);
  auto field = std::find_if(
    v_fields->GetList().begin(), v_fields->GetList().end(),
    /// KH-note: sym::Symbol of the same name only has one instance.
    [this](type::Field * f) -> bool { return f->name_ == this->sym_; }
  );
  if (field == v_fields->GetList().end()) {
    errormsg->Error(this->pos_, "field %s doesn't exist", this->sym_->Name().data());
    return error_ty_;
  }
  return (*field)->ty_;
}

type::Ty *SubscriptVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   int labelcount,
                                   err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// SubscriptVar: access member in an array.
  /// Possible failures:
  /// - subscript is not an Int (not included)
  /// - parent is not an array (testcase 24)
  assert(this->var_);
  assert(this->subscript_);
  auto var_t = this->var_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (var_t == error_ty_) {
    return error_ty_;
  }
  auto var_act = var_t->ActualTy();
  if (typeid(*var_act) != typeid(type::ArrayTy)) {
    errormsg->Error(var_->pos_, "array type required");
    return error_ty_;
  }
  auto sub_t = this->subscript_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (sub_t != error_ty_ && !sub_t->IsSameType(type::IntTy::Instance())) {
    /// KH-note: ignore it for error recovery; the result only depends on this->var_.
    errormsg->Error(subscript_->pos_, "array index must be int");
  }
  return static_cast<type::ArrayTy *>(var_act)->ty_;
}

type::Ty *VarExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note: VarExp: An exp of single variable.
  assert(this->var_);
  return this->var_->SemAnalyze(venv, tenv, labelcount, errormsg);
}

type::Ty *NilExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  return type::NilTy::Instance();
}

type::Ty *IntExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  return type::IntTy::Instance();
}

type::Ty *StringExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  return type::StringTy::Instance();
}

type::Ty *CallExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note: 
  /// CallExp: Call function with args.
  /// Possible failures:
  /// - function undefined (testcase 18)
  /// - symbol is not a function (not included)
  /// - given args doesn't match function declaration (testcase 34, 35)
  /// - given too many or too less args (testcase 36)
  assert(this->func_);
  assert(this->args_);
  auto entry = venv->Look(this->func_);
  if (entry == nullptr || typeid(*entry) != typeid(env::FunEntry)) {
    errormsg->Error(this->pos_, "undefined function %s", this->func_->Name().data());
    return error_ty_;
  }

  auto func_entry = static_cast<env::FunEntry *>(entry);
  auto formal_tys = func_entry->formals_->GetList();
  auto arg_tys = this->args_->GetList();

  auto f_itr = formal_tys.begin();
  auto a_itr = arg_tys.begin();
  for (; f_itr != formal_tys.end() && a_itr != arg_tys.end(); f_itr ++, a_itr ++) {
    auto arg_ty = (*a_itr)->SemAnalyze(venv, tenv, labelcount, errormsg);
    /// KH-note: error recovery
    if (arg_ty == error_ty_) {
      continue;
    }
    /// KH-note: 
    /// testcase 35 just returns and ignores further check.
    /// That's to blame...
    if (!arg_ty->IsSameType(*f_itr)) {
      errormsg->Error((*a_itr)->pos_, "para type mismatch");
      return func_entry->result_;
    }
  }

  bool f_end = f_itr == formal_tys.end();
  bool a_end = a_itr == arg_tys.end();
  if (!f_end || !a_end) {
    errormsg->Error((*a_itr)->pos_, "too %s params in function %s", 
      f_end ? "many" : a_end ? "less" : "???", this->func_->Name().data());
  }
  return func_entry->result_;
}

type::Ty *OpExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note: 
  /// In tiger, operators has 3 types:
  /// - Arithmetic: INT OP INT
  /// - Condition: INT OP INT, bcuz 1 for TRUE, 0 for FALSE
  /// - Comparation: VAL OP VAL(same type)
  /// No auto type converting.
  /// Possible failures:
  /// - not required int (testcase 21, 26, 43)
  /// - comparing values of different types (testcase 13, 14)
  assert(this->left_);
  assert(this->right_);
  auto l_ty = this->left_->SemAnalyze(venv, tenv, labelcount, errormsg);
  auto r_ty = this->right_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (l_ty == error_ty_ || r_ty == error_ty_) {
    return type::IntTy::Instance();
  }

  switch (this->oper_) {
    case absyn::Oper::PLUS_OP:
    case absyn::Oper::MINUS_OP:
    case absyn::Oper::TIMES_OP:
    case absyn::Oper::DIVIDE_OP:
    case absyn::Oper::AND_OP:
    case absyn::Oper::OR_OP:
    {
      auto error_arg =
        (!type::IntTy::Instance()->IsSameType(l_ty)) ? this->left_ :
        (!type::IntTy::Instance()->IsSameType(r_ty)) ? this->right_ :
        nullptr;
      if (error_arg) {
        errormsg->Error(error_arg->pos_, "integer required");
      }
      break;
    }
    case absyn::Oper::EQ_OP:
    case absyn::Oper::NEQ_OP:
    case absyn::Oper::LT_OP:
    case absyn::Oper::GT_OP:
    case absyn::Oper::LE_OP:
    case absyn::Oper::GE_OP:
    {
      if (!l_ty->IsSameType(r_ty)) {
        errormsg->Error(this->pos_, "same type required");
      }
      break;
    }
    default:
    {
      errormsg->Error(this->right_->pos_ - 1, "UNKNOWN OPERATOR");
      return error_ty_;
    }
  }
  return type::IntTy::Instance();
}

type::Ty *RecordExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// RecordExp: used to init a record object.
  /// Possible failures:
  /// - record type undefined (testcase 33)
  /// - symbol is not a record (not included)
  /// - field type mismatch (not included)
  assert(this->typ_);
  assert(this->fields_);
  auto rec_t = tenv->Look(this->typ_);
  if (rec_t == nullptr) {
    errormsg->Error(this->pos_, "undefined type %s", this->typ_->Name().data());
    return error_ty_;
  }
  auto rec_act = rec_t->ActualTy();
  if (typeid(*rec_act) != typeid(type::RecordTy)) {
    errormsg->Error(this->pos_, "%s is not a record", this->typ_->Name().data());
    return error_ty_;
  }

  auto rec_fl = static_cast<type::RecordTy *>(rec_act)->fields_;
  assert(rec_fl);
  auto rec_fields = rec_fl->GetList();
  auto arg_fields = this->fields_->GetList();
  /// KH-note:
  /// Args can be in any order, as it's given by FIELD:VALUE.
  /// Using map is much more convenient.
  std::unordered_map<sym::Symbol *, type::Ty *> rec_map;
  std::unordered_map<sym::Symbol *, absyn::Exp *> arg_map;
  for (auto itr : rec_fields) {
    assert(itr->ty_);
    rec_map.emplace(itr->name_, itr->ty_);
  }
  for (auto itr : arg_fields) {
    assert(itr->exp_);
    auto check_dup_arg = arg_map.emplace(itr->name_, itr->exp_);
    if (!check_dup_arg.second) {
      errormsg->Error(itr->exp_->pos_, "duplicated field");
      continue;
    }
    
    auto expected_ty = rec_map.find(itr->name_);
    if (expected_ty == rec_map.end()) {
      errormsg->Error(itr->exp_->pos_, "undefined field");
      continue;
    }
    auto given_ty = itr->exp_->SemAnalyze(venv, tenv, labelcount, errormsg);
    if (given_ty == error_ty_) {
      continue;
    }
    if (!given_ty->IsSameType(expected_ty->second)) {
      errormsg->Error(itr->exp_->pos_, "field type mismatch");
      continue;
    }
  }
  for (auto itr : rec_fields) {
    auto check_field = arg_map.find(itr->name_);
    if (check_field == arg_map.end()) {
      errormsg->Error(this->pos_, "uninitialized field %s", itr->name_->Name().data());
      continue;
    }
  }
  return rec_act;
}

type::Ty *SeqExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note: SeqExp is sequence of exps, the last is to return.
  assert(this->seq_);
  type::Ty * result = error_ty_;
  for (auto itr : this->seq_->GetList()) {
    result = itr->SemAnalyze(venv, tenv, labelcount, errormsg);
  }
  return result;
}

type::Ty *AssignExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// Assign returns void according to tiger manual.
  /// Possible failures:
  /// - assign a r-only var (loop variable) (testcase 11)
  /// - type mismatch (testcase 23)
  assert(this->var_);
  assert(this->exp_);
  auto lv_ty = this->var_->SemAnalyze(venv, tenv, labelcount, errormsg);
  auto rv_ty = this->exp_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (lv_ty != error_ty_ && rv_ty != error_ty_) {
    if (!lv_ty->IsSameType(rv_ty)) {
      errormsg->Error(this->pos_, "unmatched assign exp");
    }
    /// KH-note: NIL is considered same as RECORD but can't be lvalue.
    if (lv_ty == type::NilTy::Instance()) {
      errormsg->Error(this->pos_, "can't assign a nil");
    }
    if (typeid(* this->var_) == typeid(absyn::SimpleVar)) {
      auto entry = venv->Look(static_cast<absyn::SimpleVar *>(this->var_)->sym_);
      if (entry->readonly_) {
        assert(typeid(*entry) == typeid(env::VarEntry));
        errormsg->Error(this->pos_, "loop variable can't be assigned");
      }
    }
  }
  return type::VoidTy::Instance();
}

type::Ty *IfExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// The else stm might be null.
  /// Possible failures:
  /// - condition is not int (not included)
  /// - then and else mismatch (testcase 9)
  /// - then returns non-void without else (testcase 15)
  assert(this->test_);
  assert(this->then_);
  auto test_ty = this->test_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (test_ty != error_ty_ && !test_ty->IsSameType(type::IntTy::Instance())) {
    errormsg->Error(this->test_->pos_, "if-then condition must have int type");
  }
  auto then_ty = this->then_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (this->elsee_) {
    auto else_ty = this->elsee_->SemAnalyze(venv, tenv, labelcount, errormsg);
    /// KH-note: error recovery
    if (then_ty == error_ty_) {
      return else_ty;
    }
    if (else_ty != error_ty_ && !then_ty->IsSameType(else_ty)) {
      errormsg->Error(this->elsee_->pos_, "then exp and else exp type mismatch");
    }
    return then_ty;
  }
  else {
    if (then_ty != error_ty_ && then_ty != type::VoidTy::Instance()) {
      errormsg->Error(then_->pos_, "if-then exp's body must produce no value");
    }
    return type::VoidTy::Instance();
  }
}

type::Ty *WhileExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// Be careful with BREAK and labelcount.
  /// Possible failures:
  /// - condition is not int (not included)
  /// - body returns non-void (testcase 10)
  assert(this->test_);
  assert(this->body_);
  auto test_ty = this->test_->SemAnalyze(venv, tenv, labelcount, errormsg);
  auto body_ty = this->body_->SemAnalyze(venv, tenv, labelcount + 1, errormsg);
  if (test_ty != error_ty_ && !test_ty->IsSameType(type::IntTy::Instance())) {
    errormsg->Error(test_->pos_, "while test must produce int value");
  }
  if (body_ty != error_ty_ && body_ty != type::VoidTy::Instance()) {
    errormsg->Error(body_->pos_, "while body must produce no value");
  }
  return type::VoidTy::Instance();
}

type::Ty *ForExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// Loop var should be added to venv of the loop.
  /// Possible failures:
  /// - boundaries are not int (testcase 11)
  /// - body returns non-void (not included)
  assert(this->var_);
  assert(this->lo_ && this->hi_);
  assert(this->body_);
  auto lo_ty = this->lo_->SemAnalyze(venv, tenv, labelcount, errormsg);
  auto hi_ty = this->hi_->SemAnalyze(venv, tenv, labelcount, errormsg);
  absyn::Exp * err_bound = 
    (lo_ty != error_ty_ && !lo_ty->IsSameType(type::IntTy::Instance())) ? this->lo_ :
    (hi_ty != error_ty_ && !hi_ty->IsSameType(type::IntTy::Instance())) ? this->hi_ :
    nullptr;
  if (err_bound) {
    errormsg->Error(err_bound->pos_, "for exp's range type is not integer");
  }

  venv->BeginScope();
  venv->Enter(this->var_, new env::VarEntry(type::IntTy::Instance(), true));
  auto body_ty = this->body_->SemAnalyze(venv, tenv, labelcount + 1, errormsg);
  if (body_ty != error_ty_ && body_ty != type::VoidTy::Instance()) {
    errormsg->Error(this->body_->pos_, "for-loop body must produce no value");
  }
  venv->EndScope();
  return type::VoidTy::Instance();
}

type::Ty *BreakExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// Possible failures:
  /// - not inside any loop (testcase 50)
  if (labelcount <= 0) {
    errormsg->Error(this->pos_, "break is not inside any loop");
  }
  return type::VoidTy::Instance();
}

type::Ty *LetExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note: deal declarations and check body.
  assert(this->decs_);
  assert(this->body_);
  venv->BeginScope();
  tenv->BeginScope();
  for (auto itr : this->decs_->GetList()) {
    itr->SemAnalyze(venv, tenv, labelcount, errormsg);
  }
  auto res = this->body_->SemAnalyze(venv, tenv, labelcount, errormsg);
  tenv->EndScope();
  venv->EndScope();
  return res;
}

type::Ty *ArrayExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// A new array object.
  /// Possible failures:
  /// - use undefined or non-array type (not included)
  /// - size is not int (not included)
  /// - initial value is not declared type (testcase 32)
  assert(this->typ_);
  assert(this->size_);
  assert(this->init_);
  auto array_t = tenv->Look(this->typ_);
  if (array_t == nullptr) {
    errormsg->Error(this->pos_, "undefined array type");
    return error_ty_;
  }
  auto array_act = array_t->ActualTy();
  if (typeid(* array_act) != typeid(type::ArrayTy)) {
    errormsg->Error(this->pos_, "type %s is not array", this->typ_->Name().data());
    return error_ty_;
  }
  auto elem_t = static_cast<type::ArrayTy *>(array_act)->ty_;
  auto size_t = this->size_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (size_t != error_ty_ && !size_t->IsSameType(type::IntTy::Instance())) {
    errormsg->Error(this->size_->pos_, "array size must be an int");
  }
  auto init_t = this->init_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (init_t != error_ty_ && !init_t->IsSameType(elem_t)) {
    errormsg->Error(this->init_->pos_, "type mismatch");
  }
  /// KH-note: see test 29. Remember, no auto type converting
  return array_t;
}

type::Ty *VoidExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// Why is this a problem???
  return type::VoidTy::Instance();
}

void FunctionDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// A list of some function declarations.
  /// Functions in the same list can call each other, even if the caller is declared before the callee.
  /// If a function was declared before this list, it can be overrided.
  assert(this->functions_);

  /// KH-note:
  /// Part1. add all function names to venv
  /// result_ could be null for void func.
  /// the scope has been declared in LET.
  /// Possible failures:
  /// - duplicated function name in the same list (testcase 39)
  /// - function name already exists for a var or other usage (not included)
  /// - undeclared non-void return type (not included)
  std::list<FunDec *> valid_decs;
  for (auto fd : this->functions_->GetList()) {
    assert(fd->body_);
    assert(fd->params_);
    auto entry = venv->Look(fd->name_);
    if (entry && typeid(*entry) != typeid(env::FunEntry)) {
      errormsg->Error(fd->pos_, "A non-function symbol with the same name has been defined");
      continue;
    }
    auto dup_check = std::find_if(valid_decs.begin(), valid_decs.end(), 
      [&fd](absyn::FunDec * itr) -> bool {return itr->name_ == fd->name_;});
    if (dup_check != valid_decs.end()) {
      errormsg->Error(fd->pos_, "two functions have the same name");
      continue;
    }
    type::Ty * res_t = type::VoidTy::Instance();
    if (fd->result_) {
      res_t = tenv->Look(fd->result_);
      if (res_t == nullptr) {
        errormsg->Error(fd->pos_, "function return type is undefined");
        continue;; 
      }
    }
    valid_decs.emplace_back(fd);
    venv->Enter(fd->name_, new env::FunEntry(fd->params_->MakeFormalTyList(tenv, errormsg), res_t));
  }

  /// KH-note:
  /// Part2. check function body
  /// result_ is now VoidTy for void func.
  /// Possible failures:
  /// - undefined formal type (not included)
  /// - void func has return value (testcase 21, 40)
  /// - return type doesn't match body type (not included)
  for (auto fd : valid_decs) {
    auto look_res = venv->Look(fd->name_);
    /// KH-note: error recovery
    if (look_res == nullptr) {
      continue;
    }
    auto entry = static_cast<env::FunEntry *>(look_res);
    /// KH-note: scope for function body
    venv->BeginScope();
    bool check_body = true;
    auto formals = fd->params_->GetList();
    for (auto formal : formals) {
      auto formal_t = tenv->Look(formal->typ_);
      if (formal_t == nullptr) {
        errormsg->Error(formal->pos_, "function formal type undefined");
        /// KH-note: can't check function body with formal undefined.
        check_body = false;
        break;
      }
      venv->Enter(formal->name_, new env::VarEntry(formal_t));
    }
    if (check_body) {
      auto body_t = fd->body_->SemAnalyze(venv, tenv, labelcount, errormsg);
      if (body_t != error_ty_) {
        if (!body_t->IsSameType(entry->result_)) {
          errormsg->Error(fd->pos_, 
            entry->result_ == type::VoidTy::Instance() ?
            "procedure returns value" :
            "body type doesn't match function return type");
        }
      }
    }
    venv->EndScope();
  }
}

void VarDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                        err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// Declare a var with init value. Var type may be auto detected.
  /// Can override previous var dec with same name.
  /// Possible failures:
  /// - name has been declared as function (not included)
  /// - request undeclared var type (not included)
  /// - init value can't match var type (not included)
  /// - init a record to NIL without declaring its type (testcase 45)
  assert(this->init_);
  auto entry_v = venv->Look(this->var_);
  if (entry_v && typeid(*entry_v) != typeid(env::VarEntry)) {
    errormsg->Error(this->pos_, "variable name has been declared as non-variable");
    return;
  }
  auto init_t = this->init_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (this->typ_ == nullptr) {
    if (init_t == type::NilTy::Instance()) {
      errormsg->Error(this->pos_, "init should not be nil without type specified");
    }
    else if (init_t != error_ty_) {
      venv->Enter(this->var_, new env::VarEntry(init_t));
    }
  }
  else {
    auto expected_ty = tenv->Look(this->typ_);
    if (expected_ty == nullptr) {
      errormsg->Error(pos_, "undefined type %s", this->typ_->Name().data());
    }
    else if (init_t != error_ty_) {
      if (!expected_ty->IsSameType(init_t)) {
        errormsg->Error(pos_, "function and body type mismatch");
      }
      venv->Enter(this->var_, new env::VarEntry(expected_ty));
    }
  }
}

void TypeDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                         err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  /// KH-note:
  /// This is similiar to FunctionDec.
  /// Possible failures:
  /// - types with same name in the same list (testcase 38)
  /// - direct type circle (testcase 16)

  /// KH-note: step1. check duplicated types
  assert(this->types_);
  std::list<NameAndTy *> non_duplicated_decs;
  for (auto td : this->types_->GetList()) {
    assert(td->ty_);
    auto dup_check = std::find_if(non_duplicated_decs.begin(), non_duplicated_decs.end(), 
      [&td](absyn::NameAndTy * itr) -> bool {return itr->name_ == td->name_;});
    if (dup_check != non_duplicated_decs.end()) {
      errormsg->Error(this->pos_, "two types have the same name");
      continue;
    }
    non_duplicated_decs.emplace_back(td);
  }

  /// KH-note: 
  /// step2. build dependency map
  /// If type a=b, set key=a, val=b
  /// It's also a way to detect direct circle.
  std::unordered_map<sym::Symbol *, sym::Symbol *> dependency;
  std::unordered_map<sym::Symbol *, bool> dec_status;
  auto check_max = 1;
  for (auto td : non_duplicated_decs) {
    if (typeid(*(td->ty_)) == typeid(absyn::NameTy)) {
      auto td_n = static_cast<absyn::NameTy *>(td->ty_);
      auto dep_check = std::find_if(non_duplicated_decs.begin(), non_duplicated_decs.end(), 
        [&td_n](absyn::NameAndTy * itr) -> bool {return itr->name_ == td_n->name_;});
      if (dep_check != non_duplicated_decs.end()) {
        dependency.emplace(td->name_, (*dep_check)->name_);
        check_max ++;
      }
    }
    dec_status.emplace(td->name_, true);
  }
  
  /// KH-note:
  /// step3. check dependency loop.
  /// Start from any symbol, go for at most len(non_duplicated_decs) moves.
  /// If meet itself, then there's loop, and switch all in this loop to false.
  for (auto td : non_duplicated_decs) {
    auto find_res = dependency.find(td->name_);
    if (find_res != dependency.end()) {
      auto checked = dec_status.find(td->name_);
      assert(checked != dec_status.end());
      if (!checked->second) {
        /// KH-note: This symbol is already invalid.
        continue;
      }
      auto itr = td->name_;
      auto check_i = 0;
      while (check_i < check_max) {
        auto itr_next = dependency.find(itr);
        if (itr_next == dependency.end()) {
          break;
        }
        check_i ++;
        itr = itr_next->second;
        if (itr == td->name_) {
          /// KH-note:
          /// Loop detected, but errormsg isn't enough.
          /// Then mark all decs in the loop invalid.
          errormsg->Error(td->ty_->pos_, "illegal type cycle");
          do {
            dec_status.insert_or_assign(itr, false);
            auto itr_n = dependency.find(itr);
            assert(itr_n != dependency.end());
            itr = itr_n->second;
          } while (itr != td->name_);
          break;
        }
      }
    }
  }

  /// KH-note: 
  /// step4. check and fill left.
  /// But first, for any "current valid" dec type a = <dec>, we add <a, null> to tenv.
  /// These types could be not well-defined.
  /// If use tenv->Look somewhere and get a not well-defined type,
  /// the nullptr will be an signal. 
  std::list<NameAndTy *> valid_decs;
  for (auto td : non_duplicated_decs) {
    auto checked = dec_status.find(td->name_);
    assert(checked != dec_status.end());
    if (checked->second) {
      valid_decs.emplace_back(td);
    }
  }
  for (auto td : valid_decs) {
    tenv->Enter(td->name_, new type::NameTy(td->name_, nullptr));
  }
  for (auto td : valid_decs) {
    auto res_ty= td->ty_->SemAnalyze(tenv, errormsg);
    static_cast<type::NameTy *>(tenv->Look(td->name_))->ty_ = res_ty;
  }
}

type::Ty *NameTy::SemAnalyze(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  auto ty = tenv->Look(this->name_);
  if (ty == nullptr) {
    errormsg->Error(pos_, "undefined type %s", this->name_->Name().data());
    return error_ty_;
  }
  return new type::NameTy(this->name_, ty);
}

type::Ty *RecordTy::SemAnalyze(env::TEnvPtr tenv,
                               err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  return new type::RecordTy(this->record_->MakeFieldList(tenv, errormsg));
}

type::Ty *ArrayTy::SemAnalyze(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab4 code here */
  assert(this->array_);
  auto ty = tenv->Look(this->array_);
  if (ty == nullptr) {
    errormsg->Error(pos_, "undefined type %s", this->array_->Name().data());
    return error_ty_;
  }
  return new type::ArrayTy(ty);
}

} // namespace absyn

namespace sem {

void ProgSem::SemAnalyze() {
  FillBaseVEnv();
  FillBaseTEnv();
  absyn_tree_->SemAnalyze(venv_.get(), tenv_.get(), errormsg_.get());
}
} // namespace sem

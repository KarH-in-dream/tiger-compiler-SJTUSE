#include "straightline/slp.h"

#include <cstdio>
#include <iostream>

static int mmax(int a, int b) {
  return a > b ? a : b;
}

namespace A {
int A::CompoundStm::MaxArgs() const {
  // Return the bigger one.
  return mmax(
    this->stm1->MaxArgs(), 
    this->stm2->MaxArgs());
}

Table *A::CompoundStm::Interp(Table *t) const {
  // Caution: stm1 first.
  return this->stm2->Interp(
    this->stm1->Interp(t));
}

int A::AssignStm::MaxArgs() const {
  // id is not an arg, just ask exp.
  return this->exp->MaxArgs();
}

Table *A::AssignStm::Interp(Table *t) const {
  // Calc the Exp and assign it. 
  // Context should be passed to the Exp.
  IntAndTable * result = this->exp->Interp(t);
  // Update the context, but by appending.
  // Updating the old value may lead to bugs
  // bcuz they may not exist in the same context.
  return result->t->Update(id, result->i);
}

int A::PrintStm::MaxArgs() const {
  // What if an even longer PrintStm is
  // hidden in some Eseq of exps?
  return mmax(
    this->exps->NumExps(), 
    this->exps->MaxArgs());
}

Table *A::PrintStm::Interp(Table *t) const {
  // traverse and print.
  auto res = this->exps->Interp(t);
  // while not the last one
  while (res->next != nullptr) {
    // print the current value
    printf("%d ", res->i);
    // pass to the next exp with updated context
    res = res->next->Interp(res->t);
  }
  // print the last one and enter
  printf("%d\n", res->i);
  return res->t;
}

int IdExp::MaxArgs() const {
  // An id has no args.
  return 0;
}

IntAndTable *IdExp::Interp(Table * t) const {
  // Search in the context
  // Notice AssignStm doesn't include IdExp
  // So Lookup never fails unless invalid input
  return new IntAndTable(t->Lookup(id), t);
}

int NumExp::MaxArgs() const {
  // A num has no args.
  return 0;
}

IntAndTable *NumExp::Interp(Table *t) const {
  // just value
  return new IntAndTable(this->num, t);
}

int OpExp::MaxArgs() const {
  // simple max
  return mmax(
    this->left->MaxArgs(), 
    this->right->MaxArgs());
}

IntAndTable *OpExp::Interp(Table *t) const {
  // calc both children first
  auto l = this->left->Interp(t);
  // Notice the context has changed
  auto r = this->right->Interp(l->t);
  int res = 0;
  switch (this->oper) {
    case BinOp::PLUS:
      res = l->i + r->i; break;
    case BinOp::MINUS:
      res = l->i - r->i; break;
    case BinOp::TIMES:
      res = l->i * r->i; break;
    case BinOp::DIV:
      res = l->i / r->i; break;
  }
  // Notice the context has changed
  return new IntAndTable(res, r->t);
}

int EseqExp::MaxArgs() const {
  return mmax(
    this->stm->MaxArgs(), 
    this->exp->MaxArgs());
}

IntAndTable *EseqExp::Interp(Table *t) const {
  // calc stm first
  auto nt = this->stm->Interp(t);
  return this->exp->Interp(nt);
}

int PairExpList::MaxArgs() const {
  // simple max
  return mmax(
    this->exp->MaxArgs(), 
    this->tail->MaxArgs());
}

int PairExpList::NumExps() const {
  // Plus current head
  return this->tail->NumExps() + 1;
}

IntTableExpL *PairExpList::Interp(Table *t) const {
  auto res = this->exp->Interp(t);
  return new IntTableExpL(
    res->i, res->t, this->tail);
}

int LastExpList::MaxArgs() const {
  return this->exp->MaxArgs();
}

int LastExpList::NumExps() const {
  return 1;
}

IntTableExpL *LastExpList::Interp(Table *t) const {
  auto res = this->exp->Interp(t);
  return new IntTableExpL(
    res->i, res->t, nullptr);
}

int Table::Lookup(const std::string &key) const {
  if (id == key) {
    return value;
  } else if (tail != nullptr) {
    return tail->Lookup(key);
  } else {
    assert(false);
  }
}

Table *Table::Update(const std::string &key, int val) const {
  return new Table(key, val, this);
}
}  // namespace A

#include "tiger/translate/translate.h"

#include <tiger/absyn/absyn.h>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/x64frame.h"
#include "tiger/semant/types.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <stack>

extern frame::Frags *frags;
extern frame::RegManager *reg_manager;
extern llvm::IRBuilder<> *ir_builder;
extern llvm::Module *ir_module;
/// KH-note:
/// To create basic block, we should provide the function to append.
/// Each time we fill a function body, we should modify this.
std::stack<llvm::Function *> func_stack;
/// KH-note: this is used to get BREAK target.
std::stack<llvm::BasicBlock *> loop_stack;
/// KH-note:
/// These functions are tiger-lib functions, but you should declare them.
/// They are not included in VEnv, so user can't call them directly.
llvm::Function *alloc_record;
llvm::Function *init_array;
llvm::Function *string_equal;
/// KH-note: this is used to output frame info to end of .ll file.
std::vector<std::pair<std::string, frame::Frame *>> frame_info;

/// KH-note:
/// In tiger, all int values are i32.
/// But for llvm, cond-br only accept i1 as cond value.
/// Thus, sometimes OpExp should return i1 instead, and sometimes i1 should be extended to i32 as value.
/// That's why I create a stack here. If stack top is true, then OpExp can return i1, or else do zext.
/// It's not enough to cover all situations that an i32 value is used as cond in tiger.
/// Look at another function of mine, tr::convertToI1 for more.
std::stack<bool> op_needs_cond;

/// KH-note: It's too late when I realized this function is already provided...
bool CheckBBTerminatorIsBranch(llvm::BasicBlock *bb) {
  auto inst = bb->getTerminator();
  if (inst) {
    llvm::BranchInst *branchInst = llvm::dyn_cast<llvm::BranchInst>(inst);
    if (branchInst && !branchInst->isConditional()) {
      return true;
    }
  }
  return false;
}
/// KH-note: and this one as well.
int getActualFramesize(tr::Level *level) {
  return level->frame_->calculateActualFramesize();
}

namespace tr {

Access *Access::AllocLocal(Level *level, bool escape) {
  return new Access(level, level->frame_->AllocLocal(escape));
}

class ValAndTy {
public:
  type::Ty *ty_;
  llvm::Value *val_;
  /// KH-note: 
  /// It's used to pass the last block to write after some part's translation.
  /// When creating phi, knowing which block the value comes from is necessary.
  /// However I choose to make sure ir_builder always focus on the last block after translation
  ///   and use ir_builder.GetInsertBlock() instead.
  /// (this is removed by TA in lab5-part2...)
  llvm::BasicBlock *last_bb_;

  ValAndTy(llvm::Value *val, type::Ty *ty) : ty_(ty), val_(val) {}
};

void ProgTr::OutputIR(std::string_view filename) {
  std::string llvmfile = std::string(filename) + ".ll";
  std::error_code ec;
  llvm::raw_fd_ostream out(llvmfile, ec, llvm::sys::fs::OpenFlags::OF_Text);
  ir_module->print(out, nullptr);
}

/// KH-note:
/// Convert a value to i1.
/// This is quite useful in IF and WHILE. 
/// op_needs_cond can only ensure OpExp returns an i1 or i32, but
///   there's cases where i32 is directly used as cond.
/// see merge.tig line 15, an i32 CallExp is used as cond.
void convertToI1(ValAndTy * value) {
  if (value->ty_->ActualTy() == type::IntTy::Instance()) {
    llvm::Type* val_ty = value->val_->getType();
    if (val_ty->getIntegerBitWidth() != 1) {
      value->val_ = ir_builder->CreateICmpNE(
        value->val_, 
        llvm::ConstantInt::get(type::IntTy::Instance()->GetLLVMType(), 0)
      );
    }
  }
  if (value->ty_->ActualTy() == type::NilTy::Instance()) {
    value->val_ = llvm::ConstantInt::getBool(llvm::Type::getInt1Ty(ir_module->getContext()), false);
    value->ty_ = type::IntTy::Instance();
  }
  /// KH-note: I suppose no auto conversion with records and strings...
}

/// KH-note:
/// This is my helper function to create a function inside (not under) given level.
/// It includes:
/// - record function in frame_info
/// - create function type (include adding %sp and sl to params)
/// It returns the created function.
llvm::Function * MyCreateFunction(Level * level, llvm::Type * Result, const std::vector<llvm::Type *> &Params) {

  /// KH-note: some values for quick calculation
  auto type_i64 = llvm::Type::getInt64Ty(ir_module->getContext());

  /// KH-note: record frame. what if the function overwrides another?
  /// Read code in semant/types.cc: if two types have the same name, ".%d" will be 
  ///   automatically added to type name.
  std::string level_name = level->frame_->Name()->Name();
  frame_info.push_back({level_name, level->frame_});

  /// KH-note: 
  /// Add caller %sp and static link to formal.
  /// llvm::ArrayRef doesn't accepts std::list as input.
  auto params_vec = new std::vector<llvm::Type *>{ type_i64, type_i64 };
   
  for (auto arg_type : Params) {
    params_vec->push_back(arg_type);
  }

  /// KH-note: create function type and function itself
  auto type_func = llvm::FunctionType::get(Result, *params_vec, false);
  auto created_func = llvm::Function::Create(type_func, 
    /// KH-note: this means function entry can be accessed globally 
    /// ("直接把全局变量的名字放在了符号表中。这样的话，这个函数可以在链接时被其他编译单元看到").
    llvm::GlobalValue::ExternalLinkage,
    level_name, ir_module
  );
  return created_func;
}  

/// KH-note:
/// This is my helper function to start function body.
/// It includes:
/// - set global function stack
/// - create function framesize global value (not initialized)
/// - create and bind ir_builder to starting block
/// - create frame sp
/// - (optional) copy args (include static link) to outgo area
/// It should be paired with MyCreateReturnFunction.
void MyCreateStartFunction(llvm::Function * created_func, Level * level, const std::vector<llvm::Type *> &Params, bool copy_arg) {

  /// KH-note: some values for quick calculation
  auto type_i64 = llvm::Type::getInt64Ty(ir_module->getContext());
  auto type_i64_ptr = llvm::Type::getInt64PtrTy(ir_module->getContext());
  auto const_0_i64 = llvm::ConstantInt::get(type_i64, 0);
  auto level_name = created_func->getName().str();

  /// KH-note: fill function stack
  func_stack.push(created_func);
  
  /// KH-note: create global framesize and init to 0 (real value is unknown here)
  auto global_framesize = 
    (llvm::GlobalVariable *)ir_module->getOrInsertGlobal(
      level_name + "_framesize_global",
      type_i64
    );
  global_framesize->setInitializer(const_0_i64);
  level->frame_->framesize_global = global_framesize;

  /// KH-note: create sp and set level
  auto func_entry_block = llvm::BasicBlock::Create(
    ir_module->getContext(),
    level_name, 
    created_func
  );
  ir_builder->SetInsertPoint(func_entry_block);
  auto sp_input = created_func->getArg(0);
  auto loaded_framesize = ir_builder->CreateLoad(
    type_i64,
    global_framesize
  );
  auto sp_real = ir_builder->CreateSub(
    sp_input, 
    loaded_framesize,
    level_name + "_sp"
  );
  level->set_sp(sp_real);

  /// KH-note: copy args (optional)
  if (copy_arg) {
    /// KH-note:
    /// This step only copy args from input to outgo area.
    /// If anyone wants to access it, they should read from outgo area
    ///   which means addrs calculated here can't be reused in function body.
    /// If someday InRegAccess wins the resurrection match, code here can't be reused
    ///   bcuz ToLLVMVal() returns addr **value** as i64, not type*.

    auto itr_acc = level->frame_->formals_->cbegin();
    if (itr_acc == level->frame_->formals_->cend()) {
      std::cerr << __FILE__ << ' ' << __LINE__ << ':' << "frame formal empty\n";
    }
    
    /// KH-note: step1. for static link
    {
      auto sl_access = *itr_acc;
      auto val = sl_access->ToLLVMVal(sp_real);
      auto sl_ptr = ir_builder->CreateIntToPtr(val, type_i64_ptr, "sl_ptr");
      ir_builder->CreateStore(created_func->getArg(1), sl_ptr);
    }

    /// KH-note: step2. for other params
    auto loops = Params.size();
    if (loops != level->frame_->formals_->size() - 1) {
      std::cerr << __FILE__ << ' ' << __LINE__ << ':' << "frame formal size and given arg types mismatch\n";
    }
    for (auto i = 0; i < loops; i ++) {
      itr_acc ++;
      auto arg = created_func->getArg(i + 2);
      auto access = *itr_acc;
      auto type_arg_ptr = llvm::PointerType::get(Params[i], 0);
      
      auto val = access->ToLLVMVal(sp_real);
      auto ptr = ir_builder->CreateIntToPtr(val, type_arg_ptr);
      ir_builder->CreateStore(arg, ptr);
    }
  }
}

/// KH-note:
/// This is my helper function to end a function.
/// It includes:
/// - create return
/// - set framesize global
/// - set global function stack
/// It should be paired with MyCreateStartFunction.
void MyCreateReturnFunction(Level * level, ValAndTy * Result) {

  if (Result->ty_ == type::VoidTy::Instance()) {
    ir_builder->CreateRetVoid();
  }
  else {
    ir_builder->CreateRet(Result->val_);
  }

  auto type_i64 = llvm::Type::getInt64Ty(ir_module->getContext());
  auto real_framesize = level->frame_->calculateActualFramesize();
  auto const_fs_i64 = llvm::ConstantInt::get(type_i64, real_framesize);
  level->frame_->framesize_global->setInitializer(const_fs_i64);

  /// KH-note: pop function stack
  func_stack.pop();
}

void ProgTr::Translate() {
  FillBaseVEnv();
  FillBaseTEnv();
  /* TASK: Put your lab5-part1 code here */

  /// KH-note:
  /// alloc_record, init_array, string_equal
  /// These are not pre-declared (wtf)
  auto type_i32 = llvm::Type::getInt32Ty(ir_module->getContext());
  auto type_i1 = llvm::Type::getInt1Ty(ir_module->getContext());
  auto type_i64 = llvm::Type::getInt64Ty(ir_module->getContext());
  auto type_str = type::StringTy::Instance()->GetLLVMType();
  auto fty_alloc_record = llvm::FunctionType::get(
    type_i64, { type_i32 }, false);
  auto fty_init_array = llvm::FunctionType::get(
    type_i64, { type_i32, type_i64 }, false);
  auto fty_string_equal = llvm::FunctionType::get(
    type_i1, { type_str, type_str }, false);
  /// KH-note: just create it like that in FillBaseVEnv, it will be linked.
  alloc_record = llvm::Function::Create(
    fty_alloc_record, llvm::Function::ExternalLinkage, "alloc_record", ir_module);
  init_array = llvm::Function::Create(
    fty_init_array, llvm::Function::ExternalLinkage, "init_array", ir_module);
  string_equal = llvm::Function::Create(
    fty_string_equal, llvm::Function::ExternalLinkage, "string_equal", ir_module);

  /// KH-note:
  /// ProgTr holds the level "tigermain", while absyn_tree_ only has a root exp.
  /// Thus it's ProgTr's duty to create main function.

  /// KH-note:
  /// tigermain()'s frame doesn't contain space for static link.
  /// Its level is not created with tr::level::NewLevel, but static link is still its formal.

  auto mainf = MyCreateFunction(this->main_level_.get(), type_i32, {});
  MyCreateStartFunction(mainf, this->main_level_.get(), {}, false);
  
  /// KH-note: translate function body
  auto tree_tr_res = this->absyn_tree_->Translate(
    this->venv_.get(), 
    this->tenv_.get(), 
    this->main_level_.get(), 
    this->errormsg_.get()
  );

  /// To match type declared in tigermain
  if (tree_tr_res->ty_ != type::IntTy::Instance()) {
    tree_tr_res->val_ = llvm::ConstantInt::get(type_i32, 0);
    tree_tr_res->ty_ = type::IntTy::Instance();
  }
  MyCreateReturnFunction(this->main_level_.get(), tree_tr_res);
}

} // namespace tr

namespace absyn {

/// KH-note:
/// This is my helper function to calculate static link from current to target.
/// @return a static link value of i64 type, equal to target's %sp.
llvm::Value * MyCalculateStaticLink(tr::Level * current, tr::Level * target) {
  auto type_i64 = llvm::Type::getInt64Ty(ir_module->getContext());
  auto type_i64_ptr = llvm::Type::getInt64PtrTy(ir_module->getContext());

  auto lv_sp = current->get_sp();
  auto cur_level = current;

  while (cur_level != target) {
    /// KH-note: not in the same level, move up.
    /// static link is always the first formal in frame.
    auto static_link = *(cur_level->frame_->formals_->begin());
    auto sl_addr_val = static_link->ToLLVMVal(lv_sp);
    auto sl_addr_ptr = ir_builder->CreateIntToPtr(sl_addr_val, type_i64_ptr);
    /// KH-note: static link is parent frame's stack pointer.
    lv_sp = ir_builder->CreateLoad(type_i64, sl_addr_ptr);
    cur_level = cur_level->parent_;
  }

  return lv_sp;
}

tr::ValAndTy *AbsynTree::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level,
                                   err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: AbsynTree can do nothing more.
  /// Obviously it doesn't want i1 as return value.
  op_needs_cond.push(false);
  auto root_res = this->root_->Translate(venv, tenv, level, errormsg);
  op_needs_cond.pop();
  return root_res;
}

void TypeDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv, tr::Level *level,
                        err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note:
  /// Actually we do similiar work like semant here.
  /// So how to get the generated type for llvm?
  /// Call GetLLVMType()! It even auto creates record type for you.
  for (auto ty : this->types_->GetList()) {
    tenv->Enter(ty->name_, new type::NameTy(ty->name_, nullptr));
  }
  for (auto ty : this->types_->GetList()) {
    auto found = (type::NameTy *)tenv->Look(ty->name_);
    found->ty_ = ty->ty_->Translate(tenv, errormsg);
  }
}

void FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                            tr::Level *level, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note:
  /// Like in semant, this step is also completed in 2 steps:
  /// - First create functions
  /// - Then fill function body

  for (auto fdec : this->functions_->GetList()) {
    std::list<bool> f_escape = {};
    for (auto p : fdec->params_->GetList()) {
      f_escape.push_back(p->escape_);
    }
    auto new_lev = tr::Level::NewLevel(level, fdec->name_, f_escape);
    auto ret_ty = fdec->result_ ?
      tenv->Look(fdec->result_) :
      type::VoidTy::Instance();
    std::vector<llvm::Type *> formal;
    auto fty_list = fdec->params_->MakeFormalTyList(tenv, errormsg);
    for (auto fty : fty_list->GetList()) {
      formal.push_back(fty->GetLLVMType());
    }
    auto mk_func = tr::MyCreateFunction(
      new_lev, ret_ty->GetLLVMType(), formal
    );

    /// KH-note: use lab5 version entry constructor.
    auto entry = new env::FunEntry(
      new_lev,
      fty_list,
      ret_ty,
      mk_func->getFunctionType(),
      mk_func
    );
    venv->Enter(fdec->name_, entry);
  }

  for (auto fdec : this->functions_->GetList()) {
    auto entry = (env::FunEntry *)venv->Look(fdec->name_);
    /// KH-note:
    /// Following code will fill function body, which means setting writer to another block.
    /// I should set the writer back when the function is done.
    auto remember_block = ir_builder->GetInsertBlock();
    venv->BeginScope();
    {
      /// KH-note:
      /// Put all params excluding %sp and static link to venv.
      /// This time, we should remember which access is binded with this param.
      auto access_list = entry->level_->frame_->formals_;
      auto param_list = fdec->params_->GetList();
      auto itr_access = access_list->begin();
      /// KH-note: skip the static link
      itr_access ++;
      auto itr_param = param_list.begin();

      /// KH-note: setup VEnv for function body
      while (itr_access != access_list->end() && itr_param != param_list.end()) {
        auto arg_entry = new env::VarEntry(
          /// KH-note: this is not a local access, so don't need access local.
          new tr::Access(entry->level_, *itr_access),
          tenv->Look((*itr_param)->typ_)
        );
        venv->Enter((*itr_param)->name_, arg_entry);
        itr_access ++;
        itr_param ++;
      }
      if (itr_access != access_list->end() || itr_param != param_list.end()) {
        std::cerr << __FILE__ << ' ' << __LINE__ << ':' << "list mismatch\n";
      }

      /// KH-note: start to translate function body. (a bit mess here...)
      std::vector<llvm::Type *> formal;
      auto fty_list = fdec->params_->MakeFormalTyList(tenv, errormsg);
      for (auto fty : fty_list->GetList()) {
        formal.push_back(fty->GetLLVMType());
      }
      tr::MyCreateStartFunction(
        entry->func_, entry->level_, formal, true
      );

      /// KH-note: translate body and return. shouldn't return i1 as function result.
      op_needs_cond.push(false);
      auto body_res = fdec->body_->Translate(venv, tenv, entry->level_, errormsg);
      op_needs_cond.pop();
      tr::MyCreateReturnFunction(entry->level_, body_res);
    }
    venv->EndScope();
    /// KH-note: back to track
    ir_builder->SetInsertPoint(remember_block);
  }
}

void VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv, tr::Level *level,
                       err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note:
  /// alloc space in stack, then store init value in access.
  /// note this->typ_ might be nullptr when its type is decided by init_.
  /// init_ doesn't depend on var_, so calculate init_ first.
  op_needs_cond.push(false);
  auto init_res = this->init_->Translate(venv, tenv, level, errormsg);
  op_needs_cond.pop();

  auto ty = init_res->ty_->GetLLVMType();
  auto ptr_ty = llvm::PointerType::get(ty, 0);
  auto access = level->frame_->AllocLocal(this->escape_);

  auto arg_entry = new env::VarEntry(
    new tr::Access(level, access), 
    init_res->ty_
  );
  venv->Enter(this->var_, arg_entry);

  auto get_addr = access->ToLLVMVal(level->get_sp());
  auto get_ptr = ir_builder->CreateIntToPtr(get_addr, ptr_ty);
  ir_builder->CreateStore(init_res->val_, get_ptr);
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: copied from semant.cc
  auto ty = tenv->Look(this->name_);
  return new type::NameTy(this->name_, ty);
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: copied from semant.cc
  return new type::RecordTy(this->record_->MakeFieldList(tenv, errormsg));
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: copied from semant.cc
  auto ty = tenv->Look(this->array_);
  return new type::ArrayTy(ty);
}

tr::ValAndTy *SimpleVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level,
                                   err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  auto entry = (env::VarEntry *)venv->Look(this->sym_);

  /// KH-note: Escape should be considered here.
  /// Go static link till var's level, then calc addr.
  /// Details are in PPT
  auto static_link = MyCalculateStaticLink(level, entry->access_->level_);

  auto sv_addr_val = entry->access_->access_->ToLLVMVal(static_link);
  auto type_sv_val = entry->ty_->GetLLVMType();
  auto type_sv_ptr = llvm::PointerType::get(type_sv_val, 0);
  /// KH-note:
  /// Why return ptr type? Bcuz Var is used both for lvalue and rvalue.
  /// Read code in qsort.tig.ll line 60,61 for evidence.
  /// All SimpleVars are given a name in .ll.
  auto sv_addr_ptr = ir_builder->CreateIntToPtr(
    sv_addr_val, 
    type_sv_ptr,
    entry->access_->level_->frame_->name_->Name() + "_" + this->sym_->Name() + "_ptr"
  );
  return new tr::ValAndTy(sv_addr_ptr, entry->ty_);
}

tr::ValAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level,
                                  err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  auto type_i32 = llvm::Type::getInt32Ty(ir_module->getContext());
  /// KH-note: this is of type MyStruct**, bcuz it's a var, not exp
  /// A normal RecordExp (not NilExp) has type MyStruct*
  auto var_res = this->var_->Translate(venv, tenv, level, errormsg);
  auto rec_ty = (type::RecordTy *)(var_res->ty_->ActualTy());
  /// KH-note:
  /// A deeper sight into GEP: it's just a address calculation without looking into stackframes.
  /// It can't automatically knows which MyStruct.a* is through an MyStruct**,
  ///   bcuz the MyStruct** is not an array; it stores MyStruct* value.
  /// Thus I need a load here to get the real data.
  auto real_rec_ptr = ir_builder->CreateLoad(rec_ty->GetLLVMType(), var_res->val_);

  /// KH-note: find in record fields to decide target field's offset.
  auto rec_fields = rec_ty->fields_->GetList();
  /// KH-note: 
  /// field is stored by address in record.
  /// We don't need to concern about wordsize; llvm accept field number, not the real byte offset.
  int field_offset = 0;
  type::Ty * field_ty = nullptr;
  for (auto rec_f : rec_fields) {
    if (rec_f->name_ == this->sym_) {
      /// KH-note: field found. I suppose field exists (bcuz semant)
      field_ty = rec_f->ty_;
      break;
    }
    field_offset ++;
  }

  if (field_ty == nullptr) {
    std::cerr << __FILE__ << ' ' << __LINE__ << ':' << "field 404\n";
  }
  /// KH-note: get element. The result is ptr to target field.
  auto field_var = ir_builder->CreateGEP(
    rec_ty->GetLLVMType()->getNonOpaquePointerElementType(),
    real_rec_ptr,
    {
      llvm::ConstantInt::get(type_i32, 0),
      llvm::ConstantInt::get(type_i32, field_offset)
    }
  );
  return new tr::ValAndTy(field_var, field_ty);
}

tr::ValAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level,
                                      err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  auto var_res = this->var_->Translate(venv, tenv, level, errormsg);
  auto arr_ty = (type::ArrayTy *)(var_res->ty_->ActualTy());
  /// KH-note: Similiar to FieldVar, I need a load here.
  /// var is address of the array object.
  /// In tiger, one array's elems are closely stored somewhere in heap.
  auto real_arr_ptr = ir_builder->CreateLoad(arr_ty->GetLLVMType(), var_res->val_);

  /// KH-note:
  /// subscript is an exp.
  /// Different from Var, Exp is translated to pure value and will never be assigned.
  op_needs_cond.push(false);
  auto sub_res = this->subscript_->Translate(venv, tenv, level, errormsg);
  op_needs_cond.pop();

  /// KH-note: [elemtype] -> elemtype (actually elemtype* -> elemtype)
  auto res_val = ir_builder->CreateGEP(
    arr_ty->ty_->GetLLVMType(),
    real_arr_ptr,
    sub_res->val_
  );
  return new tr::ValAndTy(res_val, arr_ty->ty_);
}

tr::ValAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level,
                                err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: simply load var value through var_ptr
  auto var_res = this->var_->Translate(venv, tenv, level, errormsg);
  auto val_res = ir_builder->CreateLoad(var_res->ty_->GetLLVMType(), var_res->val_);
  return new tr::ValAndTy(val_res, var_res->ty_);
}

tr::ValAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level,
                                err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: 
  /// I make NilExp as i64 0, but actually it should be a nullptr.
  /// For RecordTy, GetLLVMType actually returns a ptr value, so a SimpleVar of RecordTy
  ///   is actually MyStruct**, its VarExp value is MyStruct*
  /// However, llvm IR's null is not like that in C/C++; it must have a type.
  /// So I just gave NilExp a special temp value; it might be unused.
  auto type_i64 = llvm::Type::getInt64Ty(ir_module->getContext());
  return new tr::ValAndTy(llvm::ConstantInt::get(type_i64, 0), type::NilTy::Instance());
}

tr::ValAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level,
                                err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: all int values are i32 in tiger, you can call IntTy::Instance().GetLLVMType() too
  auto type_i32 = llvm::Type::getInt32Ty(ir_module->getContext());
  return new tr::ValAndTy(llvm::ConstantInt::get(type_i32, this->val_), type::IntTy::Instance());
}

tr::ValAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level,
                                   err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: the function is given, so you can feel easy.
  auto global_str = type::StringTy::CreateGlobalStringStructPtr(this->str_);
  return new tr::ValAndTy(global_str, type::StringTy::Instance());
}

tr::ValAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level,
                                 err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */

  /// KH-note:
  /// Runtime-system calls don't have %sp and static link as args. How to decide whether 
  ///   a function is a runtime-system function?
  /// Answer: look its level. All runtime-system functions and tigermain are defined in 
  ///   main_level_, whose parent is nullptr.
  auto func_entry = (env::FunEntry *)(venv->Look(this->func_));
  bool is_builtin = (func_entry->level_->parent_ == nullptr);

  /// KH-note: calculate param exp values
  std::vector<llvm::Value *> arg_vec = {};
  if (!is_builtin) {
    /// KH-note: add %sp and static link
    arg_vec.push_back(level->get_sp());
    /// KH-note: 
    /// The static link should be calculated.
    /// current level must be lower than where the function is declared to call it.
    /// func_entry->level_->parent_ is where the function is declared, not func_entry->level_.
    auto sl = MyCalculateStaticLink(level, func_entry->level_->parent_);
    arg_vec.push_back(sl);
  }
  op_needs_cond.push(false);
  for (auto arg : this->args_->GetList()) {
    auto arg_res = arg->Translate(venv, tenv, level, errormsg);
    arg_vec.push_back(arg_res->val_);
  }
  op_needs_cond.pop();

  /// KH-note: create call
  auto call_res = ir_builder->CreateCall(func_entry->func_, arg_vec);
  /// KH-note: allocate space in current frame's out_go area.
  /// Actually, system built-in functions don't need outgo_area. But given testcases also 
  ///   save space for static link and params for them.
  level->frame_->AllocOutgoSpace((this->args_->GetList().size() + 1) * reg_manager->WordSize());
  return new tr::ValAndTy(call_res, func_entry->result_);
}

tr::ValAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level,
                               err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  auto type_i1 = llvm::Type::getInt1Ty(ir_module->getContext());
  auto type_i32 = llvm::Type::getInt32Ty(ir_module->getContext());
  auto type_i64 = llvm::Type::getInt64Ty(ir_module->getContext());

  /// KH-note: some values for simplicy.
  bool use_arithmetic = this->oper_ == Oper::PLUS_OP || this->oper_ == Oper::MINUS_OP ||
    this->oper_ == Oper::TIMES_OP || this->oper_ == Oper::DIVIDE_OP;
  bool use_logical = this->oper_ == Oper::AND_OP || this->oper_ == Oper::OR_OP;
  bool use_equality = this->oper_ == Oper::EQ_OP || this->oper_ == Oper::NEQ_OP;
  bool use_compare = this->oper_ == Oper::LT_OP || this->oper_ == Oper::LE_OP ||
    this->oper_ == Oper::GT_OP || this->oper_ == Oper::GE_OP;

  if (use_arithmetic || use_compare) {
    /// KH-note: these operators are only for i32 type
    op_needs_cond.push(false);
  }
  if (use_logical) {
    /// KH-note: these operators are only for i1 type
    op_needs_cond.push(true);
  }

  /// KH-note: no matter what, translate l&r first
  auto l_res = this->left_->Translate(venv, tenv, level, errormsg);
  /// KH-note: '&' and '|' has shortcut 
  /// - if left_ can decide result, then the right won't be calculated.
  auto r_res = (use_logical) ? nullptr :
    this->right_->Translate(venv, tenv, level, errormsg);

  if (use_arithmetic || use_compare || use_logical) {
    op_needs_cond.pop();
  }
  if (use_logical) {
    tr::convertToI1(l_res);
  }

  llvm::Value * result;
  switch (this->oper_) {
    /// KH-note:
    /// The most simple arithmetic.
    /// These opers only accepts Int inputs.
    case Oper::PLUS_OP: {
      result = ir_builder->CreateAdd(l_res->val_, r_res->val_);
      break;
    }
    case Oper::MINUS_OP: {
      result = ir_builder->CreateSub(l_res->val_, r_res->val_);
      break;
    }
    case Oper::TIMES_OP: {
      result = ir_builder->CreateMul(l_res->val_, r_res->val_);
      break;
    }
    case Oper::DIVIDE_OP: {
      result = ir_builder->CreateSDiv(l_res->val_, r_res->val_);
      break;
    }

    /// KH-note: enable shortcut
    case Oper::AND_OP: {
      /// KH-note: read merge.tig as sample
      auto test_ = llvm::BasicBlock::Create(
        ir_module->getContext(),
        "opand_right_test",
        func_stack.top()
      );
      auto next_ = llvm::BasicBlock::Create(
        ir_module->getContext(),
        "opand_next",
        func_stack.top()
      );

      /// KH-note: if left is false, then result is false.
      ir_builder->CreateCondBr(l_res->val_, test_, next_);
      /// KH-note: used for PHI
      auto source_block_left = ir_builder->GetInsertBlock();

      /// KH-note: move to right
      ir_builder->SetInsertPoint(test_);
      op_needs_cond.push(true);
      r_res = this->right_->Translate(venv, tenv, level, errormsg);
      tr::convertToI1(r_res);
      op_needs_cond.pop();
      auto source_block_right = ir_builder->GetInsertBlock();
      /// KH-note:
      /// Here won't be interrupted by BREAK, bcuz BREAK returns void
      ///   but any end-of-path in OPEXP must finally return a value.
      ir_builder->CreateBr(next_);

      /// KH-note: set next
      ir_builder->SetInsertPoint(next_);
      auto phi = ir_builder->CreatePHI(type_i1, 2);
      /// KH-note: if skip test, then it must be false.
      phi->addIncoming(llvm::ConstantInt::getBool(type_i1, false), source_block_left);
      phi->addIncoming(r_res->val_, source_block_right);

      result = phi;
      break;
    }
    case Oper::OR_OP: {
      auto test_ = llvm::BasicBlock::Create(
        ir_module->getContext(),
        "opor_right_test",
        func_stack.top()
      );
      auto next_ = llvm::BasicBlock::Create(
        ir_module->getContext(),
        "opor_next",
        func_stack.top()
      );

      /// KH-note: if left is true, then result is true.
      ir_builder->CreateCondBr(l_res->val_, next_, test_);
      auto source_block_left = ir_builder->GetInsertBlock();

      /// KH-note: right should be calculated later bcuz it may have side effects.
      ir_builder->SetInsertPoint(test_);
      op_needs_cond.push(true);
      r_res = this->right_->Translate(venv, tenv, level, errormsg);
      tr::convertToI1(r_res);
      op_needs_cond.pop();
      auto source_block_right = ir_builder->GetInsertBlock();
      ir_builder->CreateBr(next_);

      ir_builder->SetInsertPoint(next_);
      auto phi = ir_builder->CreatePHI(type_i1, 2);
      phi->addIncoming(llvm::ConstantInt::getBool(type_i1, true), source_block_left);
      phi->addIncoming(r_res->val_, source_block_right);

      result = phi;
      break;
    }
    /// KH-note:
    /// EQ and NEQ accepts string and record as formals.
    /// Depending on input type, it will have different translations.
    case Oper::EQ_OP: {
      if (l_res->ty_->ActualTy() == type::IntTy::Instance()) {
        result = ir_builder->CreateICmpEQ(l_res->val_, r_res->val_);
      }
      else if (l_res->ty_->ActualTy() == type::StringTy::Instance()) {
        result = ir_builder->CreateCall(string_equal, { l_res->val_, r_res->val_ });
      }
      else {
        /// KH-note: l_res may be nil.
        /// Comparing records equals to comparing their address.
        /// l&r_res.val are %MyStruct*, so PrtToInt64 is required before comparison.
        /// And note that NilExp is translated to an i64 int with type NilTy (no need to convert)
        if (l_res->ty_ != type::NilTy::Instance()) {
          l_res->val_ = ir_builder->CreatePtrToInt(l_res->val_, type_i64);
        }
        if (r_res->ty_ != type::NilTy::Instance()) {
          r_res->val_ = ir_builder->CreatePtrToInt(r_res->val_, type_i64);
        }
        result = result = ir_builder->CreateICmpEQ(l_res->val_, r_res->val_);
      }
      break;
    }
    case Oper::NEQ_OP: {
      /// KH-note: similiar to EQ
      if (l_res->ty_->ActualTy() == type::IntTy::Instance()) {
        result = ir_builder->CreateICmpNE(l_res->val_, r_res->val_);
      }
      else if (l_res->ty_->ActualTy() == type::StringTy::Instance()) {
        /// KH-note: we don't have string_unequal_, so here needs more translation.
        /// Testcase doesn't include this case.
        auto eq_res = ir_builder->CreateCall(string_equal, { l_res->val_, r_res->val_ });
        result = ir_builder->CreateICmpEQ(eq_res, llvm::ConstantInt::get(type_i1, 0));
      }
      else {
        if (l_res->ty_ != type::NilTy::Instance()) {
          l_res->val_ = ir_builder->CreatePtrToInt(l_res->val_, type_i64);
        }
        if (r_res->ty_ != type::NilTy::Instance()) {
          r_res->val_ = ir_builder->CreatePtrToInt(r_res->val_, type_i64);
        }
        result = result = ir_builder->CreateICmpNE(l_res->val_, r_res->val_);
      }
      break;
    }
    /// KH-note: string comparison function is not provided.
    /// Tiger int are signed, so use signed ICmp here.
    case Oper::GT_OP: {
      result = ir_builder->CreateICmpSGT(l_res->val_, r_res->val_);
      break;
    }
    case Oper::LT_OP: {
      result = ir_builder->CreateICmpSLT(l_res->val_, r_res->val_);
      break;
    }
    case Oper::GE_OP: {
      result = ir_builder->CreateICmpSGE(l_res->val_, r_res->val_);
      break;
    }
    case Oper::LE_OP: {
      result = ir_builder->CreateICmpSLE(l_res->val_, r_res->val_);
      break;
    }
    default: {
      std::cerr << __FILE__ << ' ' << __LINE__ << ':' << "Unknown OPER\n";
      result = nullptr;
    }
  }

  if ((!op_needs_cond.top()) && (use_equality || use_logical || use_compare)) {
    /// KH-note: can't return i1 in these situations
    result = ir_builder->CreateZExt(result, type_i32);
  }

  return new tr::ValAndTy(result, type::IntTy::Instance());
}

tr::ValAndTy *RecordExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level,
                                   err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  auto type_i32 = llvm::Type::getInt32Ty(ir_module->getContext());

  /// KH-note:
  /// How to create a record?
  /// First, allocate space for it. Each attr takes a int size. Then, fill arg exps.
  /// Note: use ActualTy(), bcuz types in tenv are actually NameTys.
  auto rec_ty = (type::RecordTy *)(tenv->Look(this->typ_)->ActualTy());
  auto rec_attrs = rec_ty->fields_->GetList().size();
  auto param_rec_size = llvm::ConstantInt::get(type_i32, rec_attrs * reg_manager->WordSize());
  auto rec_alloc_res = ir_builder->CreateCall(alloc_record, { param_rec_size });
  auto rec_ptr = ir_builder->CreateIntToPtr(rec_alloc_res, rec_ty->GetLLVMType());

  /// KH-note: fill arg values
  for (auto ef : this->fields_->GetList()) {
    op_needs_cond.push(false);
    auto ef_res = ef->exp_->Translate(venv, tenv, level, errormsg);
    op_needs_cond.pop();

    /// KH-note: llvm can't tell an field's id with its name; we need to count it manually.
    auto itr = rec_ty->fields_->GetList().begin();
    for (int i = 0; i < rec_attrs; i ++) {
      if ((*itr)->name_ == ef->name_) {
        /// KH-note: get the id of current field.
        /// MyStruct* -> MyStruct -> Attribute*
        auto attr_ptr = ir_builder->CreateGEP(
          rec_ty->GetLLVMType()->getNonOpaquePointerElementType(),
          rec_ptr,
          {
            llvm::ConstantInt::get(type_i32, 0),
            llvm::ConstantInt::get(type_i32, i)
          }
        );
        ir_builder->CreateStore(ef_res->val_, attr_ptr);
        break;
      }
      itr ++;
    }
  }

  return new tr::ValAndTy(rec_ptr, rec_ty);
}

tr::ValAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level,
                                err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: the last exp in seq is the result
  tr::ValAndTy * result;
  for (auto ex : this->seq_->GetList()) {
    op_needs_cond.push(false);
    result = ex->Translate(venv, tenv, level, errormsg);
    op_needs_cond.pop();
  }
  return result;
}

tr::ValAndTy *AssignExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level,
                                   err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: this is type*
  auto var_res = this->var_->Translate(venv, tenv, level, errormsg);

  /// KH-note: this is value
  op_needs_cond.push(false);
  auto exp_res = this->exp_->Translate(venv, tenv, level, errormsg);
  /// KH-note: remember Nil and Record are not the same llvm type
  if (exp_res->ty_ == type::NilTy::Instance()) {
    exp_res->val_ = llvm::ConstantPointerNull::get((llvm::PointerType *)var_res->ty_->GetLLVMType());
  }
  op_needs_cond.pop();

  ir_builder->CreateStore(exp_res->val_, var_res->val_);
  /// KH-note: return void type. I want to reduce duplicated code
  return VoidExp(this->pos_).Translate(venv, tenv, level, errormsg);
}

tr::ValAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level,
                               err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  auto test_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "if_test",
    func_stack.top()
  );
  auto then_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "if_then",
    func_stack.top()
  );
  auto else_b = this->elsee_ == nullptr ?
    nullptr :
    llvm::BasicBlock::Create(
      ir_module->getContext(),
      "if_else",
      func_stack.top()
    );
  auto next_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "if_next",
    func_stack.top()
  );

  /// KH-note: no exp can be appended after BREAK, so no need to check break here.
  ir_builder->CreateBr(test_b);

  ir_builder->SetInsertPoint(test_b);
  op_needs_cond.push(true);
  auto cond_res = this->test_->Translate(venv, tenv, level, errormsg);
  /// KH-note: input cond may be of i32 type
  tr::convertToI1(cond_res);
  op_needs_cond.pop();

  ir_builder->CreateCondBr(
    cond_res->val_,
    then_b,
    this->elsee_ ? else_b : next_b
  );

  ir_builder->SetInsertPoint(then_b);
  /// KH-note: if (if cond1 then cond2 else cond3) then (do something)
  /// That's too complicated. I will not allow cond2 to return i1 then...
  op_needs_cond.push(false);
  auto then_res = this->then_->Translate(venv, tenv, level, errormsg);
  op_needs_cond.pop();
  auto source_block_then = ir_builder->GetInsertBlock();
  /// KH-note: 
  /// Check BREAK. If this branch already ends with break,
  ///   then here shouldn't be another br.
  if (!source_block_then->getTerminator()) {
    ir_builder->CreateBr(next_b);
  }

  if (this->elsee_) {
    ir_builder->SetInsertPoint(else_b);
    op_needs_cond.push(false);
    auto else_res = this->elsee_->Translate(venv, tenv, level, errormsg);
    op_needs_cond.pop();
    auto source_block_else = ir_builder->GetInsertBlock();
    /// KH-note: check BREAK.
    if (!source_block_else->getTerminator()) {
      ir_builder->CreateBr(next_b);
    }
    
    ir_builder->SetInsertPoint(next_b);
    if (then_res->ty_ == type::VoidTy::Instance()) {
      return VoidExp(this->pos_).Translate(venv, tenv, level, errormsg);
    }
    else {
      /// KH-note:
      /// Deal with the case that one branch is Nil.
      /// In this case, the IFEXP's return value must have some record type.
      auto res_ty = then_res->ty_;
      if (then_res->ty_ == type::NilTy::Instance()) {
        res_ty = else_res->ty_;
        then_res->val_ = llvm::ConstantPointerNull::get((llvm::PointerType *)res_ty->GetLLVMType());
      }
      if (else_res->ty_ == type::NilTy::Instance()) {
        else_res->val_ = llvm::ConstantPointerNull::get((llvm::PointerType *)res_ty->GetLLVMType());
      }

      /// KH-note: never directly use then_ and else_ in PHI
      auto phi = ir_builder->CreatePHI(then_res->ty_->GetLLVMType(), 2);
      phi->addIncoming(then_res->val_, source_block_then);
      phi->addIncoming(else_res->val_, source_block_else);

      return new tr::ValAndTy(phi, then_res->ty_);
    }
  }
  else {
    ir_builder->SetInsertPoint(next_b);
    return VoidExp(this->pos_).Translate(venv, tenv, level, errormsg);
  }
}

tr::ValAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level,
                                  err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  auto test_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "while_test",
    func_stack.top()
  );
  auto body_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "while_body",
    func_stack.top()
  );
  auto next_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "while_next",
    func_stack.top()
  );

  ir_builder->CreateBr(test_b);

  ir_builder->SetInsertPoint(test_b);
  /// KH-note: read comments in IfExp
  op_needs_cond.push(true);
  auto test_res = this->test_->Translate(venv, tenv, level, errormsg);
  tr::convertToI1(test_res);
  op_needs_cond.pop();
  ir_builder->CreateCondBr(test_res->val_, body_b, next_b);

  ir_builder->SetInsertPoint(body_b);
  /// KH-note: There might be BREAK in body, so mark where it should go with loop_stack.
  loop_stack.push(next_b);
  this->body_->Translate(venv, tenv, level, errormsg);
  loop_stack.pop();
  auto last_block_body = ir_builder->GetInsertBlock();
  /// KH-note: check BREAK. If someone simply breaks without doing a full loop...
  if (!last_block_body->getTerminator()) {
    ir_builder->CreateBr(test_b);
  }

  /// KH-note: return void
  ir_builder->SetInsertPoint(next_b);
  return VoidExp(this->pos_).Translate(venv, tenv, level, errormsg);
}

tr::ValAndTy *ForExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level,
                                err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: 
  /// ForExp can be translated into WhileExp
  /// But that's too complicated to construct a new While.

  auto test_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "for_test",
    func_stack.top()
  );
  auto body_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "for_body",
    func_stack.top()
  );
  auto incre_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "for_incre",
    func_stack.top()
  );
  auto next_b = llvm::BasicBlock::Create(
    ir_module->getContext(),
    "for_next",
    func_stack.top()
  );

  /// KH-note: first calculate low and high bounds.
  op_needs_cond.push(false);
  auto low_res = this->lo_->Translate(venv, tenv, level, errormsg);
  auto high_res = this->hi_->Translate(venv, tenv, level, errormsg);
  op_needs_cond.pop();

  venv->BeginScope();
  /// KH-note: define loop variable
  auto loop_access = level->frame_->AllocLocal(this->escape_);
  auto loop_var_entry = new env::VarEntry(
    new tr::Access(level, loop_access), 
    type::IntTy::Instance(),
    true
  );
  venv->Enter(this->var_, loop_var_entry);

  /// KH-note: init loop variable
  auto loop_var_addr = loop_access->ToLLVMVal(level->get_sp());
  auto loop_var_type = loop_var_entry->ty_->GetLLVMType();
  auto loop_var_ptr = ir_builder->CreateIntToPtr(
    loop_var_addr, 
    llvm::PointerType::get(loop_var_type, 0)
  );
  ir_builder->CreateStore(low_res->val_, loop_var_ptr);
  ir_builder->CreateBr(test_b);

  {
    ir_builder->SetInsertPoint(test_b);
    auto loop_var_val = ir_builder->CreateLoad(loop_var_type, loop_var_ptr);
    auto test_res = ir_builder->CreateICmpSLE(loop_var_val, high_res->val_);
    ir_builder->CreateCondBr(test_res, body_b, next_b);

    ir_builder->SetInsertPoint(body_b);
    /// KH-note: remember loop_stack here!
    loop_stack.push(next_b);
    this->body_->Translate(venv, tenv, level, errormsg);
    loop_stack.pop();
    /// KH-note: check BREAK. If someone simply breaks without doing a full loop...
    auto last_block_body = ir_builder->GetInsertBlock();
    if (!last_block_body->getTerminator()) {
      ir_builder->CreateBr(incre_b);
    }

    /// KH-note: create increase
    ir_builder->SetInsertPoint(incre_b);
    auto new_loop_val = ir_builder->CreateAdd(loop_var_val, llvm::ConstantInt::get(loop_var_type, 1));
    ir_builder->CreateStore(new_loop_val, loop_var_ptr);
    /// KH-note: this will never be effected by BREAK
    ir_builder->CreateBr(test_b);
  }
  venv->EndScope();

  ir_builder->SetInsertPoint(next_b);
  return VoidExp(this->pos_).Translate(venv, tenv, level, errormsg);
}

tr::ValAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level,
                                  err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: simply follow the loop stack.
  /// A tricky case is that it may cause more than 1 Br at the end of some block.
  ///   For example, BREAK is at the end of then_ of some IFEXP.
  /// Read PPT to see how to solve it.
  ir_builder->CreateBr(loop_stack.top());
  return VoidExp(this->pos_).Translate(venv, tenv, level, errormsg);
}

tr::ValAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level,
                                err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  venv->BeginScope();
  tenv->BeginScope();
  
  for (auto dec : this->decs_->GetList()) {
    dec->Translate(venv, tenv, level, errormsg);
  }
  auto result = this->body_->Translate(venv, tenv, level, errormsg);
  
  tenv->EndScope();
  venv->EndScope();

  return result;
}

tr::ValAndTy *ArrayExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level,
                                  err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: init_array accepts i64 as init value.
  /// I can't understand why... so my impl may be different here.
  /// Testcases don't include an ArrayExp with non-const init.
  
  auto type_i64 = llvm::Type::getInt64Ty(ir_module->getContext());

  op_needs_cond.push(false);
  auto val_size = this->size_->Translate(venv, tenv, level, errormsg);
  auto val_init = this->init_->Translate(venv, tenv, level, errormsg);
  op_needs_cond.pop();
  
  /// KH-note: need some translation if value isn't i64 type
  auto real_init = val_init->val_;
  if (val_init->ty_ == type::IntTy::Instance()) {
    real_init = ir_builder->CreateSExt(val_init->val_, type_i64);
  }
  else if (val_init->ty_ != type::NilTy::Instance()) {
    real_init = ir_builder->CreatePtrToInt(val_init->val_, type_i64);
  }

  auto create_res = ir_builder->CreateCall(init_array, { val_size->val_, real_init });
  /// KH-note: init_array returns i64, not target type ptr. Needs convert.
  auto arr_elem_type = val_init->ty_->GetLLVMType();
  auto arr_type = llvm::PointerType::get(arr_elem_type, 0);
  auto arr_res = ir_builder->CreateIntToPtr(create_res, arr_type);

  return new tr::ValAndTy(arr_res, tenv->Look(this->typ_));
}

tr::ValAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level,
                                 err::ErrorMsg *errormsg) const {
  /* TASK: Put your lab5-part1 code here */
  /// KH-note: this is... meaningless.
  return new tr::ValAndTy(nullptr, type::VoidTy::Instance());
}

} // namespace absyn
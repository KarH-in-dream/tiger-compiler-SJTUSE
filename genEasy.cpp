#include <iostream>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <map>

std::map<std::string, llvm::Function *> functions;

llvm::Function *addFunction(std::shared_ptr<llvm::Module> ir_module, std::string name, llvm::FunctionType *type)
{
    llvm::Function *function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, ir_module.get());
    for (auto &arg : function->args())
        arg.addAttr(llvm::Attribute::NoUndef);
    functions.insert(std::make_pair(name, function));
    return function;
}

void buildGlobal(std::shared_ptr<llvm::Module> ir_module)
{
}

void buildMain(std::shared_ptr<llvm::Module> ir_module)
{
    llvm::IRBuilder<> builder(ir_module->getContext());

    llvm::Function *main = addFunction(ir_module, "main", llvm::FunctionType::get(llvm::Type::getInt32Ty(ir_module->getContext()), false));
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(ir_module->getContext(), "", main);
    builder.SetInsertPoint(entry);

    /// KH-note: An easy llvm-IR guide at https://evian-zhang.github.io/llvm-ir-tutorial
    /// In llvm IR, comments start with ';' and function like "//" in C/C++.

    // TODO
    /// KH-note: for simplicy
    auto &context = ir_module->getContext();
    const auto type_i32 = llvm::Type::getInt32Ty(context);
    /// KH-note: these are not individual sentences in IR; these are constants.
    auto constant_null = llvm::ConstantInt::get(type_i32, 0);
    auto constant_1 = llvm::ConstantInt::get(type_i32, 1);
    auto constant_2 = llvm::ConstantInt::get(type_i32, 2);

    /// KH-note: 
    /// '''
    /// However, to support Tiger features like static linking, we adopt a 
    /// more assembly-like approach to manually allocate and manage 
    /// variables on the stack.
    /// '''
    /// I guess "%1" in origin/easy.ll is the static link. Or I just can't understand it...
    auto static_link = builder.CreateAlloca(type_i32, nullptr, "sl");
    /// KH-note:
    /// All local variables are located on stack, through 'alloca'
    /// Other value are stored in virtual registers (may also on stack).
    /// set ArraySize to nullptr to set it as a single var.
    /// "align 4" in .ll file is automatically added based on Ty.
    auto a1 = builder.CreateAlloca(type_i32, nullptr, "a1");
    auto b1 = builder.CreateAlloca(type_i32, nullptr, "b1");

    /// KH-note: Store value to some location. align is also auto-set.
    builder.CreateStore(constant_null, static_link);
    builder.CreateStore(constant_1, a1);
    builder.CreateStore(constant_2, b1);

    /// KH-note:
    /// Load local variable to temp values.
    /// llvm IR follows strict SSA(Static Single Assignment), so all temp values can't be assigned twice.
    /// That's why there's so much "%n"s.
    auto a2 = builder.CreateLoad(type_i32, a1, "a2");
    auto b2 = builder.CreateLoad(type_i32, b1, "b2");
    /// KH-note: create br(branch). SLT = signed less than
    auto comp_res = builder.CreateICmpSLT(a2, b2);

    /// KH-note:
    /// Code block's parent is the function, not the previous block.
    /// ;preds is auto-set by calculating brs.
    /// Each block should end with br or ret.
    auto entry_then = llvm::BasicBlock::Create(context, "then", main);
    auto entry_final = llvm::BasicBlock::Create(context, "final", main);
    builder.CreateCondBr(comp_res, entry_then, entry_final);

    /// KH-note: move curser to inputed block's end
    builder.SetInsertPoint(entry_then);
    auto constant_3 = llvm::ConstantInt::get(type_i32, 3);
    builder.CreateStore(constant_3, b1);
    builder.CreateBr(entry_final);

    builder.SetInsertPoint(entry_final);
    auto a3 = builder.CreateLoad(type_i32, a1, "a3");
    auto b3 = builder.CreateLoad(type_i32, b1, "b3");
    auto add_res = builder.CreateAdd(a3, b3, "sum");
    builder.CreateRet(add_res);
}

void buildFunction(std::shared_ptr<llvm::Module> ir_module)
{
    buildMain(ir_module);
}

int main()
{
    llvm::LLVMContext context;
    std::shared_ptr<llvm::Module> ir_module = std::make_shared<llvm::Module>("easy", context);
    ir_module->setTargetTriple("x86_64-pc-linux-gnu");

    buildGlobal(ir_module);
    buildFunction(ir_module);

    ir_module->print(llvm::outs(), nullptr);

    return 0;
}
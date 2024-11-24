#include <iostream>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <map>

std::map<std::string, llvm::StructType *> struct_types;
std::map<std::string, llvm::GlobalVariable *> global_values;
std::map<std::string, llvm::Function *> functions;

/// KH-note: llvm IR supports type. Its fields can be any supported array-like struct.
llvm::StructType *addStructType(std::shared_ptr<llvm::Module> ir_module, std::string name, std::vector<llvm::Type *> fields)
{
    llvm::StructType *struct_type = llvm::StructType::create(ir_module->getContext(), name);
    struct_type->setBody(fields);
    struct_types.insert(std::make_pair(name, struct_type));
    return struct_type;
}

llvm::GlobalVariable *addGlobalValue(std::shared_ptr<llvm::Module> ir_module, std::string name, llvm::Type *type, llvm::Constant *initializer, int align)
{
    llvm::GlobalVariable *global = (llvm::GlobalVariable *)ir_module->getOrInsertGlobal(name, type);
    global->setInitializer(initializer);
    /// KH-note: "dso_local" == the variable is defined in this file, and any file can access it.
    global->setDSOLocal(true);
    global->setAlignment(llvm::MaybeAlign(align));
    global_values.insert(std::make_pair(name, global));
    return global;
}

llvm::GlobalVariable *addGlobalString(std::shared_ptr<llvm::Module> ir_module, std::string name, std::string value)
{
    llvm::GlobalVariable *global = (llvm::GlobalVariable *)ir_module->getOrInsertGlobal(name, llvm::ArrayType::get(llvm::Type::getInt8Ty(ir_module->getContext()), value.size() + 1));
    global->setInitializer(llvm::ConstantDataArray::getString(ir_module->getContext(), value, true));
    global->setDSOLocal(true);
    /// KH-note: this is only used to add a <nameless const value>
    global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    global->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
    global->setConstant(true);
    global->setAlignment(llvm::MaybeAlign(1));
    global_values.insert(std::make_pair(name, global));
    return global;
}

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
    llvm::IRBuilder<> ir_builder(ir_module->getContext());

    // add struct type
    llvm::Type *Edge_s_fields[] = {llvm::Type::getInt32Ty(ir_module->getContext()), llvm::Type::getInt32Ty(ir_module->getContext()), llvm::Type::getInt64Ty(ir_module->getContext())};
    llvm::StructType *Edge_s = addStructType(ir_module, "struct.Edge_s", std::vector<llvm::Type *>(Edge_s_fields, Edge_s_fields + 3));

    // add global value
    llvm::PointerType::get(Edge_s, 0);
    llvm::GlobalVariable *edge1 = addGlobalValue(ir_module, "edge1", Edge_s, llvm::ConstantStruct::get(Edge_s, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ir_module->getContext()), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(ir_module->getContext()), 0), llvm::ConstantInt::get(llvm::Type::getInt64Ty(ir_module->getContext()), 5)), 8);
    llvm::GlobalVariable *edge2 = addGlobalValue(ir_module, "edge2", Edge_s, llvm::ConstantStruct::get(Edge_s, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ir_module->getContext()), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(ir_module->getContext()), 0), llvm::ConstantInt::get(llvm::Type::getInt64Ty(ir_module->getContext()), 10)), 8);
    addGlobalValue(ir_module, "allDist", llvm::ArrayType::get(llvm::ArrayType::get(llvm::Type::getInt32Ty(ir_module->getContext()), 3), 3), llvm::ConstantAggregateZero::get(llvm::ArrayType::get(llvm::ArrayType::get(llvm::Type::getInt32Ty(ir_module->getContext()), 3), 3)), 16);
    addGlobalValue(ir_module, "dist", llvm::ArrayType::get(llvm::PointerType::get(Edge_s, 0), 3), llvm::ConstantArray::get(llvm::ArrayType::get(llvm::PointerType::get(Edge_s, 0), 3), {llvm::ConstantExpr::getBitCast(edge1, llvm::PointerType::get(Edge_s, 0)), llvm::ConstantExpr::getBitCast(edge2, llvm::PointerType::get(Edge_s, 0)), llvm::ConstantPointerNull::get(llvm::PointerType::get(Edge_s, 0))}), 16);
    addGlobalValue(ir_module, "minDistance", llvm::Type::getInt64Ty(ir_module->getContext()), llvm::ConstantInt::get(llvm::Type::getInt64Ty(ir_module->getContext()), 5), 8);

    // add global string
    llvm::GlobalVariable *str = addGlobalString(ir_module, ".str", "%lld\00");
    llvm::GlobalVariable *str1 = addGlobalString(ir_module, ".str1", "%lld %lld %d\n\00");

    // add external function
    addFunction(ir_module, "__isoc99_scanf", llvm::FunctionType::get(llvm::Type::getInt32Ty(ir_module->getContext()), llvm::PointerType::get(llvm::Type::getInt8Ty(ir_module->getContext()), 0), true));
    addFunction(ir_module, "printf", llvm::FunctionType::get(llvm::Type::getInt32Ty(ir_module->getContext()), llvm::PointerType::get(llvm::Type::getInt8Ty(ir_module->getContext()), 0), true));
}

void buildCaculateDistance(std::shared_ptr<llvm::Module> ir_module)
{
    llvm::IRBuilder<> builder(ir_module->getContext());

    llvm::Function *caculateDistance = addFunction(ir_module, "caculateDistance", llvm::FunctionType::get(llvm::Type::getVoidTy(ir_module->getContext()), false));
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(ir_module->getContext(), "", caculateDistance);
    builder.SetInsertPoint(entry);

    // TODO
    /// KH-note: for simplicy
    auto &context = ir_module->getContext();
    const auto type_i32 = llvm::Type::getInt32Ty(context);
    const auto type_i64 = llvm::Type::getInt64Ty(context);
    const auto type_edge = struct_types.at("struct.Edge_s");
    const auto type_edge_ptr = llvm::PointerType::get(type_edge, 0);
    const auto type_arr_edge = llvm::ArrayType::get(type_edge_ptr, 3);
    auto constant_0 = llvm::ConstantInt::get(type_i32, 0);
    auto constant_1 = llvm::ConstantInt::get(type_i32, 1);
    auto constant_2 = llvm::ConstantInt::get(type_i32, 2);
    auto constant_NUM = llvm::ConstantInt::get(type_i32, 3);
    auto constant_0_64 = llvm::ConstantInt::get(type_i64, 0);
    auto global_dist = global_values.at("dist");
    auto global_minDistance = global_values.at("minDistance");

    /// KH-note: I'm lazy so I create all blocks together.
    auto block_loop_check = llvm::BasicBlock::Create(context, "loop_check", caculateDistance);
    auto block_loop_main = llvm::BasicBlock::Create(context, "loop_main", caculateDistance);
    auto block_cond_true = llvm::BasicBlock::Create(context, "cond_t", caculateDistance);
    auto block_cond_false = llvm::BasicBlock::Create(context, "cond_f", caculateDistance);
    auto block_val_upd = llvm::BasicBlock::Create(context, "val_upd", caculateDistance);
    auto block_loop_incre = llvm::BasicBlock::Create(context, "loop_incre", caculateDistance);
    auto block_final = llvm::BasicBlock::Create(context, "ret", caculateDistance);

    /// KH-note: This function has no "static link" (I wonder is that really a static link?)
    auto ptr_k = builder.CreateAlloca(type_i32, nullptr, "k");
    auto ptr_kDist = builder.CreateAlloca(type_i64, nullptr, "kDist");
    builder.CreateStore(constant_0, ptr_k);
    builder.CreateBr(block_loop_check);

    builder.SetInsertPoint(block_loop_check);
    auto k_val = builder.CreateLoad(type_i32, ptr_k, "k1");
    auto check_res = builder.CreateICmpSLT(k_val, constant_NUM, "check_res");
    builder.CreateCondBr(check_res, block_loop_main, block_final);

    builder.SetInsertPoint(block_loop_main);
    auto k_val2 = builder.CreateLoad(type_i32, ptr_k, "k2");
    /// KH-note: signed extend.
    auto k_val_as_subscript = builder.CreateSExt(k_val2, type_i64, "k_subscript");
    /// KH-note
    /// GEP == Get Element Pointer. It can be used in arrays and records.
    /// GEP doesn't write memory; it only calculates target's addr.
    /// Let's see how to get &(disk[k]) through &dist:
    /// - step1. access <dist> by <&dist>, offset <0>
    /// - step2. access <&(disk[k])> by <dist>, offset <k>
    /// That's why here we need a 0 as first input. BAM!!!
    auto kth_ptr_edge = builder.CreateInBoundsGEP(
        type_arr_edge, 
        global_dist, 
        {
            constant_0_64,
            k_val_as_subscript
        }, 
        "dist_k_addr"
    );
    auto kth_edge_addr = builder.CreateLoad(type_edge_ptr, kth_ptr_edge, "dist_k1");
    /// KH-note:
    /// Records (or structs if you don't like tiger) are also regarded as arrays,
    /// But llvm will handle different fields' size. We just give it the id (similar to tuple!).
    /// - step1. access <dist[k]> by <&(dist[k])>, offset <0>
    /// - step2. access <&(disk[k].w)> by <dist[k]>, offset <2>
    /// So we can see '->' costs 2 moves in IR. Small BAM!!
    auto kth_edge_ptr_w = builder.CreateInBoundsGEP(
        type_edge, 
        kth_edge_addr, 
        {
            constant_0,
            constant_2
        }, 
        "dist_k_w"
    );
    auto kth_edge_w_val = builder.CreateLoad(type_i64, kth_edge_ptr_w, "disk_k_w1");
    builder.CreateStore(kth_edge_w_val, ptr_kDist);
    auto comp_val_kDist = builder.CreateLoad(type_i64, ptr_kDist, "kDist1");
    auto comp_val_minDist = builder.CreateLoad(type_i64, global_minDistance, "minDist1");
    auto comp_res = builder.CreateICmpSLT(comp_val_kDist, comp_val_minDist, "comp_res");
    builder.CreateCondBr(comp_res, block_cond_true, block_cond_false);

    builder.SetInsertPoint(block_cond_true);
    auto choose_val_true = builder.CreateLoad(type_i64, ptr_kDist, "kDist2");
    builder.CreateBr(block_val_upd);

    builder.SetInsertPoint(block_cond_false);
    auto choose_val_false = builder.CreateLoad(type_i64, global_minDistance, "minDist2");
    builder.CreateBr(block_val_upd);

    builder.SetInsertPoint(block_val_upd);
    /// KH-note: PHI command is used to fetch values from different blocks.
    auto choose_val = builder.CreatePHI(type_i64, 2, "mind_phi");
    choose_val->addIncoming(choose_val_true, block_cond_true);
    choose_val->addIncoming(choose_val_false, block_cond_false);
    builder.CreateStore(choose_val, global_minDistance);
    builder.CreateBr(block_loop_incre);

    builder.SetInsertPoint(block_loop_incre);
    auto k_val3 = builder.CreateLoad(type_i32, ptr_k, "k3");
    auto updated_k_val = builder.CreateNSWAdd(k_val3, constant_1, "k_new");
    builder.CreateStore(updated_k_val, ptr_k);
    builder.CreateBr(block_loop_check);

    builder.SetInsertPoint(block_final);
    builder.CreateRetVoid();
}

void buildMain(std::shared_ptr<llvm::Module> ir_module)
{
    llvm::IRBuilder<> builder(ir_module->getContext());

    llvm::Function *main = addFunction(ir_module, "main", llvm::FunctionType::get(llvm::Type::getInt32Ty(ir_module->getContext()), false));
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(ir_module->getContext(), "", main);
    builder.SetInsertPoint(entry);
    
    // TODO
    /// KH-note: for simplicy
    auto &context = ir_module->getContext();
    const auto type_char = llvm::Type::getInt8Ty(context);
    const auto type_char_ptr = llvm::Type::getInt8PtrTy(context);
    const auto type_i32 = llvm::Type::getInt32Ty(context);
    const auto type_i64 = llvm::Type::getInt64Ty(context);
    const auto type_edge = struct_types.at("struct.Edge_s");
    const auto type_arr_edge = llvm::ArrayType::get(llvm::PointerType::get(type_edge, 0), 3);
    const auto type_str_in = llvm::ArrayType::get(type_char, 5);
    const auto type_str_out = llvm::ArrayType::get(type_char, 13);
    auto constant_0 = llvm::ConstantInt::get(type_i32, 0);
    auto constant_1 = llvm::ConstantInt::get(type_i32, 1);
    auto constant_2 = llvm::ConstantInt::get(type_i32, 2);
    auto constant_0_64 = llvm::ConstantInt::get(type_i64, 0);
    auto constant_2_64 = llvm::ConstantInt::get(type_i64, 2);
    auto constant_5_64 = llvm::ConstantInt::get(type_i64, 5);
    auto constant_10_64 = llvm::ConstantInt::get(type_i64, 10);
    auto constant_format_in = global_values.at(".str");
    auto constant_format_out = global_values.at(".str1");
    auto global_dist = global_values.at("dist");
    auto global_minDistance = global_values.at("minDistance");
    auto global_allDist = global_values.at("allDist");
    /// KH-note:
    /// This will lead to Segfault:
    /// - const auto args_list_format = llvm::ArrayRef<llvm::Type *>{ type_char_ptr };
    /// That's because ArrayRef "does not own the underlying data, it is expected to be 
    ///   used in situations where the data resides in some other buffer, whose lifetime 
    ///   extends past that of the ArrayRef"
    /// So here's the safe idea:
    std::vector<llvm::Type *> args_list_format{ type_char_ptr };
    /// KH-note: When isVarArg is true, function will accept (args...) as parameter.
    auto args_formatStr = llvm::FunctionType::get(type_i32, args_list_format, true);
    /// KH-note:
    /// Here, getOrInsertFunction will be same as getFunction.
    /// Imported functions will be auto-declared.
    auto func_scanf = ir_module->getOrInsertFunction("__isoc99_scanf", args_formatStr);
    auto func_printf = ir_module->getOrInsertFunction("printf", args_formatStr);

    auto static_link = builder.CreateAlloca(type_i32, nullptr, "sl");
    auto ptr_edge = builder.CreateAlloca(type_edge, nullptr, "edge");
    builder.CreateStore(constant_0, static_link);
    auto edge_ptr_w = builder.CreateInBoundsGEP(
        type_edge, 
        ptr_edge, 
        { 
            constant_0, 
            constant_2 
        }, 
        "edge_w_ptr"
    );
    /// KH-note: nested operations will also be nested in .ll
    auto scanf_res = builder.CreateCall(
        func_scanf, 
        {
            builder.CreateInBoundsGEP(
                type_str_in, 
                constant_format_in, 
                {
                    constant_0_64,
                    constant_0_64
                }
            ),
            edge_ptr_w
        }, 
        "scanf_res"
    );
    builder.CreateStore(
        ptr_edge, 
        builder.CreateInBoundsGEP(
            type_arr_edge,
            global_dist, 
            {
                constant_0_64,
                constant_2_64
            }
        )
    );
    auto edge_ptr_w2 = builder.CreateInBoundsGEP(
        type_edge, 
        ptr_edge, 
        {
            constant_0,
            constant_2
        }, 
        "edge_w2"
    );
    auto edge_w_val_64 = builder.CreateLoad(type_i64, edge_ptr_w2, "edge_w_64val");
    auto edge_w_val_32 = builder.CreateTrunc(edge_w_val_64, type_i32, "edge_w_32val");
    builder.CreateStore(
        edge_w_val_32, 
        builder.CreateInBoundsGEP(
            global_allDist->getValueType(), 
            global_allDist, 
            {
                constant_0_64,
                constant_0_64,
                constant_0_64
            }
        )
    );
    builder.CreateCall(ir_module->getFunction("caculateDistance"));
    auto minDistance_prtval = builder.CreateLoad(type_i64, global_minDistance, "minDist1");
    auto edge_ptr_w3 = builder.CreateInBoundsGEP(
        type_edge, 
        ptr_edge, 
        {
            constant_0,
            constant_2
        }, 
        "edge_w3"
    );
    auto edge_w_prtval1 = builder.CreateLoad(type_i64, edge_ptr_w3, "edge_w_64val2");
    auto edge_w_prtval2 = builder.CreateNSWAdd(edge_w_prtval1, constant_5_64, "edge_prt1");
    auto edge_w_prtval3 = builder.CreateNSWAdd(edge_w_prtval2, constant_10_64, "edge_prt2");
    auto allDist_prtval = builder.CreateLoad(
        type_i32,
        builder.CreateInBoundsGEP(
            global_allDist->getValueType(), 
            global_allDist, 
            {
                constant_0_64,
                constant_0_64,
                constant_0_64
            }
        ),
        "allDist_p"
    );
    auto printf_res = builder.CreateCall(
        func_printf, 
        {
            builder.CreateInBoundsGEP(
                constant_format_out->getValueType(), 
                constant_format_out, 
                {
                    constant_0_64,
                    constant_0_64
                }
            ),
            minDistance_prtval,
            edge_w_prtval3,
            allDist_prtval
        }, 
        "printf_res"
    );
    builder.CreateRet(constant_0);
}

void buildFunction(std::shared_ptr<llvm::Module> ir_module)
{
    buildCaculateDistance(ir_module);
    buildMain(ir_module);
}

int main(int, char **)
{
    llvm::LLVMContext context;
    std::shared_ptr<llvm::Module> ir_module = std::make_shared<llvm::Module>("calculateDistance", context);
    ir_module->setTargetTriple("x86_64-pc-linux-gnu");

    buildGlobal(ir_module);
    buildFunction(ir_module);

    ir_module->print(llvm::outs(), nullptr);

    return 0;
}

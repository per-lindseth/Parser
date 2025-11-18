#pragma once

// --- LLVM IR Backend (CodeGen) ---

#include <memory>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

class LLVMCodeGen {
public:
    LLVMCodeGen()
        : context(), builder(context), module(std::make_unique<llvm::Module>("sprout", context)) {
    }

    // Generate IR for the entire program (list of statements)
    void generate(const std::vector<Stmt*>& stmts) {
        // Declare printf: int printf(const char*, ...)
        llvm::FunctionType* printfTy = llvm::FunctionType::get(
            builder.getInt32Ty(), { builder.getInt8PtrTy() }, true);
        llvm::Function::Create(printfTy, llvm::Function::ExternalLinkage, "printf", module.get());

        // Create main function: int main()
        llvm::FunctionType* mainTy = llvm::FunctionType::get(builder.getInt32Ty(), false);
        llvm::Function* mainFn = llvm::Function::Create(mainTy, llvm::Function::ExternalLinkage, "main", module.get());
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", mainFn);
        builder.SetInsertPoint(entry);

        // Create a simple map of variable name -> alloca
        for (Stmt* s : stmts) {
            if (auto* ls = dynamic_cast<LetStmt*>(s)) {
                // allocate an i32 on the stack
                llvm::AllocaInst* alloc = builder.CreateAlloca(builder.getInt32Ty(), nullptr, ls->name);
                symbolTable[ls->name] = alloc;
                llvm::Value* val = codegenExpr(ls->expr);
                builder.CreateStore(val, alloc);
            }
            else if (auto* ps = dynamic_cast<PrintStmt*>(s)) {
                llvm::Value* v = codegenExpr(ps->expr);
                // create format string global
                llvm::Constant* formatConst = llvm::ConstantDataArray::getString(context, "%d
                    ");
                    llvm::GlobalVariable * formatStr = new llvm::GlobalVariable(*module, formatConst->getType(), true,
                        llvm::GlobalValue::PrivateLinkage, formatConst, "fmt");
                llvm::Value * zero = llvm::ConstantInt::get(builder.getInt32Ty(), 0);
                llvm::Value * indices[] = { zero, zero };
                llvm::Value * fmtPtr = builder.CreateInBoundsGEP(formatStr->getValueType(), formatStr, indices);
                builder.CreateCall(module->getFunction("printf"), { fmtPtr, v });
            }
            else {
                // expression statement: evaluate and drop
                if (auto* es = dynamic_cast<ExprStmt*>(s)) {
                    codegenExpr(es->expr);
                }
            }
        }

        // return 0
        builder.CreateRet(llvm::ConstantInt::get(builder.getInt32Ty(), 0));

        // Validate the generated code, and print it out
        if (llvm::verifyModule(*module, &llvm::errs())) {
            throw std::runtime_error("Generated LLVM IR is invalid");
        }

        module->print(llvm::outs(), nullptr);
    }

    llvm::Value* codegenExpr(Expr* e) {
        if (auto* n = dynamic_cast<NumberExpr*>(e)) {
            return llvm::ConstantInt::get(builder.getInt32Ty(), n->value);
        }
        if (auto* v = dynamic_cast<VariableExpr*>(e)) {
            auto it = symbolTable.find(v->name);
            if (it == symbolTable.end()) throw std::runtime_error("Unknown variable: " + v->name);
            return builder.CreateLoad(builder.getInt32Ty(), it->second);
        }
        if (auto* b = dynamic_cast<BinaryExpr*>(e)) {
            llvm::Value* l = codegenExpr(b->left);
            llvm::Value* r = codegenExpr(b->right);
            switch (b->op) {
            case '+': return builder.CreateAdd(l, r);
            case '-': return builder.CreateSub(l, r);
            case '*': return builder.CreateMul(l, r);
            case '/': return builder.CreateSDiv(l, r);
            default: throw std::runtime_error("Unsupported binary op");
            }
        }
        throw std::runtime_error("Unknown expr in codegen");
    }

private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::unordered_map<std::string, llvm::AllocaInst*> symbolTable;
};

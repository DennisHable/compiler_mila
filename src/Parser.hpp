#ifndef PJPPROJECT_PARSER_HPP
#define PJPPROJECT_PARSER_HPP

#include <memory>
#include <memory>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include "Lexer.hpp"
#include <Ast_nodes.cpp>

#endif //PJPPROJECT_PARSER_HPP
class Parser {
public:
    Parser(FILE *file);

    std::unique_ptr<ExprAST> ParamNext(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> Param(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> OptDeclVar(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> Type(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> Body(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> DeclFunc(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> RmDeclConst2(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> RmDeclConst(std::unique_ptr<ExprAST> lhs);

    void err(const std::string &msg);

    std::unique_ptr<ExprAST> DeclConst(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> RmDeclVar(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> NextVarDecl(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> DeclVar(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> Decl(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> Factor(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> RmTerm(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> Term(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> RmExpr(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> Expr(std::unique_ptr<ExprAST> lhs);

    std::string RelOp(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> NextCond(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> Cond(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> PtElse(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> RemExprList(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> ExprList(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> ForNext(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> Cmd(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> RmCmd(std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> CompCmd(std::unique_ptr<ExprAST> lhs);

    bool Parse();                    // parse
    const llvm::Module& Generate();  // generate

    ~Parser() = default;

    struct SymbolInfo {
        llvm::Value* val;
        bool isConst;
    };

private:
    int getNextToken();

    Lexer m_Lexer;                   // lexer is used to read tokens
    int CurTok;                      // to keep the current token

    std::unique_ptr<ExprAST> program;

    std::map<std::string, llvm::Function*> functionTable;
    std::vector<std::map<std::string, llvm::Value*>> symbolTable1;
    std::vector<llvm::BasicBlock*> breakBlocks;
    std::map<std::string, llvm::Value*> arrayStartMap;

    llvm::LLVMContext MilaContext;   // llvm context
    llvm::IRBuilder<> MilaBuilder;   // llvm builder
    llvm::Module MilaModule;         // llvm module
};

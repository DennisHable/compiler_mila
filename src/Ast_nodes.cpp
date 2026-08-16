#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
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

class ExprAST {
public:
    virtual ~ExprAST() = default;

    virtual llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                                 string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                                 ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                                 arrayStartMap) = 0;
};

enum VarType {
    INTEGER,
    STRING,
    VOID
};

class NumberInt : public ExprAST {
    int value;

public:
    NumberInt(const int value) : value(value) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class IdentNode : public ExprAST {
    std::string name;
    public:
    const std::string& getName() {return name;}
    IdentNode(const std::string &name) : name(name) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class ArrayIndexNode : public ExprAST {
    std::string arrayName;
    VarType arrType;
    std::unique_ptr<ExprAST> index;
public:
    ArrayIndexNode(std::string arrayName,
                   VarType arrType,
                   std::unique_ptr<ExprAST> index): arrayName(std::move(arrayName)),
                                                    arrType(arrType),
                                                    index(std::move(index)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class ArrayAssignNode : public ExprAST {
    std::string arrayName;
    std::unique_ptr<ExprAST> index;
    std::unique_ptr<ExprAST> val;
public:
    ArrayAssignNode(std::string arrayName,
                   std::unique_ptr<ExprAST> index,
                   std::unique_ptr<ExprAST> val): arrayName(std::move(arrayName)),
                                                  index(std::move(index)),
                                                  val(std::move(val)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class Variable : public ExprAST {
    std::string name;
    //std::unique_ptr<ExprAST> type;
    std::unique_ptr<ExprAST> next;
public:
    const std::string& getName() {return name;}
    Variable(const std::string &name,
      //       std::unique_ptr<ExprAST> type,
             std::unique_ptr<ExprAST> next) : name(name),
        //                                      type(std::move(type)),
                                              next(std::move(next)) {}

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class BinaryExpr : public ExprAST {
    std::unique_ptr<ExprAST> left;
    std::unique_ptr<ExprAST> right;
    char operation;

public:
    BinaryExpr(std::unique_ptr<ExprAST> left,
               std::unique_ptr<ExprAST> right,
               char operation): left(std::move(left)),
                                right(std::move(right)),
                                operation(operation) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class UnaryExpr : public ExprAST {
    std::unique_ptr<ExprAST> operand;
    char operation;

public:
    UnaryExpr(std::unique_ptr<ExprAST> operand,
              char operation): operand(std::move(operand)),
                               operation(operation) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class If : public ExprAST {
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> thenPt;
    std::unique_ptr<ExprAST> elsePt;

public:
    If(std::unique_ptr<ExprAST> condition,
       std::unique_ptr<ExprAST> thenPt,
       std::unique_ptr<ExprAST> elsePt): condition(std::move(condition)),
                                         thenPt(std::move(thenPt)),
                                         elsePt(std::move(elsePt)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class MultCondNode : public ExprAST {
    std::unique_ptr<ExprAST> left;
    std::string op;
    std::unique_ptr<ExprAST> right;
public:
    MultCondNode(std::unique_ptr<ExprAST> left, std::string op, std::unique_ptr<ExprAST> right): left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class BreakNode : public ExprAST {
public:
    BreakNode() = default;
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class ExitNode : public ExprAST {
public:
    ExitNode() = default;
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};


class CondNode : public ExprAST {
    std::unique_ptr<ExprAST> exprLeft;
    std::string relOp;
    std::unique_ptr<ExprAST> exprRight;

public:
    CondNode(std::unique_ptr<ExprAST> exprLeft,
         std::string relOp,
         std::unique_ptr<ExprAST> exprRight): exprLeft(std::move(exprLeft)),
                                              relOp(std::move(relOp)),
                                              exprRight(std::move(exprRight)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class While : public ExprAST {
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> body;

public:
    While(std::unique_ptr<ExprAST> condition,
          std::unique_ptr<ExprAST> body): condition(std::move(condition)),
                                          body(std::move(body)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class For : public ExprAST {
    std::string varName;
    std::unique_ptr<ExprAST> varInitVal;
    std::unique_ptr<ExprAST> endExpr;
    bool isTo = true; // true = 'to' or false = 'downto'
    std::unique_ptr<ExprAST> body;

public:
    For(std::string varName,
        std::unique_ptr<ExprAST> varInitVal,
        std::unique_ptr<ExprAST> endExpr,
        bool isTo,
        std::unique_ptr<ExprAST> body): varName(std::move(varName)),
                                        varInitVal(std::move(varInitVal)),
                                        endExpr(std::move(endExpr)),
                                        isTo(isTo),
                                        body(std::move(body)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class CmdNode : public ExprAST {
    std::unique_ptr<ExprAST> cmd;
    std::unique_ptr<ExprAST> next;

public:
    CmdNode(std::unique_ptr<ExprAST> cmd,
            std::unique_ptr<ExprAST> next): cmd(std::move(cmd)),
                                            next(std::move(next)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class CompCmdNode : public ExprAST {
    std::unique_ptr<ExprAST> block;

public:
    CompCmdNode(std::unique_ptr<ExprAST> block): block(std::move(block)) {}

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};


class ParamNode : public ExprAST {
    std::string name;
    VarType varType;
    std::unique_ptr<ExprAST> value;
public:
    std::unique_ptr<ExprAST> next;

    ParamNode(std::string name,
              VarType varType,
              std::unique_ptr<ExprAST> value,
              std::unique_ptr<ExprAST> next): name(std::move(name)),
                                              varType(varType),
                                              value(std::move(value)),
                                              next(std::move(next)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;

    void getLLVMArgTypes(std::vector<llvm::Type *> &argTypes, llvm::LLVMContext &context);
};

class FunctionCall : public ExprAST {
    std::string name;
    std::unique_ptr<ExprAST> args;
public:
    FunctionCall(std::string name,
                 std::unique_ptr<ExprAST> args): name(std::move(name)),
                                                 args(std::move(args)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};


class StringNode : public ExprAST {
    std::string value;
public:
    StringNode(std::string value): value(std::move(value)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class Function : public ExprAST {
    std::string name;
    //  std::map<std::string, std::pair<VarType, std::unique_ptr<ExprAST>>> args;
    std::unique_ptr<ExprAST> args;
    std::unique_ptr<ExprAST> vars;
    VarType returnType; // procedura má returnType = VOID
    std::unique_ptr<ExprAST> body;
    bool isForward;

public:
    Function(std::string name,
                 std::unique_ptr<ExprAST> args,
                 std::unique_ptr<ExprAST> vars,
                 VarType returnType,
                 std::unique_ptr<ExprAST> body,
                 bool isForward): name(std::move(name)),
                                                 args(std::move(args)),
                                                 vars(std::move(vars)),
                                                 returnType(returnType),
                                                 body(std::move(body)),
                                                 isForward(isForward) {}

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class DeclVars : public ExprAST {
    std::unique_ptr<ExprAST> var;
    std::unique_ptr<ExprAST> next;
public:
    std::unique_ptr<ExprAST>& getVar() {return var;}
    std::unique_ptr<ExprAST>& getNext() {return next;}
    DeclVars(std::unique_ptr<ExprAST> var,
             std::unique_ptr<ExprAST> next): var(std::move(var)),
                                             next(std::move(next)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class ArrDeclNode : public ExprAST {
    std::string arrName;
    VarType arrType;
    std::unique_ptr<ExprAST> startIndex;
    std::unique_ptr<ExprAST> endIndex;
public:
    ArrDeclNode(std::string arrName,
                VarType arrType,
                std::unique_ptr<ExprAST> startIndex,
                std::unique_ptr<ExprAST> endIndex): arrName(std::move(arrName)),
                                                    arrType(arrType),
                                                    startIndex(std::move(startIndex)),
                                                    endIndex(std::move(endIndex)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class DeclConsts : public ExprAST {
    std::unique_ptr<ExprAST> constVal;
    std::unique_ptr<ExprAST> next;
public:
    DeclConsts(std::unique_ptr<ExprAST> constVal,
               std::unique_ptr<ExprAST> next): constVal(std::move(constVal)),
                                               next(std::move(next)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class DeclNode : public ExprAST {
    std::unique_ptr<ExprAST> decl;
    std::unique_ptr<ExprAST> next;
public:
    DeclNode(std::unique_ptr<ExprAST> decl,
               std::unique_ptr<ExprAST> next): decl(std::move(decl)),
                                               next(std::move(next)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};


class DeclFunctions : public ExprAST {
    std::unique_ptr<ExprAST> func;
    std::unique_ptr<ExprAST> next;
public:
    DeclFunctions(std::unique_ptr<ExprAST> func,
                  std::unique_ptr<ExprAST> next): func(std::move(func)),
                                                  next(std::move(next)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class Program : public ExprAST {
    std::string programName;
    std::unique_ptr<ExprAST> decl; // functions, vars, consts
    std::unique_ptr<ExprAST> body;

public:
    Program(const std::string &programName,
            std::unique_ptr<ExprAST> decl,
            std::unique_ptr<ExprAST> body): programName(programName),
                                            decl(std::move(decl)),
                                            body(std::move(body)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class Assign : public ExprAST {
    std::string variable;
    std::unique_ptr<ExprAST> value;

public:
    Assign(std::string variable,
           std::unique_ptr<ExprAST> value): variable(std::move(variable)),
                                            value(std::move(value)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class Operator : public ExprAST {
    std::unique_ptr<ExprAST> left;
    std::unique_ptr<ExprAST> op;
    std::unique_ptr<ExprAST> right;

public:
    Operator(std::unique_ptr<ExprAST> left,
             std::unique_ptr<ExprAST> op,
             std::unique_ptr<ExprAST> right): left(std::move(left)),
                                              op(std::move(op)),
                                              right(std::move(right)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class DecNode : public ExprAST {
    std::string var;
public:
    DecNode(std::string var): var(std::move(var)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class ExprListNode : public ExprAST {
public:
    std::unique_ptr<ExprAST> cmd;
    std::unique_ptr<ExprAST> next;

    ExprListNode(std::unique_ptr<ExprAST> cmd,
             std::unique_ptr<ExprAST> next): cmd(std::move(cmd)),
                                             next(std::move(next)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class RelOpNode : public ExprAST {
    std::string relOp;

public:
    RelOpNode(std::string relOp): relOp(std::move(relOp)) {
    }

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class DataTypeNode : public ExprAST {
    VarType type;
    std::unique_ptr<ExprAST> next;

public:
    DataTypeNode(VarType type,
                 std::unique_ptr<ExprAST> next): type(type), next(std::move(next)) {}

    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class DeclConstNode : public ExprAST {
    std::string name;
    std::unique_ptr<ExprAST> value;

public:
    DeclConstNode(std::string  name,
                  std::unique_ptr<ExprAST> value): name(std::move(name)),
                                                   value(std::move(value)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};

class WritelnNode : public ExprAST {
    std::unique_ptr<ExprAST> param;

public:
    WritelnNode(std::unique_ptr<ExprAST> param): param(std::move(param)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};



class ReadlnNode : public ExprAST {
    std::unique_ptr<ExprAST> param;

public:
    ReadlnNode(std::unique_ptr<ExprAST> param): param(std::move(param)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};


class WriteNode : public ExprAST {
    std::unique_ptr<ExprAST> param;

public:
    WriteNode(std::unique_ptr<ExprAST> param): param(std::move(param)) {}
    llvm::Value *codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule, std::vector<std::map<std::
                         string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std
                         ::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &
                         arrayStartMap) override;
};
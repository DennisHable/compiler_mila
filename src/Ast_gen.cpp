#include <Ast_nodes.cpp>
#include <iostream>
#include <vector>
#include <bits/ostream.tcc>

llvm::Value* findVar(const std::string& varName, std::vector<std::map<std::string, llvm::Value *>>& namedValues) {
    for (auto iter = namedValues.rbegin(); iter != namedValues.rend(); iter++) {
        if (iter->find(varName) != iter->end()) {
            return iter->find(varName)->second;
        }
    }

    return nullptr;
    //throw std::runtime_error("Could not find variable " + varName);
}

void insertData(const std::string& varName, llvm::Value *value, std::vector<std::map<std::string, llvm::Value *>>& namedValues) {
    for (auto iter = namedValues.rbegin(); iter != namedValues.rend(); iter++) {
        if (iter->find(varName) != iter->end()) {
            iter->at(varName) = value;
            return;
        }
    }

    throw std::runtime_error("Could not find variable(insert) " + varName);
}

llvm::Value *StringNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                 llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                 *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                 paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    return nullptr;
}


llvm::Value *NumberInt::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(MilaContext), value, true);
}

llvm::Value *Variable::codegen(llvm::LLVMContext &MilaContext,
                               llvm::IRBuilder<> &MilaBuilder,
                               llvm::Module &MilaModule,
                               std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                               Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                               , llvm::Value *> &arrayStartMap) {
    // std::cout << "var decl.\n";
    //if (!findVar(name, namedValues))
        //throw std::runtime_error("Variable '" + name + "' redeclared.");

    auto *alloca = MilaBuilder.CreateAlloca(llvm::Type::getInt32Ty(MilaContext), nullptr, name);
    namedValues.back()[name] = alloca;
    return alloca;
}

llvm::Value *BinaryExpr::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                 llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                 *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                 paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    llvm::Value *L = left->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    llvm::Value *R = right->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (!L || !R)
        return nullptr;

    switch (operation) {
        case '+':
            return MilaBuilder.CreateAdd(L, R, "add");
        case '-':
            return MilaBuilder.CreateSub(L, R, "sub");
        case '*':
            return MilaBuilder.CreateMul(L, R, "mult");
        case '/':
            return MilaBuilder.CreateSDiv(L, R, "div");
        case '%':
            return MilaBuilder.CreateSRem(L, R, "mod");
        default:
            throw std::invalid_argument("Invalid binary operator.");
    }
}

llvm::Value *UnaryExpr::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    return MilaBuilder.CreateNeg(operand->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap), "neg");
}

llvm::Value *If::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                         std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                         Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                         , llvm::Value *> &arrayStartMap) {
    auto fn = MilaBuilder.GetInsertBlock()->getParent();
    auto *bbThen = llvm::BasicBlock::Create(MilaContext, "ifthen", fn);
    llvm::BasicBlock *bbElse = nullptr;
    if (elsePt) bbElse = llvm::BasicBlock::Create(MilaContext, "ifelse", fn);
    auto *bbAfter = llvm::BasicBlock::Create(MilaContext, "ifafter", fn);

    auto *cond = condition->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (elsePt)
        MilaBuilder.CreateCondBr(cond, bbThen, bbElse);
    else
        MilaBuilder.CreateCondBr(cond, bbThen, bbAfter);

    MilaBuilder.SetInsertPoint(bbThen);
    thenPt->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (!bbThen->getTerminator()) {
        MilaBuilder.SetInsertPoint(bbThen);
        MilaBuilder.CreateBr(bbAfter); // jump
    }
    if (elsePt) {
        MilaBuilder.SetInsertPoint(bbElse);
        elsePt->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
        if (!bbElse->getTerminator()) // není return
            MilaBuilder.CreateBr(bbAfter); // jump
    }

    MilaBuilder.SetInsertPoint(bbAfter);
    return nullptr;
}

llvm::Value *CondNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                               std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                               Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                               , llvm::Value *> &arrayStartMap) {
    llvm::Value *left = exprLeft->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    llvm::Value *right = exprRight->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    if (relOp == "=")
        return MilaBuilder.CreateICmpEQ(left, right, "eq");
    if (relOp == "<>")
        return MilaBuilder.CreateICmpNE(left, right, "neq");
    if (relOp == "<")
        return MilaBuilder.CreateICmpSLT(left, right, "lt");
    if (relOp == ">")
        return MilaBuilder.CreateICmpSGT(left, right, "gt");
    if (relOp == "<=")
        return MilaBuilder.CreateICmpSLE(left, right, "le");
    if (relOp == ">=")
        return MilaBuilder.CreateICmpSGE(left, right, "ge");
    throw std::invalid_argument("Invalid relational operator.");
}

llvm::Value *While::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                            std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                            Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                            , llvm::Value *> &arrayStartMap) {
    // pro umístění návěstí cond, while těla, kódu po while
    auto func = MilaBuilder.GetInsertBlock()->getParent();

    llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(MilaContext, "while_cond", func);
    llvm::BasicBlock *BodyBB = llvm::BasicBlock::Create(MilaContext, "while_body", func);
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(MilaContext, "while_end", func);

    // jdi na podmínku
    MilaBuilder.CreateBr(CondBB);

    MilaBuilder.SetInsertPoint(CondBB);
    llvm::Value *CondVal = condition->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (!CondVal) return nullptr;

    // podle pravdivosti podmínky -> tělo nebo kód za while
    MilaBuilder.CreateCondBr(CondVal, BodyBB, AfterBB);

    // kam se bude skákat (kód po while), když by se někdy měl vykonat příkaz break
    breakBlocks.push_back(AfterBB);

    // tělo while cyklu
    MilaBuilder.SetInsertPoint(BodyBB);
    body->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    // po konci smyčky -> zpět na ověření podmínky
    MilaBuilder.CreateBr(CondBB);

    breakBlocks.pop_back();

    // kód po while
    MilaBuilder.SetInsertPoint(AfterBB);

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(MilaContext));
}

llvm::Value *For::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                          std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                          Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                          , llvm::Value *> &arrayStartMap) {
    /*
    * class For : public ExprAST {
        std::string varName;
        std::unique_ptr<ExprAST> varInitVal;
        std::unique_ptr<ExprAST> endExpr;
        bool isTo = true; // true = 'to' or false = 'downto'
        std::unique_ptr<ExprAST> body;
        ...
    */

    // načtení řídící proměnné
    auto varAlloca = findVar(varName, namedValues);

    // init
    llvm::Value *startVal = varInitVal->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    MilaBuilder.CreateStore(startVal, varAlloca);
    insertData(varName, varAlloca, namedValues);

    // poslední hodnota
    llvm::Value *endVal = endExpr->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(MilaContext, "for_cond", currentFunction);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(MilaContext, "for_body", currentFunction);
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(MilaContext, "for_after", currentFunction);

    // skok na podmínku
    MilaBuilder.CreateBr(condBB);
    MilaBuilder.SetInsertPoint(condBB);
    llvm::Value *curVal = MilaBuilder.CreateLoad(llvm::Type::getInt32Ty(MilaContext), varAlloca, varName);

    llvm::Value *cond = nullptr;
    if (isTo)
        cond = MilaBuilder.CreateICmpSLE(curVal, endVal, "for_cond1"); // i <= end
    else
        cond = MilaBuilder.CreateICmpSGE(curVal, endVal, "for_cond1"); // i >= end

    MilaBuilder.CreateCondBr(cond, bodyBB, afterBB);

    // tělo
    MilaBuilder.SetInsertPoint(bodyBB);
    breakBlocks.push_back(afterBB);

    body->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    // inkrementace nebo dekrementace podle to/downto
    llvm::Value *step = llvm::ConstantInt::get(llvm::Type::getInt32Ty(MilaContext), 1);
    curVal = MilaBuilder.CreateLoad(llvm::Type::getInt32Ty(MilaContext), varAlloca, varName);
    llvm::Value *nextVal = nullptr;

    if (isTo)
        nextVal = MilaBuilder.CreateAdd(curVal, step, "next_val");
    else
        nextVal = MilaBuilder.CreateSub(curVal, step, "next_val");

    MilaBuilder.CreateStore(nextVal, varAlloca);

    MilaBuilder.CreateBr(condBB);  // zpět do podmínky

    breakBlocks.pop_back();
    MilaBuilder.SetInsertPoint(afterBB);
    return nullptr;
}

llvm::Value *BreakNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                                std::vector<std::map<std::string, llvm::Value *>> &namedValues,
                                std::vector<llvm::BasicBlock *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *>
                                &functionTable, int paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    // není v žádném while nebo for => prázdný vektor
    if (breakBlocks.empty()) {
        throw std::invalid_argument("Error. 'break' out of loop.");
    }

    // skok za smyčku
    return MilaBuilder.CreateBr(breakBlocks.back());
}

llvm::Value *ExitNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                               std::vector<std::map<std::string, llvm::Value *>> &namedValues,
                               std::vector<llvm::BasicBlock *> &breakBlocks,
                               llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::
                               string, llvm::Value *> &arrayStartMap) {
    llvm::Type *retType = currentFunction->getReturnType();

    if (retType->isVoidTy()) {
        return MilaBuilder.CreateRetVoid();
    }

    std::string functionName = currentFunction->getName().str();
    llvm::Value* retVar = findVar(functionName, namedValues);
    if (!retVar)
        throw std::invalid_argument("WTF???");
    llvm::Value* retVal = MilaBuilder.CreateLoad(retType, retVar, "ret_val");
    return MilaBuilder.CreateRet(retVal);
}

llvm::Value *CmdNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                              std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                              Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                              , llvm::Value *> &arrayStartMap) {
    if (next) next->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (cmd) cmd->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    return nullptr;
}

llvm::Value *ParamNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    if (!currentFunction)
        return nullptr;

    if (next) next->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex + 1, arrayStartMap);

    auto &localScope = namedValues.back();    // poslední frame

    llvm::AllocaInst *alloca = MilaBuilder.CreateAlloca(
         llvm::Type::getInt32Ty(MilaContext), nullptr, name);

    llvm::Argument *arg = currentFunction->getArg(paramIndex);

    MilaBuilder.CreateStore(arg, alloca);

    localScope[name] = alloca;

    return alloca;
}

llvm::Value *MultCondNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                   llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                   *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                   paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {

    llvm::Value* L = left->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    llvm::Value* R = right->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    if (!L || !R) throw std::runtime_error("Err: How?");

    // Předpokládáme, že L a R jsou i1 (bool)
    if (op == "or")
        return MilaBuilder.CreateOr(L, R, "orcond");
    if (op == "and")
        return MilaBuilder.CreateAnd(L, R, "andcond");
    if (op == "xor")
        return MilaBuilder.CreateXor(L, R, "andcond");
    return nullptr;
}

llvm::Value *FunctionCall::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                   llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                   *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                   paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    auto fn = functionTable.find(name);
    if (fn == functionTable.end()) {
        throw std::runtime_error("Unknown function: " + name);
    }
    llvm::Function *calleeFunc = fn->second;

    std::vector<llvm::Value *> argsV;
    ExprListNode *argNode = dynamic_cast<ExprListNode *>(args.get());
    while (argNode) {
        llvm::Value *arg = argNode->cmd->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
        if (arg)
            argsV.push_back(arg);

        argNode = dynamic_cast<ExprListNode *>(argNode->next.get());
    }

    llvm::Value *call = MilaBuilder.CreateCall(calleeFunc, argsV);
    if (!calleeFunc->getReturnType()->isVoidTy()) {
        call->setName("calltmp");
    }
    return call;
}


void ParamNode::getLLVMArgTypes(std::vector<llvm::Type*> &argTypes, llvm::LLVMContext &context) {
    argTypes.push_back(llvm::Type::getInt32Ty(context));

    if (next)
        dynamic_cast<ParamNode*>(next.get())->getLLVMArgTypes(argTypes, context);

}

llvm::Type *VarTypeToLLVMType(VarType type, llvm::LLVMContext &context) {
    switch (type) {
        case VarType::VOID:
            return llvm::Type::getVoidTy(context);
        case VarType::INTEGER:
            return llvm::Type::getInt32Ty(context);
        default:
            return nullptr;
    }
}

llvm::Value *Function::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                               llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                               *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                               paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    std::string& functionName = name;

    // návratový typ
    llvm::Type *retType = VarTypeToLLVMType(returnType, MilaContext);

    // typy paramentrů funkce
    std::vector<llvm::Type*> argTypes;
    if (args)
        dynamic_cast<ParamNode*>(args.get())->getLLVMArgTypes(argTypes, MilaContext);

    llvm::FunctionType *funcType = llvm::FunctionType::get(retType, argTypes, false);

    auto it = functionTable.find(functionName);
    llvm::Function *function = nullptr;

    if (it != functionTable.end()) {
        function = it->second;
        if (!function->empty() && !isForward) {
            throw std::runtime_error("Function is already defined.");
        }
    } else {
        // vytvoření nové fce
        function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, MilaModule);
        functionTable[functionName] = function;
    }


    if (!isForward) {
        // vytvořit blok, nastavit na něj builder
        llvm::BasicBlock *entryBB = llvm::BasicBlock::Create(MilaContext, "entry", function);
        MilaBuilder.SetInsertPoint(entryBB);

        namedValues.emplace_back();

        if (args) {
            args->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, function, functionTable, paramIndex, arrayStartMap);
        }

        if (vars) {
            vars->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, function, functionTable, paramIndex, arrayStartMap);
        }


        llvm::Value *retVar = nullptr;


        if (returnType != VOID) {
            retVar = MilaBuilder.CreateAlloca(retType, nullptr, functionName);
            namedValues.back()[functionName] = retVar;
        }

        if (body) {
            body->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, function, functionTable, paramIndex, arrayStartMap);
        }

        // je to funkce -> načíst hodnotu z proměnné (jméno stejné jako jméno funkce) a vrátit
        if (returnType != VOID) {
            llvm::Value *retValue = MilaBuilder.CreateLoad(retType, retVar, "retval");
            MilaBuilder.CreateRet(retValue);
        } else {
            MilaBuilder.CreateRetVoid();
        }
        namedValues.pop_back();
    }

    return function;
}

llvm::Value *CompCmdNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                  llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                  *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                  paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {

    // "nový rámec" pro data => vnořený blok
    namedValues.emplace_back();

    block->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    namedValues.pop_back();
    return nullptr;
}

llvm::Value *DecNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                              llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                              *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                              paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    auto ptr = findVar(var, namedValues);

    if (llvm::dyn_cast<llvm::Constant>(ptr)) {
        std::cerr << "Err: Cannot change constant '" << var << std::endl;
        return nullptr;
    }
    llvm::Value *val = MilaBuilder.CreateLoad(llvm::Type::getInt32Ty(MilaContext), ptr, var + "tmp");

    llvm::Value *dec = MilaBuilder.CreateSub(val, llvm::ConstantInt::get(llvm::Type::getInt32Ty(MilaContext), 1),
                                             var + "tmpDec");

    MilaBuilder.CreateStore(dec, ptr);
    return dec;
}

llvm::Value *DeclVars::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                               std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                               Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                               , llvm::Value *> &arrayStartMap) {
    if (next) next->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (var) var->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    return nullptr;
}

llvm::Value *DeclConsts::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                 llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                 *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                 paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    if (next) next->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (constVal) constVal->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    return nullptr;
}

llvm::Value *DeclNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                               llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                               *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                               paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    if (next) next->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (decl) decl->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    return nullptr;
}

llvm::Value *DeclFunctions::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                    llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                    *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                    paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    llvm::IRBuilder<>::InsertPoint oldIP = MilaBuilder.saveIP();

    //llvm::BasicBlock *declBB = llvm::BasicBlock::Create(MilaContext, "decl_func_block", currentFunction);
    //MilaBuilder.SetInsertPoint(declBB);
    if (next) next->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (func) func->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    MilaBuilder.restoreIP(oldIP);
    return nullptr;
}

llvm::Value *Program::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                              std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                              Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                              , llvm::Value *> &arrayStartMap) {
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(MilaContext), false);
    llvm::Function *MainFunction = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", MilaModule);

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(MilaContext, "entry", MainFunction);
    MilaBuilder.SetInsertPoint(BB);
    namedValues.emplace_back();

    if (decl)
        decl->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, MainFunction, functionTable, paramIndex, arrayStartMap);

    MilaBuilder.SetInsertPoint(BB);

    if (body)
        body->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, MainFunction, functionTable, paramIndex, arrayStartMap);

    MilaBuilder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(MilaContext), 0));
    return nullptr;
}

llvm::Value *IdentNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    //std::cout << "var use.\n";
    auto it = findVar(name, namedValues);

    if (llvm::dyn_cast<llvm::Constant>(it)) {
        return it;
    }

    return MilaBuilder.CreateLoad(llvm::Type::getInt32Ty(MilaContext), it, name);
}

llvm::Value *Assign::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                             std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                             Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                             , llvm::Value *> &arrayStartMap) {
    /*if (namedValues.find(variable) != namedValues.end())
        throw std::runtime_error("Variable '" + variable + "' redeclared.");
*/

    //  std::cout << "var assign.\n";

    llvm::Value *val = value->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    llvm::Value* ptr = findVar(variable, namedValues);
    if (llvm::dyn_cast<llvm::Constant>(ptr)) {
        std::cerr << "Err: Cannot assign to constant '" << variable << std::endl;
        return nullptr;
    }
    return MilaBuilder.CreateStore(val, ptr);
}

llvm::Value *Operator::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder, llvm::Module &MilaModule,
                               std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock *> &breakBlocks, llvm::
                               Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int paramIndex, std::map<std::string
                               , llvm::Value *> &arrayStartMap) {
    return nullptr;
}

llvm::Value *RelOpNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    return nullptr;
}

llvm::Value *DataTypeNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                   llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                   *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                   paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    return next->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
}

llvm::Value *ExprListNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                   llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                   *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                   paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    if (next) next->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (cmd) cmd->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    return nullptr;
}

llvm::Value *DeclConstNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                    llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                    *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                    paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    llvm::Value *val = value->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    auto constantVal = llvm::dyn_cast<llvm::Constant>(val);
    if (!constantVal) {
        std::cerr << "Must be a compile-time constant." << std::endl;
        return nullptr;
    }

    namedValues.back()[name] = constantVal;
    return constantVal;
}

llvm::Value *WritelnNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                  llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                  *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                  paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    llvm::Value *val = param->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (!val) return nullptr;

    llvm::Function *writelnFunc = MilaModule.getFunction("writeln");
    if (!writelnFunc) {
        std::vector<llvm::Type *> Ints(1, llvm::Type::getInt32Ty(MilaContext));
        llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(MilaContext), Ints, false);
        writelnFunc = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "writeln", MilaModule);
    }

    return MilaBuilder.CreateCall(writelnFunc, {val}, "writelnCall");
}

llvm::Value *WriteNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    llvm::Value *val = param->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    if (!val) return nullptr;

    llvm::Function *writelnFunc = MilaModule.getFunction("write");
    if (!writelnFunc) {
        std::vector<llvm::Type *> Ints(1, llvm::Type::getInt32Ty(MilaContext));
        llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(MilaContext), Ints, false);
        writelnFunc = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "write", MilaModule);
    }

    return MilaBuilder.CreateCall(writelnFunc, {val}, "writeCall");
}

llvm::Value *ReadlnNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                 llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                 *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                 paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    //std::cout << "var readln.\n";
    IdentNode *ident = dynamic_cast<IdentNode *>(param.get());
    if (!ident) {
        std::cerr << "Only variables allowed in readln." << std::endl;
        return nullptr;
    }

    llvm::Value *ptr = findVar(ident->getName(), namedValues);

    llvm::Function *readlnFunc = MilaModule.getFunction("readln");
    if (!readlnFunc) {
        std::vector<llvm::Type *> args{llvm::PointerType::getUnqual(llvm::Type::getInt32Ty(MilaContext))};
        llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(MilaContext), args, false);
        readlnFunc = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "readln", MilaModule);
    }

    llvm::Value *call = MilaBuilder.CreateCall(readlnFunc, {ptr}, "readlnCall");

    return call;
}

llvm::Value *ArrayAssignNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                      llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                      *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                      paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    /*
    class ArrayAssignNode : public ExprAST {
        std::string arrayName;
        std::unique_ptr<ExprAST> index;
        std::unique_ptr<ExprAST> val;
    */

    llvm::Value *arrPtr = findVar(arrayName, namedValues);
    if (!arrPtr) {
        throw std::runtime_error("Unknown array");
    }

    // index - startIndex
    llvm::Value *indexVal = index->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    llvm::Value *startOffset = arrayStartMap[arrayName];
    llvm::Value *realIndex = MilaBuilder.CreateSub(indexVal, startOffset, "index");

    // adresa prvku
    llvm::Type *elementType = llvm::Type::getInt32Ty(MilaContext);
    llvm::Value *elemPtr = MilaBuilder.CreateGEP(elementType, arrPtr, realIndex, "elem_ptr");

    // hodnota co se tam bude ukládat
    llvm::Value *rhsVal = val->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    return MilaBuilder.CreateStore(rhsVal, elemPtr);
}

llvm::Value *ArrayIndexNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                     llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                     *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                     paramIndex, std::map<std::string, llvm::Value *> &arrayStartMap) {
    /*
    class ArrayIndexNode : public ExprAST {
        std::string arrayName;
        VarType arrType;
        std::unique_ptr<ExprAST> index;
    */

    llvm::Value *indexVal = index->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    llvm::Value *startOffset = arrayStartMap[arrayName]; // buď konstantní, nebo dohledaná z uložené mapy
    llvm::Value *realIndex = MilaBuilder.CreateSub(indexVal, startOffset);

    // přístup na prvek
    llvm::Value *ptr = findVar(arrayName, namedValues);
    llvm::Type *elementType = VarTypeToLLVMType(arrType, MilaContext);
    auto ptr2 = MilaBuilder.CreateGEP(elementType, ptr, realIndex, "arr_elem");
    return MilaBuilder.CreateLoad(elementType, ptr2, "load_elem");
}

llvm::Value *ArrDeclNode::codegen(llvm::LLVMContext &MilaContext, llvm::IRBuilder<> &MilaBuilder,
                                 llvm::Module &MilaModule, std::vector<std::map<std::string, llvm::Value *>> &namedValues, std::vector<llvm::BasicBlock
                                 *> &breakBlocks, llvm::Function *currentFunction, std::map<std::string, llvm::Function *> &functionTable, int
                                 paramIndex, std::map<std::string, llvm::Value*> &arrayStartMap) {
    /*
    class ArrDeclNode : public ExprAST {
        std::string arrName;
        VarType arrType;
        std::unique_ptr<ExprAST> startIndex;
        std::unique_ptr<ExprAST> endIndex;
    */

    // indexy
    llvm::Value *startVal = startIndex->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);
    llvm::Value *endVal = endIndex->codegen(MilaContext, MilaBuilder, MilaModule, namedValues, breakBlocks, currentFunction, functionTable, paramIndex, arrayStartMap);

    // velikost (statického) pole size = end - start + 1
    llvm::Value *diff = MilaBuilder.CreateSub(endVal, startVal, "arr_size_tmp");
    llvm::Value *size = MilaBuilder.CreateAdd(diff, llvm::ConstantInt::get(llvm::Type::getInt32Ty(MilaContext), 1), "arr_size");

    llvm::Type *elementType = VarTypeToLLVMType(arrType, MilaContext); // typ prvků v poli

    llvm::Value *rawArray = MilaBuilder.CreateAlloca(elementType, size, arrName);

    // pointer na pole a počáteční index
    namedValues.back()[arrName] = rawArray;
    arrayStartMap[arrName] = startVal;

    return rawArray;
}
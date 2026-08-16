#include "Parser.hpp"

void debug(std::string msg, int spaceCnt = 0) {
    for (int i = 0; i < spaceCnt; i++) std::cout << "-";
    std::cout << "+ " << msg << std::endl;
}

Parser::Parser(FILE *file): MilaContext(),
                            MilaBuilder(MilaContext),
                            MilaModule("mila", MilaContext),
                            m_Lexer(file) {
}

// Type -> integer
std::unique_ptr<ExprAST> Parser::Type(std::unique_ptr<ExprAST> lhs) {
    if (CurTok != tok_integer) err("Keyword 'integer' expected.");
    return std::make_unique<DataTypeNode>(INTEGER, std::move(lhs));
}

// ----- function -----

// 11) Body -> forward ;
// 12) Body -> CompCmd
std::unique_ptr<ExprAST> Parser::Body(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_begin:
            return CompCmd(nullptr);
        case tok_forward:
            getNextToken();
            if (CurTok != tok_semicolon) err("Expected: ';'.");
            return nullptr;
        default:
            err("Keyword 'begin' or 'forward' expected.");
    }
    return nullptr;
}

// 8) ParamNext -> ; Param
// 9) ParamNext -> ε
std::unique_ptr<ExprAST> Parser::ParamNext(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_semicolon:
            getNextToken();
            return Param(nullptr);
        case tok_brackets:
            if (m_Lexer.identifierStr() != ")") err("Expected ')'");
            return lhs;
        default:
            err("Expected ')' or ';'.");
            break;
    }
    return nullptr;
}

// 7) Param -> ident : Type ParamNext
std::unique_ptr<ExprAST> Parser::Param(std::unique_ptr<ExprAST> lhs) {
    if (CurTok == Token::tok_identifier) {
        std::string id1 = m_Lexer.identifierStr();
        getNextToken();
        if (CurTok == Token::tok_colon) {
            getNextToken();
            VarType tmp1 = INTEGER;
            getNextToken();
            std::unique_ptr<ExprAST> tmp2 = ParamNext(nullptr);
            return std::make_unique<ParamNode>(id1, tmp1, nullptr, std::move(tmp2));
        }
        err("Expected: ':' after '" + id1 + "' .");
    }
    return nullptr;
}

// 61) OptDeclVar ->
// 62) OptDeclVar -> DeclVar
std::unique_ptr<ExprAST> Parser::OptDeclVar(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_begin: // 61
        case tok_forward: // 61
            return lhs;
        case tok_var: // 62
            return DeclVar(nullptr);
    }
    return nullptr;
}


// 6)  DeclFunc -> function ident ( Param ) : Type ; OptDeclVar Body
// 70) DeclFunc -> procedure ident ( Param ) ; OptDeclVar Body
std::unique_ptr<ExprAST> Parser::DeclFunc(std::unique_ptr<ExprAST> lhs) {
    VarType tmp1 = VOID;
    if (CurTok == tok_function) {
        getNextToken();
        if (CurTok == tok_identifier) {
            std::string id1 = m_Lexer.identifierStr();
            getNextToken();
            if (CurTok == Token::tok_brackets && m_Lexer.identifierStr() == "(") {
                getNextToken();
                auto params = Param(nullptr);
                //std::cout << "CurTok: " << CurTok << std::endl;
                if (CurTok == Token::tok_brackets && m_Lexer.identifierStr() == ")") {
                    getNextToken();
                    if (CurTok == Token::tok_colon) {
                        getNextToken();
                        tmp1 = INTEGER; // TODO
                        getNextToken();
                        if (CurTok != Token::tok_semicolon) err("Expected: ';'.");
                        getNextToken();
                        auto vars = OptDeclVar(nullptr);
                        std::unique_ptr<ExprAST> tmp2 = Body(nullptr);
                        if (CurTok == tok_semicolon) getNextToken();
                        return std::make_unique<DeclFunctions>(std::make_unique<Function>(id1, std::move(params), std::move(vars), tmp1, std::move(tmp2), tmp2 == nullptr), std::move(lhs));
                    }
                    err("Expected: ':'");
                } else err("Expected: ')' #0");
            } else err("Expected: '(' #2");
        } else err("Identifier expected.");
    } else if (CurTok == tok_procedure) {
        getNextToken();
        if (CurTok == tok_identifier) {
            std::string id1 = m_Lexer.identifierStr();
            getNextToken();
            if (CurTok == Token::tok_brackets && m_Lexer.identifierStr() == "(") {
                getNextToken();
                auto params = Param(nullptr);
                if (CurTok == Token::tok_brackets && m_Lexer.identifierStr() == ")") {
                    getNextToken();
                    tmp1 = VOID;
                    if (CurTok != Token::tok_semicolon) err("Expected: ';'.");
                    getNextToken();
                    auto vars = OptDeclVar(nullptr);
                    std::unique_ptr<ExprAST> tmp2 = Body(nullptr);
                    if (CurTok == tok_semicolon) getNextToken();
                    return std::make_unique<DeclFunctions>(std::make_unique<Function>(id1, std::move(params), std::move(vars), tmp1, std::move(tmp2), tmp2 == nullptr), std::move(lhs));
                } err("Expected: ')' #1");
            } else err("Expected: '(' #0");
        } else err("Identifier expected.");
    } else err("Keyword 'function' expected.");
    return nullptr;
}

// ----- end -----


// 45) RmDeclConst2 -> ident = number RmDeclConst
// 46) RmDeclConst2 ->
std::unique_ptr<ExprAST> Parser::RmDeclConst2(std::unique_ptr<ExprAST> lhs) {
    if (CurTok == Token::tok_identifier) {
        std::string id1 = m_Lexer.identifierStr();
        getNextToken();
        if (CurTok == Token::tok_assign) {
            getNextToken();
            if (CurTok == Token::tok_number) {
                int num = m_Lexer.numVal();
                getNextToken();
                return RmDeclConst(std::make_unique<DeclConsts>(
                    std::make_unique<DeclConstNode>(id1, std::make_unique<NumberInt>(num)), std::move(lhs)));
            }
            err("Number expected.");
        } else err("Expected: '=' after '" + id1 + "'.");
    } else if (CurTok == tok_begin ||
               CurTok == tok_const ||
               CurTok == tok_function ||
               CurTok == tok_var) {
        return lhs;
    } else
        err("Identifier expected. 333");
    return nullptr;
}

// 14) RmDeclConst -> , ident = number RmDeclConst
// 15) RmDeclConst -> ; RmDeclConst2
std::unique_ptr<ExprAST> Parser::RmDeclConst(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_comma:
            getNextToken();
            if (CurTok == Token::tok_identifier) {
                std::string id1 = m_Lexer.identifierStr();
                getNextToken();
                if (CurTok == Token::tok_assign) {
                    getNextToken();
                    if (CurTok == Token::tok_number) {
                        int num = m_Lexer.numVal();
                        return RmDeclConst(std::make_unique<DeclConsts>(
                            std::make_unique<DeclConstNode>(id1, std::make_unique<NumberInt>(num)), std::move(lhs)));
                    }
                    err("Number expected.");
                } else err("Expected: '=' after '" + id1 + "'.");
            } else err("Identifier expected. 4");
            break;
        case tok_semicolon:
            getNextToken();
            return std::make_unique<DeclConsts>(RmDeclConst2(nullptr), std::move(lhs));
    }
    return nullptr;
}

void Parser::err(const std::string &msg) {
    throw std::invalid_argument(msg);
}

// 13) DeclConst -> const ident = number RmDeclConst
std::unique_ptr<ExprAST> Parser::DeclConst(std::unique_ptr<ExprAST> lhs) {
    if (CurTok == Token::tok_const) {
        getNextToken();
        if (CurTok == Token::tok_identifier) {
            std::string id1 = m_Lexer.identifierStr();
            getNextToken();
            if (CurTok == Token::tok_assign) {
                getNextToken();
                if (CurTok == Token::tok_number) {
                    auto val = std::make_unique<NumberInt>(m_Lexer.numVal());
                    getNextToken();
                    return RmDeclConst(std::make_unique<DeclConstNode>(id1, std::move(val)));
                }
                err("Number expected.");
            } else err("Operator '=' expected.");
        } else err("Identifier expected");
    } else err("Keyword 'const' expected");
    return nullptr;
}

// 17) RmDeclVar -> , ident RmDeclVar
// 18) RmDeclVar -> : Type
// 71) RmDeclVar -> : array [ Expr . . Expr ] of integer
std::unique_ptr<ExprAST> Parser::RmDeclVar(std::unique_ptr<ExprAST> lhs) {
    std::unique_ptr<ExprAST> res;
    switch (CurTok) {
        case tok_comma: // 17
            getNextToken();
            if (CurTok == Token::tok_identifier) {
                res = std::make_unique<Variable>(m_Lexer.identifierStr(), nullptr);
                getNextToken();
                return RmDeclVar(std::make_unique<DeclVars>(std::move(res), std::move(lhs)));
            }
            err("Identifier expected. 2222");
            break;
        case tok_colon: // 18 / 71
            getNextToken();
            if (CurTok == tok_array) {
                getNextToken();
               // auto tmp = dynamic_cast<Variable*>(dynamic_cast<DeclVars*>(lhs.get())->getVar().get());
               // auto rest = std::move(dynamic_cast<DeclVars*>(lhs.get())->getNext());
                if (CurTok != tok_brackets || m_Lexer.identifierStr() != "[") err("Expected: '['.");
                getNextToken();
                auto tmp1 = Expr(nullptr);
                if (CurTok != tok_dot) err("Expected: '.'.");
                getNextToken();
                if (CurTok != tok_dot) err("Expected: '.'.");
                getNextToken();
                auto tmp2 = Expr(nullptr);
                if (CurTok != tok_brackets || m_Lexer.identifierStr() != "]") err("Expected: ']'.");
                getNextToken();
                if (CurTok != tok_of) err("Expected keyword: 'of'.");
                getNextToken();
                if (CurTok != tok_integer) err("Expected keyword: 'integer'.");
           //     return std::make_unique<DeclVars>(std::make_unique<ArrDeclNode>(tmp->getName(), INTEGER, std::move(tmp1), std::move(tmp2)), std::move(rest));
            }
            return Type(std::move(lhs));
        default:
            err("Expected: ',' or ';'.");
    }
    return nullptr;
}

// 50) NextVarDecl -> ident RmDeclVar ; NextVarDecl
// 51) NextVarDecl ->
std::unique_ptr<ExprAST> Parser::NextVarDecl(std::unique_ptr<ExprAST> lhs) {
    std::unique_ptr<ExprAST> tmp1;
    std::unique_ptr<ExprAST> tmp2;
    switch (CurTok) {
        case tok_identifier: // 50
            tmp1 = std::make_unique<Variable>(m_Lexer.identifierStr(), nullptr);
            getNextToken();
            tmp2 = RmDeclVar(std::make_unique<DeclVars>(std::move(tmp1), std::move(lhs)));
            getNextToken();
            if (CurTok != Token::tok_semicolon) {
                err("Semicolon expected.");
            }
            getNextToken();
            return NextVarDecl(std::move(tmp2));
        case tok_begin: // 51
        case tok_const: // 51
        case tok_function: // 51
        case tok_var: // 51
            return lhs;
        default:
            err("Expected: ',' or ';'.");
    }
    return nullptr;
}

// 17) RmDeclVar -> , ident RmDeclVar
// 18) RmDeclVar -> : Type
// 71) RmDeclVar -> : array [ Expr . . Expr ] of integer
// 50) NextVarDecl -> ident RmDeclVar ; NextVarDecl
// 51) NextVarDecl ->
// 16) DeclVar -> var ident RmDeclVar ; NextVarDecl

// DeclVar =*> var ident , ident , ident : Type ; ident : array [ Expr . . Expr ] of integer ;
std::unique_ptr<ExprAST> Parser::DeclVar(std::unique_ptr<ExprAST> lhs) {
    std::unique_ptr<ExprAST> res;
    if (CurTok != tok_var) err("Expected keyword: 'var'.");
    do {
        std::vector<std::string> varNames;
        getNextToken();
        if (CurTok == Token::tok_identifier) {
            varNames.push_back(m_Lexer.identifierStr());
            getNextToken();
            while(CurTok == Token::tok_comma) {
                getNextToken();
                if (CurTok != Token::tok_identifier) err("Identifier expected.");
                varNames.push_back(m_Lexer.identifierStr());
                getNextToken();
            }
            if (CurTok == Token::tok_colon) {
                getNextToken();
                if (CurTok == tok_integer) {
                    getNextToken();
                    if (!res) res = std::make_unique<Variable>(varNames[0], nullptr);
                    else res = std::make_unique<DeclVars>(std::make_unique<Variable>(varNames[0], nullptr), std::move(res));
                    for (size_t i = 1; i < varNames.size(); i++) {
                        res = std::make_unique<DeclVars>(std::make_unique<Variable>(varNames[i], nullptr), std::move(res));
                    }
                } else if (CurTok == Token::tok_array) {
                    getNextToken();
                    if (CurTok != tok_brackets || m_Lexer.identifierStr() != "[") err("Expected: '['.");
                    getNextToken();
                    auto start = Expr(nullptr);
                    if (CurTok != tok_dot) err("Expected: '.'.");
                    getNextToken();
                    if (CurTok != tok_dot) err("Expected: '.'.");
                    getNextToken();
                    auto end = Expr(nullptr);
                    if (CurTok != tok_brackets || m_Lexer.identifierStr() != "]") err("Expected: ']'.");
                    getNextToken();
                    if (CurTok != tok_of) err("Expected keyword: 'of'.");
                    getNextToken();
                    if (CurTok != tok_integer) err("Expected keyword: 'integer'.");
                    getNextToken();
                    if (!res) res = std::make_unique<ArrDeclNode>(varNames[0], INTEGER, std::move(start), std::move(end));
                    else res = std::make_unique<DeclVars>(std::make_unique<ArrDeclNode>(varNames[0], INTEGER, std::move(start), std::move(end)), std::move(res));
                    for (size_t i = 1; i < varNames.size(); i++) {
                        res = std::make_unique<DeclVars>(std::make_unique<ArrDeclNode>(varNames[i], INTEGER, std::move(start), std::move(end)), std::move(res));
                    }
                } else err("Expected keyword: 'array' or 'integer'.");
            }  else err("Expected: ':'.");
        }
    } while (CurTok == Token::tok_semicolon);
    return res;

    /*
    if (CurTok == Token::tok_var) {
        getNextToken();
        if (CurTok == Token::tok_identifier) {
            std::string id1 = m_Lexer.identifierStr();
            getNextToken();
            auto res = RmDeclVar(std::make_unique<Variable>(id1, nullptr));
            getNextToken();
            if (CurTok != Token::tok_semicolon) err("Missing semicolon.");
            getNextToken();
            return NextVarDecl(std::move(res));
        }
        err("Identifier expected.");
    } else err("Keyword 'var' expected.");
    return nullptr;*/
}

// 2) Decl -> DeclFunc Decl
// 3) Decl -> DeclConst Decl
// 4) Decl -> DeclVar Decl
// 5) Decl -> ε
std::unique_ptr<ExprAST> Parser::Decl(std::unique_ptr<ExprAST> lhs) {
    std::unique_ptr<ExprAST> tmp1;
    std::unique_ptr<ExprAST> tmp2;
    switch (CurTok) {
        case tok_begin: // 5
            return lhs;
        case tok_const: // 3
            tmp1 = DeclConst(nullptr);
            tmp2 = Decl(nullptr);
            return std::make_unique<DeclNode>(std::move(tmp2), std::move(tmp1));
        case tok_function: // 2
        case tok_procedure: // 2
            tmp1 = DeclFunc(nullptr);
            tmp2 = Decl(nullptr);
            return std::make_unique<DeclNode>(std::move(tmp2), std::move(tmp1));
        case tok_var: // 4
            tmp1 = DeclVar(nullptr);
            tmp2 = Decl(nullptr);
            return std::make_unique<DeclNode>(std::move(tmp2), std::move(tmp1));
        default:
            err("Expected: Declaration of functions, consts, variables or main body.");
    }
    return nullptr;
}

// 68) Factor -> ' ident '
// 49) Factor -> ident
// 50) Factor -> number
// 51) Factor -> ( Expr )
// 67) Factor -> ident ( ExprList )
// 72) Factor -> ident [ Expr ]
std::unique_ptr<ExprAST> Parser::Factor(std::unique_ptr<ExprAST> lhs) {
    std::unique_ptr<ExprAST> res;
    std::string id1;
    switch (CurTok) {
        case tok_brackets: // 51
            if (m_Lexer.identifierStr() == "(") {
                // 51
                getNextToken();
                res = Expr(nullptr);
                if (CurTok != tok_brackets || m_Lexer.identifierStr() != ")") err("Expected ')'.");
                getNextToken();
                return std::move(res);
            }
            err("Expected: '(' or '[' #1");
            break;
        case tok_identifier: // 49 / 67 / 72
            id1 = m_Lexer.identifierStr();
            getNextToken();
            if (CurTok == tok_brackets) {
                if (m_Lexer.identifierStr() == "[") {
                    // 72
                    getNextToken();
                    res = Expr(nullptr);
                    if (CurTok != tok_brackets || m_Lexer.identifierStr() != "]") err("Expected ']'.");
                    getNextToken();
                    return std::make_unique<ArrayIndexNode>(id1, INTEGER, std::move(res));
                }
                if (m_Lexer.identifierStr() == "(") {
                    getNextToken();
                    res = std::make_unique<FunctionCall>(id1, ExprList(nullptr));
                    if (CurTok != tok_brackets || m_Lexer.identifierStr() != ")") err("Expected: ')' #9 -> " + m_Lexer.identifierStr());
                    getNextToken();
                    return res;
                }
            }
            return std::move(std::make_unique<IdentNode>(id1));
        case tok_number: // 50
            res = std::make_unique<NumberInt>(m_Lexer.numVal());
            getNextToken();
            return std::move(res);
        case tok_apostr:
            id1 = m_Lexer.identifierStr();
            getNextToken();
            return std::make_unique<StringNode>(id1);
        default:
            err("Expected: identifier, number or '(' or '>'<'.");
    }
    return nullptr;
}

// 46) RmTerm -> * Factor RmTerm
// 47) RmTerm -> div Factor RmTerm
// 53) RmTerm -> mod Factor RmTerm
// 48) RmTerm -> ε
std::unique_ptr<ExprAST> Parser::RmTerm(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_operator:
            if (m_Lexer.identifierStr() == "*") {
                // 46
                getNextToken();
                return RmTerm(std::make_unique<BinaryExpr>(std::move(lhs), Factor(nullptr), '*'));
            }
            if (m_Lexer.identifierStr() == "+" ||
                m_Lexer.identifierStr() == "-") {
                // 48
                return lhs;
            }
            err("Expected: '*' or '/' or '+' or '-'.");
            break;
        case tok_div: // 47
            getNextToken();
            return RmTerm(std::make_unique<BinaryExpr>(std::move(lhs), Factor(nullptr), '/'));
        case tok_mod:
            getNextToken();
            return RmTerm(std::make_unique<BinaryExpr>(std::move(lhs), Factor(nullptr), '%'));

        // 48
        case tok_brackets:
            if (m_Lexer.identifierStr() != ")" &&
                m_Lexer.identifierStr() != "]") err("Expected: ')' or ']' #2");
        case tok_semicolon:
        case tok_assign:
        case tok_less:
        case tok_greater:
        case tok_lessequal:
        case tok_greaterequal:
        case tok_notequal:
        case tok_do:
        case tok_dot:
        case tok_downto:
        case tok_else:
        case tok_end:
        case tok_then:
        case tok_to:
        case tok_and:
        case tok_xor:
        case tok_or:
        case tok_comma:
            return lhs;
        default: err("Expected: operator or keyword. #1");
    }
    return nullptr;
}

// 45) Term -> Factor RmTerm
std::unique_ptr<ExprAST> Parser::Term(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_brackets:
            if (m_Lexer.identifierStr() == "(" ||
                m_Lexer.identifierStr() == "[") {
                // 45
                return RmTerm(Factor(nullptr));
            }
            err("Expected: '(' #2");
            break;
        case tok_identifier: // 45
        case tok_number: // 45
        case tok_apostr:
            return RmTerm(Factor(nullptr));
        default:
            err("Expected: identifier, number or '('.");
    }
    return nullptr;
}

// 42) RmExpr -> + Term RmExpr
// 43) RmExpr -> - Term RmExpr
// 44) RmExpr -> ε
std::unique_ptr<ExprAST> Parser::RmExpr(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_operator:
            if (m_Lexer.identifierStr() == "+") {
                // 42
                getNextToken();
                return RmExpr(std::make_unique<BinaryExpr>(std::move(lhs), Term(nullptr), '+'));
            }
            if (m_Lexer.identifierStr() == "-") {
                // 43
                getNextToken();
                return RmExpr(std::make_unique<BinaryExpr>(std::move(lhs), Term(nullptr), '-'));
            }
            err("Expected: '+' or '-'.");
            break;

        // 44
        case tok_brackets:
            if (m_Lexer.identifierStr() != ")" &&
                m_Lexer.identifierStr() != "]") err("Expected: ')' or ']' #3");
        case tok_semicolon:
        case tok_assign:
        case tok_less:
        case tok_greater:
        case tok_lessequal:
        case tok_greaterequal:
        case tok_notequal:
        case tok_do:
        case tok_dot:
        case tok_downto:
        case tok_else:
        case tok_end:
        case tok_then:
        case tok_to:
        case tok_and:
        case tok_xor:
        case tok_or:
        case tok_comma:
            return lhs;
        default: err("Expected: operator or keyword. #2");
    }
    return nullptr;
}

// 40) Expr -> Term RmExpr
// 41) Expr -> - Term RmExpr
std::unique_ptr<ExprAST> Parser::Expr(std::unique_ptr<ExprAST> lhs) {
    std::string id1;
    switch (CurTok) {
        case tok_brackets:
            if (m_Lexer.identifierStr() == "(" ||
                m_Lexer.identifierStr() == "[") {
                // 40
                return RmExpr(Term(nullptr));
            }
            err("Expected: '(' #3");
            break;
        case tok_operator:
            if (m_Lexer.identifierStr() == "-") {
                // 41
                getNextToken();
                return RmExpr(std::move(std::make_unique<UnaryExpr>(Term(nullptr), '-')));
            }
            err("Expected: '-'");
            break;
        case tok_apostr:
        case tok_identifier: // 40
            return RmExpr(Term(nullptr));
        case tok_number: // 40
            return RmExpr(Term(nullptr));
        default:
            err("Expected: identifier, number, '(' or '-'");
    }
    return nullptr;
}

// 34) RelOp -> =
// 35) RelOp -> <>
// 36) RelOp -> <
// 37) RelOp -> >
// 38) RelOp -> <=
// 39) RelOp -> >=
std::string Parser::RelOp(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_assign:
            return "=";
        case tok_notequal:
            return "<>";
        case tok_less:
            return "<";
        case tok_greater:
            return ">";
        case tok_lessequal:
            return "<=";
        case tok_greaterequal:
            return ">=";
    }
    throw std::invalid_argument("Unknown operator.");
}

// 65) NextCond -> or Cond
// 66) NextCond -> xor Cond
// 67) NextCond -> and Cond
std::unique_ptr<ExprAST> Parser::NextCond(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_or:
            getNextToken();
            return std::make_unique<MultCondNode>(std::move(lhs),"or", Cond(nullptr));
        case tok_xor:
            getNextToken();
            return std::make_unique<MultCondNode>(std::move(lhs),"xor", Cond(nullptr));
        case tok_and:
            getNextToken();
            return std::make_unique<MultCondNode>(std::move(lhs),"and", Cond(nullptr));
    }
    return lhs;
}

// 33) Cond -> Expr RelOp Expr NextCond
// 55) Cond -> ( Expr RelOp Expr ) NextCond
std::unique_ptr<ExprAST> Parser::Cond(std::unique_ptr<ExprAST> lhs) {
    std::unique_ptr<ExprAST> tmp1;
    std::string tmp2;
    std::unique_ptr<ExprAST> tmp3;
    switch (CurTok) {
        case tok_brackets: // 55
            if (m_Lexer.identifierStr() != "(") err("Expected: '(' #5");
            getNextToken();
            tmp1 = Expr(nullptr);
            tmp2 = RelOp(nullptr);
            getNextToken();
            tmp3 = Expr(nullptr);
            if (m_Lexer.identifierStr() != ")") err("Expected: ')' #6");
            getNextToken();
            return NextCond(std::make_unique<CondNode>(std::move(tmp1), std::move(tmp2), std::move(tmp3)));
        case tok_operator:
            if (m_Lexer.identifierStr() != "-") err("Expected: '-'");
        case tok_identifier:
        case tok_number:
            tmp1 = Expr(nullptr);
            tmp2 = RelOp(nullptr);
            getNextToken();
            tmp3 = Expr(nullptr);
            return NextCond(std::make_unique<CondNode>(std::move(tmp1), std::move(tmp2), std::move(tmp3)));
        default:
            err("Expected: '(' or '-' or identifier or number.");
    }
    return nullptr;
}

// 31) PtElse -> else Cmd
// 32) PtElse -> ε
std::unique_ptr<ExprAST> Parser::PtElse(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_else: // 31
            return Cmd(nullptr);
        case tok_semicolon: // 32
        case tok_end: // 32
            return lhs;
        default:
            err("Expected 'else' or ';' or 'end'.");
    }
    return nullptr;
}

// 58) RemExprList -> , Expr RemExprList
// 59) RemExprList ->
std::unique_ptr<ExprAST> Parser::RemExprList(std::unique_ptr<ExprAST> lhs) {
    std::unique_ptr<ExprAST> tmp1;
    switch (CurTok) {
        case tok_comma:
            getNextToken();
            tmp1 = Expr(nullptr);
            return std::make_unique<ExprListNode>(std::move(tmp1), RemExprList(std::move(lhs)));
        case tok_brackets:
            if (m_Lexer.identifierStr() != ")") err("Expected: ')' #5");
            return std::move(lhs);
    }
    return std::move(lhs);
}

// 56) ExprList -> Expr RemExprList
// 57) ExprList ->
std::unique_ptr<ExprAST> Parser::ExprList(std::unique_ptr<ExprAST> lhs) {
    std::unique_ptr<ExprAST> tmp1;
    std::unique_ptr<ExprAST> tmp2;
    switch (CurTok) {
        case tok_identifier: // 56
        case tok_number: // 56
            tmp1 = Expr(nullptr);
            tmp2 = RemExprList(std::move(lhs));
            return std::make_unique<ExprListNode>(std::move(tmp1), std::move(tmp2));
        case tok_brackets: // 56 / 57
            if (m_Lexer.identifierStr() == ")") // 57
                return std::move(lhs);
            tmp1 = Expr(nullptr);
            tmp2 = RemExprList(std::move(lhs));
            return std::make_unique<ExprListNode>(std::move(tmp1), std::move(tmp2));
        case tok_operator: // 56
            if (m_Lexer.identifierStr() != "-") err("Expected: '-'");
            tmp1 = Expr(nullptr);
            tmp2 = RemExprList(std::move(lhs));
            return std::make_unique<ExprListNode>(std::move(tmp1), std::move(tmp2));
        default:
            err("Expected: identifier, number, bracket or operator.");
    }
    return nullptr;
}

// 73) Cmd -> ident [ Expr ] := Expr
// 22) Cmd -> ident := Expr
// 23) Cmd -> write Expr
// 24) Cmd -> if Cond then Cmd PtElse
// 25) Cmd -> while Cond do CompCmd
// 26) Cmd -> for ident := Expr to Expr do Cmd
// 27) Cmd -> CompCmd
// 28) Cmd -> ε
// 29) Cmd -> for ident := Expr downto Expr do Cmd
// 30) Cmd -> writeln Expr
// 49) Cmd -> readln Expr
// 60) Cmd -> ident ( ExprList )
// 61) Cmd -> dec Expr
// 65) Cmd -> break
// 66) Cmd -> exit
std::unique_ptr<ExprAST> Parser::Cmd(std::unique_ptr<ExprAST> lhs) {
    getNextToken();
    std::unique_ptr<ExprAST> tmp;
    std::unique_ptr<ExprAST> tmp1;
    std::unique_ptr<ExprAST> tmp2;
    std::string id1;
    switch (CurTok) {
        case tok_identifier: // 22 / 23 / 30 / 49 / 60 / 61
            id1 = m_Lexer.identifierStr();
            if (id1 == "write") {
                // rule 23
                getNextToken();
                return std::make_unique<WriteNode>(Expr(nullptr));
            }

            if (id1 == "writeln") {
                // rule 30
                getNextToken();
                return std::make_unique<WritelnNode>(Expr(nullptr));
            }

            if (id1 == "readln") {
                // rule 49
                getNextToken();
                return std::make_unique<ReadlnNode>(Expr(nullptr));
            }

            if (id1 == "dec") {
                // rule 61
                getNextToken();
                if (CurTok == tok_brackets && m_Lexer.identifierStr() != "(") err("Expected: '(' #6");
                else getNextToken();
                std::string val = m_Lexer.identifierStr();
                getNextToken();
                if (CurTok == tok_brackets) {
                    if (m_Lexer.identifierStr() != ")")
                        err("Expected: ')' #7");
                    getNextToken();
                }
                if (CurTok != tok_semicolon) err("Expected: ';'");
                getNextToken();
                return std::make_unique<DecNode>(std::move(val));
            }

            getNextToken();
            if (CurTok == tok_brackets) {
                if (m_Lexer.identifierStr() == "[") {
                    getNextToken();
                    tmp1 = Expr(nullptr);
                    if (CurTok != tok_brackets || m_Lexer.identifierStr() != "]") err("Expected: ']'");
                    getNextToken();
                    if (CurTok != tok_colon) err("Expected: ':'");
                    getNextToken();
                    if (CurTok != tok_assign) err("Expected: assignment");
                    getNextToken();
                    tmp2 = Expr(nullptr);
                    return std::make_unique<ArrayAssignNode>(id1, std::move(tmp1), std::move(tmp2));
                }
                if (m_Lexer.identifierStr() != "(") err("Expected: '(' or '[' #5");
                getNextToken();
                tmp = std::make_unique<FunctionCall>(id1, ExprList(nullptr));
                if (CurTok != tok_brackets || m_Lexer.identifierStr() != ")") err("Expected: ')' #15 -> " + m_Lexer.identifierStr());
                getNextToken();
                return tmp;
            }

            // 22
            if (CurTok == tok_colon) {
                getNextToken();
                if (CurTok == tok_assign) {
                    getNextToken();
                    return std::make_unique<Assign>(id1, Expr(nullptr));
                }
                err("Expected: '=' after ':'.");
            } else err("Expected ':' after '" + id1 + "'.");

            break;
        case tok_if: // 24
            getNextToken();
            tmp = Cond(nullptr);
            if (CurTok == Token::tok_then) {
                tmp1 = Cmd(nullptr);
                tmp2 = PtElse(nullptr);
                return std::make_unique<If>(std::move(tmp),
                                            std::move(tmp1),
                                            std::move(tmp2));
            }
            err("Keyword 'then' expected.");
            break;
        case tok_while: // 25
            getNextToken();
            tmp = Cond(nullptr);
            if (CurTok == Token::tok_do) {
                getNextToken();
                return std::make_unique<While>(std::move(tmp), CompCmd(nullptr));
            }
            err("Keyword 'do' expected.");
            break;
        case tok_for: // 26 / 29
            // 26) Cmd -> for ident := Expr to Expr do CompCmd
            // 29) Cmd -> for ident := Expr downto Expr do CompCmd
            getNextToken();
            if (CurTok == Token::tok_identifier) {
                id1 = m_Lexer.identifierStr();
                getNextToken();
                if (CurTok == Token::tok_colon) {
                    getNextToken();
                    if (CurTok == Token::tok_assign) {
                        getNextToken();
                        tmp1 = Expr(nullptr);
                        if (CurTok == tok_to) {
                            getNextToken();
                            // 26
                            tmp2 = Expr(nullptr);
                            if (CurTok == tok_do) {
                                return std::make_unique<For>(id1, std::move(tmp1), std::move(tmp2), true,
                                                             Cmd(nullptr));
                            }
                            err("Keyword 'do' expected.");
                            break;
                        }
                        if (CurTok == Token::tok_downto) {
                            getNextToken();
                            tmp2 = Expr(nullptr);
                            if (CurTok == tok_do) {
                                return std::make_unique<For>(id1, std::move(tmp1), std::move(tmp2), false,
                                                             Cmd(nullptr));
                            }
                            err("Keyword 'do' expected.");
                            break;
                        }
                        err("Expected: 'to' or 'downto'.");
                    } else err("Expected: '=' after ':'.");
                } else err("Expected: ':' after '" + id1 + "'.");
            } else err("Identifier expected. 0");
            break;
        case tok_begin: // 27
            // 27) Cmd -> CompCmd
            //if (CurTok == tok_semicolon) getNextToken();
            return CompCmd(nullptr);
        case tok_semicolon: // 28
        case tok_else: // 28
        case tok_end: // 28
            return lhs;
        case tok_exit: // 66
            getNextToken();
            return std::make_unique<ExitNode>();
        case tok_break: // rule 65
            getNextToken();
            return std::make_unique<BreakNode>();
        default:
            err("Expected: identifier or 'if' or 'while' or 'for' or 'begin' or ';' or 'else' or 'end'.");
    }
    return nullptr;
}

// 20) RmCmd -> ; Cmd RmCmd
// 21) RmCmd -> ε
std::unique_ptr<ExprAST> Parser::RmCmd(std::unique_ptr<ExprAST> lhs) {
    switch (CurTok) {
        case tok_semicolon: // 20
            return std::make_unique<CmdNode>(RmCmd(Cmd(nullptr)), std::move(lhs));
        case tok_end: // 21
            return lhs;
        default:
            err("Semicolon or keyword 'end' expected.");
    }
    return nullptr;
}

// 19) CompCmd -> begin Cmd RmCmd end
std::unique_ptr<ExprAST> Parser::CompCmd(std::unique_ptr<ExprAST> lhs) {
    if (CurTok == tok_begin) {
        std::unique_ptr<ExprAST> ptr = std::make_unique<CompCmdNode>(RmCmd(Cmd(nullptr)));
        if (CurTok != tok_end) err("Keyword 'end' expected.");
        getNextToken();
        return ptr;
    }
    err("Keyword 'begin' expected.");
    return nullptr;
}

bool Parser::Parse() {
    // 1) Program -> program ident ; Decl CompCmd .
    getNextToken();
    if (CurTok == Token::tok_program) {
        getNextToken();
        if (CurTok == Token::tok_identifier) {
            std::string id1 = m_Lexer.identifierStr();
            getNextToken();
            if (CurTok == Token::tok_semicolon) {
                getNextToken();
                auto tmp1 = Decl(nullptr);
                auto tmp2 = CompCmd(nullptr);
                if (CurTok != tok_dot) err("Symbol '.' expected.");
                program = std::make_unique<Program>(id1, std::move(tmp1), std::move(tmp2));
            } else err("Missing semicolon.");
        } else err("File name expected.");
    } else err("Keyword 'program' expected.");
    return true;
}

const llvm::Module &Parser::Generate() {
    symbolTable1.emplace_back();
    program->codegen(MilaContext, MilaBuilder, MilaModule, symbolTable1, breakBlocks, nullptr, functionTable, 0, arrayStartMap);
    return this->MilaModule;
}

/**
 * @brief Simple token buffer.
 *
 * CurTok is the current token the parser is looking at
 * getNextToken reads another token from the lexer and updates curTok with ts result
 * Every function in the parser will assume that CurTok is the cureent token that needs to be parsed
 */
int Parser::getNextToken() {
    CurTok = m_Lexer.gettok();
    //    printf("%d\n", CurTok);
    return CurTok;
}

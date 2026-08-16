#include "Lexer.hpp"

#include <fstream>

#include "unordered_map"

const static std::unordered_map<std::string, Token> symbols =
    {{"if", Token::tok_if},
     {"while", Token::tok_while},
     {"program", Token::tok_program},
     {"begin", Token::tok_begin},
     {"end", Token::tok_end},
     {"const", Token::tok_const},
     {"then", Token::tok_then},
     {"else", Token::tok_else},
     {"exit", Token::tok_exit},
     {"var", Token::tok_var},
     {"integer", Token::tok_integer},
     {"for", Token::tok_for},
     {"do", Token::tok_do},
     {"function", Token::tok_function},
     {"mod", Token::tok_mod},
     {"div", Token::tok_div},
     {"not", Token::tok_not},
     {"and", Token::tok_and},
     {"xor", Token::tok_xor},
     {"or", Token::tok_or},
     {"break", Token::tok_break},
     {"to", Token::tok_to},
     {"downto", Token::tok_downto},
     {"forward", Token::tok_forward},
     {"procedure", Token::tok_procedure},
     {"array", Token::tok_array},
     {"of", Token::tok_of},
    };

bool isValidChar(char actChar) {
    return (actChar >= 'a' && actChar <= 'z') ||
           (actChar >= 'A' && actChar <= 'Z') ||
           actChar == '_';
}

bool isDigit(char actChar, int base) {
    if (base == 16 && ((actChar >= 'a' && actChar <= 'f') || (actChar >= 'A' && actChar <= 'F'))) return true;
    if (base == 8 && actChar >= '8') return false;
    return actChar >= '0' && actChar <= '9';
}

Token Lexer::dfa(char& actChar, bool& next, int& base, bool& ok) {
    while (true) {
        if (next) actChar = getc(file);
//         std::cout << "-> " << actChar << std::endl;
        if (actChar == -1) return Token::tok_eof;
        next = true;
        switch (actState) {
            case 0:
                m_IdentifierStr = "";
                m_NumVal = 0;
                if (iswspace(actChar)) { // skip WS
                    break;
                }
                if (actChar != 0 && isDigit(actChar, base)) {
                    m_NumVal = actChar - '0';
                    actState = 1;
                    break;
                }

                m_IdentifierStr = actChar;

                if (actChar == '-') {
                    actState = 5;
                    next = false;
                    break;
                }
                if (isValidChar(actChar)) {
                    actState = 3;
                    break;
                }
                if (actChar == '=') {
                    actState = 4;
                    next = false;
                    break;
                }
                if (actChar == '+' || actChar == '*' || actChar == '/') {
                    actState = 5;
                    next = false;
                    break;
                }
                if (actChar == '[' || actChar == ']' || actChar == '(' || actChar == ')') {
                    actState = 6;
                    next = false;
                    break;
                }
                if (actChar == '.') {
                    actState = 7;
                    next = false;
                    break;
                }
                if (actChar == ';') {
                    actState = 8;
                    next = false;
                    break;
                }
                if (actChar == ':') {
                    actState = 9;
                    next = false;
                    break;
                }
                if (actChar == ',') {
                    actState = 10;
                    next = false;
                    break;
                }
                if (actChar == '<') {
                    actState = 11;
                    break;
                }
                if (actChar == '>') {
                    actState = 12;
                    break;
                }
                if (actChar == '$') {
                    actState = 13;
                    next = false;
                    break;
                }
                if (actChar == '&') {
                    actState = 14;
                    next = false;
                    break;
                }
                if (actChar == '\'') {
                    actState = 15;
                    break;
                }
                throw std::invalid_argument("Lexer: Invalid character");
            case 1:
                if (isDigit(actChar, base)) {
                    m_NumVal = base * m_NumVal + actChar - '0';
                    break;
                }
                if (flag) {
                    m_NumVal *= -1;
                    flag = false;
                }
                next = false;
                actState = 0;
                base = 10;
                return Token::tok_number;
            case 2:
                if (actChar != 0 && isDigit(actChar, base)) {
                    m_NumVal = actChar - '0';
                    flag = true;
                    actState = 1;
                    break;
                }
                next = false;
                actState = 0;
                return Token::tok_operator;
            case 3:
                if (isValidChar(actChar) || isDigit(actChar, base)) {
                    m_IdentifierStr.push_back(actChar);
                    break;
                }
                next = false;
                actState = 0;
                if (symbols.find(m_IdentifierStr) != symbols.end()) return symbols.find(m_IdentifierStr)->second;
                return Token::tok_identifier;
            case 4:
                actState = 0;
                return Token::tok_assign;
                //throw std::invalid_argument("Lexer: Invalid character");
            case 5:
                actState = 0;
                return Token::tok_operator;
                //throw std::invalid_argument("Lexer: Invalid character");
            case 6:
                actState = 0;
                next = true;
                return Token::tok_brackets;
                //throw std::invalid_argument("Lexer: Invalid character");
            case 7:
                actState = 0;
                return Token::tok_dot;
            case 8:
                actState = 0;
                return Token::tok_semicolon;
                //throw std::invalid_argument("Lexer: Invalid character");
            case 9:
                actState = 0;
                return Token::tok_colon;
            case 10:
                actState = 0;
                return Token::tok_comma;
            case 11:
                if (actChar == '=') {
                    actState = 0;
                    return Token::tok_lessequal;
                }
                if (actChar == '>') {
                    actState = 0;
                    return Token::tok_notequal;
                }
                actState = 0;
                return Token::tok_less;
            case 12:
                if (actChar == '=') {
                    actState = 0;
                    return Token::tok_greaterequal;
                }
                actState = 0;
                return Token::tok_greater;
            case 13:
                base = 16;
                actState = 1;
                break;
            case 14:
                base = 8;
                actState = 1;
                break;
            case 15:
                if (actChar == '\\') {
                    ok = true;
                    break;
                }
                if (ok || actChar != '\'') {
                    ok = false;
                    m_IdentifierStr.push_back(actChar);
                    break;
                }
                actState = 0;
                if (actChar == '\'') return Token::tok_apostr;
                throw std::runtime_error("Lexer: Invalid character");
        }
    }
}

/**
 * @brief Function to return the next token from standard input
 *
 * the variable 'm_IdentifierStr' is set there in case of an identifier,
 * the variable 'm_NumVal' is set there in case of a number.
 */
int Lexer::gettok() {
    return dfa(actChar, next, base, ok);
}


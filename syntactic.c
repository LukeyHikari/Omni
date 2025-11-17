#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LEXEME_LENGTH 256

// Token Type Enumeration
typedef enum {
    Token_Identifier,
    Token_Character,
    Token_String,
    Token_Number,
    Token_Operator,
    Token_CodeEnd,
    Token_Unknown,
    Token_Delimeter,
    Token_Comment,
    Token_Arithmetic_Operator,
    Token_Arithmetic_Operator_DIV,
    Token_Boolean_Operator,
    Token_Boolean_Operator_AND,
    Token_Boolean_Operator_OR,
    Token_Assignment_Operator,
    Token_Builtin_Constant,
    Token_Keyword_If,
    Token_Keyword_Else,
    Token_Keyword_ElseIf,
    Token_Keyword_For,
    Token_Keyword_For_In,
    Token_Keyword_For_Range,
    Token_Keyword_Int,
    Token_Keyword_Decimal,
    Token_Keyword_Char,
    Token_Keyword_String,
    Token_Keyword_Boolean,
    Token_Keyword_Read,
    Token_Keyword_Write,
    Token_Reserved_True,
    Token_Reserved_False,
    Token_Reserved_Null,
    Token_Noise_Do,
    Token_Delim_LPAR,
    Token_Delim_RPAR,
    Token_Delim_LBRAC,
    Token_Delim_RBRAC,
    Token_Delim_Comma,
    Token_Delim_SQuote,
    Token_Delim_DQuote,
    Token_Delim_Period,
    Token_Delim_Newline,
    Token_Delim_Space
} Token_Type; 

// Lookup Table for string representation of token types; for reconversion to Token_Type
const char* tokenTypeStrings[] = {
    "Identifier",
    "Character",
    "String",
    "Number",
    "Operator",
    "CodeEnd",
    "Unknown",
    "Delimeter",
    "Comment",
    "Arithmetic_Operator",
    "Arithmetic_Operator_DIV",
    "Boolean_Operator",
    "Boolean_Operator_AND",
    "Boolean_Operator_OR",
    "Assignment_Operator",
    "Builtin_Constant",
    "Keyword_If",
    "Keyword_Else",
    "Keyword_ElseIf",
    "Keyword_For",
    "Keyword_For_In",
    "Keyword_For_Range",
    "Keyword_Int",
    "Keyword_Decimal",
    "Keyword_Char",
    "Keyword_String",
    "Keyword_Boolean",
    "Keyword_Read",
    "Keyword_Write",
    "Reserved_True",
    "Reserved_False",
    "Reserved_Null",
    "Noise_Do",
    "Delim_LPAR",
    "Delim_RPAR",
    "Delim_LBRAC",
    "Delim_RBRAC",
    "Delim_Comma",
    "Delim_SQuote",
    "Delim_DQuote",
    "Delim_Period",
    "Delim_Newline"
};

// Token Struct
typedef struct {
    Token_Type type;
    char lexeme[MAX_LEXEME_LENGTH];
} Token;

// Helper function prototypes
Token peek();
Token previous();
Token advance();
Token expect(Token_Type type);
Token_Type revertToTokenType(const char* parsedType);
bool atEnd();
bool match(Token_Type type);
void reportSyntaxError(const char* msg);

// Parser function prototypes
void parseProgram();
void parseStatements();
void parseStatement();
void parseDeclareAssign();
void parseAssign();
void parseIf();
void parseElseIf();
void parseElse();
void parseIterative();
void parseBlocks();
void parseRead();
void parseWrite();
void parseExpression();
void parseOrExpression();
void parseAndExpression();
void parseEqualityExpression();
void parseRelationalExpression();
void parseAddSubExpression();
void parseMulDivExpression();
void parseExponentExpression();
void parseUnaryExpression();
void parseValueExpression();

// Token Stream
Token* tokens;
int curToken = 0;

int main(){
    printf("Hello world");
}

#pragma region Helper Functions
Token peek(){
    return tokens[curToken];
}

Token previous(){
    if(curToken == 0) return tokens[0];
    return tokens[curToken - 1];
}

Token advance(){
    if(!atEnd()) curToken++;
    return previous();
}

Token expect(Token_Type type){
    if(peek().type == type) return advance();
    const char* errorMsg = "Unexpected Token Type";
    // For more complex error handling in the future
    // fprintf(errorMsg, "Expected token type %d, but got unexpected type %d", type, peek().type);
    reportSyntaxError(errorMsg);

    return peek();
}

Token_Type revertToTokenType(const char* parsedType){
    for (int i = 0; i < sizeof(tokenTypeStrings)/sizeof(tokenTypeStrings[0]); i++) {
        if (strcmp(tokenTypeStrings[i], parsedType) == 0) {
            return (Token_Type)i;
        }
    }
    return Token_Unknown;  // fallback if not found
}

bool atEnd(){
    return tokens[curToken].type == Token_CodeEnd;
}

bool match(Token_Type type){
    if(atEnd() || peek().type != type) return 0;
    advance();
    return 1;
}

void reportSyntaxError(const char* msg) {
    fprintf(stderr, "SYNTAX ERROR: %s\n", msg);
    exit(1);
}

void handleDo(){
    // Potential bug TO FIX: Handle end of token stream
    if(peek().type == Token_Noise_Do) advance();
}
#pragma endregion

void parseProgram(){
    parseStatements();
    expect(Token_CodeEnd);
}

void parseStatements(){
    while(tokens[curToken].type != Token_CodeEnd)
        parseStatement();
}

void parseStatement(){
    switch(tokens[curToken].type){
        case Token_Keyword_Int: case Token_Keyword_Decimal: case Token_Keyword_Char:
        case Token_Keyword_String: case Token_Keyword_Boolean:
            parseDeclareAssign();
        break;
        case Token_Identifier:
            parseAssign();
        break;
        case Token_Keyword_If:
            parseIf();
        break;
        case Token_Keyword_For:
            parseIterative();
        break;
        case Token_Keyword_Read:
            parseRead();
        break;
        case Token_Keyword_Write:
            parseWrite();
        break;
        default:
            reportSyntaxError("Invalid Start");
            curToken++;
            return;
    }
}

#pragma region Statement Functions
void parseDeclareAssign(){
    switch(tokens[curToken].type){
        case Token_Keyword_Int:
            expect(Token_Keyword_Int);
        break;
        case Token_Keyword_Decimal:
            expect(Token_Keyword_Decimal);
        break;
        case Token_Keyword_Char:
            expect(Token_Keyword_Char);
        break;
        case Token_Keyword_String:
            expect(Token_Keyword_String);
        break;
        case Token_Keyword_Boolean:
            expect(Token_Keyword_Boolean);
        break;
        default:
            reportSyntaxError("Expected type keyword");
            curToken++;
            return;
    }
    expect(Token_Identifier);

    if(peek().type == Token_Assignment_Operator){
        expect(Token_Assignment_Operator);
        parseExpression();
    }
    expect(Token_Delim_Newline);
}

void parseAssign(){
    expect(Token_Identifier);
    expect(Token_Assignment_Operator);
    parseExpression();
    expect(Token_Delim_Newline);
}

void parseIf(){
    expect(Token_Keyword_If);
    expect(Token_Delim_LPAR);
    parseExpression();
    expect(Token_Delim_RPAR);
    handleDo();
    expect(Token_Delim_Newline);
    //parseStatement();
    parseBlocks();

    while(tokens[curToken].type == Token_Keyword_ElseIf) parseElseIf();

    if(tokens[curToken].type == Token_Keyword_Else) parseElse();
}

void parseElseIf(){
    expect(Token_Keyword_ElseIf);
    expect(Token_Delim_LPAR);
    parseExpression();
    expect(Token_Delim_RPAR);
    handleDo();
    expect(Token_Delim_Newline);
    //parseStatement();
    parseBlocks();

    //if(tokens[curToken].type == Token_Keyword_Else) parseElse();
}

void parseElse(){
    expect(Token_Keyword_Else);
    handleDo();
    expect(Token_Delim_Newline);
    //parseStatement();
    parseBlocks();
}

void parseIterative(){
    expect(Token_Keyword_For);
    expect(Token_Identifier);
    expect(Token_Keyword_For_In);
    expect(Token_Keyword_For_Range);
    expect(Token_Delim_LPAR);
    parseExpression();
    expect(Token_Delim_Comma);
    parseExpression();
    expect(Token_Delim_RPAR);
    handleDo();
    expect(Token_Delim_Newline);
    //parseStatement();
    parseBlocks();
}

void parseBlocks(){
    // Recursively gets statements in conditional and iterative code blocks
    while(1){
        switch(tokens[curToken].type){
            case Token_Keyword_If: case Token_Keyword_For: case Token_Keyword_Int: case Token_Keyword_Decimal:
            case Token_Keyword_Char: case Token_Keyword_String: case Token_Keyword_Boolean:
                parseStatement();
            break;
            case Token_Keyword_ElseIf: case Token_Keyword_Else:
                return;
            break;
            default:
                reportSyntaxError("Unexpected token encountered in block");
                curToken++;
                return;
        }
    }
}

void parseRead(){
    expect(Token_Keyword_Read);
    expect(Token_Delim_LPAR);
    expect(Token_Identifier);
    expect(Token_Delim_RPAR);
    expect(Token_Delim_Newline);
}

void parseWrite(){
    expect(Token_Keyword_Write);
    expect(Token_Delim_LPAR);
    parseExpression();
    while(tokens[curToken].type == Token_Delim_Comma){
        expect(Token_Delim_Comma);
        parseExpression();
    }
    expect(Token_Delim_RPAR);
    expect(Token_Delim_Newline);
}
#pragma endregion

#pragma region Expresison Functions
void parseExpression(){
    parseOrExpression();
}

void parseOrExpression(){
    parseAndExpression(); // Parse the left side

    // Use match() on OR to determine if we need to parse right side
    while (match(Token_Boolean_Operator_OR)) parseAndExpression(); // Parse the right side
}

void parseAndExpression(){
    parseEqualityExpression(); // Parse the left side

    // Use match() on AND to determine if we need to parse right hside
    while (match(Token_Boolean_Operator_AND)) parseEqualityExpression(); // Parse the right side
}

void parseEqualityExpression(){
    parseRelationalExpression(); // Parse the left side

    // Use peek() to determine if we need to parse right side
    while (peek().type == Token_Boolean_Operator && 
           (strcmp(peek().lexeme, "==") == 0 || 
            strcmp(peek().lexeme, "!=") == 0)) 
    {
        advance(); // Consume the '==' or '!='
        parseRelationalExpression(); // Parse the right side
    }
}

void parseRelationalExpression(){
    parseAddSubExpression(); // Parse the left side

    // Use peek() to determine if we need to parse right side
    while (peek().type == Token_Boolean_Operator &&
           (strcmp(peek().lexeme, "<") == 0 || 
            strcmp(peek().lexeme, ">") == 0 ||
            strcmp(peek().lexeme, "<=") == 0 || 
            strcmp(peek().lexeme, ">=") == 0)) 
    {
        advance(); // Consume the operator
        parseAddSubExpression(); // Parse the right side
    }
}

void parseAddSubExpression(){
    parseMulDivExpression(); // Parse the left side

    // Use peek() to determine if we need to parse right side
    while (peek().type == Token_Arithmetic_Operator &&
           (strcmp(peek().lexeme, "+") == 0 || 
            strcmp(peek().lexeme, "-") == 0)) 
    {
        advance(); // Consume the '+' or '-'
        parseMulDivExpression(); // Parse the right side
    }
}

void parseMulDivExpression(){
    parseExponentExpression(); // Parse the left side

    // Use peek() to determine if we need to parse right side
    while ((peek().type == Token_Arithmetic_Operator &&
               (strcmp(peek().lexeme, "*") == 0 || 
                strcmp(peek().lexeme, "/") == 0 || 
                strcmp(peek().lexeme, "%") == 0)) ||
                peek().type == Token_Arithmetic_Operator_DIV) 
    {
        advance(); // Consume the operator
        parseExponentExpression(); // Parse the right side
    }
}

void parseExponentExpression(){
    parseUnaryExpression(); // Parse the left base

    if (peek().type == Token_Arithmetic_Operator && strcmp(peek().lexeme, "^") == 0) {
        advance(); // Consume the '^'
        parseExponentExpression(); // Recursive call for the right side (Exponent)
    }
}

void parseUnaryExpression(){
    if ((peek().type == Token_Boolean_Operator && strcmp(peek().lexeme, "!") == 0) ||
        (peek().type == Token_Arithmetic_Operator && strcmp(peek().lexeme, "-") == 0)) 
    {
        advance(); // Consume the '!' or '-'
        parseUnaryExpression(); // Recursive call for !!true / !!!!false
    } else {
        parseValueExpression(); // Base case
    }
}

void parseValueExpression(){
    // Handle literals and identifiers
    if (match(Token_Number) ||
        match(Token_Identifier) ||
        match(Token_Reserved_True) ||
        match(Token_Reserved_False) ||
        match(Token_String) ||
        match(Token_Character) ||
        match(Token_Builtin_Constant) || 
        match(Token_Reserved_Null)) {
        return; // consume value
    }

    // Handle “(“ <expr> “)”
    if (match(Token_Delim_LPAR)) {
        parseExpression(); // Recursively parse the expression inside
        expect(Token_Delim_RPAR);
        return;
    }

    // If we get here, no valid value was found
    reportSyntaxError("Expected a value, but got nothing");
}
#pragma endregion
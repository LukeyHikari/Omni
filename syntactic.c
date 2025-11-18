#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LEXEME_LENGTH 256
// Token Loader
#define INITIAL_TOKEN_CAPACITY 2048

// Token Type Enumeration
typedef enum {
    Token_Identifier,              // 0
    Token_Character,               // 1
    Token_String,                  // 2
    Token_Number,                  // 3
    Token_Operator,                // 4
    Token_CodeEnd,                 // 5
    Token_Unknown,                 // 6
    Token_Delimeter,               // 7
    Token_Comment,                 // 8
    Token_Arithmetic_Operator,     // 9
    Token_Arithmetic_Operator_DIV, // 10
    Token_Boolean_Operator,        // 11
    Token_Boolean_Operator_AND,    // 12
    Token_Boolean_Operator_OR,     // 13
    Token_Assignment_Operator,     // 14
    Token_Builtin_Constant,        // 15
    Token_Keyword_If,              // 16
    Token_Keyword_Else,            // 17
    Token_Keyword_ElseIf,          // 18
    Token_Keyword_For,             // 19
    Token_Keyword_For_In,          // 20
    Token_Keyword_For_Range,       // 21
    Token_Keyword_Int,             // 22
    Token_Keyword_Decimal,         // 23
    Token_Keyword_Char,            // 24
    Token_Keyword_String,          // 25
    Token_Keyword_Boolean,         // 26
    Token_Keyword_Read,            // 27
    Token_Keyword_Write,           // 28
    Token_Reserved_True,           // 29
    Token_Reserved_False,          // 30
    Token_Reserved_Null,           // 31
    Token_Noise_Do,                // 32
    Token_Delim_LPAR,              // 33
    Token_Delim_RPAR,              // 34
    Token_Delim_LBRAC,             // 35
    Token_Delim_RBRAC,             // 36
    Token_Delim_Comma,             // 37
    Token_Delim_SQuote,            // 38
    Token_Delim_DQuote,            // 39
    Token_Delim_Period,            // 40
    Token_Delim_Newline,           // 41
    Token_Delim_Space              // 42
} Token_Type; 

// Lookup Table for string representation of token types; for reconversion to Token_Type
const char* tokenTypeStrings[] = {
    "Identifier",               // 0
    "Character",                // 1
    "String",                   // 2
    "Number",                   // 3
    "Operator",                 // 4
    "CodeEnd",                  // 5
    "Unknown",                  // 6
    "Delimeter",                // 7
    "Comment",                  // 8
    "Arithmetic_Operator",      // 9
    "Arithmetic_Operator_DIV",  // 10
    "Boolean_Operator",         // 11
    "Boolean_Operator_AND",     // 12
    "Boolean_Operator_OR",      // 13
    "Assignment_Operator",      // 14
    "Builtin_Constant",         // 15
    "Keyword_If",               // 16
    "Keyword_Else",             // 17
    "Keyword_ElseIf",           // 18
    "Keyword_For",              // 19
    "Keyword_For_In",           // 20
    "Keyword_For_Range",        // 21
    "Keyword_Int",              // 22
    "Keyword_Decimal",          // 23
    "Keyword_Char",             // 24
    "Keyword_String",           // 25
    "Keyword_Boolean",          // 26
    "Keyword_Read",             // 27
    "Keyword_Write",            // 28
    "Reserved_True",            // 29
    "Reserved_False",           // 30
    "Reserved_Null",            // 31
    "Noise_Do",                 // 32
    "Delim_LPAR",               // 33
    "Delim_RPAR",               // 34
    "Delim_LBRAC",              // 35
    "Delim_RBRAC",              // 36
    "Delim_Comma",              // 37
    "Delim_SQuote",             // 38
    "Delim_DQuote",             // 39
    "Delim_Period",             // 40
    "Delim_Newline",            // 41
    "Delim_Space"               // 42
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
void handleComments();

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

// Loader function prototypes
static char* trim(char* s);
static void ensureTokenCapacity();
static bool parseLineToLexemeAndType (char* line, char* out_lexeme, size_t lexeme_sz, char* out_type, size_t type_sz);
void loadTokensFromSymbolTable(const char* filepath);

// Token Stream
Token* tokens;
int curToken = 0;

// Global token storage (already declared earlier as Token* tokens; int curToken;)
int token_count = 0;
int token_capacity = 0;

int main() {
    // Allocate tokens pointer and load from uploaded symbol table
    tokens = NULL; // Loader will allocate
    loadTokensFromSymbolTable("D:\\Files\\School\\University\\3Y1S\\7. PPL\\Mini PL\\Omni\\output\\symbol_table.txt"); 

    // Call parser functions
    parseProgram();

    printf("Parsing finished.\n");
    return 0;
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
    if(type != peek().type){
        printf("Current Token: %s | Expected Token: %d | Peeked Token: %d\n",
                    tokens[curToken].lexeme, type, peek().type);
        printf("Other Check: %s\n", tokens[curToken-1].lexeme);
    } 
    if(peek().type == type) return advance();
    const char* errorMsg = "Unexpected Token Type";
    // For more complex error handling in the future
    // fprintf(errorMsg, "Expected token type %d, but got unexpected type %d", type, peek().type);
    reportSyntaxError(errorMsg);

    return peek();
}

Token_Type revertToTokenType(const char* parsedType){
    int streamSize = sizeof(tokenTypeStrings)/sizeof(tokenTypeStrings[0]);
    for (int i = 0; i < streamSize; i++) {
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
    //exit(1);
}

void handleDo(){
    // Potential bug TO FIX: Handle end of token stream
    if(peek().type == Token_Noise_Do) advance();
}

void handleComments(){
    // For same line comments
    // Potential bug TO FIX: Handle end of token stream
    if(peek().type == Token_Comment) advance();
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
        case Token_Comment: case Token_Delim_Newline:
            advance();
            break;
        default:
            //reportSyntaxError("Invalid Start");
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
    handleComments();
    expect(Token_Delim_Newline);
}

void parseAssign(){
    expect(Token_Identifier);
    expect(Token_Assignment_Operator);
    parseExpression();
    handleComments();
    expect(Token_Delim_Newline);
}

void parseIf(){
    expect(Token_Keyword_If);
    expect(Token_Delim_LPAR);
    parseExpression();
    expect(Token_Delim_RPAR);
    handleDo();
    handleComments();
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
    handleComments();
    expect(Token_Delim_Newline);
    //parseStatement();
    parseBlocks();

    //if(tokens[curToken].type == Token_Keyword_Else) parseElse();
}

void parseElse(){
    expect(Token_Keyword_Else);
    handleDo();
    handleComments();
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
    // TO FIX: Potential bug that doesn't follow grammar of for
    if(peek().type == Token_Delim_RPAR) expect(Token_Delim_RPAR);
    else{
        while(tokens[curToken].type == Token_Delim_Comma){
            expect(Token_Delim_Comma);
            parseExpression();
            // printf("Parsed expression: %s\n", tokens[curToken-1].lexeme);
            // printf("Parsing next token: %s\n", tokens[curToken].lexeme);
            if(peek().type == Token_Delim_RPAR) {
                expect(Token_Delim_RPAR);
                break;
            }
            //printf("Parsed expression: %s\n", tokens[curToken].lexeme);
        }
    }
    handleDo();
    handleComments();
    expect(Token_Delim_Newline);
    //parseStatement();
    parseBlocks();
}

void parseBlocks(){
    // Recursively gets statements in conditional and iterative code blocks
    while(1){
        switch(tokens[curToken].type){
            case Token_Keyword_If: case Token_Keyword_For: case Token_Keyword_Int: case Token_Keyword_Decimal:
            case Token_Keyword_Char: case Token_Keyword_String: case Token_Keyword_Boolean: case Token_Identifier:
            case Token_Keyword_Write: case Token_Keyword_Read:
                parseStatement();
            break;
            case Token_Keyword_ElseIf: case Token_Keyword_Else:
                return;
            break;
            case Token_Delim_Newline:
                return;
            break;
            default:
                printf("Parsed expression: %s\n", tokens[curToken-1].lexeme);
                printf("Parsing next token: %s\n", tokens[curToken].lexeme);
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
    handleComments();
    expect(Token_Delim_Newline);
}

void parseWrite(){
    expect(Token_Keyword_Write);
    expect(Token_Delim_LPAR);
    parseExpression();
    // printf("Parsed expression: %s\n", tokens[curToken-1].lexeme);
    // printf("Parsing next token: %s\n", tokens[curToken].lexeme);
    if(peek().type == Token_Delim_RPAR) expect(Token_Delim_RPAR);
    else{
        while(tokens[curToken].type == Token_Delim_Comma){
            expect(Token_Delim_Comma);
            parseExpression();
            // printf("Parsed expression: %s\n", tokens[curToken-1].lexeme);
            // printf("Parsing next token: %s\n", tokens[curToken].lexeme);
            if(peek().type == Token_Delim_RPAR) {
                expect(Token_Delim_RPAR);
                break;
            }
            //printf("Parsed expression: %s\n", tokens[curToken].lexeme);
        }
    }
    //expect(Token_Delim_RPAR);
    handleComments();
    if(peek().type != Token_CodeEnd)
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

#pragma region Loader functions
// Trim whitespace from both ends (in place)
static char* trim(char* s) {
    if(!s) return s;
    // Left trim
    while(isspace((unsigned char)*s)) s++;
    if(*s == 0) return s;
    // Right trim
    char* end = s + strlen(s) - 1;
    while(end > s && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return s;
}

// Ensure capacity for adding tokens
static void ensureTokenCapacity() {
    if(tokens == NULL) {
        token_capacity = INITIAL_TOKEN_CAPACITY;
        tokens = (Token*)malloc(sizeof(Token) * token_capacity);
        if(tokens == NULL) {
            fprintf(stderr, "Failed to allocate tokens array\n");
            exit(1);
        }
        token_count = 0;
    } else if(token_count >= token_capacity) {
        token_capacity *= 2;
        tokens = (Token*)realloc(tokens, sizeof(Token) * token_capacity);
        if(tokens == NULL) {
            fprintf(stderr, "Failed to reallocate tokens array\n");
            exit(1);
        }
    }
}

// Parse a line assumed to contain "LEXEME ... TOKEN_TYPE"
static bool parseLineToLexemeAndType (char* line, char* out_lexeme, size_t lexeme_sz, char* out_type, size_t type_sz) {
    if(line == NULL) return false;
    // Make a copy so we can modify
    char buf[1024];
    strncpy(buf, line, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char* p = buf;
    p = trim(p);
    if(*p == '\0') return false;

    // Skip lines that are separators or headers
    if(strstr(p, "LEXEME") || strstr(p, "TOKEN_TYPE") || strstr(p, "----")) return false;

    // Find last whitespace to split type
    size_t len = strlen(p);
    // Find the start index of the last word
    int i = (int)len - 1;
    while(i >= 0 && isspace((unsigned char)p[i])) i--;
    if(i < 0) return false;
    int end = i;
    // Move i back until we hit whitespace or beginning
    while(i >= 0 && !isspace((unsigned char)p[i])) i--;
    int start_type = i + 1;

    // Extract type
    int type_len = end - start_type + 1;
    if(type_len <= 0 || type_len >= (int)type_sz) return false;
    strncpy(out_type, p + start_type, type_len);
    out_type[type_len] = '\0';

    // Lexeme is everything before start_type
    p[start_type] = '\0';
    char* lex = trim(buf);
    if(lex == NULL || *lex == '\0') return false;

    // Copy lexeme
    strncpy(out_lexeme, lex, lexeme_sz - 1);
    out_lexeme[lexeme_sz - 1] = '\0';

    return true;
}

// Main loader function
void loadTokensFromSymbolTable(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Unable to open symbol table file: %s\n", filepath);
        exit(1);
    }

    ensureTokenCapacity();

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        size_t ln = strlen(line);
        if (ln > 0 && (line[ln - 1] == '\n' || line[ln - 1] == '\r')) {
            line[ln - 1] = '\0';
            ln--;
            // Handle CRLF
            if (ln > 0 && line[ln - 1] == '\r') { line[ln - 1] = '\0'; ln--; }
        }

        char lexeme[MAX_LEXEME_LENGTH];
        char typeStr[128];

        if (!parseLineToLexemeAndType(line, lexeme, sizeof(lexeme), typeStr, sizeof(typeStr))) {
            // If line did not parse into lexeme+type -> skip
            continue;
        }

        // Convert textual token type to Token_Type enum using your existing function
        Token_Type ttype = revertToTokenType(typeStr);


        // Add token to global array
        ensureTokenCapacity();
        tokens[token_count].type = ttype;
        // Copy lexeme safely
        strncpy(tokens[token_count].lexeme, lexeme, MAX_LEXEME_LENGTH - 1);
        tokens[token_count].lexeme[MAX_LEXEME_LENGTH - 1] = '\0';

        token_count++;

        // If we encounter CodeEnd in the file we can keep reading but usually EOF follows
        if(ttype == Token_CodeEnd) break;
    }

    fclose(f);

    // If file didn't contain a CodeEnd token, append one
    if (token_count == 0 || tokens[token_count-1].type != Token_CodeEnd) {
        ensureTokenCapacity();
        tokens[token_count].type = Token_CodeEnd;
        tokens[token_count].lexeme[0] = '\0';
        token_count++;
    }

    // Reset parser index
    curToken = 0;
    // Optional: debug print count
    printf("Loaded %d tokens from %s\n", token_count, filepath);
}
#pragma endregion
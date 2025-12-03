#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LEXEME_LENGTH 256
// Token Loader
#define INITIAL_TOKEN_CAPACITY 2048
// AST Node
#define INITIAL_BLOCK_STATEMENTS 8

/* =========================
   DEFINITIONS & ENUMERATIONS
   ========================= */

#pragma region Token Definitions
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
    int line_number;  // ADD: line number field
} Token;

#pragma endregion

#pragma region AST nodes definition

// Node type enumeration
typedef enum {
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_UNARY_OP,
    NODE_BINARY_OP,
    NODE_ASSIGN,
    NODE_DECLARE_ASSIGN,
    NODE_IF_STMT,
    NODE_FOR_STMT,
    NODE_READ_STMT,
    NODE_WRITE_STMT,
    NODE_BLOCK
} NodeType;

// Define specific data that each node type contains
typedef struct AST_Node AST_Node;
typedef struct {
    Token token; // literal
} LiteralNode;

typedef struct {
    Token token; // identifier 
} IdentifierNode;

typedef struct {
    Token op;      // unary operator 
    AST_Node* right; // expression on the right
} UnaryOpNode;

typedef struct {
    Token op;      // binary operator
    AST_Node* left;
    AST_Node* right;
} BinaryOpNode;

typedef struct {
    Token identifier;
    AST_Node* expression; // value being assigned
} AssignNode;

typedef struct {
    Token type;       // data type keyword
    Token identifier;
    AST_Node* expression; // value being assigned
} DeclareAssignNode;

typedef struct {
    AST_Node* condition;
    AST_Node* thenBranch;   // NODE_BLOCK
    AST_Node* elseBranch;   // can be NULL, NODE_BLOCK(else), or another NODE_IF_STMT(else if)
} IfStmtNode;

typedef struct {
    Token identifier;
    int argCount;
    AST_Node** rangeArgs; // dynamic array for 1-3 arguments
    AST_Node* body;       // NODE_BLOCK
} ForStmtNode;

typedef struct {
    Token identifier; // identifier of variable to store into
} ReadStmtNode;

typedef struct {
    int argCount; 
    int capacity; //number of expressions we have room for
    AST_Node** expressions; // dynamic array of expressions to write
} WriteStmtNode;

typedef struct {
    int count; 
    int capacity; //number of statements we have room for
    AST_Node** statements; // dynamic array of statements
} BlockNode;

struct AST_Node {
    NodeType type;
    union {
        LiteralNode       literal;
        IdentifierNode    identifier;
        UnaryOpNode       unaryOp;
        BinaryOpNode      binaryOp;
        AssignNode        assign;
        DeclareAssignNode declareAssign;
        IfStmtNode        ifStmt;
        ForStmtNode       forStmt;
        ReadStmtNode      readStmt;
        WriteStmtNode     writeStmt;
        BlockNode         block;
    } data;
};
#pragma endregion

/* =========================
   FUNCTION PROTOTYPES
   ========================= */

#pragma region Function Prototypes
/* Token / Loader */
void loadTokensFromSymbolTable(const char* filepath);
Token peek();
Token previous();
Token advance();
Token expect(Token_Type type);
Token_Type revertToTokenType(const char* parsedType);
bool match(Token_Type type);
bool atEnd();
void handleDo();
void handleComments();

/* Parser top-level */
AST_Node* parse();
AST_Node* parseStatement();
AST_Node* parseDeclareAssign();
AST_Node* parseAssign();
AST_Node* parseIf();
AST_Node* parseIterative();
AST_Node* parseRead();
AST_Node* parseWrite();
AST_Node* parseExpression();
AST_Node* parseOrExpression();
AST_Node* parseAndExpression();
AST_Node* parseEqualityExpression();
AST_Node* parseRelationalExpression();
AST_Node* parseAddSubExpression();
AST_Node* parseMulDivExpression();
AST_Node* parseExponentExpression();
AST_Node* parseUnaryExpression();
AST_Node* parseValueExpression();
AST_Node* parseBlocks();
AST_Node* parseElse();
void reportSyntaxError(const char* msg);

/* AST helpers */
AST_Node* createNode(NodeType type);
AST_Node* createLiteralNode(Token token);
AST_Node* createIdentifierNode(Token token);
AST_Node* createUnaryOpNode(Token op, AST_Node* right);
AST_Node* createBinaryOpNode(Token op, AST_Node* left, AST_Node* right);
AST_Node* createAssignNode(Token identifier, AST_Node* expression);
AST_Node* createDeclareAssignNode(Token type, Token identifier, AST_Node* expression);
AST_Node* createIfStmtNode(AST_Node* condition, AST_Node* thenBranch, AST_Node* elseBranch);
AST_Node* createForStmtNode(Token identifier, AST_Node* body);
void addForRangeArg(AST_Node* forNode, AST_Node* arg);
AST_Node* createReadStmtNode(Token identifier);
AST_Node* createWriteStmtNode();
void addWriteExpression(AST_Node* writeNode, AST_Node* expr);
AST_Node* createBlockNode();
void addStatementToBlock(AST_Node* blockNode, AST_Node* statement);
void printAST(AST_Node* node, int indent);
void writeAST_SExpr(const char* filepath, AST_Node* root);
void freeAST(AST_Node* node);

/* JSON writer internal */
static void writeNodeSExpr_internal(FILE* f, AST_Node* node);

/* Utility */
static char* trim(char* s);
static void ensureTokenCapacity();
static bool parseLineToLexemeAndType (char* line, char* out_lexeme, size_t lexeme_sz, char* out_type, size_t type_sz, int* out_line_number);  // ADD: line_number parameter
#pragma endregion

/* =========================
   GLOBAL VARIABLES
   ========================= */

Token* tokens = NULL;
int curToken = 0;
int token_count = 0;
int token_capacity = 0;

/* =========================
   int main
   ========================= */

int main() {
    // Early debug to confirm program start
    printf("PROGRAM START\n");
    fflush(stdout);

    // Allocate tokens pointer and load from uploaded symbol table
    tokens = NULL; // Loader will allocate
    loadTokensFromSymbolTable("D:\\Files\\School\\University\\3Y1S\\7. PPL\\Mini PL\\Omni\\output\\symbol_table.txt"); 

    // Call top-level parser function
    printf("\n--- Parsing Program ---\n");
    AST_Node* root = parse();
    printf("--- Parsing Finished ---\n");

    // Debug: print loaded token count and some tokens for verification
    printf("\n--- DEBUG: Loaded Tokens ---\n");
    printf("Total tokens: %d\n", token_count);
    for (int i = 0; i < token_count && i < 20; ++i) {
        printf("[%02d] %s : '%s'\n", i, tokenTypeStrings[tokens[i].type], tokens[i].lexeme);
    }
    fflush(stdout);

    // Print the generated AST
    printf("\n--- Abstract Syntax Tree ---\n");
    printAST(root, 0);
    printf("---------------------------\n");

    // Write AST to JSON file for semantic analyzer
    writeAST_SExpr("D:\\Files\\School\\University\\3Y1S\\7. PPL\\Mini PL\\Omni\\ast.json", root);

    // Free all allocated memory
    freeAST(root);
    free(tokens); // Free the token array itself
    
    printf("\nAST memory freed. Program complete.\n");
    return 0;
}

/* =========================
   FUNCTION IMPLEMENTATIONS
   ========================= */

#pragma region Parser Helper Functions

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
    // if(type != peek().type){
    //     printf("Current token: %s | Previous token: %s | Expected Token: %s | Peeked Token: %s\n",
    //             tokens[curToken].lexeme, tokens[curToken-1].lexeme, tokenTypeStrings[type], tokenTypeStrings[peek().type]);
    // } 
    if(peek().type == type) return advance();
    char* errorMsg = malloc(256);
    snprintf(errorMsg, 256, "Syntax Error: Expected token type %s, but got %s",
            tokenTypeStrings[type], tokenTypeStrings[peek().type]);
    // For more complex error handling in the future
    // fprintf(errorMsg, "Expected token type %d, but got unexpected type %d", type, peek().type);
    reportSyntaxError(errorMsg);
    free((void*)errorMsg);

    return peek();
}

Token_Type revertToTokenType(const char* parsedType){
    int streamSize = sizeof(tokenTypeStrings)/sizeof(tokenTypeStrings[0]);
    for (int i = 0; i < streamSize; i++) {
        if (strcmp(tokenTypeStrings[i], parsedType) == 0) {
            return (Token_Type)i;
        }
    }
    return Token_Unknown;   // fallback if not found
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
    fprintf(stderr, "SYNTAX ERROR @ Line %d: %s\n", tokens[curToken].line_number, msg);
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

#pragma region Parser Functions

AST_Node* parse() {
    AST_Node* programBody = createBlockNode();
    while (!atEnd()) {
        addStatementToBlock(programBody, parseStatement());
    }
    expect(Token_CodeEnd);
    return programBody;
}

AST_Node* parseStatement(){
    // check for NULL from comment/newline before returning
    AST_Node* statementNode = NULL; 

    switch(tokens[curToken].type){
        case Token_Keyword_Int: case Token_Keyword_Decimal: case Token_Keyword_Char:
        case Token_Keyword_String: case Token_Keyword_Boolean:
            statementNode = parseDeclareAssign();
            break;
        case Token_Identifier:
            statementNode = parseAssign();
            break;
        case Token_Keyword_If:
            statementNode = parseIf();
            break;
        case Token_Keyword_For:
            statementNode = parseIterative();
            break;
        case Token_Keyword_Read:
            statementNode = parseRead();
            break;
        case Token_Keyword_Write:
            statementNode = parseWrite();
            break;
        case Token_Comment: case Token_Delim_Newline:
            advance();
            // statementNode remains NULL
            break;
        default:
            reportSyntaxError("Invalid Start of Statement (Might be caused by other syntax errors).");
            curToken++; // Try to recover by consuming token
            // statementNode remains NULL
            break;
    }
    return statementNode;
}

AST_Node* parseDeclareAssign(){
    Token typeToken;
    switch(tokens[curToken].type){
        case Token_Keyword_Int:
            typeToken = expect(Token_Keyword_Int);
            break;
        case Token_Keyword_Decimal:
            typeToken = expect(Token_Keyword_Decimal);
            break;
        case Token_Keyword_Char:
            typeToken = expect(Token_Keyword_Char);
            break;
        case Token_Keyword_String:
            typeToken = expect(Token_Keyword_String);
            break;
        case Token_Keyword_Boolean:
            typeToken = expect(Token_Keyword_Boolean);
            break;
        default:
            reportSyntaxError("Expected type keyword");
            curToken++;
            return NULL; // Return NULL on error
    }
    
    Token identifier = expect(Token_Identifier);
    AST_Node* expression = NULL; // Default to NULL (declaration only)

    if(peek().type == Token_Assignment_Operator){
        expect(Token_Assignment_Operator);
        expression = parseExpression(); // Parse the assigned value
    }
    
    handleComments();
    if(peek().type != Token_CodeEnd) expect(Token_Delim_Newline);

    return createDeclareAssignNode(typeToken, identifier, expression);
}

AST_Node* parseAssign(){
    Token identifier = expect(Token_Identifier);
    expect(Token_Assignment_Operator);
    AST_Node* expression = parseExpression();
    
    handleComments();
    if(peek().type != Token_CodeEnd) expect(Token_Delim_Newline);

    return createAssignNode(identifier, expression);
}

AST_Node* parseIf(){
    expect(Token_Keyword_If);
    expect(Token_Delim_LPAR);
    AST_Node* condition = parseExpression();
    expect(Token_Delim_RPAR);
    handleDo();
    expect(Token_Delim_LBRAC);
    handleComments();
    expect(Token_Delim_Newline);

    AST_Node* thenBranch = parseBlocks();
    expect(Token_Delim_RBRAC);
    if(peek().type != Token_CodeEnd) expect(Token_Delim_Newline);

    AST_Node* elseBranch = NULL;

    if (match(Token_Keyword_ElseIf)) {
        // An 'else if' is just an 'if' statement in the else branch
        // We "fake" the 'if' keyword for the parseIf function
        // This is a bit of a hack, let's parse it manually
        expect(Token_Delim_LPAR);
        AST_Node* elseIfCondition = parseExpression();
        expect(Token_Delim_RPAR);
        handleDo();
        expect(Token_Delim_LBRAC);
        handleComments();
        expect(Token_Delim_Newline);
        
        AST_Node* elseIfThen = parseBlocks();
        expect(Token_Delim_RBRAC);
        if(peek().type != Token_CodeEnd) expect(Token_Delim_Newline);
        //AST_Node* elseIfElse = NULL; // Start of the next chain

        // Manually build the nested if-statement
        AST_Node* elseIfNode = createIfStmtNode(elseIfCondition, elseIfThen, NULL);
        elseBranch = elseIfNode; // This is the 'else' branch of the *current* node

        AST_Node* current = elseIfNode; // Pointer to chain the 'else' branches

        // Loop for subsequent 'else if's
        while (match(Token_Keyword_ElseIf)) {
            expect(Token_Delim_LPAR);
            AST_Node* nextCond = parseExpression();
            expect(Token_Delim_RPAR);
            handleDo();
            expect(Token_Delim_LBRAC);
            handleComments();
            expect(Token_Delim_Newline);
            AST_Node* nextThen = parseBlocks();
            expect(Token_Delim_RBRAC);
            if(peek().type != Token_CodeEnd) expect(Token_Delim_Newline);
            
            AST_Node* nextIfNode = createIfStmtNode(nextCond, nextThen, NULL);
            current->data.ifStmt.elseBranch = nextIfNode;
            current = nextIfNode;
        }

        // After all 'else if's, check for a final 'else'
        if (match(Token_Keyword_Else)) {
            current->data.ifStmt.elseBranch = parseElse();
        }

    }
    else if (match(Token_Keyword_Else)) {
        elseBranch = parseElse();
    }
    
    // Return the HEAD of the if chain
    return createIfStmtNode(condition, thenBranch, elseBranch);
}

AST_Node* parseElse(){
    // 'Else' token was already consumed by match() in parseIf
    handleDo();
    expect(Token_Delim_LBRAC);
    handleComments();
    expect(Token_Delim_Newline);
    AST_Node* elseBlock = parseBlocks();
    expect(Token_Delim_RBRAC);
    if(peek().type != Token_CodeEnd) expect(Token_Delim_Newline);
    return elseBlock;
}

AST_Node* parseIterative(){
    expect(Token_Keyword_For);
    Token identifier = expect(Token_Identifier);
    expect(Token_Keyword_For_In);
    expect(Token_Keyword_For_Range);
    expect(Token_Delim_LPAR);

    // Create the FOR node *before* parsing its body
    // We will create the body (BlockNode) later
    AST_Node* forNode = createForStmtNode(identifier, NULL);
    
    // Parse first range argument
    addForRangeArg(forNode, parseExpression());

    while(match(Token_Delim_Comma)){
        addForRangeArg(forNode, parseExpression());
        if (forNode->data.forStmt.argCount > 3) {
            reportSyntaxError("Too many arguments for range()");
            // continue parsing anyway
        }
    }
    
    expect(Token_Delim_RPAR);
    handleDo();
    expect(Token_Delim_LBRAC);
    handleComments();
    expect(Token_Delim_Newline);
    
    // Now parse the body and attach it
    forNode->data.forStmt.body = parseBlocks();
    expect(Token_Delim_RBRAC);
    if(peek().type != Token_CodeEnd) expect(Token_Delim_Newline);
    
    return forNode;
}

AST_Node* parseBlocks(){
    AST_Node* blockNode = createBlockNode();

    while(1){
        switch(tokens[curToken].type){
            // Any token that can start a statement
            case Token_Keyword_If: case Token_Keyword_For: case Token_Keyword_Int: 
            case Token_Keyword_Decimal: case Token_Keyword_Char: case Token_Keyword_String: 
            case Token_Keyword_Boolean: case Token_Identifier:
            case Token_Keyword_Write: case Token_Keyword_Read:
                addStatementToBlock(blockNode, parseStatement());
                break;
            
            // Tokens that *end* a block
            case Token_Keyword_ElseIf: 
            case Token_Keyword_Else:
            case Token_Delim_RBRAC:
            case Token_CodeEnd: // End of file
                return blockNode;
            case Token_Delim_Newline: case Token_Comment:
                advance();
                break;
            default:{
                // We hit a token that doesn't start a statement AND doesn't end a block.
                // This is a syntax error.
                // printf("Parsed expression: %s\n", tokens[curToken-1].lexeme);
                // printf("Parsing next token: %s\n", tokens[curToken].lexeme);
                const char* errorMsg = malloc(256);
                snprintf((char*)errorMsg, 256, "Unexpected token '%s' encountered in block, expected a statement or block terminator",
                            tokens[curToken].lexeme);
                reportSyntaxError(errorMsg);
                free((void*)errorMsg);
                curToken++; // Consume to avoid infinite loop
                return blockNode; // Return what we have
            }
        }
    }
}

AST_Node* parseRead(){
    expect(Token_Keyword_Read);
    expect(Token_Delim_LPAR);
    Token identifier = expect(Token_Identifier);
    expect(Token_Delim_RPAR);
    handleComments();
    if(peek().type != Token_CodeEnd) expect(Token_Delim_Newline);
    
    return createReadStmtNode(identifier);
}

AST_Node* parseWrite(){
    expect(Token_Keyword_Write);
    expect(Token_Delim_LPAR);

    AST_Node* writeNode = createWriteStmtNode();
    
    // Parse first expression
    addWriteExpression(writeNode, parseExpression());

    while(match(Token_Delim_Comma)){
        addWriteExpression(writeNode, parseExpression());
    }
    
    expect(Token_Delim_RPAR);
    handleComments();
    if(peek().type != Token_CodeEnd) expect(Token_Delim_Newline);
        
    return writeNode;
}

AST_Node* parseExpression(){
    return parseOrExpression();
}

AST_Node* parseOrExpression(){
    AST_Node* left = parseAndExpression(); // Parse the left side

    while (match(Token_Boolean_Operator_OR)) {
        Token op = previous();
        AST_Node* right = parseAndExpression(); // Parse the right side
        left = createBinaryOpNode(op, left, right);
    }
    return left;
}

AST_Node* parseAndExpression(){
    AST_Node* left = parseEqualityExpression(); // Parse the left side

    while (match(Token_Boolean_Operator_AND)) {
        Token op = previous();
        AST_Node* right = parseEqualityExpression(); // Parse the right side
        left = createBinaryOpNode(op, left, right);
    }
    return left;
}

AST_Node* parseEqualityExpression(){
    AST_Node* left = parseRelationalExpression(); // Parse the left side

    while (peek().type == Token_Boolean_Operator && 
           (strcmp(peek().lexeme, "==") == 0 || 
            strcmp(peek().lexeme, "!=") == 0)) 
    {
        Token op = advance(); // Consume the '==' or '!='
        AST_Node* right = parseRelationalExpression(); // Parse the right side
        left = createBinaryOpNode(op, left, right);
    }
    return left;
}

AST_Node* parseRelationalExpression(){
    AST_Node* left = parseAddSubExpression(); // Parse the left side

    while (peek().type == Token_Boolean_Operator &&
           (strcmp(peek().lexeme, "<") == 0 || 
            strcmp(peek().lexeme, ">") == 0 ||
            strcmp(peek().lexeme, "<=") == 0 || 
            strcmp(peek().lexeme, ">=") == 0)) 
    {
        Token op = advance(); // Consume the operator
        AST_Node* right = parseAddSubExpression(); // Parse the right side
        left = createBinaryOpNode(op, left, right);
    }
    return left;
}

AST_Node* parseAddSubExpression(){
    AST_Node* left = parseMulDivExpression(); // Parse the left side

    while (peek().type == Token_Arithmetic_Operator &&
           (strcmp(peek().lexeme, "+") == 0 || 
            strcmp(peek().lexeme, "-") == 0)) 
    {
        Token op = advance(); // Consume the '+' or '-'
        AST_Node* right = parseMulDivExpression(); // Parse the right side
        left = createBinaryOpNode(op, left, right);
    }
    return left;
}

AST_Node* parseMulDivExpression(){
    AST_Node* left = parseExponentExpression(); // Parse the left side

    while ((peek().type == Token_Arithmetic_Operator &&
                (strcmp(peek().lexeme, "*") == 0 || 
                 strcmp(peek().lexeme, "/") == 0 || 
                 strcmp(peek().lexeme, "%") == 0)) ||
                 peek().type == Token_Arithmetic_Operator_DIV) 
    {
        Token op = advance(); // Consume the operator
        AST_Node* right = parseExponentExpression(); // Parse the right side
        left = createBinaryOpNode(op, left, right);
    }
    return left;
}

AST_Node* parseExponentExpression(){
    AST_Node* left = parseUnaryExpression(); // Parse the left base

    if (peek().type == Token_Arithmetic_Operator && strcmp(peek().lexeme, "^") == 0) {
        Token op = advance(); // Consume the '^'
        // Recursive call for right-associativity
        AST_Node* right = parseExponentExpression(); 
        left = createBinaryOpNode(op, left, right);
    }
    return left;
}

AST_Node* parseUnaryExpression(){
    if ((peek().type == Token_Boolean_Operator && strcmp(peek().lexeme, "!") == 0) ||
        (peek().type == Token_Arithmetic_Operator && strcmp(peek().lexeme, "-") == 0)) 
    {
        Token op = advance(); // Consume the '!' or '-'
        AST_Node* right = parseUnaryExpression(); // Recursive call
        return createUnaryOpNode(op, right);
    } else {
        return parseValueExpression(); // Base case
    }
}

AST_Node* parseValueExpression(){
    // Handle literals
    if (match(Token_Number) ||
        match(Token_Reserved_True) ||
        match(Token_Reserved_False) ||
        match(Token_String) ||
        match(Token_Character) ||
        match(Token_Builtin_Constant) || 
        match(Token_Reserved_Null)) {
        return createLiteralNode(previous()); // Use token we just consumed
    }

    // Handle identifier
    if (match(Token_Identifier)) {
        return createIdentifierNode(previous());
    }

    // Handle “(“ <expr> “)”
    if (match(Token_Delim_LPAR)) {
        AST_Node* expr = parseExpression(); // Recursively parse the expression inside
        expect(Token_Delim_RPAR);
        return expr;
    }

    // If we get here, no valid value was found
    reportSyntaxError("Expected a value (Literal, Identifier, or '(' )");
    return NULL; // Return NULL on error
}
#pragma endregion

#pragma region AST functions
// reusable create node function
AST_Node* createNode(NodeType type) {
    AST_Node* node = (AST_Node*)malloc(sizeof(AST_Node));
    if (!node) {
        fprintf(stderr, "AST: Failed to allocate node\n");
        exit(1);
    }
    node->type = type;
    return node;
}

//specific nodes

AST_Node* createLiteralNode(Token token) {
    AST_Node* node = createNode(NODE_LITERAL);
    node->data.literal.token = token;
    return node;
}

AST_Node* createIdentifierNode(Token token) {
    AST_Node* node = createNode(NODE_IDENTIFIER);
    node->data.identifier.token = token;
    return node;
}

AST_Node* createUnaryOpNode(Token op, AST_Node* right) {
    AST_Node* node = createNode(NODE_UNARY_OP);
    node->data.unaryOp.op = op;
    node->data.unaryOp.right = right;
    return node;
}

AST_Node* createBinaryOpNode(Token op, AST_Node* left, AST_Node* right) {
    AST_Node* node = createNode(NODE_BINARY_OP);
    node->data.binaryOp.op = op;
    node->data.binaryOp.left = left;
    node->data.binaryOp.right = right;
    return node;
}

AST_Node* createAssignNode(Token identifier, AST_Node* expression) {
    AST_Node* node = createNode(NODE_ASSIGN);
    node->data.assign.identifier = identifier;
    node->data.assign.expression = expression;
    return node;
}

AST_Node* createDeclareAssignNode(Token type, Token identifier, AST_Node* expression) {
    AST_Node* node = createNode(NODE_DECLARE_ASSIGN);
    node->data.declareAssign.type = type;
    node->data.declareAssign.identifier = identifier;
    node->data.declareAssign.expression = expression;
    return node;
}

AST_Node* createIfStmtNode(AST_Node* condition, AST_Node* thenBranch, AST_Node* elseBranch) {
    AST_Node* node = createNode(NODE_IF_STMT);
    node->data.ifStmt.condition = condition;
    node->data.ifStmt.thenBranch = thenBranch;
    node->data.ifStmt.elseBranch = elseBranch;
    return node;
}

AST_Node* createForStmtNode(Token identifier, AST_Node* body) {
    AST_Node* node = createNode(NODE_FOR_STMT);
    node->data.forStmt.identifier = identifier;
    node->data.forStmt.body = body;
    node->data.forStmt.argCount = 0;

    // start with capacity of 3 args
    node->data.forStmt.rangeArgs = (AST_Node**)malloc(sizeof(AST_Node*) * 3);
    if (!node->data.forStmt.rangeArgs) {
        fprintf(stderr, "AST: Failed to allocate FOR range args\n");
        exit(1);
    }
    return node;
}

//helper function for range function
void addForRangeArg(AST_Node* forNode, AST_Node* arg) {
    if (forNode->type != NODE_FOR_STMT || forNode->data.forStmt.argCount >= 3) {
        return; 
    }
    forNode->data.forStmt.rangeArgs[forNode->data.forStmt.argCount++] = arg;
}

AST_Node* createReadStmtNode(Token identifier) {
    AST_Node* node = createNode(NODE_READ_STMT);
    node->data.readStmt.identifier = identifier;
    return node;
}

AST_Node* createWriteStmtNode() {
    AST_Node* node = createNode(NODE_WRITE_STMT);
    node->data.writeStmt.argCount = 0;
    node->data.writeStmt.capacity = INITIAL_BLOCK_STATEMENTS;
    node->data.writeStmt.expressions = (AST_Node**)malloc(sizeof(AST_Node*) * node->data.writeStmt.capacity);
    if (!node->data.writeStmt.expressions) {
        fprintf(stderr, "AST: Failed to allocate WRITE expressions\n");
        exit(1);
    }
    return node;
}

void addWriteExpression(AST_Node* writeNode, AST_Node* expr) {
    if (writeNode->type != NODE_WRITE_STMT) return;
    WriteStmtNode* data = &writeNode->data.writeStmt;
    
    // resize if capacity is not enough
    if (data->argCount >= data->capacity) {
        data->capacity *= 2;
        data->expressions = (AST_Node**)realloc(data->expressions, sizeof(AST_Node*) * data->capacity);
        if (!data->expressions) {
            fprintf(stderr, "AST: Failed to reallocate WRITE expressions\n");
            exit(1);
        }
    }
    data->expressions[data->argCount++] = expr;
}

AST_Node* createBlockNode() {
    AST_Node* node = createNode(NODE_BLOCK);
    node->data.block.count = 0;
    node->data.block.capacity = INITIAL_BLOCK_STATEMENTS;
    node->data.block.statements = (AST_Node**)malloc(sizeof(AST_Node*) * node->data.block.capacity);
    if (!node->data.block.statements) {
        fprintf(stderr, "AST: Failed to allocate BLOCK statements\n");
        exit(1);
    }
    return node;
}

void addStatementToBlock(AST_Node* blockNode, AST_Node* statement) {
    if (blockNode->type != NODE_BLOCK || statement == NULL) return;
    BlockNode* data = &blockNode->data.block;

    // resize if capacity is not enough
    if (data->count >= data->capacity) {
        data->capacity *= 2;
        data->statements = (AST_Node**)realloc(data->statements, sizeof(AST_Node*) * data->capacity);
        if (!data->statements) {
            fprintf(stderr, "AST: Failed to reallocate BLOCK statements\n");
            exit(1);
        }
    }
    data->statements[data->count++] = statement;
}

#pragma endregion

#pragma region AST Terminal Printing Function
void printAST(AST_Node* node, int indent) {
    if (node == NULL) {
        printf("%*s(NULL)\n", indent, "");
        return;
    }

    printf("%*s", indent, "");

    switch (node->type) {
        case NODE_BLOCK:
            printf("[Block]\n");
            for (int i = 0; i < node->data.block.count; i++) {
                printAST(node->data.block.statements[i], indent + 2);
            }
            break;
        case NODE_LITERAL:
            printf("(Literal: %s)\n", node->data.literal.token.lexeme);
            break;
        case NODE_IDENTIFIER:
            printf("(Identifier: %s)\n", node->data.identifier.token.lexeme);
            break;
        case NODE_UNARY_OP:
            printf("(UnaryOp: %s)\n", node->data.unaryOp.op.lexeme);
            printAST(node->data.unaryOp.right, indent + 2);
            break;
        case NODE_BINARY_OP:
            printf("(BinaryOp: %s)\n", node->data.binaryOp.op.lexeme);
            printAST(node->data.binaryOp.left, indent + 2);
            printAST(node->data.binaryOp.right, indent + 2);
            break;
        case NODE_ASSIGN:
            printf("(Assign: %s)\n", node->data.assign.identifier.lexeme);
            printAST(node->data.assign.expression, indent + 2);
            break;
        case NODE_DECLARE_ASSIGN:
            printf("(Declare %s: %s)\n", 
                node->data.declareAssign.type.lexeme, 
                node->data.declareAssign.identifier.lexeme);
            if (node->data.declareAssign.expression) {
                printAST(node->data.declareAssign.expression, indent + 2);
            }
            break;
        case NODE_IF_STMT:
            printf("[If]\n");
            printf("%*s(Condition)\n", indent + 2, "");
            printAST(node->data.ifStmt.condition, indent + 4);
            printf("%*s(Then)\n", indent + 2, "");
            printAST(node->data.ifStmt.thenBranch, indent + 4);
            if (node->data.ifStmt.elseBranch) {
                printf("%*s(Else)\n", indent + 2, "");
                printAST(node->data.ifStmt.elseBranch, indent + 4);
            }
            break;
        case NODE_FOR_STMT:
            printf("[For %s in range(...)]\n", node->data.forStmt.identifier.lexeme);
            for(int i = 0; i < node->data.forStmt.argCount; i++) {
                printf("%*s(Range Arg %d)\n", indent + 2, "", i+1);
                printAST(node->data.forStmt.rangeArgs[i], indent + 4);
            }
            printf("%*s(Body)\n", indent + 2, "");
            printAST(node->data.forStmt.body, indent + 4);
            break;
        case NODE_READ_STMT:
            printf("(Read: %s)\n", node->data.readStmt.identifier.lexeme);
            break;
        case NODE_WRITE_STMT:
            printf("[Write]\n");
            for (int i = 0; i < node->data.writeStmt.argCount; i++) {
                printAST(node->data.writeStmt.expressions[i], indent + 2);
            }
            break;
        default:
            printf("(Unknown Node)\n");
            break;
    }
}

#pragma endregion

#pragma region AST Memory Freeing Function

void freeAST(AST_Node* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_UNARY_OP:
            freeAST(node->data.unaryOp.right);
            break;
        case NODE_BINARY_OP:
            freeAST(node->data.binaryOp.left);
            freeAST(node->data.binaryOp.right);
            break;
        case NODE_ASSIGN:
            freeAST(node->data.assign.expression);
            break;
        case NODE_DECLARE_ASSIGN:
            freeAST(node->data.declareAssign.expression); 
            break;
        case NODE_IF_STMT:
            freeAST(node->data.ifStmt.condition);
            freeAST(node->data.ifStmt.thenBranch);
            freeAST(node->data.ifStmt.elseBranch);
            break;
        case NODE_FOR_STMT:
            for (int i = 0; i < node->data.forStmt.argCount; i++) {
                freeAST(node->data.forStmt.rangeArgs[i]);
            }
            free(node->data.forStmt.rangeArgs);
            freeAST(node->data.forStmt.body);
            break;
        case NODE_WRITE_STMT:
            for (int i = 0; i < node->data.writeStmt.argCount; i++) {
                freeAST(node->data.writeStmt.expressions[i]);
            }
            free(node->data.writeStmt.expressions);
            break;
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++) {
                freeAST(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;
        
        case NODE_LITERAL:
        case NODE_IDENTIFIER:
        case NODE_READ_STMT:
            break;
    }
    
    // free the node 
    free(node);
}

#pragma endregion

#pragma region JSON Printing Function

static void writeNodeSExpr_internal(FILE* f, AST_Node* node);

static void writeNodeSExpr_internal(FILE* f, AST_Node* node) {
    if (node == NULL) {
        fprintf(f, "null");
        return;
    }

    switch (node->type) {
        //leaf nodes
        case NODE_LITERAL:
            fprintf(f, "%s", node->data.literal.token.lexeme);
            break;

        case NODE_IDENTIFIER:
            fprintf(f, "%s", node->data.identifier.token.lexeme);
            break;

        case NODE_BINARY_OP:
            fprintf(f, "(%s ", node->data.binaryOp.op.lexeme);
            
            fprintf(f, "("); // Wrap Left
            writeNodeSExpr_internal(f, node->data.binaryOp.left);
            fprintf(f, ") ("); // Close Left, Wrap Right
            writeNodeSExpr_internal(f, node->data.binaryOp.right);
            fprintf(f, "))"); // Close Right, Close Op
            break;

        case NODE_UNARY_OP:
            fprintf(f, "(%s ", node->data.unaryOp.op.lexeme);
            
            fprintf(f, "("); // Wrap Operand
            writeNodeSExpr_internal(f, node->data.unaryOp.right);
            fprintf(f, "))"); // Close Operand, Close Op
            break;

        case NODE_ASSIGN:
            fprintf(f, "(ASSIGN (%s) (", node->data.assign.identifier.lexeme);
            writeNodeSExpr_internal(f, node->data.assign.expression);
            fprintf(f, "))");
            break;

        case NODE_DECLARE_ASSIGN:
            fprintf(f, "(DECL (%s) (%s)", 
                node->data.declareAssign.type.lexeme,
                node->data.declareAssign.identifier.lexeme);
            
            if (node->data.declareAssign.expression) {
                fprintf(f, " ");
                writeNodeSExpr_internal(f, node->data.declareAssign.expression);
            }
            fprintf(f, ")");
            break;

        case NODE_IF_STMT:
            fprintf(f, "(IF_STMT ");
            writeNodeSExpr_internal(f, node->data.ifStmt.condition);
            fprintf(f, " ");
            
            //print Then block
            writeNodeSExpr_internal(f, node->data.ifStmt.thenBranch);
            
            //check if there is an Else or Else If
            if (node->data.ifStmt.elseBranch) {
                // Check type to decide the label
                if (node->data.ifStmt.elseBranch->type == NODE_IF_STMT) {
                    fprintf(f, " (ELSEIF "); //If elseif
                } else {
                    fprintf(f, " (ELSE ");   //else
                }
                
                writeNodeSExpr_internal(f, node->data.ifStmt.elseBranch);
                fprintf(f, ")"); // Close else/elseif tag
            }
            fprintf(f, ")");
            break;

        case NODE_FOR_STMT:
            fprintf(f, "(FOR_STMT %s (RANGE", node->data.forStmt.identifier.lexeme);
            for (int i = 0; i < node->data.forStmt.argCount; ++i) {
                fprintf(f, " ");
                writeNodeSExpr_internal(f, node->data.forStmt.rangeArgs[i]);
            }
            fprintf(f, ") ");
            writeNodeSExpr_internal(f, node->data.forStmt.body);
            fprintf(f, ")");
            break;

        case NODE_BLOCK:
            fprintf(f, "(BLOCK");
            for (int i = 0; i < node->data.block.count; ++i) {
                fprintf(f, " ");
                writeNodeSExpr_internal(f, node->data.block.statements[i]);
            }
            fprintf(f, ")");
            break;

        case NODE_READ_STMT:
            fprintf(f, "(READ (%s))", node->data.readStmt.identifier.lexeme);
            break;

        case NODE_WRITE_STMT:
            fprintf(f, "(WRITE");
            for (int i = 0; i < node->data.writeStmt.argCount; ++i) {
                fprintf(f, " ("); // Open wrapper for this argument
                writeNodeSExpr_internal(f, node->data.writeStmt.expressions[i]);
                fprintf(f, ")");  // Close wrapper
            }
            fprintf(f, ")");
            break;

        default:
            fprintf(f, "(UNKNOWN)");
            break;
    }
}

// Public Function to Write the File
void writeAST_SExpr(const char* filepath, AST_Node* root) {
    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Failed to open output file: %s\n", filepath);
        return;
    }

    // If root is a block, print each top-level statement on a new line
    // to keep the file clean and readable.
    if (root && root->type == NODE_BLOCK) {
        for (int i = 0; i < root->data.block.count; ++i) {
            writeNodeSExpr_internal(f, root->data.block.statements[i]);
            fprintf(f, "\n");
        }
    } else {
        writeNodeSExpr_internal(f, root);
        fprintf(f, "\n");
    }

    fclose(f);
    printf("Wrote AST to %s\n", filepath);
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

// Parse a line assumed to contain "LEXEME ... TOKEN_TYPE ... LINE_NUMBER"
static bool parseLineToLexemeAndType (char* line, char* out_lexeme, size_t lexeme_sz, char* out_type, size_t type_sz, int* out_line_number) {
    if(line == NULL) return false;
    
    // Make a copy so we can modify
    char buf[1024];
    strncpy(buf, line, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char* p = buf;
    p = trim(p);
    if(*p == '\0') return false;

    // Skip lines that are separators or headers
    if(strstr(p, "LEXEME") || strstr(p, "TOKEN_TYPE") || strstr(p, "LINE_NUMBER") || strstr(p, "----")) return false;

    // The format is: LEXEME (20 chars) TOKEN_TYPE (20 chars) LINE_NUMBER (remaining)
    // We need to parse from right to left to extract line_number, then token_type, then lexeme
    
    size_t len = strlen(p);
    
    // Find last non-whitespace character
    int i = (int)len - 1;
    while(i >= 0 && isspace((unsigned char)p[i])) i--;
    if(i < 0) return false;
    
    int end_line_num = i;
    
    // Move back to find start of line number (all digits)
    while(i >= 0 && isdigit((unsigned char)p[i])) i--;
    int start_line_num = i + 1;
    
    // Extract line number
    if(start_line_num > end_line_num) return false;
    char line_num_str[32];
    int line_num_len = end_line_num - start_line_num + 1;
    if(line_num_len <= 0 || line_num_len >= 32) return false;
    strncpy(line_num_str, p + start_line_num, line_num_len);
    line_num_str[line_num_len] = '\0';
    *out_line_number = atoi(line_num_str);
    
    // Now find token type (before line number)
    // Move past whitespace before line number
    i = start_line_num - 1;
    while(i >= 0 && isspace((unsigned char)p[i])) i--;
    if(i < 0) return false;
    
    int end_type = i;
    
    // Move back to find start of token type (non-whitespace)
    while(i >= 0 && !isspace((unsigned char)p[i])) i--;
    int start_type = i + 1;
    
    // Extract token type
    int type_len = end_type - start_type + 1;
    if(type_len <= 0 || type_len >= (int)type_sz) return false;
    strncpy(out_type, p + start_type, type_len);
    out_type[type_len] = '\0';
    
    // Now extract lexeme (everything before token type)
    p[start_type] = '\0';
    char* lex = trim(buf);
    if(lex == NULL || *lex == '\0') return false;

    // Handle special case for newline
    if(strcmp(lex, "\\n") == 0) {
        out_lexeme[0] = '\n';
        out_lexeme[1] = '\0';
    } else {
        strncpy(out_lexeme, lex, lexeme_sz - 1);
        out_lexeme[lexeme_sz - 1] = '\0';
    }

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
        int line_number = 0;

        if (!parseLineToLexemeAndType(line, lexeme, sizeof(lexeme), typeStr, sizeof(typeStr), &line_number)) {
            // If line did not parse into lexeme+type+line_number -> skip
            continue;
        }

        // Convert textual token type to Token_Type enum using your existing function
        Token_Type ttype = revertToTokenType(typeStr);

        // Add token to global array
        ensureTokenCapacity();
        tokens[token_count].type = ttype;
        tokens[token_count].line_number = line_number;
        
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
        tokens[token_count].line_number = (token_count > 0) ? tokens[token_count-1].line_number : 1;
        token_count++;
    }

    // Reset parser index
    curToken = 0;
    // Optional: debug print count
    printf("Loaded %d tokens from %s\n", token_count, filepath);
}
#pragma endregion
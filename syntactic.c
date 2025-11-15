#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

//Function Declarations
void parse_expression();
void parse_or_expr();
void parse_and_expr();
void parse_equality_expr();
void parse_relational_expr();
void parse_addsub_expr();
void parse_muldiv_expr();
void parse_exponent_expr();
void parse_unary_expr();
void parse_value_expr();

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

// Structure to hold one token
typedef struct {
    Token_Type type;
    const char* lexeme;
} Token;

Token* tokens;           // Pointer to the start of the token stream
int current_token = 0;   // Index of next token to be read

//--------------------Helper Functions--------------------

// Returns current token without consuming it
Token peek() {
    return tokens[current_token];
}

// Returns the previouus token
Token previous() {
    return tokens[current_token - 1];
}

// Checks if we are at the end of token stream
int is_at_end() {
    return tokens[current_token].type == Token_CodeEnd;
}

// Consumes current token, advancing the pointer
Token advance() {
    if (!is_at_end()) {
        current_token++;
    }
    return previous();
}

// Checks if current token matches the given type
int match(Token_Type type) {
    if (is_at_end() || peek().type != type) {
        return 0;
    }
    advance();
    return 1;
}

// Reports a syntax error and exits
void syntax_error(const char* message, const char* found_lexeme) {
    fprintf(stderr, "SYNTAX ERROR: %s. Found '%s' (token %d).\n",
            message, found_lexeme, current_token);
    exit(1);
}

// Demands the current token to be the expected type
Token expect(Token_Type type, const char* error_message) {
    if (peek().type == type) {
        return advance();
    }
    syntax_error(error_message, peek().lexeme);

    // Not functional to
    return peek();
}

//--------------------Main Logic Based on BNF--------------------

// <value_expr> → <digits> | <identifier> | <bool> | <constant> | “(“ <expr> “)”
void parse_value_expr() {
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
        parse_expression(); // Recursively parse the expression inside
        expect(Token_Delim_RPAR, "Expected ')' after expression in parentheses");
        return;
    }

    // If we get here, no valid value was found
    syntax_error("Expected a value (number, identifier, literal, or '(')", peek().lexeme);
}

// <unary_expr> → <value_expr> | (“-” | “!”) <unary_expr>    
void parse_unary_expr() {
    if ((peek().type == Token_Boolean_Operator && strcmp(peek().lexeme, "!") == 0) ||
        (peek().type == Token_Arithmetic_Operator && strcmp(peek().lexeme, "-") == 0)) 
    {
        advance(); // Consume the '!' or '-'
        parse_unary_expr(); // Recursive call for !!true / !!!!false
    } else {
        parse_value_expr(); // Base case
    }
}

// <exponent_expr> → <unary_expr> | <unary_expr> “^” <exponent_expr>	
void parse_exponent_expr() {
    parse_unary_expr(); // Parse the left base

    if (peek().type == Token_Arithmetic_Operator && strcmp(peek().lexeme, "^") == 0) {
        advance(); // Consume the '^'
        parse_exponent_expr(); // Recursive call for the right side (Exponent)
    }
}

// <muldiv_expr> → <exponent_expr> | <muldiv_expr> (“*” | “/” | “%” | “DIV”) <exponent_expr>
void parse_muldiv_expr() {
    parse_exponent_expr(); // Parse the left side

    // Use peek() to determine if we need to parse right side
    while ((peek().type == Token_Arithmetic_Operator &&
               (strcmp(peek().lexeme, "*") == 0 || 
                strcmp(peek().lexeme, "/") == 0 || 
                strcmp(peek().lexeme, "%") == 0)) ||
              peek().type == Token_Arithmetic_Operator_DIV) 
    {
        advance(); // Consume the operator
        parse_exponent_expr(); // Parse the right side
    }
}

// <addsub_expr> → <muldiv_expr>  | <addsub_expr> (“+” | “-”) <muldiv_expr>
void parse_addsub_expr() {
    parse_muldiv_expr(); // Parse the left side

    // Use peek() to determine if we need to parse right side
    while (peek().type == Token_Arithmetic_Operator &&
           (strcmp(peek().lexeme, "+") == 0 || 
            strcmp(peek().lexeme, "-") == 0)) 
    {
        advance(); // Consume the '+' or '-'
        parse_muldiv_expr(); // Parse the right side
    }
}

// <relational_expr> → <addsub_expr> | <relational_expr> ( “<” | “>” | “<=” | “>=”)  <addsub_expr>	
void parse_relational_expr() {
    parse_addsub_expr(); // Parse the left side

    // Use peek() to determine if we need to parse right side
    while (peek().type == Token_Boolean_Operator &&
           (strcmp(peek().lexeme, "<") == 0 || 
            strcmp(peek().lexeme, ">") == 0 ||
            strcmp(peek().lexeme, "<=") == 0 || 
            strcmp(peek().lexeme, ">=") == 0)) 
    {
        advance(); // Consume the operator
        parse_addsub_expr(); // Parse the right side
    }
}

// <equality_expr> → <relational_expr>  | <equality_expr> (“==” | “!=”) <relational_expr>
void parse_equality_expr() {
    parse_relational_expr(); // Parse the left side

    // Use peek() to determine if we need to parse right side
    while (peek().type == Token_Boolean_Operator && 
           (strcmp(peek().lexeme, "==") == 0 || 
            strcmp(peek().lexeme, "!=") == 0)) 
    {
        advance(); // Consume the '==' or '!='
        parse_relational_expr(); // Parse the right side
    }
}

// <and_expr> → <equality_expr> | <and_expr> “and” <equality_expr>
void parse_and_expr() {
    parse_equality_expr(); // Parse the left side

    // Use match() on AND to determine if we need to parse right hside
    while (match(Token_Boolean_Operator_AND)) {
        parse_equality_expr(); // Parse the right side
    }
}

// <or_expr> → <and_expr> | <or_expr> “or” <and_expr>
void parse_or_expr() {
    parse_and_expr(); // Parse the left side

    // Use match() on OR to determine if we need to parse right side
    while (match(Token_Boolean_Operator_OR)) {
        parse_and_expr(); // Parse the right side
    }
}

// <expr> → <or_expr>
void parse_expression() {
    parse_or_expr();
}

//--------------------FOR TESTING ONLY--------------------
void test(Token* test_stream, const char* test_name) {
    printf("--- Running Test: %s ---\n", test_name);

    // Point global tokens array to our test stream
    tokens = test_stream;
    current_token = 0; // Reset global token index

    // Run parser
    if (!is_at_end()) {
        parse_expression();
    }

    if (!is_at_end()) {
        // Post syntax error if additional tokens appear after a valid expression (this test is only for 1 line expressions)
        syntax_error("Unexpected token after valid expression", peek().lexeme);
    }

    printf("!!! Parsing successful! Reached end of tokens !!!\n\n");
}

int main(){
    Token test_case[] = {
            {Token_Boolean_Operator, "!"},
            {Token_Reserved_False, "false"},
            {Token_Boolean_Operator_OR, "or"},
            {Token_Delim_LPAR, "("},
            {Token_Identifier, "a"},
            {Token_Arithmetic_Operator, "+"},
            {Token_Number, "5"},
            {Token_Delim_RPAR, ")"},
            {Token_Arithmetic_Operator, "*"},
            {Token_Arithmetic_Operator, "-"}, // Unary minus
            {Token_Identifier, "c"},
            {Token_Arithmetic_Operator, "^"},
            {Token_Number, "2"},
            {Token_Boolean_Operator, ">="},
            {Token_Number, "10"},
            {Token_Boolean_Operator_AND, "and"},
            {Token_Number, "1"},
            {Token_Boolean_Operator, "=="},
            {Token_Number, "1"},
            {Token_CodeEnd, "EOF"} // Must end with CodeEnd
        };
        test(test_case, "Test 1");

    return 0;
}
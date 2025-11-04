#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LEXEME_LENGTH 256
#define KEYWORD_COUNT 16 // Total number of keywords; can be made dynamic if needed

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
    Token_Delim_LBRAK,
    Token_Delim_RBRAK,
    Token_Delim_Comma,
    Token_Delim_SQuote,
    Token_Delim_DQuote,
    Token_Delim_Period
} Token_Type; 

// Token Structure
typedef struct {
    Token_Type type;
    char lexeme[MAX_LEXEME_LENGTH];
    int line_number;
} Token;

// Keyword Structure
typedef struct{
    char* word;
    Token_Type type;
} Keyword;

// Lookup Table
Keyword keywords[] = {
    {"if", Token_Keyword_If},
    {"else", Token_Keyword_Else},
    {"elseif", Token_Keyword_ElseIf},
    {"for", Token_Keyword_For},
    {"int", Token_Keyword_Int},
    {"decimal", Token_Keyword_Decimal},
    {"char", Token_Keyword_Char},
    {"string", Token_Keyword_String},
    {"boolean", Token_Keyword_Boolean},
    {"read", Token_Keyword_Read},
    {"write", Token_Keyword_Write},
    {"true", Token_Reserved_True},
    {"false", Token_Reserved_False},
    {"null", Token_Reserved_Null},
    {"do", Token_Noise_Do},
    {"DIV", Token_Arithmetic_Operator_DIV},
    {"OR", Token_Boolean_Operator_OR},
    {"AND", Token_Boolean_Operator_AND}
};

//Automaton States
typedef enum{
    STATE_START,
    STATE_IN_IDENTIFIER,
    STATE_IN_NUMBER,
    STATE_IN_OPERATOR,
    STATE_IN_DELIMETER,
    STATE_IN_CHAR,
    STATE_IN_CHAR_EXPECT_CLOSE,
    STATE_IN_CHAR_ESCAPE,
    STATE_IN_STRING,
    STATE_IN_STRING_ESCAPE,
    STATE_IN_SINGLE_LINE_COMMENT,
    STATE_IN_BLOCK_COMMENT,
    STATE_DONE,
} AutomatonState;

// Function Prototypes
Token getNextToken(FILE* srcFile);
Token_Type getlexemeType(const char* lexeme);
void outputToken(Token token);

int main(){
    printf("Hello, world!\n");
    return 0;
}

Token getNextToken(FILE* srcFile){
    Token token;
    AutomatonState currentState = STATE_START;
    char ch; // Current character
    int lexemeIndex = 0; // Index for lexeme
    token.line_number = 1; // Initialize line number
    memset(token.lexeme, 0, MAX_LEXEME_LENGTH); // Clear lexeme buffer

    while((ch = fgetc(srcFile)) != EOF){
        switch(currentState){
            case STATE_START: // Start state
                if(isspace(ch)){
                    if(ch == '\n') token.line_number++;
                    continue; // Ignore whitespace
                }
                else token.lexeme[lexemeIndex++] = ch; // Add character to lexeme

                if(isalpha(ch) || ch == '_') currentState = STATE_IN_IDENTIFIER; // Identifier state
                else if(isdigit(ch)) currentState = STATE_IN_NUMBER; // Number state
                else if(ch == '"') currentState = STATE_IN_STRING; // String state
                else if(ch == '\'') currentState = STATE_IN_CHAR; // Character state
                else if(ch == '~'){ // Comment state
                    int next = fgetc(srcFile);
                    if(next == '/'){
                        token.lexeme[lexemeIndex++] = ch; // Add character to comment
                        token.lexeme[lexemeIndex++] = next; // Add character to comment
                        currentState = STATE_IN_BLOCK_COMMENT;
                    }
                    else{
                        token.lexeme[lexemeIndex++] = ch; // Add character to comment
                        ungetc(next, srcFile); // Put back the non-comment character
                        currentState = STATE_IN_SINGLE_LINE_COMMENT;
                    }
                }
                else if(strchr("+-*=%!<>", ch)) currentState = STATE_IN_OPERATOR; // Operator state
                else if(strchr("(){}[],.", ch)) currentState = STATE_IN_DELIMETER; // Delimeter state
                else {
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = getlexemeType(token.lexeme);
                    return token; // Return single character tokens
                }
                break;

            case STATE_IN_IDENTIFIER: // Identifier state
                if(isalnum(ch) || ch == '_'){
                    token.lexeme[lexemeIndex++] = ch;
                } else {
                    ungetc(ch, srcFile); // Put back the non-identifier character
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = getlexemeType(token.lexeme);
                    return token;
                }
                break;

            case STATE_IN_NUMBER: // Number state
                if(isdigit(ch) || ch == '.') token.lexeme[lexemeIndex++] = ch;
                else { // Not a number character
                    ungetc(ch, srcFile); // Put back the non-number character
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = Token_Number;
                    return token;
                }
                break;
            
            case STATE_IN_STRING:
                if(ch != '"'){
                    token.lexeme[lexemeIndex++] = ch; // Add character to string
                } 
                else { // End of string
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = Token_String;
                    return token;
                }
                break;

            case STATE_IN_CHAR:
                if(ch != '\''){
                    token.lexeme[lexemeIndex++] = ch; // Add character to char
                } 
                else { // End of character
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = Token_Character;
                    return token;
                }
                break;

            case STATE_IN_SINGLE_LINE_COMMENT:
                if (ch == '\n' || ch == EOF) {
                    currentState = STATE_DONE;
                    token.type = Token_Comment;
                    token.lexeme[lexemeIndex] = '\0';
                    return token;
                } 
                else token.lexeme[lexemeIndex++] = ch; // Add character to comment
                break;

            case STATE_IN_BLOCK_COMMENT: // Block comment state
                if(ch == '/') {
                    int next = fgetc(srcFile);
                    if(next == '~'){
                        token.lexeme[lexemeIndex++] = ch; // Add character to comment
                        token.lexeme[lexemeIndex++] = next; // Add character to comment
                        currentState = STATE_DONE;
                        token.type = Token_Comment;
                        token.lexeme[lexemeIndex] = '\0';
                        return token;
                    }
                    currentState = STATE_IN_BLOCK_COMMENT; // Stay in block comment
                    ungetc(next, srcFile); // Go back to previous character, not end of comment
                    token.lexeme[lexemeIndex++] = ch; // Add character to comment
                }
                else if (ch == EOF) {
                    currentState = STATE_DONE;
                    token.type = Token_Unknown;
                    strcpy(token.lexeme, "Unclosed block comment");
                    return token;
                }
                else if(ch == '\n') {
                    token.line_number++;
                    token.lexeme[lexemeIndex++] = ch; // Add character to comment
                }
                else {
                    token.lexeme[lexemeIndex++] = ch; // Add character to comment
                }
                break;

            case STATE_IN_OPERATOR: // Operator state (not implemented)
                // Handle multi-character operators here
                break;

            case STATE_IN_DELIMETER: // Delimeter state (not implemented)
                // Handle delimeters here
                break;

            case STATE_DONE: // End of file state
                token.type = Token_CodeEnd;
                return token;
            
            default:
                break;
        }
    }

    return token;
}
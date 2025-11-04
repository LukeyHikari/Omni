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
    STATE_IN_CHAR,
    STATE_IN_CHAR_EXPECT_CLOSE,
    STATE_IN_CHAR_ESCAPE,
    STATE_IN_STRING,
    STATE_IN_STRING_ESCAPE,
    STATE_IN_TILDE,
    STATE_IN_SINGLE_LINE_COMMENT,
    STATE_IN_BLOCK_COMMENT,
    STATE_IN_BLOCK_COMMENT_TILDE,
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
    int state = 0; // Start state
    AutomatonState currentState = STATE_START;
    char ch; // Current character
    int lexemeIndex = 0; // Index for lexeme
    token.line_number = 1; // Initialize line number
    memset(token.lexeme, 0, MAX_LEXEME_LENGTH); // Clear lexeme buffer

    while((ch = fgetc(srcFile)) != EOF){
        switch(state){
            case 0: // Start state
                if(isspace(ch)){
                    if(ch == '\n') token.line_number++;
                    continue; // Ignore whitespace
                }
                else token.lexeme[lexemeIndex++] = ch; // Add character to lexeme

                if(isalpha(ch) || ch == '_') state = 1; // Identifier state
                else if(isdigit(ch)) state = 2; // Number state
                else if(ch == '~') state = 3; // Comment state
                else if(ch == '"') state = 4; // String state
                else if(ch == '\'') state = 5; // Character state
                //else if(strchr("+-*=%!<>", ch)) state = 6; // Operator state
                else if(strchr("(){}[],.;", ch)) state = 7; // Delimeter state
                else {
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = getlexemeType(token.lexeme);
                    return token; // Return single character tokens
                }
                break;

            case 1: // Identifier state
                if(isalnum(ch) || ch == '_'){
                    token.lexeme[lexemeIndex++] = ch;
                } else {
                    ungetc(ch, srcFile); // Put back the non-identifier character
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = getlexemeType(token.lexeme);
                    return token;
                }
                break;

            case 2: // Number state
                if(isdigit(ch) || ch == '.') token.lexeme[lexemeIndex++] = ch;
                else { // Not a number character
                    ungetc(ch, srcFile); // Put back the non-number character
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = Token_Number;
                    return token;
                }
                break;
            
            case 3: // Comment state (not implemented)
                // Handle comments  here
                break;

            /*case STATE_IN_TILDE:
                if(currentChar == '/'){
                    lexemeBuffer[lexemeIndex++] = getChar();
                    currentState = STATE_IN_BLOCK_COMMENT;
                }
                else{
                    currentState = STATE_IN_SINGLE_LINE_COMMENT;
                }
                break;
            
            case STATE_IN_SINGLE_LINE_COMMENT:
                if (currentChar == '\n' || currentChar == '\0') {
                    currentState = STATE_DONE;
                    return createToken(Token_Single_Line_Comment, lexemeBuffer, lexemeIndex);
                }
                else {
                    lexemeBuffer[lexemeIndex++] = getChar();
                }
                break;

            case STATE_IN_BLOCK_COMMENT:
                if(currentChar == '/') {
                    lexemeBuffer[lexemeIndex++] = getChar();
                    currentState = STATE_IN_BLOCK_COMMENT_TILDE;
                }
                else if (currentChar == '\0') {
                    currentState = STATE_DONE;
                    return createToken(Token_Unknown, "Unclosed block comment", 22);
                }
                else {
                    lexemeBuffer[lexemeIndex++] = getChar();
    
                }
                break;
                
            case STATE_IN_BLOCK_COMMENT_TILDE:
                if (currentChar == '~') {
                    lexemeBuffer[lexemeIndex++] = getChar(); 
                    currentState = STATE_DONE;
                    return createToken(Token_Block_Comment, lexemeBuffer, lexemeIndex);
                }
                else if (currentChar == '\0') {
                    currentState = STATE_DONE;
                    return createToken(Token_Unknown, "Unclosed block comment", 22);
                }
                else {
                    currentState = STATE_IN_BLOCK_COMMENT;
                }
                break;*/

            case 4: // String state (not implemented)
                if(ch != '"'){
                    token.lexeme[lexemeIndex++] = ch; // Add character to string
                } 
                else { // End of string
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = Token_String;
                    return token;
                }
                break;

            case 5: // Character state (not implemented)
                if(ch != '\''){
                    token.lexeme[lexemeIndex++] = ch; // Add character to char
                } 
                else { // End of character
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = Token_Character;
                    return token;
                }
                break;

            case 6: // Operator state (not implemented)
                // Handle multi-character operators here
                break;

            case 7: // Delimeter state (not implemented)
                // Handle delimeters here
                break;

            case 8: // End of file state
                token.type = Token_CodeEnd;
                return token;
            
            default:
                break;
        }
    }

    return token;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LEXEME_LENGTH 256
#define KEYWORD_COUNT (sizeof(keywords) / sizeof(Keyword)) 

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

// Lookup Table for keywords
Keyword keywords[] = {
    {"if", Token_Keyword_If},
    {"else", Token_Keyword_Else},
    {"elseif", Token_Keyword_ElseIf},
    {"for", Token_Keyword_For},
    {"in", Token_Keyword_For_In},
    {"range", Token_Keyword_For_Range},
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
    {"or", Token_Boolean_Operator_OR},
    {"and", Token_Boolean_Operator_AND}
};

//Automaton States
typedef enum{
    STATE_START,
    STATE_IN_IDENTIFIER,
    STATE_IN_NUMBER,
    STATE_IN_OPERATOR,
    STATE_IN_DELIMETER,
    STATE_IN_CHAR,
    STATE_IN_STRING,
    STATE_IN_STRING_ESCAPE,
    STATE_IN_SINGLE_LINE_COMMENT,
    STATE_IN_BLOCK_COMMENT,
    STATE_IN_EQUAL,
    STATE_IN_BOOL_OPERATOR,
    STATE_DONE,
} AutomatonState;

// Lookup Table for string representation of token types; PURELY FOR DEBUGGING AND OUTPUT PURPOSES
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
    "Delim_LBRAK",
    "Delim_RBRAK",
    "Delim_Comma",
    "Delim_SQuote",
    "Delim_DQuote",
    "Delim_Period"
};

// Function Prototypes
Token getNextToken(FILE* srcFile);
Token_Type getlexemeType(const char* lexeme);
const char* outputToken(Token token);
static bool hasOmniExtension (const char* filename);
static FILE* openOmniFile (const char* filename);
static void errorAndExit(const char* message);

int main(int argc, char *argv[]) {
    // Expect filename argument; terminal usage: ./lexical_analyzer <file.omni>
    if (argc < 2) {
        printf("Usage: %s <file.omni>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    FILE* src = openOmniFile(filename);  //Handles both checking + opening of the file
    printf("Processing file: %s\n\n", filename);

    // Open the output file
    FILE* outFile = fopen("symbol_table.txt", "w");
    if (outFile == NULL) {
    perror("Error opening symbol_table.txt");
    fclose(src);
    return EXIT_FAILURE;
    }

    // Write a header to the symbol table
    fprintf(outFile, "%-20s %-30s %s\n", "LEXEME", "TOKEN_TYPE", "LINE");
    fprintf(outFile, "------------------------------------------------------------\n");

    Token token;
    int token_count = 0;

    do {
        token = getNextToken(src);

        //Stop on EOF or unknown token
        if (token.type == Token_CodeEnd || token.type == Token_Unknown)
            break;

        printf("%s\n", outputToken(token));
        fprintf(outFile, "%s\n", outputToken(token)); // Write to symbol table file
        token_count++;

    } while (1);

    // Show unknown token error if encountered
    if (token.type == Token_Unknown) printf("\nEncountered unknown token: %s\n", token.lexeme);

    // Summary
    printf("\nProcessed %d tokens\n", token_count);

    fclose(src);

    // Return success if no unknown tokens were found
    return (token.type == Token_Unknown) ? EXIT_FAILURE : EXIT_SUCCESS;
}

Token getNextToken(FILE* srcFile){
    Token token;
    AutomatonState currentState = STATE_START;
    char ch; // Current character
    int lexemeIndex = 0; // Index for lexeme
    bool decimalPointEncountered = false; // Flag to track if a decimal point has been encountered in a number
    token.line_number = 1; // Initialize line number
    memset(token.lexeme, 0, MAX_LEXEME_LENGTH); // Clear lexeme buffer

    while((ch = fgetc(srcFile)) != EOF){
        switch(currentState){
            #pragma region Start State
            case STATE_START: // Start state
                if(isspace(ch)){
                    if(ch == '\n') token.line_number++;
                    continue; // Ignore whitespace
                }
                else token.lexeme[lexemeIndex++] = ch; // Add character to lexeme

                if(isalpha(ch) || ch == '_') currentState = STATE_IN_IDENTIFIER;
                else if(isdigit(ch)) currentState = STATE_IN_NUMBER;
                else if(ch == '"') currentState = STATE_IN_STRING;
                else if(ch == '\'') currentState = STATE_IN_CHAR;
                else if(ch == '~'){ // Comment state
                    int next = fgetc(srcFile);
                    if(next == '/'){
                        token.lexeme[lexemeIndex++] = ch; // Add character to comment
                        token.lexeme[lexemeIndex++] = next; // Add character to comment
                        currentState = STATE_IN_BLOCK_COMMENT;
                    }
                    else{
                        token.lexeme[lexemeIndex++] = ch; // Add character to comment
                        ungetc(next, srcFile); // Put back the non-comment character; still a comment, but not part of the operator
                        currentState = STATE_IN_SINGLE_LINE_COMMENT;
                    }
                }
                else if(strchr("+-*=%!<>", ch)) currentState = STATE_IN_OPERATOR;
                else if(strchr("(){}[],.", ch)) currentState = STATE_IN_DELIMETER;
                else {
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = Token_Unknown;
                    return token;
                }
                break;
            #pragma endregion

            #pragma region Identifier State
            case STATE_IN_IDENTIFIER: // Identifier state
                if(isalnum(ch) || ch == '_'){
                    token.lexeme[lexemeIndex++] = ch;
                }
                else {
                    ungetc(ch, srcFile); // Put back the non-identifier character
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = getlexemeType(token.lexeme);
                    return token;
                }
                break;
            #pragma endregion
            
            #pragma region Number State
            case STATE_IN_NUMBER: // Number state
                if(isdigit(ch)) token.lexeme[lexemeIndex++] = ch;
                else if(ch == '.' && !decimalPointEncountered){ // Handle decimal point
                    decimalPointEncountered = true; // Mark that a dot has been encountered
                    token.lexeme[lexemeIndex++] = ch;
                }
                else { // Not a number character
                    ungetc(ch, srcFile); // Put back the non-number character
                    token.lexeme[lexemeIndex] = '\0';
                    token.type = Token_Number;
                    return token;
                }
                break;
            #pragma endregion
            
            #pragma region String and Char States
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
            #pragma endregion
            
            #pragma region Comment States
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
                    if(next == EOF){
                        currentState = STATE_DONE;
                        token.type = Token_Unknown;
                        strcpy(token.lexeme, "Unclosed block comment");
                        return token;
                    }
                    if(next == '~'){
                        token.lexeme[lexemeIndex++] = ch; // Add character to comment
                        token.lexeme[lexemeIndex++] = next; // Add character to comment
                        currentState = STATE_DONE;
                        token.type = Token_Comment;
                        token.lexeme[lexemeIndex] = '\0';
                        return token;
                    }
                    ungetc(next, srcFile); // Go back to previous character, not end of comment
                    token.lexeme[lexemeIndex++] = ch; // Add / to comment if not end of comment 
                }
                else{
                    if(ch == '\n') token.line_number++; // Add line number for new lines in comments
                    token.lexeme[lexemeIndex++] = ch; // Add character to comment
                }
                break;
            #pragma endregion

            #pragma region Delimeter State
            case STATE_IN_DELIMETER: // Delimeter state 
                token.lexeme[lexemeIndex] = '\0';
                token.type = getlexemeType(token.lexeme);
                return token;
            #pragma endregion

            #pragma region Operator States
            case STATE_IN_OPERATOR:
                switch(ch){
                    case '+':
                    case '-':
                    case '*':
                    case '%':
                    case '/':
                    case '^':
                        token.lexeme[lexemeIndex++] = ch;
                        token.lexeme[lexemeIndex] = '\0';
                        token.type = Token_Arithmetic_Operator;
                        return token;
                    case '=':
                        currentState = STATE_IN_EQUAL;
                        token.lexeme[lexemeIndex++] = ch;
                        break;
                    case '!':
                    case '<':
                    case '>':
                        currentState = STATE_IN_BOOL_OPERATOR;
                        token.lexeme[lexemeIndex++] = ch;
                        break;
                    default:
                    break;
                }
            break;

            case STATE_IN_EQUAL:
                switch(ch){
                    case '=':
                        token.lexeme[lexemeIndex++] = ch;
                        token.lexeme[lexemeIndex] = '\0';
                        token.type = Token_Boolean_Operator;
                        return token;
                    default:
                        ungetc(ch, srcFile); // Put back the non-equal character
                        token.lexeme[lexemeIndex] = '\0';
                        token.type = getlexemeType(token.lexeme);
                        return token;
                }
                break;
            
            case STATE_IN_BOOL_OPERATOR:
                switch(ch){
                    case '=':
                        token.lexeme[lexemeIndex++] = ch;
                        token.lexeme[lexemeIndex] = '\0';
                        token.type = Token_Boolean_Operator;
                        return token;
                    default:
                        ungetc(ch, srcFile); // Put back the non-operator character
                        token.lexeme[lexemeIndex] = '\0';
                        token.type = getlexemeType(token.lexeme);
                        return token;
                }
                break;
            #pragma endregion

            case STATE_DONE: // End of file state
                token.type = Token_CodeEnd;
                strcpy(token.lexeme, "EOF");
                return token;
            
            default:
                break;
        }
    }

    return token;
}

Token_Type getlexemeType(const char* lexeme){
    // Check if lexeme is a keyword; STATE_IN_IDENTIFIER only
    for(int i = 0; i < KEYWORD_COUNT; i++){
        if(strcmp(lexeme, keywords[i].word) == 0){
            return keywords[i].type;
        }
    }

    // Check for single character delimeters and operators
    switch(lexeme[0]){
        case '(': return Token_Delim_LPAR;
        case ')': return Token_Delim_RPAR;
        case '{': return Token_Delim_LBRAC;
        case '}': return Token_Delim_RBRAC;
        case '[': return Token_Delim_LBRAK;
        case ']': return Token_Delim_RBRAK;
        case ',': return Token_Delim_Comma;
        case '\'': return Token_Delim_SQuote;
        case '"': return Token_Delim_DQuote;
        case '.': return Token_Delim_Period;
        case '+': return Token_Arithmetic_Operator;
        case '*': return Token_Arithmetic_Operator;
        case '/': return Token_Arithmetic_Operator;
        case '-': return Token_Arithmetic_Operator;
        case '^': return Token_Arithmetic_Operator;
        case '%': return Token_Arithmetic_Operator;
        case '=': return Token_Assignment_Operator;
        case '!': return Token_Boolean_Operator;
        case '<': return Token_Boolean_Operator;
        case '>': return Token_Boolean_Operator;
        default: break;
    }
    
    return Token_Identifier; // Default to identifier
}

const char* outputToken(Token token){
    static char output[512];
    snprintf(output, sizeof(output), "'%s'\t%s\t%d", 
             token.lexeme, tokenTypeStrings[token.type], token.line_number);

    /* Return the formatted string
    Example Output:
    'if'    Keyword_If    3

    USE DIRECTLY FOR PRINTING OR WHEN OUTPUTING TO SYMBOL TABLE
    */
    return output;
}

static bool hasOmniExtension (const char* filename){
    // Check if a filename is provided
    if (filename == NULL) return false;

    // Get the file extension
    const char* ext = strrchr (filename, '.');

    // Checking for char length of ".omni"
    if (strlen(ext) != 5) return false; 

    // Compare the extension with ".omni"
    if (ext != NULL && strcmp (ext, ".omni") == 0) return true;     
    else return false;  
}

//Display error message and exit
static void errorAndExit(const char* message) {
    fprintf(stderr, "\n**************************************************\n");
    fprintf(stderr, "*** ERROR: %s\n", message);
    fprintf(stderr, "*** This program only accepts files with the '.omni' extension.\n");
    fprintf(stderr, "**************************************************\n");
    exit(EXIT_FAILURE);
}

//Open the .omni file or show error and exit
static FILE* openOmniFile (const char* filename) {
    //Verify file extension
    if (!hasOmniExtension(filename)) errorAndExit("Invalid file type");

    //Try opening the file for reading (binary mode)
    FILE* file = fopen(filename, "rb"); 
    if (!file) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Failed to open the file: %s", filename);
        errorAndExit(buf);
    }
    return file; //Return the opened file pointer
}
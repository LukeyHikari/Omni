#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#define MAX_LEXEME_LENGTH 256

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


typedef struct {
    Token_Type type;
    char lexeme[MAX_LEXEME_LENGTH];
} Token;

Token* tokens = NULL; 
int curToken = 0;    
int token_count = 0;
int token_capacity = 0;

#define INITIAL_TOKEN_CAPACITY 2048

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

// --- Stub Functions (to make it runnable) ---
Token_Type revertToTokenType(const char* typeStr) {
    if (strcmp(typeStr, "Identifier") == 0) return Token_Identifier;
    if (strcmp(typeStr, "Character") == 0) return Token_Character;
    if (strcmp(typeStr, "String") == 0) return Token_String;
    if (strcmp(typeStr, "Number") == 0) return Token_Number;
    if (strcmp(typeStr, "CodeEnd") == 0) return Token_CodeEnd;
    if (strcmp(typeStr, "Comment") == 0) return Token_Comment;
    if (strcmp(typeStr, "Arithmetic_Operator") == 0) return Token_Arithmetic_Operator;
    if (strcmp(typeStr, "Arithmetic_Operator_DIV") == 0) return Token_Arithmetic_Operator_DIV;
    if (strcmp(typeStr, "Boolean_Operator") == 0) return Token_Boolean_Operator;
    if (strcmp(typeStr, "Boolean_Operator_AND") == 0) return Token_Boolean_Operator_AND;
    if (strcmp(typeStr, "Boolean_Operator_OR") == 0) return Token_Boolean_Operator_OR;
    if (strcmp(typeStr, "Assignment_Operator") == 0) return Token_Assignment_Operator;
    if (strcmp(typeStr, "Builtin_Constant") == 0) return Token_Builtin_Constant;
    if (strcmp(typeStr, "Keyword_If") == 0) return Token_Keyword_If;
    if (strcmp(typeStr, "Keyword_Else") == 0) return Token_Keyword_Else;
    if (strcmp(typeStr, "Keyword_ElseIf") == 0) return Token_Keyword_ElseIf;
    if (strcmp(typeStr, "Keyword_For") == 0) return Token_Keyword_For;
    if (strcmp(typeStr, "Keyword_For_In") == 0) return Token_Keyword_For_In;
    if (strcmp(typeStr, "Keyword_For_Range") == 0) return Token_Keyword_For_Range;
    if (strcmp(typeStr, "Keyword_Int") == 0) return Token_Keyword_Int;
    if (strcmp(typeStr, "Keyword_Decimal") == 0) return Token_Keyword_Decimal;
    if (strcmp(typeStr, "Keyword_Char") == 0) return Token_Keyword_Char;
    if (strcmp(typeStr, "Keyword_String") == 0) return Token_Keyword_String;
    if (strcmp(typeStr, "Keyword_Boolean") == 0) return Token_Keyword_Boolean;
    if (strcmp(typeStr, "Keyword_Read") == 0) return Token_Keyword_Read;
    if (strcmp(typeStr, "Keyword_Write") == 0) return Token_Keyword_Write;
    if (strcmp(typeStr, "Reserved_True") == 0) return Token_Reserved_True;
    if (strcmp(typeStr, "Reserved_False") == 0) return Token_Reserved_False;
    if (strcmp(typeStr, "Reserved_Null") == 0) return Token_Reserved_Null;
    if (strcmp(typeStr, "Noise_Do") == 0) return Token_Noise_Do;
    if (strcmp(typeStr, "Delim_LPAR") == 0) return Token_Delim_LPAR;
    if (strcmp(typeStr, "Delim_RPAR") == 0) return Token_Delim_RPAR;
    if (strcmp(typeStr, "Delim_LBRAC") == 0) return Token_Delim_LBRAC;
    if (strcmp(typeStr, "Delim_RBRAC") == 0) return Token_Delim_RBRAC;
    if (strcmp(typeStr, "Delim_Comma") == 0) return Token_Delim_Comma;
    if (strcmp(typeStr, "Delim_SQuote") == 0) return Token_Delim_SQuote;
    if (strcmp(typeStr, "Delim_DQuote") == 0) return Token_Delim_DQuote;
    if (strcmp(typeStr, "Delim_Period") == 0) return Token_Delim_Period;
    if (strcmp(typeStr, "Delim_Newline") == 0) return Token_Delim_Newline;
    if (strcmp(typeStr, "Delim_Space") == 0) return Token_Delim_Space;

    // Default for unknown
    return Token_Unknown;
}

// STUB: Your parser would start here
void parseProgram() {
    printf("--- parseProgram() called. Starting parse. ---\n");
    // Your parser logic would go here, using tokens[curToken]...
    printf("--- parseProgram() finished. ---\n");
}


// --- Your Main Loader Function (with DEBUGGER print added) ---

void loadTokensFromSymbolTable(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Unable to open symbol table file: %s\n", filepath);
        fprintf(stderr, "Please make sure 'symbol_table.txt' is in the same directory as the executable.\n");
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

        // --- DEBUGGER PRINT ---
        // This line shows every token as it is added to the global array
        printf("DEBUG: Added token [Index %d]: Lexeme='%s', Type='%s' (Enum: %d)\n",
               token_count,
               tokens[token_count].lexeme,
               typeStr,
               ttype);
        // --- END DEBUGGER ---

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
        
        // --- DEBUGGER PRINT for EOF ---
        printf("DEBUG: Added token [Index %d]: Lexeme='%s', Type='CodeEnd' (Enum: %d)\n",
               token_count,
               tokens[token_count].lexeme,
               (int)Token_CodeEnd);
        // --- END DEBUGGER ---

        token_count++;
    }

    // Reset parser index
    curToken = 0;
    // Optional: debug print count
    printf("\n--- Summary --- \nLoaded %d total tokens from %s\n\n", token_count, filepath);
}

// --- Your Main Function (modified file path) ---

int main() {
    // Allocate tokens pointer and load from uploaded symbol table
    // We look for the file in the *same directory* as the executable.
    loadTokensFromSymbolTable("symbol_table.txt"); 

    // Call parser functions
    parseProgram();

    printf("Parsing finished.\n");
    
    // Clean up allocated memory
    free(tokens);
    tokens = NULL;
    
    return 0;
}

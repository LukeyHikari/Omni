#ifndef LEXICAL_H
#define LEXICAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Forward declaration for symbol table entry
typedef struct {
    char lexeme[256];
    char token_type[50];
    int line_number;
} SymbolTableEntry;

// Function prototypes
bool writeSymbolTable(const char* filename, FILE* outFile);
bool runSyntacticAnalysis(void);
void cleanupSymbolTable(void);

// Global symbol table buffer
#define MAX_SYMBOL_TABLE_ENTRIES 10000
static SymbolTableEntry symbolTable[MAX_SYMBOL_TABLE_ENTRIES];
static int symbolTableCount = 0;

// Add entry to symbol table
static inline void addSymbolTableEntry(const char* lexeme, const char* token_type, int line_number) {
    if (symbolTableCount < MAX_SYMBOL_TABLE_ENTRIES) {
        strncpy(symbolTable[symbolTableCount].lexeme, lexeme, 255);
        symbolTable[symbolTableCount].lexeme[255] = '\0';
        strncpy(symbolTable[symbolTableCount].token_type, token_type, 49);
        symbolTable[symbolTableCount].token_type[49] = '\0';
        symbolTable[symbolTableCount].line_number = line_number;
        symbolTableCount++;
    }
}

// Write symbol table to file
bool writeSymbolTable(const char* filename, FILE* outFile) {
    if (outFile == NULL) {
        perror("Error: Symbol table file not open");
        return false;
    }

    fprintf(outFile, "%-20s %-20s %-10s\n", "LEXEME", "TOKEN_TYPE", "LINE_NUMBER");
    fprintf(outFile, "------------------------------------------------------------\n");

    for (int i = 0; i < symbolTableCount; i++) {
        fprintf(outFile, "%-20s %-20s %-20d\n",
                symbolTable[i].lexeme,
                symbolTable[i].token_type,
                symbolTable[i].line_number);
    }

    fflush(outFile);
    return true;
}

// Run syntactic analysis (calls ASTSyntactic.c)
bool runSyntacticAnalysis(void) {
    printf("\n*** Starting Syntactic Analysis ***\n");
    
    char command[512];
    // Pass the symbol table file as argument to ASTSyntactic
    snprintf(command, sizeof(command), ".\\ASTSyntactic.exe \"symbol_table.txt\"");
    
    int result = system(command);
    
    if (result == 0) {
        printf("*** Syntactic Analysis Completed Successfully ***\n");
        return true;
    } else {
        printf("*** Syntactic Analysis Failed (Exit Code: %d) ***\n", result);
        return false;
    }
}

// Cleanup
void cleanupSymbolTable(void) {
    symbolTableCount = 0;
}

#endif // LEXICAL_H
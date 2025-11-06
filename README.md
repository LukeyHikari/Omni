# **Omni**
OMNI, short for One Machine, Numerous Ideas, is a prefix meaning “all” which represents the essence of our proposed programming language, a unified system that merges the best syntactical features of C, Python, and Java. It is designed to promote clarity, flexibility, and collaboration in coding, making it suitable for both beginners and experienced programmers. 

## **Omni — Lexical Analyzer**
This project is a small lexical analyzer for the Omni language. 

## **How it Works**
It opens `.omni` source files, tokenizes the input, prints each token (lexeme, token type, line number), and exits when it reaches EOF or when an unknown token is encountered.

## **Requirements**
- C compiler (GCC recommended / MinGW on Windows).
- Tested on Windows PowerShell with CRLF-safe handling.

## **File Loading Rules**
- Only files with the `.omni` extension are accepted.
- If you pass a file with a different extension the program will show an error and exit.


## **Comment Syntax**
- Single-line comment: begins with a single tilde `~` and continues to end-of-line.
```powershell
~ This is a single-line comment
```
- Block (multiple-line) comment: starts with `~/` and ends with `/~`.
```powershell
~/
This is a multiple-line comment
that can span multiple lines
/~
```

## **Token Behavior**
- Each token printed includes: lexeme, token type, and line number.
- The lexer continues calling `getNextToken()` until:
	- `Token_CodeEnd` (EOF) — normal termination
	- `Token_Unknown`— stops and prints the offending lexeme (error exit)
- *Common tokens*: identifiers, numbers, strings, operators, delimiters, comments, keywords (see source for the full list).


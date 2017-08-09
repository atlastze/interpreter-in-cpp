/* Copyright (C) 2017, kylinsage <kylinsage@gmail.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /***********************************************************
  * Standalone parser for bellowing syntax:
  * expression ::= number ( ( '+' | '-' ) number )*
  * number ::= ( integer ( '.' integer? )? | '.' integer ) ( ( 'e' | 'E' ) ( '+' | '-' )? integer )?
  * integer ::= ( '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' )+
  *
  * Compile: gcc parser.c
  * Run: ./a.out filename
  **********************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/***********************************************************
 * CharStream -- characters stream
 **********************************************************/
struct CharStream {
	FILE *fp;		/* source file */
	int row;		/* current line number */
	int column;		/* current line position, start from 1 */
	char ch;		/* current character */
};

/* Initialize characters stream */
void charstream_init(struct CharStream *charStream, FILE * fp)
{
	charStream->fp = fp;
	charStream->row = 1;
	charStream->column = 1;
	charStream->ch = fgetc(charStream->fp);
}

/* Return the source character at the current position. */
char charstream_current_char(struct CharStream *charStream)
{
	return charStream->ch;
}

/* Consume the current source character and return the next character. */
char charstream_next_char(struct CharStream *charStream)
{
	charStream->ch = fgetc(charStream->fp);
	charStream->column++;
	return charStream->ch;
}

/***********************************************************
 * Scanner -- generating tokens stream
 **********************************************************/
/* Token types */
int None = 256, Integer = 257, Float = 258, Plus = 259, Minus = 260;

struct Token {
	char text[64];		/* token string */
	int row;		/* line number */
	int column;		/* column number */
	int type;		/* token type */
};

struct Scanner {
	struct CharStream *charStream;	/* source code */
	struct Token token;	/* current token */
};

/* Call struct CharStream's method */
char scanner_current_char(struct Scanner *scanner)
{
	return charstream_current_char(scanner->charStream);
}

/* Call struct CharStream's method */
char scanner_next_char(struct Scanner *scanner)
{
	return charstream_next_char(scanner->charStream);
}

/* Returns current token */
struct Token scanner_current_token(struct Scanner *scanner)
{
	return scanner->token;
}

/* Check whether a character is white space */
int isWhiteSpace(char ch)
{
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

void scanner_skip_whitespace(struct Scanner *scanner)
{
	while (isWhiteSpace(scanner_current_char(scanner))) {
		if (scanner_current_char(scanner) == '\n') {
			scanner->charStream->row++;
			scanner->charStream->column = 0;
		}
		scanner_next_char(scanner);
	}
}

/* Reset current token */
void scanner_init_token(struct Scanner *scanner)
{
	memset(scanner->token.text, 0, 64);
	scanner->token.row = scanner->charStream->row;
	scanner->token.column = scanner->charStream->column;
	scanner->token.type = None;
}

/* Append current character to current token's text */
void scanner_enter_char(struct Scanner *scanner)
{
	int n = strlen(scanner->token.text);
	if (n >= 63)
		return;
	scanner->token.text[n] = scanner->charStream->ch;
}

/* Set current token text */
void scanner_set_text(struct Scanner *scanner, char *text)
{
	strncpy(scanner->token.text, text, 64);
}

/* Set current token type */
void scanner_set_type(struct Scanner *scanner, int type)
{
	scanner->token.type = type;
}

/* Scanning unsigned integer
   Return 1 on success, otherwise, return 0 */
int scanner_integer_literal(struct Scanner *scanner)
{
	if (!isdigit(scanner_current_char(scanner)))
		return 0;

	while (isdigit(scanner_current_char(scanner))) {
		scanner_enter_char(scanner);
		scanner_next_char(scanner);
	}
	return 1;
}

/* Scanning unsigned number:
   number ::= ( integer ( '.' integer? )? | '.' integer ) ( ( 'e' | 'E' ) ( '+' | '-' )? integer )?
   Return 1 on success, otherwise, return 0
 */
int scanner_numeric_literal(struct Scanner *scanner)
{
	scanner_set_type(scanner, Integer);	/* default type */

	if (isdigit(scanner_current_char(scanner))) {
		scanner_integer_literal(scanner);
		if (scanner_current_char(scanner) == '.') {
			scanner_set_type(scanner, Float);
			scanner_enter_char(scanner);
			scanner_next_char(scanner);
			if (isdigit(scanner_current_char(scanner))) {
				scanner_integer_literal(scanner);
			}
		}
	} else if (scanner_current_char(scanner) == '.') {
		scanner_set_type(scanner, Float);
		scanner_enter_char(scanner);
		scanner_next_char(scanner);
		if (!scanner_integer_literal(scanner))
			return 0;
	} else
		return 0;

	if (scanner_current_char(scanner) == 'e'
	    || scanner_current_char(scanner) == 'E') {
		scanner_set_type(scanner, Float);
		scanner_enter_char(scanner);
		scanner_next_char(scanner);
		if (scanner_current_char(scanner) == '+' ||
		    scanner_current_char(scanner) == '-') {
			scanner_enter_char(scanner);
			scanner_next_char(scanner);
		}
		if (!scanner_integer_literal(scanner))
			return 0;
	}

	return 1;
}

/* Consume the current token and return the next token . */
struct Token scanner_next_token(struct Scanner *scanner)
{
	scanner_skip_whitespace(scanner);
	scanner_init_token(scanner);

	char ch;
	if ((ch = scanner_current_char(scanner)) != EOF) {
		switch (ch) {
		case '+':
			scanner_set_type(scanner, Plus);
			scanner_enter_char(scanner);
			scanner_next_char(scanner);
			break;
		case '-':
			scanner_set_type(scanner, Minus);
			scanner_enter_char(scanner);
			scanner_next_char(scanner);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '.':
			scanner_init_token(scanner);
			scanner_numeric_literal(scanner);
			break;
		}
	} else {
		scanner_set_text(scanner, "EOF");
		scanner_set_type(scanner, EOF);
	}
	printf(".. Scanning token: %s, position: (%d, %d), type: %d\n",
	       scanner->token.text,
	       scanner->token.row, scanner->token.column, scanner->token.type);
	return scanner->token;
}

/* Initialize a scanner */
void scanner_init(struct Scanner *scanner, struct CharStream *charStream)
{
	scanner->charStream = charStream;
	scanner_next_token(scanner);
}

/***********************************************************
 * Parser
 **********************************************************/
struct Parser {
	struct Scanner *scanner;	// from where we get tokens
};

/* Initialize a parser */
void parser_init(struct Parser *parser, struct Scanner *scanner)
{
	parser->scanner = scanner;
}

/* Call struct Scanner's method */
struct Token parser_current_token(struct Parser *parser)
{
	return scanner_current_token(parser->scanner);
}

/* Call struct Scanner's method */
struct Token parser_next_token(struct Parser *parser)
{
	return scanner_next_token(parser->scanner);
}

/* Check whether the current token matches the specific type */
int parser_match(struct Parser *parser, int type)
{
	if (parser->scanner->token.type != type)
		return 0;
	parser_next_token(parser);
	return 1;
}

/* Parsing number */
int parser_number(struct Parser *parser)
{
	if (parser_match(parser, Integer) || parser_match(parser, Float))
		return 1;
	else
		return 0;
}

/* Parsing expression */
int parser_expression(struct Parser *parser)
{
	if (!parser_number(parser))
		return 0;

	while (parser_current_token(parser).type == Plus ||
	       parser_current_token(parser).type == Minus) {
		parser_next_token(parser);
		if (!parser_number(parser))
			return 0;
	}

	if (parser_current_token(parser).type == EOF)
		return 1;
	else
		return 0;
}

/* Test */
int main(int argc, char *argv[])
{
	if (argc < 2)
		return 0;

	FILE *fp = fopen(argv[1], "r");

	struct CharStream charStream;
	charstream_init(&charStream, fp);
	struct Scanner scanner;
	scanner_init(&scanner, &charStream);
	struct Parser parser;
	parser_init(&parser, &scanner);
	if (parser_expression(&parser))
		printf("Accepted!\n");
	else
		printf("Syntax error!\n");

	fclose(fp);

	return 0;
}

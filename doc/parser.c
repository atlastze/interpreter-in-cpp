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
  * expression ::= integer ( ( '+' | '-' ) integer )*
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

/* Return the source character following the current character without
   consuming the current character. */
char charstream_peek_char(struct CharStream *charStream)
{
	return charstream_current_char(charStream);
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
int Integer = 256, Plus = 257, Minus = 258;

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

/* Initialize a token */
struct Token create_token(char *str, int r, int c, int t)
{
	printf(".. Creating token: %s (%d, %d: %d)\n", str, r, c, t);
	struct Token token;
	strncpy(token.text, str, 64);
	token.row = r;
	token.column = c;
	token.type = t;
	return token;
}

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

/* Scanning unsigned integer */
struct Token scanner_digits(struct Scanner *scanner)
{
	char buffer[64];
	int row, column;
	row = scanner->charStream->row;
	column = scanner->charStream->column;
	int i = 0;
	do {
		buffer[i] = scanner_current_char(scanner);
		buffer[++i] = '\0';
		if (i == 64)
			break;
	} while (isdigit(scanner_next_char(scanner)));

	return create_token(buffer, row, column, Integer);
}

/* Consume the current token and return the next token . */
struct Token scanner_next_token(struct Scanner *scanner)
{
	scanner_skip_whitespace(scanner);
	char ch;
	int row, column;
	row = scanner->charStream->row;
	column = scanner->charStream->column;
	if ((ch = scanner_current_char(scanner)) != EOF) {
		switch (ch) {
		case '+':
			scanner_next_char(scanner);
			scanner->token = create_token("+", row, column, Plus);
			break;
		case '-':
			scanner_next_char(scanner);
			scanner->token = create_token("-", row, column, Minus);
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
			scanner->token = scanner_digits(scanner);
			break;
		}
	} else {
		scanner->token = create_token("EOF", row, column, EOF);
	}
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

/* Parsing integer */
int parser_integer(struct Parser *parser)
{
	if (parser_match(parser, Integer))
		return 1;
	else
		return 0;
}

/* Parsing expression */
int parser_expression(struct Parser *parser)
{
	if (!parser_integer(parser))
		return 0;

	while (parser_current_token(parser).type == Plus ||
	       parser_current_token(parser).type == Minus) {
		parser_next_token(parser);
		if (!parser_integer(parser))
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

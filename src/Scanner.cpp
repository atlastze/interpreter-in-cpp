// Copyright (C) 2015-2016, kylinsage <kylinsage@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Scanner.h"
#include <iostream>

std::shared_ptr<Token> Scanner::nextToken()
{
    skipWhiteSpace();
    char ch;
    int row, column;
    row = charStream->currentRow();
    column = charStream->currentColumn();
    if ((ch = currentChar()) != EOF) {
        switch (ch) {
        case '+':
            token = std::make_shared<Token>("+", row, column, Token::Plus);
            nextChar();
            break;
        case '-':
            token = std::make_shared<Token>("-", row, column, Token::Minus);
            nextChar();
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
            token = digits();
            break;
        default:
            throw "Error in lexical analysis";
        }
    } else {
        token = std::make_shared<Token>("EOF", row, column, EOF);
    }

    return token;
}

std::shared_ptr<Token> Scanner::digits()
{
    string buffer;
    int row, column;
    row = charStream->currentRow();
    column = charStream->currentColumn();
    do {
        buffer.append(1, currentChar());
    } while (isdigit(nextChar()));

    return std::make_shared<Token>(buffer, row, column, Token::Integer);
}

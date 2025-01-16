# -*- coding: utf-8 -*-

import sys
import re
from typing import NamedTuple, Optional

class Token(NamedTuple):
    type: str
    lexeme: str
    line: int
    column: int

    def __str__(self):
        return f'{self.type} ({self.line}, {self.column}) "{self.lexeme}"'

class PascalLexer:
    def __init__(self, input_file: str):
        self.input_file = input_file
        self.current_line = 1
        self.current_column = 1

        self.keywords = {
            'array', 'begin', 'else', 'end', 'if', 'of', 'or', 'program',
            'procedure', 'then', 'type', 'var'
        }

        self.patterns = [
            ('WHITESPACE', r'[\s_]?'), 
            ('LINE_COMMENT', r'//[^\n]*'),  
            ('BLOCK_COMMENT', r'\{[^}]*\}'),
            ('STRING', r"'.*?'"),  
            ('FLOAT', r'\d+\.\d+'),  
            ('INTEGER', r'\d+'),  
            ('IDENTIFIER', r'[a-zA-Z_][a-zA-Z0-9_]*'),  
            ('ASSIGN', r':='),  
            ('LESS_EQ', r'<='),  
            ('GREATER_EQ', r'>='), 
            ('NOT_EQ', r'<>'),  
            ('MULTIPLICATION', r'\*'),  
            ('PLUS', r'\+'),  
            ('MINUS', r'-'),  
            ('DIVIDE', r'/'),  
            ('SEMICOLON', r';'),  
            ('COMMA', r','),  
            ('LEFT_PAREN', r'\('), 
            ('RIGHT_PAREN', r'\)'),  
            ('LEFT_BRACKET', r'\['),  
            ('RIGHT_BRACKET', r'\]'),  
            ('EQ', r'='),  
            ('GREATER', r'>'),  
            ('LESS', r'<'), 
            ('COLON', r':'),  
            ('DOT', r'\.'), 
            ('BAD', r'.'),  
        ]

        self.regex = re.compile(
            '|'.join(f'(?P<{name}>{pattern})' for name, pattern in self.patterns)
        )

    def next_token(self):
        in_block_comment = False
        block_comment_start_line = 0;
        block_comment_start_column = 0;
        block_comment_content = "";

        with open(self.input_file, 'r') as file:
            for line in file:
                self.current_column = 1

                if in_block_comment:
                    closing_index = line.find('}')
                    if closing_index != -1:
                        # Закрывающая скобка найдена
                        block_comment_content += line[:closing_index + 1]
                        # Заменяем символы новой строки на \n
                        block_comment_content = block_comment_content.replace('\n', '\\n')
                        yield Token(
                            'BLOCK_COMMENT',
                            block_comment_content,
                            block_comment_start_line,
                            block_comment_start_column
                        )
                        in_block_comment = False
                        self.current_column = closing_index + 2
                        line = line[closing_index + 1:] 
                    else:
                        # Закрывающая скобка не найдена, продолжаем собирать комментарий
                        block_comment_content += line
                        self.current_line += 1
                        continue


                for match in self.regex.finditer(line):
                    token_type = match.lastgroup
                    lexeme = match.group(token_type)
                    
                    if token_type == 'WHITESPACE':
                        self.current_column += len(lexeme)
                        continue

                    #Если это начало блочного комментария
                    if lexeme == '{':
                        in_block_comment = True
                        block_comment_start_line = self.current_line
                        block_comment_start_column = self.current_column
                        block_comment_content = lexeme
                        closing_index = line.find('}', self.current_column)
                        if closing_index != -1:
                            # Блочный комментарий завершился на этой же строке
                            in_block_comment = False
                            block_comment_content += line[self.current_column:closing_index + 1]
                            # Заменяем символы новой строки на \n
                            block_comment_content = block_comment_content.replace('\n', '\\n')
                            yield Token(
                                'BLOCK_COMMENT',
                                block_comment_content,
                                block_comment_start_line,
                                block_comment_start_column
                            )
                            self.current_column = closing_index + 2
                            continue
                        else:
                            # Блочный комментарий продолжается на следующей строке
                            block_comment_content += line[self.current_column:]
                            self.current_line += 1
                            continue

                    if in_block_comment:
                        continue

                    if token_type == 'IDENTIFIER' and lexeme.lower() in self.keywords:
                        token_type = lexeme.upper()

                    yield Token(token_type, lexeme, self.current_line, self.current_column)

                    self.current_column += len(lexeme)
                
                if not in_block_comment:
                    self.current_line += 1

def main():
    if len(sys.argv) != 3:
        print("Usage: python PascalLexer.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    lexer = PascalLexer(input_file)

    with open(output_file, 'w') as output:
        for token in lexer.next_token():
            print(token)
            output.write(str(token) + '\n')

if __name__ == "__main__":
    main()
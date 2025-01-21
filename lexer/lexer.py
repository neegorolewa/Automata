# -*- coding: utf-8 -*-

import sys
import re
from typing import NamedTuple, Optional
from xml.etree.ElementTree import tostring


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
        self.current_line = 0
        self.current_column = 1

        self.keywords = {
            "array",
            "begin",
            "else",
            "end",
            "if",
            "of",
            "or",
            "program",
            "procedure",
            "then",
            "type",
            "var",
        }

        self.patterns = [
            ("WHITESPACE", r"[\s]+?"),
            ("LINE_COMMENT", r"//[^\n]*"),
            ("BLOCK_COMMENT", r"\{[^}]*\}"),
            ("STRING", r"'(?:[^']|'')*'"),
            ("FLOAT", r"\d+\.\d+(?:[eE][-+]?\d+)?|\.\d+(?:[eE][-+]?\d+)?|\d+[eE][-+]?\d+"),
            ("INTEGER", r"\d+"),
            ("IDENTIFIER", r"[a-zA-Z_][a-zA-Z0-9_]*"),
            ("ASSIGN", r":="),
            ("LESS_EQ", r"<="),
            ("GREATER_EQ", r">="),
            ("NOT_EQ", r"<>"),
            ("MULTIPLICATION", r"\*"),
            ("PLUS", r"\+"),
            ("MINUS", r"-"),
            ("DIVIDE", r"/"),
            ("SEMICOLON", r";"),
            ("COMMA", r","),
            ("LEFT_PAREN", r"\("),
            ("RIGHT_PAREN", r"\)"),
            ("LEFT_BRACKET", r"\["),
            ("RIGHT_BRACKET", r"\]"),
            ("EQ", r"="),
            ("GREATER", r">"),
            ("LESS", r"<"),
            ("COLON", r":"),
            ("DOT", r"\."),
            ("BAD", r"."),
            #("BAD", r"[^a-zA-Z0-9_\s\(\)\{\}\[\];=+*/<>\-:.,]"),
        ]

        self.regex = re.compile(
            "|".join(f"(?P<{name}>{pattern})" for name, pattern in self.patterns)
        )

    def next_token(self):
        in_block_comment = False
        block_comment_start_line = 0
        block_comment_start_column = 0
        block_comment_content = ""

        in_string = False
        string_start_line = 0
        string_start_column = 0
        string_content = ""

        #invalid_char_pattern = re.compile(r"[^\x00-\x7F()\s;=+*/<>\-:.,\[\]{}a-zA-Z0-9_]")
    
        with open(self.input_file, "r") as file:
            for line in file:
                self.current_column = 1
                self.current_line += 1                

                if in_block_comment:
                    closing_index = line.find("}")
                    opening_index = line.find("{")
                    if closing_index != -1:
                        in_block_comment = False
                        if closing_index == len(line) - 1:
                            self.current_column = 1
                            print(f"Block comment ended at line {self.current_line - 1}, moving to line {self.current_line}")
                        else:
                            self.current_column = closing_index + 2
                            line = line[closing_index + 1 :]
                            print(f"Block comment ended at line {self.current_line}, column {closing_index}, continuing at column {self.current_column}")
                    elif opening_index != -1:
                        pass
                    else:
                        block_comment_content += line
                        continue

                if in_string:
                    closing_index = line.find("'")
                    if closing_index != -1:
                        in_string = False
                        string_content += line[: closing_index + 1]
                        yield Token(
                            "STRING",
                            string_content,
                            string_start_line,
                            string_start_column,
                        )
                        self.current_column = closing_index + 2
                        line = line[closing_index + 1 :]
                        print("end string")
                    else:
                        string_content += line
                        print("String continues at next line")
                        continue
            
                for match in self.regex.finditer(line):
                    token_type = match.lastgroup
                    lexeme = match.group(token_type)

                    if token_type == "WHITESPACE":
                        self.current_column += len(lexeme)
                        continue

                    if token_type == "LINE_COMMENT":
                        continue

                    if token_type == "BLOCK_COMMENT":
                        self.current_column += len(lexeme)
                        continue
                    
                    if lexeme == "{":
                        in_block_comment = True
                        block_comment_start_line = self.current_line
                        block_comment_start_column = self.current_column
                        block_comment_content = lexeme
                        closing_index = line.find("}", self.current_column)
                        if closing_index != -1:
                            in_block_comment = False
                            self.current_column = closing_index + 2
                            continue
                        else:
                            block_comment_content += line[self.current_column :]
                            print(f"Block comment starts at line {self.current_line}")
                            continue

                    if in_block_comment:
                        continue

                    if lexeme == "'":
                        if not in_string:
                            in_string = True
                            string_start_line = self.current_line
                            string_start_column = self.current_column
                            string_content = ""
                        else:
                            in_string = False
                            string_content += lexeme
                            yield Token(
                                "STRING",
                                string_content,
                                string_start_line,
                                string_start_column,
                            )
                            self.current_column += len(lexeme)
                            continue

                    if in_string:
                        string_content += line[self.current_column - 1 :]
                        self.current_column += len(line) + 1
                        continue

                    # if token_type == "BAD":
                    #     yield Token(
                    #         "BAD",
                    #         lexeme,
                    #         self.current_line,
                    #         self.current_column,
                    #     )
                    #     self.current_column += len(lexeme)
                    #     continue

                    if token_type == "IDENTIFIER":
                        if len(lexeme) > 256:
                            yield Token(
                                "BAD", lexeme, self.current_line, self.current_column
                            )
                            self.current_column += len(lexeme)
                            continue

                        if lexeme.lower() in self.keywords:
                            token_type = lexeme.upper()
                    
                    if token_type == "INTEGER":
                        try:
                            if len(lexeme) > 16:
                                yield Token(
                                    "BAD",
                                    lexeme,
                                    self.current_line,
                                    self.current_column,
                                )
                                self.current_column += len(lexeme)
                                continue
                        except ValueError:
                            yield Token(
                                "BAD", lexeme, self.current_line, self.current_column
                            )
                            self.current_column += len(lexeme)
                            continue

                    
                    yield Token(
                        token_type,
                        lexeme,
                        self.current_line,
                        self.current_column
                    )
                    self.current_column += len(lexeme)

                #if not in_block_comment and not in_string:
                    #self.current_line += 1

            if in_block_comment:
                yield Token(
                    "BAD",
                    block_comment_content,
                    block_comment_start_line,
                    block_comment_start_column,
                )

            if in_string:
                yield Token(
                    "BAD", 
                    string_content, 
                    string_start_line, 
                    string_start_column
                )


def main():
    if len(sys.argv) != 3:
        print("Usage: python PascalLexer.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    lexer = PascalLexer(input_file)

    with open(output_file, "w") as output:
        for token in lexer.next_token():
            print(token)
            output.write(str(token) + "\n")


if __name__ == "__main__":
    main()
from collections import deque
from typing import Dict, List, Set, Optional

class RegexNode:
    def __init__(self, value: str, left: Optional['RegexNode'] = None, right: Optional['RegexNode'] = None):
        self.value = value
        self.left = left
        self.right = right

    def __str__(self):
        return f"RegexNode({self.value})"

class RegexParser:
    NON_LITERAL_CHARACTERS = {'+', '*', '(', ')', '|'}

    def __init__(self):
        self.tokens = deque()

    def parse_regex(self, expression: str) -> RegexNode:
        self.tokens = deque(expression)
        return self.parse_expression()

    def parse_expression(self) -> RegexNode:
        node = self.parse_term()
        while self.tokens and self.tokens[0] == '|':
            self.tokens.popleft()
            right = self.parse_term()
            node = RegexNode("or", node, right)
        return node

    def parse_term(self) -> RegexNode:
        node = self.parse_factor()
        while self.tokens and (self.is_literal(self.tokens[0]) or self.tokens[0] == '('):
            right = self.parse_factor()
            node = RegexNode("concat", node, right)
        return node

    def parse_factor(self) -> RegexNode:
        node = self.parse_primary()
        while self.tokens and (self.tokens[0] == '*' or self.tokens[0] == '+'):
            op = "multiply" if self.tokens.popleft() == '*' else "add"
            node = RegexNode(op, node)
        return node

    def parse_primary(self) -> RegexNode:
        if not self.tokens:
            raise ValueError("Unexpected end of expression")

        token = self.tokens.popleft()
        if token == '\\':
            if not self.tokens:
                raise ValueError("Unexpected end of expression after escape character")

            escaped = self.tokens.popleft()
            if self.is_literal(escaped):
                self.tokens.appendleft(escaped)
            else:
                return RegexNode(escaped)

        if self.is_literal(token):
            return RegexNode(token)
        elif token == '(':
            node = self.parse_expression()
            if not self.tokens or self.tokens.popleft() != ')':
                raise ValueError("Mismatched parentheses")
            return node

        raise ValueError(f"Unexpected token: {token}")

    def is_literal(self, value: str) -> bool:
        return value not in self.NON_LITERAL_CHARACTERS

class State:
    def __init__(self):
        self.transitions: Dict[str, List['State']] = {}
        self.epsilon_transitions: List['State'] = []

    def add_transition(self, symbol: str, state: 'State'):
        if symbol not in self.transitions:
            self.transitions[symbol] = []
        self.transitions[symbol].append(state)

    def add_epsilon_transition(self, state: 'State'):
        self.epsilon_transitions.append(state)

class NFA:
    def __init__(self, start: State, accept: State):
        self.start = start
        self.accept = accept

class NFAConstructor:
    def build_nfa(self, node: RegexNode) -> NFA:
        if node.value == "concat":
            return self.concatenate(self.build_nfa(node.left), self.build_nfa(node.right))
        elif node.value == "or":
            return self.alternate(self.build_nfa(node.left), self.build_nfa(node.right))
        elif node.value == "multiply":
            return self.star(self.build_nfa(node.left))
        elif node.value == "add":
            return self.plus(self.build_nfa(node.left))
        else:
            return self.literal(node.value[0])

    def literal(self, symbol: str) -> NFA:
        start = State()
        accept = State()
        start.add_transition(symbol, accept)
        return NFA(start, accept)

    def concatenate(self, first: NFA, second: NFA) -> NFA:
        first.accept.add_epsilon_transition(second.start)
        return NFA(first.start, second.accept)

    def alternate(self, first: NFA, second: NFA) -> NFA:
        start = State()
        accept = State()
        start.add_epsilon_transition(first.start)
        start.add_epsilon_transition(second.start)
        first.accept.add_epsilon_transition(accept)
        second.accept.add_epsilon_transition(accept)
        return NFA(start, accept)

    def star(self, nfa: NFA) -> NFA:
        start = State()
        accept = State()
        start.add_epsilon_transition(nfa.start)
        start.add_epsilon_transition(accept)
        nfa.accept.add_epsilon_transition(nfa.start)
        nfa.accept.add_epsilon_transition(accept)
        return NFA(start, accept)

    def plus(self, nfa: NFA) -> NFA:
        start = State()
        accept = State()
        start.add_epsilon_transition(nfa.start)
        nfa.accept.add_epsilon_transition(nfa.start)
        nfa.accept.add_epsilon_transition(accept)
        return NFA(start, accept)

class NFAUtils:
    @staticmethod
    def assign_indices(start_state: State) -> Dict[State, str]:
        state_index = {}
        index = 0
        stack = [start_state]

        while stack:
            state = stack.pop()
            if state not in state_index:
                state_index[state] = f"S{index}"
                index += 1
                for states in state.transitions.values():
                    for s in states:
                        if s not in state_index:
                            stack.append(s)
                for s in state.epsilon_transitions:
                    if s not in state_index:
                        stack.append(s)

        return state_index

    @staticmethod
    def write_nfa(nfa: NFA, output_file_name: str):
        state_index = NFAUtils.assign_indices(nfa.start)
        final_state = state_index[nfa.accept]

        transitions: Dict[str, Dict[str, Set[str]]] = {}
        for state, name in state_index.items():
            transitions[name] = {}
            for symbol, states in state.transitions.items():
                transitions[name][symbol] = {state_index[s] for s in states}
            if state.epsilon_transitions:
                transitions[name]['ε'] = {state_index[s] for s in state.epsilon_transitions}

        symbols = set()
        for trans in transitions.values():
            symbols.update(trans.keys())

        # Используем кодировку utf-8 для записи файла
        with open(output_file_name, 'w', encoding='utf-8') as writer:
            writer.write(";" + ";".join("F" if s == final_state else "" for s in state_index.values()) + "\n")
            writer.write(";" + ";".join(state_index.values()) + "\n")

            for symbol in sorted(symbols):
                row = [symbol]
                for state in state_index.values():
                    targets = transitions[state].get(symbol, set())
                    row.append(",".join(sorted(targets)))
                writer.write(";".join(row) + "\n")

def main(args: List[str]):
    if len(args) < 2:
        print("Usage: python program.py <output_file> <regex>")
        return

    output_file_name = args[0]
    regex = args[1]

    parser = RegexParser()
    regex_tree = parser.parse_regex(regex)

    nfa_constructor = NFAConstructor()
    nfa = nfa_constructor.build_nfa(regex_tree)

    NFAUtils.write_nfa(nfa, output_file_name)

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
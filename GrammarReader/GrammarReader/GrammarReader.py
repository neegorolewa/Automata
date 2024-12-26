from platform import python_version
import sys
import re

def read_file(file_name):
    try:
        with open(file_name, 'r', encoding='utf-8') as file:
            return file.read().strip()
    except FileNotFoundError:
        print(f"Ошибка: файл '{file_name}' не найден.")
        sys.exit(1)
        
def process_left_grammar(grammar):
    machine = {
        "entries": [],
        "states_with_transitions": []
    }
    
    list_of_grammars = re.findall(r'^\s*<(\w+)>\s*->\s*((?:<\w+>\s+)?[\wε](?:\s*\|\s*(?:<\w+>\s+)?[\wε])*)\s*$', grammar, flags=re.MULTILINE)
    
    for pairs in list_of_grammars:
        machine["states_with_transitions"].append({"curr_state": pairs[0], "out": "", "transitions": []})
    machine["states_with_transitions"].append({"curr_state": "H", "out": "", "transitions": []})
   
    transit = []
    for pairs in list_of_grammars:
        rules = pairs[1].split('|')
        cleaned_rules = [ one_rule.strip() for one_rule in rules]
        transit_for_state = cleaned_rules
        transit.append(transit_for_state)
        
    for i in range(len(transit)):
        for t in transit[i]:
            match = re.search(r'(<[^>]+>)\s*([\wε]*)', t, flags=re.MULTILINE)
            if match:
                if match[2] not in machine["entries"]:
                    machine["entries"].append(match[2])
                for state in machine["states_with_transitions"]:
                    if state["curr_state"] == match[1]:
                        state["transitions"].append({
                            "state": machine["states_with_transitions"][i]["curr_state"], 
                            "entry": match[2]
                        })
            else:
                match = re.search(r'\s*([\wε])\s*', t, flags=re.MULTILINE)
                if match:
                    if match[1] not in machine["entries"]:
                        machine["entries"].append(match[1])
                    machine["states_with_transitions"][len(machine["states_with_transitions"]) - 1]["transitions"].append({
                        "state": machine["states_with_transitions"][i]["curr_state"], 
                        "entry": match[1]
                    })
                else:
                    print("Grammar not found")
                    sys.exit(1)
                    
    machine["states_with_transitions"][0]["out"] = "F"
    machine["states_with_transitions"] = sorted(
        machine["states_with_transitions"], 
        key=lambda x: x["curr_state"]
    )
    
    return machine

def process_right_grammar(grammar):
    machine = {
        "entries": [],
        "states_with_transitions": []
    }
    
    list_of_grammars = re.findall(r'^\s*<(\w+)>\s*->\s*([\wε](?:\s+<\w+>)?(?:\s*\|\s*[\wε](?:\s+<\w+>)?)*)\s*$', grammar, flags=re.MULTILINE)
    
    for pairs in list_of_grammars:
        machine["states_with_transitions"].append({"curr_state": pairs[0], "out": "", "transitions": []})
    machine["states_with_transitions"].append({"curr_state": "F", "out": "F", "transitions": []})
   
    transit = []
    for pairs in list_of_grammars:
        rules = pairs[1].split('|')
        cleaned_rules = [ one_rule.strip() for one_rule in rules]
        transit_for_state = cleaned_rules
        transit.append(transit_for_state)
        
    for i in range(len(transit)):
        for t in transit[i]:
            match = re.search(r'\s*([\wε])\s+<(\w+)>\s*', t, flags=re.MULTILINE)
            if match:
                if match[1] not in machine["entries"]:
                    machine["entries"].append(match[2])
                machine["states_with_transitions"][i]["transitions"].append({
                    "state": match[2],
                    "entry": match[1]
                    })
            else:
                match = re.search(r'\s*([\wε])\s*', t, flags=re.MULTILINE)
                if match:
                    if match[1] not in machine["entries"]:
                        machine["entries"].append(match[1])
                    machine["states_with_transitions"][i]["transitions"].append({
                        "state": "F", 
                        "entry": match[1]
                    })
                else:
                    print("Grammar not found")
                    sys.exit(1)
    
    return machine

def write_machine(machine, output_file_name):
    try:
        with open(output_file_name, 'w', encoding='utf-8') as file:
            for state in machine["states_with_transitions"]:
                file.write(";" + state["out"])
            file.write('\n')
            
            for state in machine["states_with_transitions"]:
                file.write(";" + state["curr_state"])
            file.write('\n')
            
            for entry in machine["entries"]:
                file.write(entry + ";")
                for state in machine["states_with_transitions"]:
                    transits = []
                    for transit in state["transitions"]:
                        if transit["entry"] == entry:
                            transits.append(transit["state"])
                    for t in transits:
                        if t != transits[len(transits) - 1]:
                            file.write(t + ",")
                        else:
                            file.write(t)
                    if state != machine["states_with_transitions"][len(machine["states_with_transitions"]) - 1]:
                        file.write(";")
                file.write('\n')        
    except FileNotFoundError:
        print("Файл не найден")
        sys.exit(1)

def process_grammar(input_file_name, output_file_name):
    grammar = read_file(input_file_name)
    
    if len(re.findall(r'^\s*<(\w+)>\s*->\s*([\wε](?:\s+<\w+>)?(?:\s*\|\s*[\wε](?:\s+<\w+>)?)*)\s*$', grammar, flags=re.MULTILINE)) == grammar.count('->'):
        machine = process_right_grammar(grammar)
    elif len(re.findall(r'^\s*<(\w+)>\s*->\s*((?:<\w+>\s+)?[\wε](?:\s*\|\s*(?:<\w+>\s+)?[\wε])*)\s*$', grammar, flags=re.MULTILINE)) == grammar.count('->'):
        machine = process_left_grammar(grammar)
    else:
        print("grammar doesn't read")
        sys.exit(1)

    write_machine(machine, output_file_name)

def main():
    if len(sys.argv) != 3:
        print("Usage: ", sys.argv[0], " <input.txt> <output.csv>")
        sys.exit(1)

    input_file_name = sys.argv[1]
    output_file_name = sys.argv[2]

    process_grammar(input_file_name, output_file_name)

if __name__ == "__main__":
    main()
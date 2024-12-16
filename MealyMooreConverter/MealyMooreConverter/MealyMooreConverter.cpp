#include "MealyMooreConverter.h"

using namespace std;

const string CONVERSION_TYPE_MEALY_TO_MOORE = "mealy-to-moore";
const string CONVERSION_TYPE_MOORE_TO_MEALY = "moore-to-mealy";

struct Mealy
{
    std::vector<std::string> states;
    std::vector<std::string> entries;
    std::vector<std::vector<std::pair<std::string, std::string>>> transitions;
};

struct Moore
{
    std::vector<std::pair<std::string, std::string>> states;
    std::vector<std::string> entries;
    std::vector<std::vector<std::string>> transitions;
};

void ReadMealy(const std::string& inFileName, Mealy& mealy)
{
    std::ifstream input(inFileName);
    std::string line;

    // Чтение состояний
    std::getline(input, line);
    std::istringstream statesStream(line);
    for (std::string state; std::getline(statesStream, state, ';'); )
        mealy.states.push_back(state);

    // Чтение переходов
    while (std::getline(input, line))
    {
        std::istringstream lineStream(line);
        std::string entry;
        std::getline(lineStream, entry, ';');
        mealy.entries.push_back(entry);

        std::vector<std::pair<std::string, std::string>> transitions;
        for (std::string transition; std::getline(lineStream, transition, ';'); )
        {
            size_t pos = transition.find('/');
            transitions.emplace_back(transition.substr(0, pos), transition.substr(pos + 1));
        }
        mealy.transitions.push_back(transitions);
    }
}

void WriteMealy(const std::string& outFileName, Mealy mealy)
{
    std::ofstream output(outFileName);

    for (const auto& state : mealy.states)
    {
        output << ";" << state;
    }
    output << std::endl;

    for (size_t i = 0; i < mealy.transitions.size(); i++)
    {
        output << mealy.entries[i] << ";";

        for (size_t j = 0; j < mealy.transitions[i].size(); j++)
        {
            output << mealy.transitions[i][j].first
                << "/" << mealy.transitions[i][j].second;

            if (j != mealy.transitions[i].size() - 1)
            {
                output << ";";
            }
        }
        output << std::endl;
    }
}

void ReadMoore(const std::string& inFileName, Moore& moore)
{
    std::ifstream input(inFileName);
    std::string line;

    std::getline(input, line);
    std::istringstream ssOut(line);
    std::getline(input, line);
    std::istringstream ssState(line);

    std::string out, state;
    while (std::getline(ssOut, out, ';') && std::getline(ssState, state, ';'))
    {
        moore.states.emplace_back(state, out);
    }

    // Чтение переходов
    while (std::getline(input, line))
    {
        std::istringstream ss(line);
        std::string entry;
        std::getline(ss, entry, ';');
        moore.entries.push_back(entry);

        std::vector<std::string> transitions;
        for (std::string transition; std::getline(ss, transition, ';'); )
        {
            transitions.push_back(transition);
        }
        moore.transitions.push_back(transitions);
    }
}

void WriteMoore(const std::string& outFileName, Moore moore)
{
    std::ofstream output(outFileName);

    for (const auto& state : moore.states)
    {
        output << ";" << state.second;
    }
    output << std::endl;

    for (const auto& state : moore.states)
    {
        output << ";" << state.first;
    }
    output << std::endl;

    for (size_t i = 0; i < moore.transitions.size(); i++)
    {
        output << moore.entries[i] << ";";
        for (size_t j = 0; j < moore.transitions[i].size(); j++)
        {
            output << moore.transitions[i][j];
            if (j != moore.transitions[i].size() - 1)
            {
                output << ";";
            }
        }
        output << std::endl;
    }
}


vector<string> findReachableStatesMoore(const MooreAutomata& moore, const string& startState)
{
    vector<string> reachableStates;
    unordered_set<string> visited;
    queue<string> queue;

    queue.push(startState);
    visited.insert(startState);

    while (!queue.empty())
    {
        string currState = queue.front();
        queue.pop();
        reachableStates.push_back(currState);

        for (const auto& inputSymbol : moore.inputSymbols)
        {
            auto it = find(moore.states.begin(), moore.states.end(), currState);
            if (it != moore.states.end())
            {
                size_t index = it - moore.states.begin();
                string nextState = moore.transitions.at(inputSymbol)[index];
                if (visited.find(nextState) == visited.end())
                {
                    visited.insert(nextState);
                    queue.push(nextState);
                }
            }
        }
    }

    return reachableStates;
}

vector<string> findReachableStatesMealy(const MealyAutomata& mealy, const string& startState)
{
    vector<string> reachableStates;
    unordered_set<string> visited;
    queue<string> queue;

    queue.push(startState);
    visited.insert(startState);

    while (!queue.empty())
    {
        string currentState = queue.front();
        queue.pop();
        reachableStates.push_back(currentState);

        for (const auto& inputSymbol : mealy.inputSymbols)
        {
            auto it = find(mealy.states.begin(), mealy.states.end(), currentState);
            if (it != mealy.states.end())
            {
                size_t index = it - mealy.states.begin();
                string nextState = mealy.transitions.at(inputSymbol)[index].state;
                if (visited.find(nextState) == visited.end())
                {
                    visited.insert(nextState);
                    queue.push(nextState);
                }
            }
        }
    }

    return reachableStates;
}

MealyAutomata convertMooreToMealy(MooreAutomata moore)
{
    MealyAutomata mealy;
    mealy.inputSymbols = moore.inputSymbols;
    mealy.states = moore.states;

    for (const auto& inputSymbol : moore.inputSymbols)
    {
        for (size_t i = 0; i < moore.transitions[inputSymbol].size(); i++)
        {
            string state = moore.transitions[inputSymbol][i];
            string output = moore.outputs[state];
            mealy.transitions[inputSymbol].push_back({ state, output });
        }
    }

    return mealy;
}

vector<pair<string, string>> extractMooreStates(const unordered_map<string, vector<MealyState>>& mealyTransitions, const std::string& startState)
{
    set<pair<string, string>> statesForMoore;
    for (const auto& transitionForOneEntry : mealyTransitions)
    {
        for (const auto& transition : transitionForOneEntry.second)
        {
            statesForMoore.insert({ transition.state, transition.output });
        }
    }

    vector<pair<string, string>> statesForMooreVector;
    bool containsStartState = false;
    for (const auto& state : statesForMoore)
    {
        statesForMooreVector.push_back(state);
        if (state.first == startState)
        {
            containsStartState = true;
        }
    }

    if (!containsStartState)
    {
        statesForMooreVector.insert(statesForMooreVector.begin(), std::make_pair(startState, ""));
    }

    return statesForMooreVector;
}

MooreAutomata convertMealyToMoore(MealyAutomata mealy)
{
    MooreAutomata moore;

    // Step 1: Find reachable states
    vector<string> reachableStates = findReachableStatesMealy(mealy, mealy.states[0]);

    // Step 2: Filter Mealy automaton to include only reachable states
    MealyAutomata filteredMealy;
    filteredMealy.inputSymbols = mealy.inputSymbols;
    filteredMealy.transitions = unordered_map<string, vector<MealyState>>();

    for (const auto& state : reachableStates)
    {
        auto it = find(mealy.states.begin(), mealy.states.end(), state);
        if (it != mealy.states.end())
        {
            size_t index = it - mealy.states.begin();
            filteredMealy.states.push_back(state);
            for (const auto& inSymbol : filteredMealy.inputSymbols)
            {
                filteredMealy.transitions[inSymbol].push_back(mealy.transitions[inSymbol][index]);
            }
        }
    }

    // Step 3: Extract Moore states
    auto statesForMoore = extractMooreStates(filteredMealy.transitions, filteredMealy.states[0]);

    // Step 4: Create states for Moore automaton
    for (size_t i = 0; i < statesForMoore.size(); i++)
    {
        moore.states.push_back("q" + to_string(i));
        moore.outputs[moore.states.back()] = statesForMoore[i].second;
    }

    // Step 5: Copy input symbols
    moore.inputSymbols = filteredMealy.inputSymbols;

    // Step 6: Create transitions for Moore automaton
    for (const auto& inputSymbol : filteredMealy.inputSymbols)
    {
        vector<string> transitions;

        for (const auto& state : filteredMealy.states)
        {
            auto it = find(filteredMealy.states.begin(), filteredMealy.states.end(), state);
            if (it != filteredMealy.states.end())
            {
                size_t index = it - filteredMealy.states.begin();
                string nextState = filteredMealy.transitions[inputSymbol][index].state;
                string nextOutput = filteredMealy.transitions[inputSymbol][index].output;
                auto itMoore = find(statesForMoore.begin(), statesForMoore.end(), make_pair(nextState, nextOutput));
                if (itMoore != statesForMoore.end())
                {
                    size_t indexMoore = itMoore - statesForMoore.begin();
                    transitions.push_back(moore.states[indexMoore]);
                }
            }
        }

        moore.transitions[inputSymbol] = transitions;
    }

    return moore;
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        cout << "Usage: " << argv[0] << " <conversion-type> <in.csv> <out.csv>" << endl;
        return 1;
    }

    string convType = argv[1];
    string inputFileName = argv[2];
    string outputFileName = argv[3];

    if (convType == CONVERSION_TYPE_MEALY_TO_MOORE)
    {
        MealyAutomata mealy = readMealy(inputFileName);
        MooreAutomata moore = convertMealyToMoore(mealy);
        writeMoore(outputFileName, moore);
    }
    else
    {
        MooreAutomata moore = readMoore(inputFileName);
        MealyAutomata mealy = convertMooreToMealy(moore);
        writeMealy(outputFileName, mealy);
    }

    return 0;
}
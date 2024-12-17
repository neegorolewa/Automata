#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <queue>
#include <unordered_map>
#include <algorithm>

using namespace std;

const string CONVERSION_TYPE_MEALY_TO_MOORE = "mealy-to-moore";

struct MealyAutomata
{
    vector<string> states;
    vector<string> entries;
    vector<vector<pair<string, string>>> transitions;
};

struct MooreAutomata
{
    vector<pair<string, string>> states;
    vector<string> entries;
    vector<vector<string>> transitions;
};

vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void readMealy(const std::string& inFileName, MealyAutomata& mealy)
{
    ifstream input(inFileName);

    string line;
    getline(input, line);
    stringstream ss(line);
    string state;
    getline(ss, state, ';');

    while (getline(ss, state, ';'))
    {
        mealy.states.push_back(state);
    }

    while (getline(input, line))
    {
        stringstream ss(line);
        string entry;
        getline(ss, entry, ';');
        mealy.entries.push_back(entry);

        string transition;
        vector<pair<string, string>> transitions;
        for (const auto& state : mealy.states)
        {
            getline(ss, transition, ';');
            size_t pos = transition.find('/');
            string transitionState = transition.substr(0, pos);
            string transitionOut = transition.substr(pos + 1, transition.size());
            transitions.push_back(make_pair(transitionState, transitionOut));
        }
        mealy.transitions.push_back(transitions);
    }
}

void writeMealy(const string& outFileName, MealyAutomata mealy)
{
    ofstream output(outFileName);

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

void readMoore(const std::string& inFileName, MooreAutomata& moore)
{
    ifstream input(inFileName);
    string line;

    getline(input, line);
    istringstream ssOut(line);
    getline(input, line);
    istringstream ssState(line);

    string out, state;
    while (getline(ssOut, out, ';') && getline(ssState, state, ';'))
    {
        moore.states.emplace_back(state, out);
    }

    while (getline(input, line))
    {
        istringstream ss(line);
        string entry;
        getline(ss, entry, ';');
        moore.entries.push_back(entry);

        vector<string> transitions;
        for (string transition; getline(ss, transition, ';'); )
        {
            transitions.push_back(transition);
        }
        moore.transitions.push_back(transitions);
    }
}

void writeMoore(const string& outFileName, MooreAutomata moore)
{
    ofstream output(outFileName);

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

unordered_set<size_t> findReachableStatesMealy(const MealyAutomata& mealy)
{
    unordered_set<string> reachableStates;
    queue<string> queue;
    queue.push(mealy.states[0]);
    reachableStates.insert(mealy.states[0]);

    while (!queue.empty())
    {
        string currentState = queue.front();
        queue.pop();

        for (size_t i = 0; i < mealy.states.size(); i++)
        {
            if (currentState == mealy.states[i])
            {
                for (size_t j = 0; j < mealy.entries.size(); j++)
                {
                    if (!reachableStates.count(mealy.transitions[j][i].first))
                    {
                        queue.push(mealy.transitions[j][i].first);
                        reachableStates.insert(mealy.transitions[j][i].first);
                    }
                }
                break;
            }
        }
    }

    unordered_set<size_t> reachableStatesIndexes;
    for (const auto& reachableState : reachableStates)
    {
        for (size_t i = 0; i < mealy.states.size(); i++)
        {
            if (reachableState == mealy.states[i])
            {
                reachableStatesIndexes.insert(i);
                break;
            }
        }
    }

    return reachableStatesIndexes;
}

unordered_set<size_t> findReachableStatesMoore(const MooreAutomata& moore)
{
    unordered_set<string> reachableStates;
    queue<string> queue;
    queue.push(moore.states[0].first);
    reachableStates.insert(moore.states[0].first);

    while (!queue.empty())
    {
        string currentState = queue.front();
        queue.pop();

        for (size_t i = 0; i < moore.states.size(); i++)
        {
            if (currentState == moore.states[i].first)
            {
                for (size_t j = 0; j < moore.entries.size(); j++)
                {
                    if (!reachableStates.count(moore.transitions[j][i]))
                    {
                        queue.push(moore.transitions[j][i]);
                        reachableStates.insert(moore.transitions[j][i]);
                    }
                }
                break;
            }
        }
    }

    unordered_set<size_t> reachableStatesIndexes;
    for (const auto& reachableState : reachableStates)
    {
        for (size_t i = 0; i < moore.states.size(); i++)
        {
            if (reachableState == moore.states[i].first)
            {
                reachableStatesIndexes.insert(i);
                break;
            }
        }
    }

    return reachableStatesIndexes;
}

void minimizeMealy(const string& inFileName, const string& outFileName)
{
    MealyAutomata mealy;
    readMealy(inFileName, mealy);

    unordered_set<size_t> reachableStates = findReachableStatesMealy(mealy);

    MealyAutomata minimizedMealy;
    minimizedMealy.entries = mealy.entries;
    minimizedMealy.transitions = vector<vector<pair<string, string>>>(minimizedMealy.entries.size());

    for (const auto& index : reachableStates)
    {
        minimizedMealy.states.push_back(mealy.states[index]);
        for (size_t i = 0; i < minimizedMealy.entries.size(); i++)
        {
            minimizedMealy.transitions[i].push_back(mealy.transitions[i][index]);
        }
    }

    writeMealy(outFileName, minimizedMealy);
}

void minimizeMoore(const string& inFileName, const string& outFileName)
{
    MooreAutomata moore;
    readMoore(inFileName, moore);

    unordered_set<size_t> reachableStates = findReachableStatesMoore(moore);

    MooreAutomata minimizedMoore;
    minimizedMoore.entries = moore.entries;
    minimizedMoore.transitions = vector<vector<string>>(minimizedMoore.entries.size());

    for (const auto& index : reachableStates)
    {
        minimizedMoore.states.push_back(moore.states[index]);
        for (size_t i = 0; i < minimizedMoore.entries.size(); i++)
        {
            minimizedMoore.transitions[i].push_back(moore.transitions[i][index]);
        }
    }

    writeMoore(outFileName, minimizedMoore);
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        cout << "Usage: " << argv[0] << " <mealy|moore> <in.csv> <out.csv>" << endl;
        return 1;
    }

    string type = argv[1];
    string inputFileName = argv[2];
    string outputFileName = argv[3];

    if (type == "mealy")
    {
        minimizeMealy(inputFileName, outputFileName);
    }
    else if (type == "moore")
    {
        minimizeMoore(inputFileName, outputFileName);
    }
    else
    {
        cout << "Invalid type. Use 'mealy' or 'moore'." << endl;
        return 1;
    }

    return 0;
}

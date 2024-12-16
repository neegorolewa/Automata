#include "MealyMooreConverter.h"

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

//unordered_set<size_t> findReachableStatesMealy(const MealyAutomata& mealy)
//{
//    unordered_set<string> reachableStates;
//    queue<string> queue;
//    queue.push(mealy.states[0]); // Начинаем с начального состояния
//    reachableStates.insert(mealy.states[0]);
//
//    while (!queue.empty())
//    {
//        string currentState = queue.front();
//        queue.pop();
//
//        // Ищем индекс текущего состояния
//        auto it = find(mealy.states.begin(), mealy.states.end(), currentState);
//        if (it == mealy.states.end())
//        {
//            // Если состояние не найдено, пропускаем
//            continue;
//        }
//
//        size_t currentIndex = distance(mealy.states.begin(), it);
//
//        // Проходим по всем входным сигналам
//        for (size_t j = 0; j < mealy.entries.size(); j++)
//        {
//            if (currentIndex >= mealy.transitions[j].size())
//            {
//                // Если индекс выходит за пределы, пропускаем
//                continue;
//            }
//
//            const auto& transition = mealy.transitions[j][currentIndex];
//            if (!reachableStates.count(transition.first))
//            {
//                queue.push(transition.first);
//                reachableStates.insert(transition.first);
//            }
//        }
//    }
//
//    // Преобразуем достижимые состояния в индексы
//    unordered_set<size_t> reachableStatesIndexes;
//    for (const auto& reachableState : reachableStates)
//    {
//        auto it = find(mealy.states.begin(), mealy.states.end(), reachableState);
//        if (it != mealy.states.end())
//        {
//            reachableStatesIndexes.insert(distance(mealy.states.begin(), it));
//        }
//    }
//
//    return reachableStatesIndexes;
//}

vector<pair<string, string>> extractMooreStates(const vector<vector<pair<string, string>>>& mealyTransitions, const std::string& startState)
{
    set<pair<string, string>> statesForMoore;
    for (const auto& transitionForOneEntry : mealyTransitions)
    {
        for (const auto& transition : transitionForOneEntry)
        {
            statesForMoore.insert(transition);
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

void convertMooreToMealy(const string& inFileName, const string& outFileName)
{
    ifstream file(inFileName);
    ofstream outFile(outFileName);
    string line;
    getline(file, line);
    vector<string> tempRowOut = split(line, ';');
    getline(file, line);
    vector<string> tempRowStates = split(line, ';');


    unordered_map<string, string> stateToOut;

    for (size_t i = 1; i < tempRowOut.size(); ++i)
    {
        stateToOut[tempRowStates[i]] = tempRowOut[i]; 
        outFile << ";" << tempRowStates[i];
    }
    outFile << "\n";

    while (getline(file, line))
    {
        vector<string> rowNextSt = split(line, ';');
        outFile << rowNextSt[0] << ";";
        for (int i = 1; i < rowNextSt.size(); i++)
        {
            outFile << rowNextSt[i] << "/" << stateToOut[rowNextSt[i]] << ";";
        }
        outFile << "\n";
    }
    file.close();
    outFile.close();
}

void convertMealyToMoore(const string& inFileName, const string outFileName)
{
    MealyAutomata mealy;
    readMealy(inFileName, mealy);

    MooreAutomata moore;

    unordered_set<size_t> reachableStates = findReachableStatesMealy(mealy);

    MealyAutomata filteredMealy;
    filteredMealy.entries = mealy.entries;
    filteredMealy.transitions = vector<vector<pair<string, string>>>(filteredMealy.entries.size());
    for (const auto& index : reachableStates)
    {
        filteredMealy.states.push_back(mealy.states[index]);
        for (size_t i = 0; i < filteredMealy.entries.size(); i++)
        {
            filteredMealy.transitions[i].push_back(mealy.transitions[i][index]);
        }
    }

    auto statesForMoore = extractMooreStates(filteredMealy.transitions, filteredMealy.states[0]);

    for (size_t i = 0; i < statesForMoore.size(); i++)
    {
        moore.states.push_back(make_pair("q" + to_string(i), statesForMoore[i].second));
    }

    moore.entries = filteredMealy.entries;

    vector<vector<string>> transitionsMoore(filteredMealy.transitions.size(),
        vector<string>(filteredMealy.transitions[0].size()));

    for (size_t i = 0; i < filteredMealy.transitions.size(); i++)
    {
        for (size_t j = 0; j < filteredMealy.transitions[0].size(); j++)
        {
            size_t k = 0;
            auto transition = filteredMealy.transitions[i][j];

            for (const auto& state : statesForMoore)
            {
                if (transition == state)
                {
                    transitionsMoore[i][j] = moore.states[k].first;
                }
                k++;
            }
        }
    }

    vector<vector<string>> transitions(filteredMealy.transitions.size(),
        vector<string>(statesForMoore.size()));

    for (size_t i = 0; i < moore.states.size(); i++)
    {
        for (size_t j = 0; j < moore.entries.size(); j++)
        {
            for (size_t k = 0; k < filteredMealy.states.size(); k++)
            {
                if (statesForMoore[i].first == filteredMealy.states[k])
                {
                    transitions[j][i] = transitionsMoore[j][k];
                }
            }
        }
    }

    moore.transitions = transitions;

    writeMoore(outFileName, moore);
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
        convertMealyToMoore(inputFileName, outputFileName);
    }
    else
    {
        convertMooreToMealy(inputFileName, outputFileName);
    }

    return 0;
}
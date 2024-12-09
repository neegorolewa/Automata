#include "MealyMooreConverter.h"

using namespace std;

const string CONVERSION_TYPE_MEALY_TO_MOORE = "mealy-to-moore";
const string CONVERSION_TYPE_MOORE_TO_MEALY = "moore-to-mealy";

struct MealyState 
{
    string state;
    string output;
};

struct MealyAutomata 
{
    vector<string> states;
    vector<string> inputSymbols;
    unordered_map<string, vector<MealyState>> transitions;
};

struct MooreAutomata 
{
    vector<string> states;
    vector<string> inputSymbols;
    unordered_map<string, string> outputs;
    unordered_map<string, vector<string>> transitions;
};

MealyAutomata readMealy(const string& inputFile) 
{
    MealyAutomata mealy;

    ifstream file(inputFile);

    if (!file.is_open()) 
    {
        cerr << "Error: Could not open file " << inputFile << endl;
        return mealy;
    }

    string line;
    bool firstLine = true; //считывание состояний

    while (getline(file, line)) 
    {
        istringstream iss(line);
        string token;
        vector<string> tokens;

        while (getline(iss, token, ';')) 
        {
            tokens.push_back(token);
        }

        if (firstLine) 
        {
            // Первая строка содержит состояния
            for (size_t i = 1; i < tokens.size(); ++i) 
            {
                mealy.states.push_back(tokens[i]);
            }

            firstLine = false;
        }
        else 
        {
            // Последующие строки содержат переходы
            string inputSymbol = tokens[0];

            mealy.inputSymbols.push_back(inputSymbol);

            for (size_t i = 1; i < tokens.size(); ++i) 
            {
                size_t slashPos = tokens[i].find('/');

                if (slashPos != string::npos) 
                {
                    string state = tokens[i].substr(0, slashPos);
                    string output = tokens[i].substr(slashPos + 1);
                    mealy.transitions[inputSymbol].push_back({ state, output });
                }
            }
        }
    }

    file.close();
    return mealy;
}

void writeMealy(const string& outputFile, const MealyAutomata& mealy) 
{
    ofstream file(outputFile);

    if (!file.is_open()) 
    {
        cerr << "Error: Could not open file " << outputFile << endl;
        return;
    }

    // Записываем состояния
    file << ";";
    for (size_t i = 0; i < mealy.states.size(); i++)  
    {
        file << mealy.states[i];
        if (i < mealy.states.size() - 1)
        {
            file << ";";
        }
    }

    file << endl;

    // Записываем переходы
    for (const auto& inputSymbol : mealy.inputSymbols) 
    {
        file << inputSymbol << ";";

        for (size_t i = 0; i < mealy.transitions.at(inputSymbol).size(); i++) 
        {
            const auto& transition = mealy.transitions.at(inputSymbol)[i];

            file << transition.state << "/" << transition.output;

            if (i < mealy.transitions.at(inputSymbol).size() - 1) 
            {
                file << ";";
            }
        }
        file << endl;
    }

    file.close();
}

MooreAutomata readMoore(const string& inputFile)
{
    MooreAutomata moore;
    ifstream file(inputFile);
    if (!file.is_open()) 
    {
        cerr << "Error: Could not open file " << inputFile << endl;
        return moore;
    }

    string line;
    bool firstLine = true;
    bool secondLine = false;
    vector<string> outputs;

    while (getline(file, line))
    {
        istringstream iss(line);
        string token;
        vector<string> tokens;

        while (getline(iss, token, ';')) 
        {
            tokens.push_back(token);
        }

        if (firstLine) {
            // Первая строка содержит выходные символы
            for (size_t i = 1; i < tokens.size(); ++i) 
            {
                outputs.push_back(tokens[i]);
            }
            firstLine = false;
            secondLine = true;
        }
        else if (secondLine) 
        {
            // Вторая строка содержит состояния
            for (size_t i = 1; i < tokens.size(); ++i) 
            {
                string state = tokens[i];
                moore.states.push_back(state);
                moore.outputs[state] = outputs[i - 1]; // Сопоставляем состояние с выходным символом
            }
            secondLine = false;
        }
        else
        {
            // Последующие строки содержат переходы
            string inputSymbol = tokens[0];
            moore.inputSymbols.push_back(inputSymbol);
            for (size_t i = 1; i < tokens.size(); ++i) 
            {
                moore.transitions[inputSymbol].push_back(tokens[i]);
            }
        }
    }

    file.close();
    return moore;
}

void writeMoore(const string& outputFile, const MooreAutomata& moore) 
{
    ofstream file(outputFile);

    if (!file.is_open()) 
    {
        cerr << "Error: Could not open file " << outputFile << endl;
        return;
    }

    // Записываем выходные символы
    file << ";";

    for (size_t i = 0; i < moore.states.size(); ++i) 
    {
        file << moore.outputs.at(moore.states[i]);

        if (i < moore.states.size() - 1) 
        {
            file << ";";
        }
    }
    file << endl;

    // Записываем состояния
    file << ";";

    for (size_t i = 0; i < moore.states.size(); ++i) 
    {
        file << moore.states[i];

        if (i < moore.states.size() - 1) 
        {
            file << ";";
        }
    }

    file << endl;

    // Записываем переходы
    for (const auto& inputSymbol : moore.inputSymbols) 
    {
        file << inputSymbol << ";";

        for (size_t i = 0; i < moore.transitions.at(inputSymbol).size(); ++i) 
        {
            file << moore.transitions.at(inputSymbol)[i];

            if (i < moore.transitions.at(inputSymbol).size() - 1) 
            {
                file << ";";
            }
        }

        file << endl;
    }

    file.close();
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
            string nextState = moore.transitions.at(inputSymbol)[find(moore.states.begin(), moore.states.end(), currState) - moore.states.begin()];
            if (visited.find(nextState) == visited.end())
            {
                visited.insert(nextState);
                queue.push(nextState);
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
            string nextState = mealy.transitions.at(inputSymbol)[find(mealy.states.begin(), mealy.states.end(), currentState) - mealy.states.begin()].state;
            if (visited.find(nextState) == visited.end()) 
            {
                visited.insert(nextState);
                queue.push(nextState);
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



MooreAutomata convertMealyToMoore(MealyAutomata mealy)
{
    MooreAutomata moore;

    

    return moore;
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        cout << "Usage: " << argv[0] << "<conversion-type> <in.csv> <out.csv>" << endl;
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
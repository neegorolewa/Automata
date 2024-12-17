#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <queue>

using namespace std;

vector<string> split(const string& s, char delimiter) 
{
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) 
    {
        tokens.push_back(token);
    }
    return tokens;
}

struct Trans
{
    string inputSym;
    string nextPos; 
    string outSym; 
};

struct TransMoore
{
    string inputSym;
    string nextPos; 
};

struct MealyState
{
    string curr;
    vector<Trans> transitions;
};

struct MooreState
{
    string state;
    string newState;
    string output;
    vector<TransMoore> transitions;
};

vector<MealyState> findMealyReachableStates(const vector<MealyState>& mealyStates)
{
    if (mealyStates.empty()) return {};

    unordered_set<string> visited;
    queue<string> toVisit;
    vector<MealyState> reachableStates;

    visited.insert(mealyStates[0].curr);
    toVisit.push(mealyStates[0].curr);
    reachableStates.push_back(mealyStates[0]);

    while (!toVisit.empty())
    {
        string currentState = toVisit.front();
        toVisit.pop();

        for (const auto& state : mealyStates)
        {
            if (state.curr == currentState)
            {
                for (const auto& transition : state.transitions)
                {
                    if (visited.find(transition.nextPos) == visited.end())
                    {
                        visited.insert(transition.nextPos);
                        toVisit.push(transition.nextPos);

                        for (const auto& nextState : mealyStates)
                        {
                            if (nextState.curr == transition.nextPos)
                            {
                                reachableStates.push_back(nextState);
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    return reachableStates;
}

vector<MooreState> findMooreReachableStates(const vector<MooreState>& mooreStates)
{
    if (mooreStates.empty()) return {};

    unordered_set<string> visited;
    queue<string> toVisit;
    vector<MooreState> reachableStates;

    visited.insert(mooreStates[0].state);
    toVisit.push(mooreStates[0].state);
    reachableStates.push_back(mooreStates[0]);

    while (!toVisit.empty())
    {
        string currentState = toVisit.front();
        toVisit.pop();

        for (const auto& state : mooreStates)
        {
            if (state.state == currentState)
            {
                for (const auto& transition : state.transitions)
                {
                    if (visited.find(transition.nextPos) == visited.end())
                    {
                        visited.insert(transition.nextPos);
                        toVisit.push(transition.nextPos);

                        for (const auto& nextState : mooreStates)
                        {
                            if (nextState.state == transition.nextPos)
                            {
                                reachableStates.push_back(nextState);
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    return reachableStates;
}


vector<MealyState> mealyMin(const vector<MealyState>& mealyAutomaton)
{
    map<string, string> newStateMap; 
    set<vector<string>> vecOutSet;
    map<vector<string>, string> vecOutNewStMap; 
    vector<MealyState> newMealyAutomaton;
    map <string, vector<string>> StateInGroup; 

    char ch = 'A';
    int stateCounter = 0;

    for (const auto& state : mealyAutomaton)
    {
        vector<string> outVector;
        for (const auto& transition : state.transitions)
        {
            outVector.push_back(transition.outSym);
        }

        if (vecOutSet.insert(outVector).second)
        {
            string newMinStateName = ch + to_string(stateCounter);
            vecOutNewStMap[outVector] = newMinStateName;  
            newStateMap[state.curr] = newMinStateName;
            stateCounter++;
        }
        else
        {
            newStateMap[state.curr] = vecOutNewStMap[outVector];
        }

    }

    map <string, vector<MealyState>> NewMealyInGroup;

    for (const auto& state : mealyAutomaton)
    {
        MealyState tempState;
        vector<Trans> tempVec;
        for (const auto& transition : state.transitions)
        {
            Trans newTrans;
            newTrans.inputSym = transition.inputSym;
            newTrans.nextPos = newStateMap[transition.nextPos];

            newTrans.outSym = transition.outSym;
            tempVec.push_back(newTrans);
        }
        tempState.curr = state.curr; 
        tempState.transitions = tempVec;
        NewMealyInGroup[newStateMap[tempState.curr]].push_back(tempState);

    }

    auto prevSize = vecOutSet.size();
    size_t currSize = 0;
    bool equal = 0;
    while (prevSize != currSize)
    {
        ch++;
        int i = 0;
        map<string, string> tempNewStateMap; 
        map <string, vector<MealyState>> tempNewMealyInGroup; 

        for (const auto& group : NewMealyInGroup)
        {
            map<vector<string>, string> vecNextNewStMap; 
            set<vector<string>> vecTransSet;
            for (const auto& state : group.second)
            {
                vector<string> transVector;
                for (const auto& transition : state.transitions)
                {
                    transVector.push_back(transition.nextPos);

                }
                if (vecTransSet.insert(transVector).second) 
                {
                    string newMinStateName = ch + to_string(i);
                    vecNextNewStMap[transVector] = newMinStateName; 
                    tempNewStateMap[state.curr] = newMinStateName;
                    i++;
                }
                else
                {
                    tempNewStateMap[state.curr] = vecNextNewStMap[transVector];
                }
                currSize += vecTransSet.size();
            }
        }
        for (const auto& state : mealyAutomaton)
        {
            MealyState tempState;
            vector<Trans> tempVec;
            for (const auto& transition : state.transitions)
            {
                Trans newTrans;
                newTrans.inputSym = transition.inputSym;
                newTrans.nextPos = tempNewStateMap[transition.nextPos];
                newTrans.outSym = transition.outSym;
                tempVec.push_back(newTrans);
            }
            tempState.curr = state.curr;
            tempState.transitions = tempVec;
            tempNewMealyInGroup[tempNewStateMap[tempState.curr]].push_back(tempState);
        }


        NewMealyInGroup = tempNewMealyInGroup;
        newStateMap = tempNewStateMap;

        if (prevSize == currSize)
        {
            break;
        }
        else
        {
            prevSize = currSize;
            currSize = 0;
        }


    }

    vector<MealyState> tempMealy;

    for (const auto& group : NewMealyInGroup)
    {
        vector<Trans> tempVec;
        MealyState tempState = group.second[0];

        for (const auto& trans : tempState.transitions)
        {
            Trans newTrans;
            newTrans.inputSym = trans.inputSym;
            newTrans.nextPos = trans.nextPos;
            newTrans.outSym = trans.outSym;
            tempVec.push_back(newTrans);
        }

        tempState.curr = group.first;
        tempState.transitions = tempVec;
        NewMealyInGroup[newStateMap[tempState.curr]].push_back(tempState);
        tempMealy.push_back(tempState);
    }

    return tempMealy;


}

vector<MooreState> mooreMin(const vector<MooreState>& mooreAutomaton)
{
    map<string, string> newStateMap;
    set<string> outSet;
    map<string, string> outNewStMap;
    vector<MealyState> newMealyAutomaton;
    map <string, vector<string>> StateInGroup; 

    char ch = 'A';
    int stateCounter = 0;

    for (const auto& state : mooreAutomaton)
    {
        vector<string> outVector;

        if (outSet.insert(state.output).second)
        {
            string newMinStateName = ch + to_string(stateCounter);
            outNewStMap[state.output] = newMinStateName; 
            newStateMap[state.state] = newMinStateName;
            stateCounter++;
        }
        else
        {
            newStateMap[state.state] = outNewStMap[state.output];
        }
    }


    map <string, vector<MooreState>> NewMooreInGroup; 

    for (const auto& state : mooreAutomaton)
    {
        MooreState tempState;
        vector<TransMoore> tempVec;
        for (const auto& transition : state.transitions)
        {
            TransMoore newTrans;
            newTrans.inputSym = transition.inputSym;
            newTrans.nextPos = newStateMap[transition.nextPos];
            tempVec.push_back(newTrans);
        }
        tempState.output = state.output;
        tempState.state = state.state;
        tempState.transitions = tempVec;
        NewMooreInGroup[newStateMap[tempState.state]].push_back(tempState);

    }

    auto prevSize = outSet.size();
    size_t currSize = 0;
    bool equal = 0;
    while (prevSize != currSize)
    {
        ch++;
        int i = 0;
        currSize = 0;
        map<string, string> tempNewStateMap; 
        map <string, vector<MooreState>> tempNewMooreInGroup; 

        for (const auto& group : NewMooreInGroup)
        {
            map<vector<string>, string> vecNextNewStMap; 
            set<vector<string>> vecTransSet;
            for (const auto& state : group.second)
            {
                vector<string> transVector;
                for (const auto& transition : state.transitions)
                {
                    transVector.push_back(transition.nextPos);
                }
                if (vecTransSet.insert(transVector).second) 
                {
                    string newMinStateName = ch + to_string(i);
                    vecNextNewStMap[transVector] = newMinStateName;
                    tempNewStateMap[state.state] = newMinStateName;
                    i++;
                }
                else
                {
                    tempNewStateMap[state.state] = vecNextNewStMap[transVector];
                }
                currSize += vecTransSet.size();
            }
        }
        for (const auto& state : mooreAutomaton)
        {
            MooreState tempState;
            vector<TransMoore> tempVec;
            for (const auto& transition : state.transitions)
            {
                TransMoore newTrans;
                newTrans.inputSym = transition.inputSym;
                newTrans.nextPos = tempNewStateMap[transition.nextPos];
                tempVec.push_back(newTrans);
            }
            tempState.state = state.state; 
            tempState.output = state.output;
            tempState.transitions = tempVec;
            tempNewMooreInGroup[tempNewStateMap[tempState.state]].push_back(tempState);
        }

        NewMooreInGroup = tempNewMooreInGroup;
        newStateMap = tempNewStateMap;

        if (prevSize == currSize)
        {
            break;
        }
        else
        {
            prevSize = currSize;
            currSize = 0;
        }

    }

    vector<MooreState> tempMealy;

    for (const auto& group : NewMooreInGroup)
    {
        vector<TransMoore> tempVec;
        MooreState tempState = group.second[0];

        for (const auto& trans : tempState.transitions)
        {
            TransMoore newTrans;
            newTrans.inputSym = trans.inputSym;
            newTrans.nextPos = trans.nextPos;
            tempVec.push_back(newTrans);
        }
        tempState.state = group.first;
        tempState.output = group.second[0].output;
        tempState.transitions = tempVec;
        NewMooreInGroup[newStateMap[tempState.state]].push_back(tempState);
        tempMealy.push_back(tempState);
    }

    return tempMealy;
}

vector<MealyState> ReadMealy(vector<MealyState>& positions, ifstream& file)
{

    string line;
    getline(file, line);
    vector<string> tempRow = split(line, ';');


    for (size_t i = 1; i < tempRow.size(); ++i) 
    {
        MealyState currPos;
        currPos.curr = tempRow[i];
        positions.push_back(currPos);
    }

    vector<string> tRow;
    while (getline(file, line))
    {
        tempRow = split(line, ';');
        string input_sym = tempRow[0];
        for (int i = 0; i < positions.size(); i++)
        {

            Trans trans;
            tRow = split(tempRow[i + 1], '/');
            trans.inputSym = input_sym;
            trans.nextPos = tRow[0];
            trans.outSym = tRow[1];
            positions[i].transitions.push_back(trans);
        }
    };
    file.close();

    return positions;
}

vector<MooreState> ReadMoore(vector<MooreState>& positions, ifstream& file)
{
    string line, lineSt;
    getline(file, line);
    vector<string> tempRow = split(line, ';');
    getline(file, lineSt);
    vector<string> tempRowSt = split(lineSt, ';');


    for (size_t i = 1; i < tempRow.size(); ++i) 
    {
        MooreState state;
        state.output = tempRow[i];
        state.state = tempRowSt[i]; 
        positions.push_back(state);
    }

    vector<string> tRow;

    while (getline(file, line))
    {
        tempRow = split(line, ';');
        string input_sym = tempRow[0];
        for (int i = 0; i < positions.size(); i++)
        {

            TransMoore trans;
            tRow = split(tempRow[i + 1], '/');
            trans.inputSym = input_sym;
            trans.nextPos = tRow[0];
            positions[i].transitions.push_back(trans);
        }
    };
    file.close();

    return positions;
}


void WriteMealy(vector<MealyState>& mealyAutomaton, ofstream& outFile) 
{
    if (outFile.is_open()) 
    {
        for (const auto& state : mealyAutomaton) 
        {
            outFile << ";" << state.curr;
        }
        outFile << "\n";

        for (size_t i = 0; i < mealyAutomaton[0].transitions.size(); ++i) 
        {
            const auto& inputSym = mealyAutomaton[0].transitions[i].inputSym;
            outFile << inputSym;

            for (const auto& state : mealyAutomaton) 
            {
                const auto& trans = state.transitions[i];
                outFile << ";" << trans.nextPos << "/" << trans.outSym;
            }
            outFile << "\n";
        }
        outFile.close();
    }
    else 
    {
        cerr << "Failed to open file for writing.\n";
    }
}

void WriteMoore(vector<MooreState>& mooreAutomaton, ofstream& outFile)
{
    if (outFile.is_open())
    {

        for (const auto& state : mooreAutomaton)
        {
            outFile << ";" << state.output;
        }
        outFile << "\n";

        for (const auto& state : mooreAutomaton)
        {
            outFile << ";" << state.state;
        }
        outFile << "\n";

        for (size_t i = 0; i < mooreAutomaton[0].transitions.size(); ++i) 
        {
            const auto& inputSym = mooreAutomaton[0].transitions[i].inputSym;
            outFile << inputSym;

            for (const auto& state : mooreAutomaton) 
            {
                const auto& trans = state.transitions[i];
                outFile << ";" << trans.nextPos;
            }
            outFile << "\n";
        }

        outFile.close();
    }
    else
    {
        cerr << "Failed to open file for writing.\n";
    }

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

    ifstream file(inputFileName);
    ofstream outFile(outputFileName);

    if (!file.is_open())
    {
        cerr << "Ошибка при открытии файла!" << endl;
        return 1;
    }

    if (type == "mealy")
    {
        vector<MealyState> mealyStates;
        mealyStates = ReadMealy(mealyStates, file);
        vector<MealyState> reachableStates = findMealyReachableStates(mealyStates);
        vector<MealyState> minimize = mealyMin(reachableStates);
        WriteMealy(minimize, outFile);
    }
    else
    {
        vector<MooreState> mooreStates;
        mooreStates = ReadMoore(mooreStates, file);
        vector<MooreState> reachableStates = findMooreReachableStates(mooreStates);
        vector<MooreState> minimize = mooreMin(reachableStates);
        WriteMoore(minimize, outFile);
    }

    return 0;
}
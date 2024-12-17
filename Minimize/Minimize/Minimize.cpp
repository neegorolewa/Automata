#include <iostream>
#include <vector>

using namespace std;

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

int main()
{

}

#pragma once

#include <vector>
#include <string>
#include <optional>

using namespace std;

class DataLoader
{
public:
    int numNodes;
    vector<int> nodeProfits;
    vector<vector<int>> distanceMatrix;
    string name;

    DataLoader(const string &filename, const optional<string> &name = nullopt);
    DataLoader(vector<int> profits, vector<vector<int>> distances, const optional<string> &name = nullopt) : numNodes(profits.size()), nodeProfits(profits), distanceMatrix(distances), name(name ? *name : "") {};
    string getName() const { return name; }
};

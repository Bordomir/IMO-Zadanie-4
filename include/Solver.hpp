#pragma once

#include <vector>
#include <string>

#include "DataLoader.hpp"

using namespace std;

class Solver
{
public:
    DataLoader *data;
    int startNode;
    vector<int> solution;
    int solutionScoreAfterIPhaseI;
    int solutionScore;

    Solver(DataLoader &data, int startNode) : data(&data), startNode(startNode), solution(), solutionScoreAfterIPhaseI(0), solutionScore(0) {};
    virtual string getAlgorithmName() = 0;
    virtual void solve() = 0;    
    int calculateLength();
    int calculateScore();
    void print();
    void saveToFile(const string &filename = "");
};

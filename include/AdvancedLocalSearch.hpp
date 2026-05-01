#pragma once

#include <vector>
#include <chrono>
#include <string>

#include "DataLoader.hpp"
#include "RandomSolver.hpp"
#include "MemorySteepLocalSearch.hpp"

using namespace std;

class AdvancedLocalSearch
{
public:
    DataLoader *data;
    RandomSolver *randomSolver;
    MemorySteepLocalSearch *LocalSearch;

    vector<int> bestSolution;
    int bestSolutionScore;

    int currentIterations;
    chrono::time_point<chrono::steady_clock> startTime;

    // Stop conditions
    bool usesTimeLimit;
    int maxIterations;
    double timeLimit;

    // Flags
    bool needsStartingSolution;
    bool usesLocalSearch;
    
    AdvancedLocalSearch(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch, int maxIterations = -1, double timeLimit = -1, bool needsStartingSolution = true, bool usesLocalSearch = true);
    virtual ~AdvancedLocalSearch() = default;
    virtual string getAlgorithmName() const = 0;
    void setData(DataLoader &data);
    void solve();
    vector<int> getRandomizedSolution() const;
    vector<int> improveSolution(vector<int> &solution) const;
    bool canContinue() const;
    virtual vector<int> createNewSolution() const = 0;
    int calculateScore(const vector<int> &solution) const;
    void print() const;
    void saveToFile(const string &filename = "") const;
};

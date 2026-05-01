#pragma once

#include <random>
#include <string>
#include <vector>

#include "AdvancedLocalSearch.hpp"
#include "DataLoader.hpp"
#include "RandomSolver.hpp"
#include "MemorySteepLocalSearch.hpp"

using namespace std;

class ILS : public AdvancedLocalSearch {
public:
    std::mt19937 rng;
    
    ILS(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch, int maxIterations = -1, double timeLimit = -1, bool needsStartingSolution = true, bool usesLocalSearch = true) : AdvancedLocalSearch(data, randomSolver, localSearch, maxIterations, timeLimit, needsStartingSolution, usesLocalSearch), rng(std::random_device{}()) {}
    ILS(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch, unsigned int seed, int maxIterations = -1, double timeLimit = -1, bool needsStartingSolution = true, bool usesLocalSearch = true) : AdvancedLocalSearch(data, randomSolver, localSearch, maxIterations, timeLimit, needsStartingSolution, usesLocalSearch), rng(seed) {}
    string getAlgorithmName() const override;
    vector<int> createNewSolution() override;
private:
    int randomInt(int min, int max);
};
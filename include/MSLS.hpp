#pragma once

#include <string>
#include <vector>

#include "AdvancedLocalSearch.hpp"
#include "DataLoader.hpp"
#include "RandomSolver.hpp"
#include "MemorySteepLocalSearch.hpp"

using namespace std;

class MSLS : public AdvancedLocalSearch {
    public:
        MSLS(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch, int maxIterations = -1, double timeLimit = -1, bool needsStartingSolution = true, bool usesLocalSearch = true) : AdvancedLocalSearch(data, randomSolver, localSearch, maxIterations, timeLimit, needsStartingSolution, usesLocalSearch) {}
        string getAlgorithmName() const override;
        vector<int> createNewSolution() const override;
};
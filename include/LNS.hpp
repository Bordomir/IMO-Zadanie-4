#pragma once

#include <string>
#include <vector>

#include "AdvancedLocalSearch.hpp"
#include "DataLoader.hpp"
#include "RandomSolver.hpp"
#include "MemorySteepLocalSearch.hpp"


class LNS : public AdvancedLocalSearch
{
public:
    LNS(DataLoader& data,
        RandomSolver& randomSolver,
        MemorySteepLocalSearch& localSearch,
        unsigned int seed,
        int destructionPercentage,
        int maxIterations,
        double timeLimit,
        bool needsStartingSolution,
        bool usesLocalSearch);

    std::string getAlgorithmName() const override;
    std::vector<int> createNewSolution() override;

private:
    std::mt19937 rng_;
    int destructionPercentage_;

    void destroy(std::vector<int>& solution);
    void repair(std::vector<int>& solution);
    int randomInt(int min, int max);
};

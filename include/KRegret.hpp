#pragma once

#include <string>
#include <optional>

#include "Solver.hpp"

class KRegret : public Solver
{
public:
    KRegret(DataLoader& data, int startNode, int k, std::optional<double> weight = std::nullopt);

    std::string getAlgorithmName() override;
    void solve() override;

private:
    // int startNode_;
    int k_;
    std::optional<double> weight_;

    void solvePhaseI();
    void solvePhaseII();
};

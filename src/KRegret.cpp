#include "../include/KRegret.hpp"
#include "../include/Common.hpp"

#include <format>
#include <utility>
#include <unordered_map>
#include <cmath>


namespace
{

struct Insertion
{
    size_t pos;
    int cost;
};

}

KRegret::KRegret(DataLoader& data, int startNode, int k, std::optional<double> weight)
    : Solver(data, startNode)
    , k_(k)
    , weight_(weight)
{
}

std::string KRegret::getAlgorithmName()
{
    std::string name = std::format("{}Regret", k_);

    if (weight_)
    {
        name += std::format("_weighted[{:g}]", *weight_);
    }

    return name;
}

void KRegret::solve()
{
    solution.clear();

    solvePhaseI();

    solutionScoreAfterIPhaseI = calculateLength();

    solvePhaseII();

    solutionScore = calculateScore();
}

void KRegret::solvePhaseI()
{
    solution.reserve(data->numNodes);
    std::vector<bool> visited(data->numNodes, false);

    solution.push_back(startNode);
    visited[startNode] = true;

    for (int i = solution.size(); i < data->numNodes; ++i)
    {
        int bestNode = -1;
        size_t bestPos = 0;
        int bestCost = std::numeric_limits<int>::max();
        int bestRegret = std::numeric_limits<int>::min();

        for (int currNode = 0; currNode < data->numNodes; ++currNode)
        {
            if (visited[currNode]) continue;

            std::vector<Insertion> insertions;
            insertions.reserve(solution.size());

            for (size_t prevIndex = 0; prevIndex < solution.size(); ++prevIndex)
            {
                size_t nextIndex = (prevIndex + 1) % solution.size();

                int prevNode = solution[prevIndex];
                int nextNode = solution[nextIndex];

                int cost = data->distanceMatrix[prevNode][currNode]
                           + data->distanceMatrix[currNode][nextNode]
                           - data->distanceMatrix[prevNode][nextNode]
                           - data->nodeProfits[currNode];

                insertions.emplace_back(prevIndex + 1, cost);
            }

            std::sort(
                insertions.begin(),
                insertions.end(),
                [](const Insertion& a, const Insertion& b)
                {
                    return a.cost < b.cost;  // sortowanie rosnąco po kosztach
                });

            int regret = 0;

            for (int j = 1; j < std::min(k_, static_cast<int>(insertions.size())); ++j)
            {
                regret += insertions[j].cost - insertions[0].cost;
            }

            if (weight_)
            {
                regret += std::round(*weight_ * insertions[0].cost);
            }

            // decyzja o wyborze nowego wierzchołka na podstawie większego żalu, w przypadku remisu na podstawie kosztu
            if (regret > bestRegret || (regret == bestRegret && insertions[0].cost < bestCost))
            {
                bestNode = currNode;
                bestPos = insertions[0].pos;
                bestCost = insertions[0].cost;
                bestRegret = regret;
            }
        }

        solution.insert(solution.begin() + bestPos, bestNode);
        visited[bestNode] = true;
    }
}

void KRegret::solvePhaseII()
{
    common::improveByRemovingNodes(solution, data);
}

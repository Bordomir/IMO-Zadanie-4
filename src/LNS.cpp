#include "../include/LNS.hpp"
#include <algorithm>
#include <numeric>

#include "../include/Common.hpp"


namespace
{
struct Insertion { size_t pos; int cost; };
}

LNS::LNS(DataLoader& data,
         RandomSolver& randomSolver,
         MemorySteepLocalSearch& localSearch,
         unsigned int seed,
         int destructionPercentage,
         int maxIterations,
         double timeLimit,
         bool needsStartingSolution,
         bool usesLocalSearch)
    : AdvancedLocalSearch(data, randomSolver, localSearch, maxIterations, timeLimit, needsStartingSolution, usesLocalSearch)
    , rng_(seed)
    , destructionPercentage_(destructionPercentage)
{
}

std::string LNS::getAlgorithmName() const
{
    return format("LNS_{}", destructionPercentage_);
}

std::vector<int> LNS::createNewSolution()
{
    std::vector<int> newSolution(bestSolution);

    destroy(newSolution);
    repair(newSolution);

    return newSolution;
}

void LNS::destroy(std::vector<int>& solution)
{
    int numNodesToRemove = (solution.size() * destructionPercentage_) / 100;
    if (numNodesToRemove == 0  && !solution.empty()) numNodesToRemove = 1;

    std::vector<int> indices(solution.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng_);

    std::vector<int> indicesToRemove(indices.begin(), indices.begin() + numNodesToRemove);
    std::sort(indicesToRemove.rbegin(), indicesToRemove.rend());

    for (int index : indicesToRemove)
    {
        solution.erase(solution.begin() + index);
    }
}

void LNS::repair(std::vector<int>& solution)
{
    if (solution.empty())
    {
        solution.push_back(std::uniform_int_distribution<int>(0, data->numNodes - 1)(rng_));
    }

    constexpr int K = 2;
    std::vector<bool> visited(data->numNodes, false);

    for (int node : solution)
    {
        visited[node] = true;
    }

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

            int kElements = std::min(K, static_cast<int>(insertions.size()));

            std::partial_sort(
                insertions.begin(),
                insertions.begin() + kElements,
                insertions.end(),
                [](const Insertion& a, const Insertion& b)
                {
                    return a.cost < b.cost;
                });

            int regret = 0;

            for (int j = 1; j < kElements; ++j)
            {
                regret += insertions[j].cost - insertions[0].cost;
            }

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

    common::improveByRemovingNodes(solution, data);
}

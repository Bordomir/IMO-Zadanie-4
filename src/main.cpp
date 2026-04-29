#include <string>
#include <vector>
#include <memory>
#include <print>
#include <random>
#include <limits>
#include <ranges>
#include <algorithm>
#include <utility>
#include <chrono>

#include "../include/DataLoader.hpp"
#include "../include/Solver.hpp"
#include "../include/RandomSolver.hpp"
#include "../include/KRegret.hpp"
#include "../include/LocalSearch.hpp"
#include "../include/MemorySteepLocalSearch.hpp"

using namespace std;

struct Statistic
{
    string data;
    string solver;
    string localSearch;
    double average = 0;
    double min = numeric_limits<double>::max();
    double max = numeric_limits<double>::min();

    Statistic(string data, string solver, string localSearch) : data(move(data)), solver(move(solver)), localSearch(move(localSearch)) {};
    void update(double value)
    {
        average += value;
        min = std::min(min, value);
        max = std::max(max, value);
    }
    void print() const
    {
        std::println("{};{};{};{:.4f};{:.4f};{:.4f}", data, solver, localSearch, average, min, max);
    }
};

int main()
{
    DataLoader dataA("../data/TSPA.csv", "DataA");
    DataLoader dataB("../data/TSPB.csv", "DataB");

    int startNode = 0;

    vector<unique_ptr<Solver>> startingPointSolvers;
    startingPointSolvers.reserve(2);
    startingPointSolvers.emplace_back(make_unique<RandomSolver>(dataA, 0));
    startingPointSolvers.emplace_back(make_unique<RandomSolver>(dataB, 1));
    
    vector<unique_ptr<Solver>> KRegrets;
    KRegrets.reserve(2);
    KRegrets.emplace_back(make_unique<KRegret>(dataA, startNode, 2));
    KRegrets.emplace_back(make_unique<KRegret>(dataB, startNode, 2));


    vector<unique_ptr<LocalSearch>> localSearches;
    localSearches.reserve(3);
    // localSearches.emplace_back(make_unique<SteepLocalSearch>(startingPointSolvers[0], MoveType::SwapEdges));
    localSearches.emplace_back(make_unique<MemorySteepLocalSearch>(startingPointSolvers[0], MoveType::SwapEdges));
    // localSearches.emplace_back(make_unique<CandidateSteepLocalSearch>(startingPointSolvers[0], 10));
    mt19937 rng{42};

    // unique_ptr<Solver> randomSolverA = make_unique<RandomSolver>(dataA, startNode);
    // randomSolverA->solve();
    // MemorySteepLocalSearch localSearch(randomSolverA, MoveType::SwapEdges);
    // localSearch.improve();

    // localSearch.print();

    // exit(0);
    
    // Experiment
    int numRuns = 100;
    int maxNode = min(dataA.numNodes, dataB.numNodes);
    int maxTestsPossible = min(numRuns, maxNode);

    std::println("Running {} tests", maxTestsPossible);

    auto allNodesView = views::iota(0, maxNode);

    vector<int> startingNodes(maxTestsPossible);
    ranges::sample(allNodesView, startingNodes.begin(), maxTestsPossible, rng);

    vector<Statistic> scoreStatistics;
    scoreStatistics.reserve(startingPointSolvers.size() * localSearches.size());
    vector<Statistic> timeStatistics;
    timeStatistics.reserve(startingPointSolvers.size() * localSearches.size());
    vector<Statistic> scoreStatisticsForKRegret;
    scoreStatisticsForKRegret.reserve(KRegrets.size());
    vector<Statistic> timeStatisticsForKRegret;
    timeStatisticsForKRegret.reserve(KRegrets.size());
    for (auto &solver : startingPointSolvers)
    {
        for (auto &localSearch : localSearches)
        {
            scoreStatistics.emplace_back(
                solver->data->getName(),
                solver->getAlgorithmName(),
                localSearch->getAlgorithmName());
            timeStatistics.emplace_back(
                solver->data->getName(),
                solver->getAlgorithmName(),
                localSearch->getAlgorithmName());
        }
    }
    for (auto &solver : KRegrets)
    {
        scoreStatisticsForKRegret.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName(),
            "None");
        timeStatisticsForKRegret.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName(),
            "None");
    }

    chrono::time_point<chrono::steady_clock> startTime, endTime;
    for (int startNode : startingNodes)
    {
        for (size_t i = 0; i < startingPointSolvers.size(); i++)
        {
            const auto &solver = startingPointSolvers[i];

            solver->startNode = startNode;
            solver->solve();

            for (size_t j = 0; j < localSearches.size(); j++)
            {
                const auto &localSearch = localSearches[j];

                localSearch->data = solver->data;
                localSearch->solution = solver->solution;

                startTime = chrono::steady_clock::now();
                localSearch->improve();
                endTime = chrono::steady_clock::now();

                int index = i * localSearches.size() + j;
                if (localSearch->solutionScore > scoreStatistics[index].max)
                {
                    localSearch->saveToFile(format("{}_{}", solver->data->getName(), solver->getAlgorithmName()));
                }

                scoreStatistics[index].update(localSearch->solutionScore);
                timeStatistics[index].update(chrono::duration<double, std::milli>(endTime - startTime).count());
            }
        }
    }
    for (int startNode : startingNodes)
    {
        for (size_t i = 0; i < KRegrets.size(); i++)
        {
            const auto &solver = KRegrets[i];

            solver->startNode = startNode;

            startTime = chrono::steady_clock::now();
            solver->solve();
            endTime = chrono::steady_clock::now();

            if (solver->solutionScore > scoreStatisticsForKRegret[i].max)
            {
                solver->saveToFile(solver->data->getName());
            }

            scoreStatisticsForKRegret[i].update(solver->solutionScore);
            timeStatisticsForKRegret[i].update(chrono::duration<double, std::milli>(endTime - startTime).count());
        }
    }
    for (auto &stat : scoreStatisticsForKRegret)
        stat.average /= maxTestsPossible;
    for (auto &stat : timeStatisticsForKRegret)
        stat.average /= maxTestsPossible;
    for (auto &stat : scoreStatistics)
        stat.average /= maxTestsPossible;
    for (auto &stat : timeStatistics)
        stat.average /= maxTestsPossible;
    
    auto allScoreStatistics = {
        scoreStatisticsForKRegret,
        scoreStatistics,
    };

    println("\nScore statistics:");
    for(const auto &stat : allScoreStatistics | views::join)
        stat.print();

    auto allTimeStatistics = {
        timeStatisticsForKRegret,
        timeStatistics
    };

    println("\nTime statistics:");
    for(const auto &stat : allTimeStatistics | views::join)
        stat.print();

    return 0;
}
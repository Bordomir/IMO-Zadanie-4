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
#include "../include/AdvancedLocalSearch.hpp"
#include "../include/MSLS.hpp"
#include "../include/ILS.hpp"

using namespace std;

struct Statistic
{
    string data;
    string solver;
    double average = 0;
    double min = numeric_limits<double>::max();
    double max = numeric_limits<double>::min();

    Statistic(string data, string solver) : data(move(data)), solver(move(solver)) {};
    void update(double value)
    {
        average += value;
        min = std::min(min, value);
        max = std::max(max, value);
    }
    void print() const
    {
        std::println("{};{};{:.4f};{:.4f};{:.4f}", data, solver, average, min, max);
    }
};

int main()
{
    DataLoader dataA("../data/TSPA.csv", "DataA");
    DataLoader dataB("../data/TSPB.csv", "DataB");

    RandomSolver randomSolverA(dataA, 0);
    RandomSolver randomSolverB(dataB, 0);

    MemorySteepLocalSearch LocalSearchA(dataA, vector<int>{});
    MemorySteepLocalSearch LocalSearchB(dataB, vector<int>{});

    vector<unique_ptr<AdvancedLocalSearch>> MSLSadvancedLocalSearches;
    MSLSadvancedLocalSearches.reserve(2);
    MSLSadvancedLocalSearches.emplace_back(make_unique<MSLS>(dataA, randomSolverA, LocalSearchA, 200, -1, false, true));
    MSLSadvancedLocalSearches.emplace_back(make_unique<MSLS>(dataB, randomSolverB, LocalSearchB, 200, -1, false, true));

    // Experiment
    int numRuns = 20;

    vector<Statistic> MSLSscoreStatistics;
    MSLSscoreStatistics.reserve(MSLSadvancedLocalSearches.size());
    vector<Statistic> MSLStimeStatistics;
    MSLStimeStatistics.reserve(MSLSadvancedLocalSearches.size());

    for (auto &solver : MSLSadvancedLocalSearches)
    {
        MSLSscoreStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        MSLStimeStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
    }
    chrono::time_point<chrono::steady_clock> startTime, endTime;
    for (int run = 0; run < numRuns; run++)
    {
        for (size_t i = 0; i < MSLSadvancedLocalSearches.size(); i++)
        {
            const auto &solver = MSLSadvancedLocalSearches[i];

            println("{} - {} - {}", run, solver->data->getName(), solver->getAlgorithmName());

            startTime = chrono::steady_clock::now();
            solver->solve();
            endTime = chrono::steady_clock::now();

            if (solver->bestSolutionScore > MSLSscoreStatistics[i].max)
            {
                solver->saveToFile(format("{}_{}", solver->data->getName(), solver->getAlgorithmName()));
            }

            MSLSscoreStatistics[i].update(solver->bestSolutionScore);
            MSLStimeStatistics[i].update(chrono::duration<double, std::milli>(endTime - startTime).count());
        }
    }
    for (auto &stat : MSLSscoreStatistics)
        stat.average /= numRuns;
    for (auto &stat : MSLStimeStatistics)
        stat.average /= numRuns;

    double timeLimitA = MSLStimeStatistics[0].average;
    double timeLimitB = MSLStimeStatistics[1].average;
    println("Time limits: {:.4f}; {:.4f}", timeLimitA, timeLimitB);

    vector<unique_ptr<AdvancedLocalSearch>> advancedLocalSearches;
    advancedLocalSearches.reserve(6);
    advancedLocalSearches.emplace_back(make_unique<ILS>(dataA, randomSolverA, LocalSearchA, 0, -1, timeLimitA, true, true));
    advancedLocalSearches.emplace_back(make_unique<ILS>(dataB, randomSolverB, LocalSearchB, 0, -1, timeLimitB, true, true));
    // TODO: Add more algorithms

    vector<Statistic> scoreStatistics;
    scoreStatistics.reserve(advancedLocalSearches.size());
    vector<Statistic> timeStatistics;
    timeStatistics.reserve(advancedLocalSearches.size());
    for (auto &solver : advancedLocalSearches)
    {
        scoreStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        timeStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
    }
    for (int run = 0; run < numRuns; run++)
    {
        for (size_t i = 0; i < advancedLocalSearches.size(); i++)
        {
            const auto &solver = advancedLocalSearches[i];

            println("{} - {} - {}", run, solver->data->getName(), solver->getAlgorithmName());

            startTime = chrono::steady_clock::now();
            solver->solve();
            endTime = chrono::steady_clock::now();

            if (solver->bestSolutionScore > scoreStatistics[i].max)
            {
                solver->saveToFile(format("{}_{}", solver->data->getName(), solver->getAlgorithmName()));
            }

            scoreStatistics[i].update(solver->bestSolutionScore);
            timeStatistics[i].update(chrono::duration<double, std::milli>(endTime - startTime).count());
        }
    }
    for (auto &stat : scoreStatistics)
        stat.average /= numRuns;
    for (auto &stat : timeStatistics)
        stat.average /= numRuns;

    
    auto allScoreStatistics = {
        MSLSscoreStatistics,
        scoreStatistics,
    };

    println("\nScore statistics:");
    for(const auto &stat : allScoreStatistics | views::join)
        stat.print();

    auto allTimeStatistics = {
        MSLStimeStatistics,
        timeStatistics
    };

    println("\nTime statistics:");
    for(const auto &stat : allTimeStatistics | views::join)
        stat.print();

    return 0;
}
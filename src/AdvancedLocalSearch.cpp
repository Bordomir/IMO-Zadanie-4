#include "../include/AdvancedLocalSearch.hpp"

#include <limits>
#include <chrono>
#include <vector>
#include <print>
#include <filesystem>
#include <string>
#include <format>
#include <fstream>

using namespace std;
using namespace filesystem;

AdvancedLocalSearch::AdvancedLocalSearch(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch, int maxIterations, double timeLimit, bool needsStartingSolution, bool usesLocalSearch) : data(&data), randomSolver(&randomSolver), LocalSearch(&localSearch)
{
    if (maxIterations > 0)
    {
        this->usesTimeLimit = false;
        this->maxIterations = maxIterations;
    }
    else if (timeLimit > 0)
    {
        this->usesTimeLimit = true;
        this->timeLimit = timeLimit;
    }
    else
    {
        this->usesTimeLimit = false;
        this->maxIterations = 200;
    }
    this->needsStartingSolution = needsStartingSolution;
    this->usesLocalSearch = usesLocalSearch;
    setData(data);
}

void AdvancedLocalSearch::setData(DataLoader &data)
{
    this->randomSolver->data = &data;
    this->LocalSearch->data = &data;
}

void AdvancedLocalSearch::solve()
{
    bestSolution = vector<int>();
    bestSolution.clear();
    bestSolutionScore = numeric_limits<int>::min();
    currentIterations = 0;
    startTime = chrono::steady_clock::now();
    if(needsStartingSolution)
    {
        bestSolution = getRandomizedSolution();
        bestSolution = improveSolution(bestSolution);
        bestSolutionScore = LocalSearch->solutionScore;
    }
    while(canContinue())
    {
        vector<int> solution = createNewSolution();
        int solutionScore;
        if(usesLocalSearch)
        {
            solution = improveSolution(solution);
            solutionScore = LocalSearch->solutionScore;
        }
        else
        {
            solutionScore = calculateScore(solution);
        }
        if(solutionScore > bestSolutionScore)
        {
            bestSolution = solution;
            bestSolutionScore = solutionScore;
        }
        currentIterations++;
    }
}

vector<int> AdvancedLocalSearch::getRandomizedSolution() const
{
    randomSolver->solve();
    return randomSolver->solution;
}

vector<int> AdvancedLocalSearch::improveSolution(vector<int> &solution) const
{
    LocalSearch->solution = solution;
    LocalSearch->improve();
    return LocalSearch->solution;
}

bool AdvancedLocalSearch::canContinue() const
{
    if (usesTimeLimit)
    {
        auto currentTime = chrono::steady_clock::now();
        auto elapsedTime = chrono::duration<double, std::milli>(currentTime-startTime).count();
        return elapsedTime < timeLimit;
    }
    else
    {
        return currentIterations < maxIterations;
    }
}

int AdvancedLocalSearch::calculateScore(const vector<int> &solution) const
{
    int score = 0;
    if (solution.size() == 0)
    {
        return score;
    }
    // + zysk z odwiedzenia pierwszego wierzchołka
    score = data->nodeProfits[solution[0]];
    for (size_t currentNode = 1; currentNode < solution.size(); currentNode++)
    {
        // - odległość currentNode - 1 -> currentNode
        score -= data->distanceMatrix[solution[currentNode - 1]][solution[currentNode]];
        // + zysk z odwiedzenia wierzchołka currentNode
        score += data->nodeProfits[solution[currentNode]];
    }
    if (solution.size() > 1)
    {
        // - odległość lastNode -> currentNode
        score -= data->distanceMatrix[solution[solution.size() - 1]][solution[0]];
    }
    return score;
}

void AdvancedLocalSearch::print() const
{
    println("Trasa:\n{}", bestSolution);
    println("Funkcja celu: {}", bestSolutionScore);
}

void AdvancedLocalSearch::saveToFile(const string &filename) const
{
    path dir = "../data/solutions";
    create_directories(dir);
    string fullPath = (dir / format("{}_{}.txt", filename, getAlgorithmName())).string();
    ofstream file(fullPath);
    println(file, "{}", bestSolutionScore);
    for (size_t i = 0; i < bestSolution.size(); i++)
    {
        println(file, "{}", bestSolution[i]);
    }
    file.close();
}
#include "../include/Solver.hpp"

#include <print>
#include <filesystem>
#include <format>
#include <fstream>

using namespace std;
using namespace filesystem;

int Solver::calculateLength()
{
    int score = 0;
    if(solution.size() == 0)
    {
        return score;
    }
    for (size_t currentNode = 1; currentNode < solution.size(); currentNode++)
    {
        // - odległość currentNode - 1 -> currentNode
        score -= data->distanceMatrix[solution[currentNode - 1]][solution[currentNode]];
    }
    if(solution.size() > 1)
    {
        // - odległość lastNode -> currentNode
        score -= data->distanceMatrix[solution[solution.size() - 1]][solution[0]];
    }
    return score;
}

int Solver::calculateScore()
{
    int score = 0;
    if(solution.size() == 0)
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
    if(solution.size() > 1)
    {
        // - odległość lastNode -> currentNode
        score -= data->distanceMatrix[solution[solution.size() - 1]][solution[0]];
    }
    return score;
}
void Solver::print()
{
    println("Trasa:\n{}",solution);
    println("Funkcja celu: {}", solutionScore);
}
void Solver::saveToFile(const string &filename)
{
    path dir = "../data/solutions";
    create_directories(dir); 
    string fullPath = (dir / format("{}_{}.txt", filename, getAlgorithmName())).string();
    ofstream file(fullPath);
    println(file, "{}", solutionScore);
    for (size_t i = 0; i < solution.size(); i++)
    {
        println(file, "{}", solution[i]);
    }
    file.close();
}
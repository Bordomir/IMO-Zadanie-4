#include "../include/LocalSearch.hpp"

#include <optional>
#include <vector>
#include <algorithm>
#include <print>
#include <filesystem>
#include <format>
#include <fstream>

using namespace std;
using namespace filesystem;

void LocalSearch::improve()
{
    solutionScore = calculateScore();
    inSolution.assign(data->numNodes, -1);
    for(size_t i = 0; i < solution.size(); i++)
    {
        inSolution[solution[i]] = i;
    }
    setMoveSet();
    bool hasImproved = true;
    while (hasImproved)
    {
        hasImproved = false;

        optional<Move> bestMove = chooseMove();

        if (bestMove)
        {
            hasImproved = true;
            changeSolution(*bestMove);
            solutionScore += bestMove->deltaScore;
            updateMoveSet(*bestMove);
        }
    }
}

int LocalSearch::calculateDeltaScore(const Move &move)
{
    int deltaScore = 0;
    switch (move.type)
    {
        case MoveType::InsertNode:
        {
            auto [node, prev, next, u1, u2, u3] = move.nodes;

            deltaScore += data->nodeProfits[node];
            if(prev < 0)
                break;
            deltaScore += data->distanceMatrix[prev][next];
            deltaScore -= data->distanceMatrix[prev][node];
            deltaScore -= data->distanceMatrix[node][next];
            break;
        }
        case MoveType::RemoveNode:
        {
            auto [node, prev, next, u1, u2, u3] = move.nodes;
            
            deltaScore -= data->nodeProfits[node];
            if(prev < 0 || node == prev)
                break;
            deltaScore -= data->distanceMatrix[prev][next];
            deltaScore += data->distanceMatrix[prev][node];
            deltaScore += data->distanceMatrix[node][next];
            break;
        }
        case MoveType::SwapNodes:
        {
            auto [p1, c1, n1, p2, c2, n2] = move.nodes;

            if(p1 == n1)
                break;

            // edge cases
            if(c1 == p2)
            {
                //p1 -> c1 -> n1 -> n2
                //p1 -> p2 -> c2 -> n2

                // p1 -> c2 -> c1 -> n2
                deltaScore += data->distanceMatrix[p1][c1];

                deltaScore -= data->distanceMatrix[p1][n1];

                deltaScore += data->distanceMatrix[n1][n2];
                
                deltaScore -= data->distanceMatrix[c1][n2];
                break;
            }
            if(c2 == p1)
            {
                //p2 -> c2 -> n2 -> n1
                //p2 -> p1 -> c1 -> n1

                // p2 -> c1 -> c2 -> n1
                deltaScore += data->distanceMatrix[p2][c2];

                deltaScore -= data->distanceMatrix[p2][n2];

                deltaScore += data->distanceMatrix[n2][n1];
                
                deltaScore -= data->distanceMatrix[c2][n1];
                break;
            }

            deltaScore += data->distanceMatrix[p1][c1];
            deltaScore += data->distanceMatrix[c1][n1];

            deltaScore -= data->distanceMatrix[p1][c2];
            deltaScore -= data->distanceMatrix[c2][n1];

            deltaScore += data->distanceMatrix[p2][c2];
            deltaScore += data->distanceMatrix[c2][n2];

            deltaScore -= data->distanceMatrix[p2][c1];
            deltaScore -= data->distanceMatrix[c1][n2];
            break;
        }
        case MoveType::SwapEdges:
        {
            auto [c1, n1, c2, n2, u1, u2] = move.nodes;

            if(c1 == c2)
                break;

            deltaScore += data->distanceMatrix[c1][n1];

            deltaScore -= data->distanceMatrix[c1][c2];

            deltaScore += data->distanceMatrix[c2][n2];
            
            deltaScore -= data->distanceMatrix[n1][n2];

            break;
        }
    }
    return deltaScore;
}

int LocalSearch::getNodeFromSolution(int solutionIndex)
{
    int n = solution.size();
    while(solutionIndex < 0) solutionIndex += n;
    while(solutionIndex >= n) solutionIndex -= n;
    return solutionIndex;
}

bool LocalSearch::checkMove(MoveType type, int node1, int node2)
{
    switch (type)
    {
        case MoveType::InsertNode:
        {
            if(inSolution[node1] > -1)
                return false;
            break;
        }
        case MoveType::RemoveNode:
        {
            if(inSolution[node1] == -1)
                return false;
            break;
        }
        case MoveType::SwapNodes:
        {
            if(inSolution[node1] == -1 || inSolution[node2] == -1)
                return false;
            break;
        }
        case MoveType::SwapEdges:
        {
            if(inSolution[node1] == -1 || inSolution[node2] == -1)
                return false;
            break;
        }
    }
    return true;
}
bool LocalSearch::checkMove(Move move)
{
    MoveType type = move.type;
    switch (type)
    {
        case MoveType::InsertNode:
        {
            auto [node, prev, next, u1, u2, u3] = move.nodes;
            if(inSolution[node] > -1)
                return false;
            break;
        }
        case MoveType::RemoveNode:
        {
            auto [node, prev, next, u1, u2, u3] = move.nodes;
            if(inSolution[node] == -1)
                return false;
            break;
        }
        case MoveType::SwapNodes:
        {
            auto [p1, c1, n1, p2, c2, n2] = move.nodes;
            if(inSolution[c1] == -1 || inSolution[c2] == -1)
                return false;
            break;
        }
        case MoveType::SwapEdges:
        {
            auto [c1, n1, c2, n2, u1, u2] = move.nodes;
            if(inSolution[c1] == -1 || inSolution[c2] == -1)
                return false;
            break;
        }
    }
    return true;
}

void LocalSearch::changeSolution(const Move &bestMove)
{
    switch (bestMove.type)
    {
        case MoveType::InsertNode:
        {
            auto [node, prev, next, u1, u2, u3] = bestMove.nodes;

            int insertIndex = prev < 0 ? 0 : ((checkEdge(prev, next) > 0 ? inSolution[prev] : inSolution[next]) + 1);
            solution.insert(solution.begin() + insertIndex, node);

            const int n = solution.size();
            inSolution[node] = insertIndex++;
            for (; insertIndex < n; insertIndex++)
                inSolution[solution[insertIndex]]++;
            break;
        }
        case MoveType::RemoveNode:
        {
            auto [node, prev, next, u1, u2, u3] = bestMove.nodes;

            int eraseIndex = inSolution[node];
            solution.erase(solution.begin() + eraseIndex);

            const int n = solution.size();
            inSolution[node] = -1;
            for (; eraseIndex < n; eraseIndex++)
                inSolution[solution[eraseIndex]]--;
            break;
        }
        case MoveType::SwapNodes:
        {
            auto [p1, c1, n1, p2, c2, n2] = bestMove.nodes;

            swap(solution[inSolution[c1]], solution[inSolution[c2]]);
            swap(inSolution[c1], inSolution[c2]);
            break;
        }
        case MoveType::SwapEdges:
        {
            auto [c1, n1, c2, n2, u1, u2] = bestMove.nodes;

            const int firstIndex = checkEdge(c1, n1) > 0 ? inSolution[c1] : inSolution[n1];
            const int secondIndex = checkEdge(c2, n2) > 0 ? inSolution[c2] : inSolution[n2];
            const int reverseStartIndex = min(firstIndex, secondIndex) + 1;
            const int reverseEndIndex = max(firstIndex, secondIndex) + 1;

            reverse(solution.begin() + reverseStartIndex, solution.begin() + reverseEndIndex);
            for (int i = reverseStartIndex; i < reverseEndIndex; i++)
                inSolution[solution[i]] = i;
            break;
        }
    }
}
int LocalSearch::checkEdge(int node1, int node2)
{
    int index1 = inSolution[node1];
    int index2 = inSolution[node2];

    if(index1 == -1 || index2 == -1) return 0;

    int diff = index2 - index1;
    int absDiff = abs(diff);

    int maxDiff = solution.size() - 1;
    if(absDiff == maxDiff) return index2 == 0 ? 1 : -1; // Edge case for circular solution
    if(absDiff == 1) return diff > 0 ? 1 : -1;

    return 0;
}
int LocalSearch::isMoveApplicable(const Move &m)
{
    switch (m.type)
    {
        case MoveType::InsertNode:
        {
            auto [node, prev, next, u1, u2, u3] = m.nodes;
            if(inSolution[node] > -1) return 0;
            if(prev < 0)
            {
                return solution.empty() ? 1 : 0;
            } 
            else
            {
                return abs(checkEdge(prev, next));
            }
            break;
        }
        case MoveType::RemoveNode:
        {
            auto [node, prev, next, u1, u2, u3] = m.nodes;
            int e1 = checkEdge(prev, node);
            int e2 = checkEdge(node, next);
            return e1 * e2;
            break;
        }
        case MoveType::SwapNodes:
        {
            auto [p1, c1, n1, p2, c2, n2] = m.nodes;
            int e1 = checkEdge(p1, c1);
            int e2 = checkEdge(c1, n1);
            int e3 = checkEdge(p2, c2);
            int e4 = checkEdge(c2, n2);
            int s1 = e1 * e2;
            int s2 = e3 * e4;
            return s1 * s2;
            break;
        }
        case MoveType::SwapEdges:
        {
            auto [c1, n1, c2, n2, u1, u2] = m.nodes;
            int e1 = checkEdge(c1, n1);
            int e2 = checkEdge(c2, n2);
            return e1 * e2;
            break;
        }
    }
    return 0;
}
int LocalSearch::calculateLength() 
{
    int score = 0;
    if (solution.size() == 0)
    {
        return score;
    }
    for (size_t currentNode = 1; currentNode < solution.size(); currentNode++)
    {
        // - odległość currentNode - 1 -> currentNode
        score -= data->distanceMatrix[solution[currentNode - 1]][solution[currentNode]];
    }
    if (solution.size() > 1)
    {
        // - odległość lastNode -> currentNode
        score -= data->distanceMatrix[solution[solution.size() - 1]][solution[0]];
    }
    return score;
}

int LocalSearch::calculateScore()
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
void LocalSearch::print()
{
    println("Trasa:\n{}", solution);
    println("Funkcja celu: {}", solutionScore);
}
void LocalSearch::saveToFile(const string &filename)
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
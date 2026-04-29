#pragma once

#include <optional>
#include <print>
#include <vector>
#include <string>
#include <memory>
#include <limits>

#include "DataLoader.hpp"
#include "Solver.hpp"

using namespace std;

enum class MoveType
{
    InsertNode = 0, // (node, prev, next = nodes) wstawienie wierzchołka node pomiędzy wierzchołki prev i next lub na sam początek jeżeli prev < 0 lub node == prev
    RemoveNode = 1, // (node, prev, next = nodes) usunięcie wierzchołka node który był pomiędzy wierzchołkami prev i next lub na samym początku jeżeli prev < 0 lub node == prev
    SwapNodes = 2, // (p1, c1, n1, p2, c2, n2 = nodes) zamiana wierchołków c1 i c2 w otoczeniach p1, n1 i p2, n2
    SwapEdges = 3 // (c1, n1, c2, n2 = nodes) zamiana krawędzi c1 -> n1 z krawędzią c2 -> n2
};

struct Move 
{
    MoveType type;
    array<int, 6> nodes;
    int deltaScore;
    
    Move(MoveType type, initializer_list<int> nodes, int deltaScore = -1) : type(type), nodes(), deltaScore(deltaScore) 
    {
        this->nodes.fill(-1);
        size_t i = 0;
        for(int node : nodes)
        {
            if(i >= 6) break;
            this->nodes[i++] = node;
        }
    }

    bool operator<(const Move &other) const
    {
        return deltaScore < other.deltaScore;
    }

void print() const
{
    // Używamy printf lub cout dla maksymalnej kompatybilności
    println("type: {:<1} | nodes: [{:<3}, {:<3}, {:<3}, {:<3}, {:<3}, {:<3}] | delta: {:<6}",
        static_cast<int>(type),
        nodes[0], 
        nodes[1], 
        nodes[2], 
        nodes[3], 
        nodes[4], 
        nodes[5],
        deltaScore
    );
}
};

class LocalSearch
{
public:
    DataLoader *data;
    vector<int> solution;
    int solutionScore;
    MoveType neighbourhoodUsed;
    vector<int> inSolution;
    vector<Move> moveSet;

    LocalSearch(unique_ptr<Solver> &solver, MoveType neighbourhood) : 
        data(solver->data), \
        solution(solver->solution),
        neighbourhoodUsed(neighbourhood)
    {};
    LocalSearch(DataLoader &data, vector<int> solution, MoveType neighbourhood) : 
        data(&data), 
        solution(solution),
        neighbourhoodUsed(neighbourhood) 
    {};
    virtual string getAlgorithmName() = 0;
    void improve();
    int calculateDeltaScore(const Move &move);
    int getNodeFromSolution(int solutionIndex);
    bool checkMove(MoveType type, int node1, int node2 = -1);
    bool checkMove(Move move);
    template<bool areIndicesGiven = true>
    Move createMove(MoveType type, int index1, int index2 = -1);
    template<bool areIndicesGiven = true>
    void addMove(MoveType type, int index1, int index2 = -1);
    virtual optional<Move> chooseMove() = 0;
    void changeSolution(const Move &move);
    virtual void setMoveSet() = 0;
    virtual void updateMoveSet(const Move &move) = 0;
    int checkEdge(int node1, int node2);
    int isMoveApplicable(const Move &move);

    int calculateLength();
    int calculateScore();
    void print();
    void saveToFile(const string &filename = "");
};

template<bool areIndicesGiven>
void LocalSearch::addMove(MoveType type, int index1, int index2)
{
    moveSet.emplace_back(createMove<areIndicesGiven>(type, index1, index2));
}
template<bool areIndicesGiven>
Move LocalSearch::createMove(MoveType type, int index1, int index2)
{
    switch (type)
    {
        default:
        case MoveType::InsertNode:
        {
            if(solution.size() == 0)
                return Move{MoveType::InsertNode, {index1}};
            else
            {
                const int index = areIndicesGiven ? index2 : inSolution[index2];
                const int prev = solution[getNodeFromSolution(index)];
                const int next = solution[getNodeFromSolution(index + 1)];
                return Move{MoveType::InsertNode, {index1, prev, next}};
            }
            break;
        }
        case MoveType::RemoveNode:
        {
            const int index = areIndicesGiven ? index1 : inSolution[index1];
            const int node = areIndicesGiven ? solution[index1] : index1;
            const int prev = solution[getNodeFromSolution(index - 1)];
            const int next = solution[getNodeFromSolution(index + 1)];
            return Move{MoveType::RemoveNode, {node, prev, next}};
            break;
        }
        case MoveType::SwapNodes:
        {
            const int solutionIndex1 = areIndicesGiven ? index1 : inSolution[index1];
            const int solutionIndex2 = areIndicesGiven ? index2 : inSolution[index2];
            const int c1 = areIndicesGiven ? solution[index1] : index1;
            const int c2 = areIndicesGiven ? solution[index2] : index2;
            const int p1 = solution[getNodeFromSolution(solutionIndex1 - 1)];
            const int n1 = solution[getNodeFromSolution(solutionIndex1 + 1)];
            const int p2 = solution[getNodeFromSolution(solutionIndex2 - 1)];
            const int n2 = solution[getNodeFromSolution(solutionIndex2 + 1)];
            return Move{MoveType::SwapNodes, {p1, c1, n1, p2, c2, n2}};
            break;
        }
        case MoveType::SwapEdges:
        {
            const int solutionIndex1 = areIndicesGiven ? index1 : inSolution[index1];
            const int solutionIndex2 = areIndicesGiven ? index2 : inSolution[index2];
            const int c1 = areIndicesGiven ? solution[index1] : index1;
            const int c2 = areIndicesGiven ? solution[index2] : index2;
            const int n1 = solution[getNodeFromSolution(solutionIndex1 + 1)];
            const int n2 = solution[getNodeFromSolution(solutionIndex2 + 1)];
            return Move{MoveType::SwapEdges, {c1, n1, c2, n2}};
            break;
        }
    }
}
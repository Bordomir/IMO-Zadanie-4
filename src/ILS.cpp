#include "../include/ILS.hpp"

#include <string>
#include <vector>
#include <random>

using namespace std;

string ILS::getAlgorithmName() const
{
    return "ILS";
}

vector<int> ILS::createNewSolution()
{
    vector<int> newSolution(bestSolution);

    vector<int> inSolution(data->numNodes, -1);
    for (size_t i = 0; i < newSolution.size(); i++)
    {
        inSolution[newSolution[i]] = i;
    }

    int inOutMoves = randomInt(2, 4);
    int swapEdgesMoves = randomInt(2, 4);

    if(newSolution.size() == 0 )
    {
        inOutMoves = 0;
        swapEdgesMoves = 0;
    }
    if(newSolution.size() == data->numNodes)
    {
        swapEdgesMoves += inOutMoves;
        inOutMoves = 0;
    }

    for (int i = 0; i < inOutMoves; i++)
    {
        int outNode = randomInt(0, newSolution.size() - 1);
        int inNode;
        do
        {
            inNode = randomInt(0, data->numNodes - 1);
        } while (inSolution[inNode] != -1);

        inSolution[newSolution[outNode]] = -1;
        newSolution[outNode] = inNode;
        inSolution[inNode] = outNode;
    }

    for (int i = 0; i < swapEdgesMoves; i++)
    {
        int node1 = randomInt(0, newSolution.size() - 1);
        int node2;
        do {
            node2 = randomInt(0, newSolution.size() - 1);
        } while(node1 == node2);

        const int reverseStartIndex = min(node1, node2) + 1;
        const int reverseEndIndex = max(node1, node2) + 1;

        reverse(newSolution.begin() + reverseStartIndex, newSolution.begin() + reverseEndIndex);
    }

    return newSolution;
}


int ILS::randomInt(int min, int max)
{
    uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}
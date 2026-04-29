#include "../include/Common.hpp"

namespace common
{

void improveByRemovingNodes(std::vector<int>& solution, const DataLoader* data)
{
    while (solution.size() > 1)
    {
        int bestImprovement = -1;
        int bestI = -1;
        int n = solution.size();
        for (int i = 0; i < n; i++)
        {
            // uzyskanie sąsiadujących elementów z uwzględnieniem cyklu
            int prev = solution[i == 0 ? n - 1 : i - 1];
            int curr = solution[i];
            int next = solution[i == n - 1 ? 0 : i + 1];
            // wyliczenie poprawy po usunęciu wierzchołka
            // + odległość prev -> curr
            int improvement = data->distanceMatrix[prev][curr];
            // - odległość prev -> next
            improvement -= data->distanceMatrix[prev][next];
            // + odległość curr -> next
            improvement += data->distanceMatrix[curr][next];
            // - zysk z odwiedzenia wierzchołka
            improvement -= data->nodeProfits[curr];
            if (improvement > bestImprovement)
            {
                bestImprovement = improvement;
                bestI = i;
            }
        }

        if (bestImprovement <= 0)
        {
            break;
        }
        solution.erase(solution.begin() + bestI);
    }
}

}

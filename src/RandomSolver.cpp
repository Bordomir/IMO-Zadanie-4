#include "../include/RandomSolver.hpp"

#include <vector>
#include <ranges>
#include <algorithm>

using namespace std;

string RandomSolver::getAlgorithmName()
{
    return "Random";
}

int RandomSolver::randomInt(int min, int max)
{
    uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

void RandomSolver::solve()
{
    // losowanie liczby wybranych wierzchołków
    int numChosenNodes = randomInt(1, data->numNodes);

    // tworzenie widoku wszystkich wierzchołków
    auto allNodesView = views::iota(0, data->numNodes);

    // losowanie numChosenNodes wierzchołków
    solution.clear();
    solution.resize(numChosenNodes);
    ranges::sample(allNodesView, solution.begin(), numChosenNodes, rng);

    // losowanie kolejności wierzchołków
    shuffle(solution.begin(), solution.end(), rng);

    // wyliczanie wyniku rozwiązania
    solutionScoreAfterIPhaseI = calculateLength();
    solutionScore = calculateScore();
}

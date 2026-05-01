#include "../include/MSLS.hpp"

#include <string>
#include <vector>

using namespace std;

string MSLS::getAlgorithmName() const
{
    return "MSLS";
}

vector<int> MSLS::createNewSolution()
{
    return getRandomizedSolution();
}

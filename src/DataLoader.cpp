#include "../include/DataLoader.hpp"

#include <filesystem>
#include <string>
#include <fstream>
#include <ranges>
#include <string_view>
#include <charconv>
#include <cmath>

using namespace std;

DataLoader::DataLoader(const string &filename, const optional<string> &name)
    : name(name ? *name : filename)
{
    // ios::binary pozostawia \r\n które są uzwględniane w rozmiarze pliku
    ifstream file(filename, ios::binary);
    if (!file.is_open())
    {
        throw runtime_error("Nie można otworzyć pliku z danymi: " + filename);
    }

    // pobranie rozmiaru pliku z systemu
    auto size = filesystem::file_size(filename); 
    
    // wczytanie całego pliku do zmiennej
    string data(size, '\0');
    file.read(data.data(), size);

    // parsowanie danych z użyciem ranges
    auto parsedData = data 
        | views::split('\n') 
        | views::transform([](auto &&line){ return string_view(line); }) 
        | views::filter([](string_view line){ return !(line.empty() || line == "\r"); }) 
        | views::transform([](string_view line)
        {
            // parsowanie linii na liczby
            auto res = line 
                | views::split(';') 
                | views::transform([](auto &&part){ return string_view(part); }) 
                | views::transform([](string_view part)
                {
                    // parsowanie tekstu na liczbę
                    int val = 0;
                    // from_chars zignoruje nie usunięte wcześniej znaki \r na końcu
                    from_chars(part.data(), part.data() + part.size(), val);
                    return val; 
                }) 
                | ranges::to<vector<int>>();
            return res;
        }) 
        | ranges::to<vector<vector<int>>>();

    numNodes = parsedData.size();

    nodeProfits = parsedData
        | views::transform([](const vector<int> &row){ return row[2]; }) 
        | ranges::to<vector<int>>();

    // obliczanie odległości euklidesowej
    distanceMatrix = vector<vector<int>>(parsedData.size(), vector<int>(parsedData.size()));
    for (size_t i = 0; i < parsedData.size(); i++)
    {
        for (size_t j = i; j < parsedData.size(); j++)
        {
            double dx = parsedData[i][0] - parsedData[j][0];
            double dy = parsedData[i][1] - parsedData[j][1];
            int dist = static_cast<int>(hypot(dx, dy) + 0.5);
            distanceMatrix[i][j] = dist;
            distanceMatrix[j][i] = dist;
        }
    }
}
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

using namespace std;

int main() {
    ifstream in("input.dat");
    ofstream out("output.dat");
    string bwt;
    if (!(in >> bwt)) return 0;
    int n = bwt.length();

    map<char, int> counts, first_occ;
    for (char c : bwt) counts[c]++;
    int sum = 0;
    for (auto& p : counts) {
        first_occ[p.first] = sum;
        sum += p.second;
    }

    vector<int> next(n);
    map<char, int> current_occ;
    int start = 0;
    for (int i = 0; i < n; ++i) {
        char c = bwt[i];
        next[first_occ[c] + current_occ[c]] = i;
        current_occ[c]++;
        if (c == '$') start = i;
    }

    int curr = start;
    string res = "";
    for (int i = 0; i < n; ++i) {
        curr = next[curr];
        res += bwt[curr];
    }
    out << res;
    return 0;
}
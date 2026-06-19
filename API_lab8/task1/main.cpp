#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int trie[10005][128];
int nodes = 1;

int main() {
    ifstream in("input.dat");
    ofstream out("output.dat");
    int n;
    if (!(in >> n)) return 0;
    string s;
    while (in >> s) {
        int u = 0;
        for (char c : s) {
            if (!trie[u][c]) {
                trie[u][c] = nodes++;
                out << u << "->" << trie[u][c] << ":" << c << "\n";
            }
            u = trie[u][c];
        }
    }
    return 0;
}
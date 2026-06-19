#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int trie[500005][128];
bool is_end[500005];
int nodes = 1;

int main() {
    ifstream in("input.dat");
    ofstream out("output.dat");
    string t;
    if (!(in >> t)) return 0;
    int n;
    in >> n;
    string s;
    while (in >> s) {
        int u = 0;
        for (char c : s) {
            if (!trie[u][c]) trie[u][c] = nodes++;
            u = trie[u][c];
        }
        is_end[u] = true;
    }

    for (int i = 0; i < t.length(); i++) {
        int u = 0;
        for (int j = i; j < t.length(); j++) {
            if (!trie[u][t[j]]) break;
            u = trie[u][t[j]];
            if (is_end[u]) {
                out << i << " ";
                break;
            }
        }
    }
    return 0;
}
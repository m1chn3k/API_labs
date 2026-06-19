#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

int main() {
    ifstream in("input.dat");
    ofstream out("output.dat");
    string s;
    if (!(in >> s)) return 0;
    int n = s.length();
    vector<int> p(n);
    for (int i = 0; i < n; ++i) p[i] = i;
    sort(p.begin(), p.end(), [&](int a, int b) {
        for (int i = 0; i < n; ++i) {
            if (s[(a + i) % n] != s[(b + i) % n])
                return s[(a + i) % n] < s[(b + i) % n];
        }
        return false;
        });
    for (int i = 0; i < n; ++i) {
        out << s[(p[i] + n - 1) % n];
    }
    return 0;
}
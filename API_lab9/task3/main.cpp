#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>

using namespace std;

int char_idx(char c) {
    if (c == '$') return 0;
    if (c == 'A') return 1;
    if (c == 'C') return 2;
    if (c == 'G') return 3;
    if (c == 'T') return 4;
    return 5;
}

int main() {
    ifstream in("input.dat");
    ofstream out("output.dat");
    string t, p;
    if (!(in >> t >> p)) return 0;
    t += '$';
    int n = t.length();
    vector<int> sa(n), rank(n), tmp(n);
    iota(sa.begin(), sa.end(), 0);
    for (int i = 0; i < n; i++) rank[i] = t[i];
    for (int k = 1; k < n; k *= 2) {
        auto cmp = [&](int a, int b) {
            if (rank[a] != rank[b]) return rank[a] < rank[b];
            int rx = a + k < n ? rank[a + k] : -1;
            int ry = b + k < n ? rank[b + k] : -1;
            return rx < ry;
            };
        sort(sa.begin(), sa.end(), cmp);
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; i++) tmp[sa[i]] = tmp[sa[i - 1]] + cmp(sa[i - 1], sa[i]);
        rank = tmp;
    }

    string bwt(n, ' ');
    for (int i = 0; i < n; i++) bwt[i] = sa[i] == 0 ? t[n - 1] : t[sa[i] - 1];

    vector<vector<int>> counts(6, vector<int>(n + 1, 0));
    vector<int> total(6, 0);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 6; j++) counts[j][i + 1] = counts[j][i];
        int idx = char_idx(bwt[i]);
        counts[idx][i + 1]++;
        total[idx]++;
    }

    int first_occ[6];
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        first_occ[i] = sum;
        sum += total[i];
    }

    int top = 0, bottom = n - 1;
    for (int i = p.length() - 1; i >= 0; i--) {
        int idx = char_idx(p[i]);
        if (total[idx] > 0) {
            top = first_occ[idx] + counts[idx][top];
            bottom = first_occ[idx] + counts[idx][bottom + 1] - 1;
        }
        else {
            top = 1; bottom = 0; break;
        }
    }

    if (bottom >= top) out << (bottom - top + 1);
    else out << 0;

    return 0;
}
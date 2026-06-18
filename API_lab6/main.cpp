#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <numeric>

using namespace std;


struct BWTResult {
    vector<uint8_t> data;
    int primary_index;
};

BWTResult bwt_encode(const vector<uint8_t>& in) {
    int n = in.size();
    if (n == 0) return { {}, 0 };
    vector<int> indices(n);
    iota(indices.begin(), indices.end(), 0);
    sort(indices.begin(), indices.end(), [&in, n](int a, int b) {
        for (int i = 0; i < n; i++) {
            uint8_t ca = in[(a + i) % n];
            uint8_t cb = in[(b + i) % n];
            if (ca != cb) return ca < cb;
        }
        return false;
        });
    BWTResult res;
    res.data.resize(n);
    for (int i = 0; i < n; i++) {
        if (indices[i] == 0) res.primary_index = i;
        res.data[i] = in[(indices[i] + n - 1) % n];
    }
    return res;
}

vector<uint8_t> bwt_decode(const vector<uint8_t>& in, int primary_index) {
    int n = in.size();
    if (n == 0) return {};
    vector<int> count(256, 0);
    for (int i = 0; i < n; i++) count[in[i]]++;
    vector<int> offset(256, 0);
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        offset[i] = sum;
        sum += count[i];
    }
    vector<int> next(n);
    for (int i = 0; i < n; i++) next[offset[in[i]]++] = i;
    vector<uint8_t> out(n);
    int curr = primary_index;
    for (int i = 0; i < n; i++) {
        curr = next[curr];
        out[i] = in[curr];
    }
    return out;
}

vector<uint8_t> mtf_encode(const vector<uint8_t>& in) {
    vector<uint8_t> out(in.size());
    vector<uint8_t> dict(256);
    iota(dict.begin(), dict.end(), 0);
    for (size_t i = 0; i < in.size(); i++) {
        uint8_t c = in[i];
        auto it = find(dict.begin(), dict.end(), c);
        int pos = distance(dict.begin(), it);
        out[i] = pos;
        dict.erase(it);
        dict.insert(dict.begin(), c);
    }
    return out;
}

vector<uint8_t> mtf_decode(const vector<uint8_t>& in) {
    vector<uint8_t> out(in.size());
    vector<uint8_t> dict(256);
    iota(dict.begin(), dict.end(), 0);
    for (size_t i = 0; i < in.size(); i++) {
        int pos = in[i];
        uint8_t c = dict[pos];
        out[i] = c;
        dict.erase(dict.begin() + pos);
        dict.insert(dict.begin(), c);
    }
    return out;
}


struct BitWriter {
    ofstream& out;
    uint32_t buf = 0;
    int bits = 0;
    BitWriter(ofstream& o) : out(o) {}
    void write(uint32_t val, int len) {
        buf |= (val << bits);
        bits += len;
        while (bits >= 8) {
            out.put(buf & 0xFF);
            buf >>= 8;
            bits -= 8;
        }
    }
    void flush() {
        if (bits > 0) out.put(buf & 0xFF);
    }
};

struct BitReader {
    ifstream& in;
    uint32_t buf = 0;
    int bits = 0;
    BitReader(ifstream& i) : in(i) {}
    int read(int len) {
        while (bits < len) {
            char c;
            if (!in.get(c)) return -1;
            buf |= ((uint8_t)c << bits);
            bits += 8;
        }
        int val = buf & ((1 << len) - 1);
        buf >>= len;
        bits -= len;
        return val;
    }
};


void encode(const string& inFile, const string& outFile, uint8_t max_bits, uint8_t clear_mode, uint8_t use_bwt, uint8_t use_mtf) {
    ifstream in(inFile, ios::binary);
    if (!in) { cerr << "Error: cannot open input file.\n"; return; }

    vector<uint8_t> data((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    in.close();

    uint32_t totalBytes = data.size();
    if (totalBytes == 0) return;

    int bwt_idx = 0;
    if (use_bwt) {
        BWTResult res = bwt_encode(data);
        data = res.data;
        bwt_idx = res.primary_index;
    }

    if (use_mtf) {
        data = mtf_encode(data);
    }

    ofstream out(outFile, ios::binary);
    out.put(max_bits);
    out.put(clear_mode);
    out.put(use_bwt);
    out.put(use_mtf);
    out.write((char*)&bwt_idx, 4);
    out.write((char*)&totalBytes, 4);

    int max_code = (1 << max_bits);
    unordered_map<string, int> dict;
    for (int i = 0; i < 256; i++) dict[string(1, (char)i)] = i;

    int dictSize = 256;
    int bitLen = 9;
    string w = "";
    BitWriter bw(out);

    for (uint8_t byte : data) {
        char c = (char)byte;
        string wc = w + c;
        if (dict.count(wc)) {
            w = wc;
        }
        else {
            bw.write(dict[w], bitLen);
            if (dictSize < max_code) {
                dict[wc] = dictSize++;
                if (dictSize == (1 << bitLen) && bitLen < max_bits) bitLen++;
            }
            else if (clear_mode) {
                dict.clear();
                for (int i = 0; i < 256; i++) dict[string(1, (char)i)] = i;
                dictSize = 256;
                bitLen = 9;
            }
            w = string(1, c);
        }
    }
    if (!w.empty()) bw.write(dict[w], bitLen);
    bw.flush();

    cout << "Successfully compressed: " << inFile << " -> " << outFile << "\n";
}

void decode(const string& inFile, const string& outFile) {
    ifstream in(inFile, ios::binary);
    if (!in) { cerr << "Error: cannot open compressed file.\n"; return; }

    uint8_t max_bits, clear_mode, use_bwt, use_mtf;
    uint32_t bwt_idx, totalBytes;

    if (!in.get((char&)max_bits) || !in.get((char&)clear_mode) ||
        !in.get((char&)use_bwt) || !in.get((char&)use_mtf)) return;
    in.read((char*)&bwt_idx, 4);
    in.read((char*)&totalBytes, 4);

    if (totalBytes == 0) return;

    int max_code = (1 << max_bits);
    vector<string> dict(max_code);
    for (int i = 0; i < 256; i++) dict[i] = string(1, (char)i);

    int dictSize = 256;
    int bitLen = 9;
    BitReader br(in);

    int oldCode = br.read(bitLen);
    if (oldCode == -1) return;

    vector<uint8_t> data;
    for (char c : dict[oldCode]) data.push_back((uint8_t)c);

    uint32_t decodedBytes = dict[oldCode].length();
    string s;
    char c = dict[oldCode][0];

    while (decodedBytes < totalBytes) {
        int newCode = br.read(bitLen);
        if (newCode == -1) break;

        if (newCode >= dictSize) s = dict[oldCode] + c;
        else s = dict[newCode];

        for (char ch : s) data.push_back((uint8_t)ch);
        decodedBytes += s.length();
        c = s[0];

        if (dictSize < max_code) {
            dict[dictSize++] = dict[oldCode] + c;
            if (dictSize == (1 << bitLen) && bitLen < max_bits) bitLen++;
        }
        else if (clear_mode) {
            dictSize = 256;
            bitLen = 9;
        }
        oldCode = newCode;
    }
    in.close();

    if (use_mtf) data = mtf_decode(data);
    if (use_bwt) data = bwt_decode(data, bwt_idx);

    ofstream out(outFile, ios::binary);
    out.write((char*)data.data(), data.size());

    cout << "Successfully decompressed: " << inFile << " -> " << outFile << "\n";
}

int main() {
    cout << "=== LZW Compression Utility (with BWT & MTF) ===\n";

    while (true) {
        cout << "\nMenu:\n";
        cout << "1. Compress a file (Encode)\n";
        cout << "2. Decompress a file (Decode)\n";
        cout << "0. Exit\n";
        cout << "Select an action: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input.\n";
            continue;
        }

        if (choice == 0) break;

        if (choice == 1) {
            string inFile, outFile;
            int max_bits, clear_mode, use_bwt, use_mtf;

            cout << "Enter input file: "; cin >> inFile;
            cout << "Enter output file ('-' for default): "; cin >> outFile;
            if (outFile == "-") outFile = inFile + ".lzw";

            cout << "Max dict size in bits (e.g., 12-16): "; cin >> max_bits;
            cout << "Clear dict on full? (1 = yes, 0 = freeze): "; cin >> clear_mode;
            cout << "Use BWT? (1 = yes, 0 = no): "; cin >> use_bwt;
            cout << "Use MTF? (1 = yes, 0 = no): "; cin >> use_mtf;

            encode(inFile, outFile, max_bits, clear_mode, use_bwt, use_mtf);

        }
        else if (choice == 2) {
            string inFile, outFile;
            cout << "Enter input file: "; cin >> inFile;
            cout << "Enter output file ('-' for default): "; cin >> outFile;
            if (outFile == "-") outFile = inFile + ".out";

            decode(inFile, outFile);
        }
    }
    return 0;
}
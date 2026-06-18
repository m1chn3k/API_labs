#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>

using namespace std;

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

void encode(const string& inFile, const string& outFile, uint8_t max_bits, uint8_t clear_mode) {
    ifstream in(inFile, ios::binary);
    if (!in) { cerr << "Error: cannot open input file.\n"; return; }

    in.seekg(0, ios::end);
    uint32_t totalBytes = in.tellg();
    in.seekg(0, ios::beg);

    ofstream out(outFile, ios::binary);
    out.put(max_bits);
    out.put(clear_mode);
    out.write((char*)&totalBytes, 4);

    if (totalBytes == 0) return;

    int max_code = (1 << max_bits);
    unordered_map<string, int> dict;
    for (int i = 0; i < 256; i++) dict[string(1, (char)i)] = i;

    int dictSize = 256;
    int bitLen = 9;
    string w = "";
    char c;
    BitWriter bw(out);

    while (in.get(c)) {
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

    uint8_t max_bits;
    uint8_t clear_mode;
    uint32_t totalBytes;

    if (!in.get((char&)max_bits) || !in.get((char&)clear_mode)) return;
    in.read((char*)&totalBytes, 4);

    ofstream out(outFile, ios::binary);
    if (totalBytes == 0) return;

    int max_code = (1 << max_bits);
    vector<string> dict(max_code);
    for (int i = 0; i < 256; i++) dict[i] = string(1, (char)i);

    int dictSize = 256;
    int bitLen = 9;
    BitReader br(in);

    int oldCode = br.read(bitLen);
    if (oldCode == -1) return;

    out << dict[oldCode];
    uint32_t decodedBytes = dict[oldCode].length();
    string s;
    char c = dict[oldCode][0];

    while (decodedBytes < totalBytes) {
        int newCode = br.read(bitLen);
        if (newCode == -1) break;

        if (newCode >= dictSize) s = dict[oldCode] + c;
        else s = dict[newCode];

        out << s;
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

    cout << "Successfully decompressed: " << inFile << " -> " << outFile << "\n";
}

int main() {
    cout << "=== LZW Compression Utility ===\n";

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
            int max_bits, clear_mode;

            cout << "Enter input file: "; cin >> inFile;
            cout << "Enter output file ('-' for default): "; cin >> outFile;
            if (outFile == "-") outFile = inFile + ".lzw";

            cout << "Max dict size in bits (e.g., 12-16): "; cin >> max_bits;
            cout << "Clear dict on full? (1 = yes, 0 = freeze): "; cin >> clear_mode;

            encode(inFile, outFile, max_bits, clear_mode);

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
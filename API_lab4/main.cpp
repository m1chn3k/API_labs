#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>

using namespace std;

struct Node {
    uint8_t ch;
    uint32_t freq;
    Node* left, * right;
    Node(uint8_t c, uint32_t f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
    Node(Node* l, Node* r) : ch(0), freq(l->freq + r->freq), left(l), right(r) {}
    ~Node() { delete left; delete right; }
};

struct Compare {
    bool operator()(Node* l, Node* r) { return l->freq > r->freq; }
};

struct BitWriter {
    ofstream& out;
    uint8_t buf = 0;
    int bits = 0;
    BitWriter(ofstream& o) : out(o) {}
    void writeBit(int b) {
        if (b) buf |= (1 << bits);
        bits++;
        if (bits == 8) { out.put(buf); buf = 0; bits = 0; }
    }
    void flush() { if (bits > 0) out.put(buf); }
};

struct BitReader {
    ifstream& in;
    uint8_t buf = 0;
    int bits = 0;
    BitReader(ifstream& i) : in(i) {}
    int readBit() {
        if (bits == 0) {
            if (!in.get((char&)buf)) return -1;
            bits = 8;
        }
        int b = (buf & 1);
        buf >>= 1;
        bits--;
        return b;
    }
};

vector<int> codes[256];

void buildCodes(Node* root, vector<int>& code) {
    if (!root) return;
    if (!root->left && !root->right) {
        if (code.empty()) code.push_back(0);
        codes[root->ch] = code;
        return;
    }
    code.push_back(0); buildCodes(root->left, code); code.pop_back();
    code.push_back(1); buildCodes(root->right, code); code.pop_back();
}

Node* buildTree(const uint32_t freqs[256]) {
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (int i = 0; i < 256; i++) {
        if (freqs[i] > 0) pq.push(new Node(i, freqs[i]));
    }
    if (pq.empty()) return nullptr;
    while (pq.size() > 1) {
        Node* l = pq.top(); pq.pop();
        Node* r = pq.top(); pq.pop();
        pq.push(new Node(l, r));
    }
    return pq.top();
}

void encode(const string& inFile, const string& outFile) {
    ifstream in(inFile, ios::binary);
    if (!in) { cerr << "Error: cannot open input file.\n"; return; }

    uint32_t freqs[256] = { 0 };
    uint32_t totalBytes = 0;
    uint8_t ch;

    while (in.read((char*)&ch, 1)) {
        freqs[ch]++;
        totalBytes++;
    }

    Node* root = buildTree(freqs);
    vector<int> code;
    for (int i = 0; i < 256; i++) codes[i].clear();
    buildCodes(root, code);

    ofstream out(outFile, ios::binary);

    out.write((char*)freqs, 1024);
    out.write((char*)&totalBytes, 4);

    in.clear();
    in.seekg(0);
    BitWriter bw(out);
    while (in.read((char*)&ch, 1)) {
        for (int bit : codes[ch]) bw.writeBit(bit);
    }
    bw.flush();

    delete root;
    cout << "Successfully compressed: " << inFile << " -> " << outFile << "\n";
}

void decode(const string& inFile, const string& outFile) {
    ifstream in(inFile, ios::binary);
    if (!in) { cerr << "Error: cannot open compressed file.\n"; return; }

    uint32_t freqs[256];
    uint32_t totalBytes = 0;

    in.read((char*)freqs, 1024);
    in.read((char*)&totalBytes, 4);

    Node* root = buildTree(freqs);
    if (!root) return;

    ofstream out(outFile, ios::binary);
    BitReader br(in);

    uint32_t decodedBytes = 0;

    while (decodedBytes < totalBytes) {
        Node* curr = root;
        while (curr->left && curr->right) {
            int bit = br.readBit();
            if (bit == -1) break;
            curr = (bit == 0) ? curr->left : curr->right;
        }
        out.put(curr->ch);
        decodedBytes++;
    }

    delete root;
    cout << "Successfully decompressed: " << inFile << " -> " << outFile << "\n";
}

int main() {
    cout << "=== Huffman Compression Utility ===\n";

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
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        if (choice == 0) {
            cout << "Exiting...\n";
            break;
        }

        if (choice == 1 || choice == 2) {
            string inFile, outFile;
            cout << "Enter input file name: ";
            cin >> inFile;
            cout << "Enter output file name (or type '-' for default): ";
            cin >> outFile;

            if (outFile == "-") {
                outFile = (choice == 1) ? inFile + ".huff" : inFile + ".out";
            }

            if (choice == 1) {
                encode(inFile, outFile);
            }
            else {
                decode(inFile, outFile);
            }
        }
        else {
            cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}
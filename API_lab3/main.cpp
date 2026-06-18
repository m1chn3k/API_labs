#include <iostream>
#include <vector>

using namespace std;

struct BitStream {
    vector<uint8_t> buf;
    int wp = 0;
    int rp = 0;

    void write(const vector<uint8_t>& data, int n) {
        for (int i = 0; i < n; ++i) {
            int bit = (data[i / 8] >> (i % 8)) & 1;
            if (wp / 8 >= buf.size()) buf.push_back(0);
            if (bit) buf[wp / 8] |= (1 << (wp % 8));
            else buf[wp / 8] &= ~(1 << (wp % 8));
            wp++;
        }
    }

    void read(vector<uint8_t>& data, int n) {
        data.assign((n + 7) / 8, 0);
        for (int i = 0; i < n; ++i) {
            int bit = 0;
            if (rp / 8 < buf.size()) {
                bit = (buf[rp / 8] >> (rp % 8)) & 1;
            }
            if (bit) data[i / 8] |= (1 << (i % 8));
            rp++;
        }
    }
};

int main() {
    BitStream bs;
    vector<uint8_t> a1 = { 0xE1, 0x01 };
    vector<uint8_t> a2 = { 0xEE, 0x00 };

    bs.write(a1, 9);
    bs.write(a2, 9);

    for (auto x : bs.buf) printf("%02X ", x);
    cout << "\n";

    vector<uint8_t> b1, b2;
    bs.read(b1, 11);
    bs.read(b2, 7);

    for (auto x : b1) printf("%02X ", x);
    cout << "\n";

    for (auto x : b2) printf("%02X ", x);
    cout << "\n";

    return 0;
}
import random
import math

N = 2**16

def h_poly(k, s_bytes):
    if len(s_bytes) % 2 != 0:
        s_bytes += b'\x00'
    x = []
    for i in range(0, len(s_bytes), 2):
        x.append(s_bytes[i] + (s_bytes[i+1] << 8))
    y = 1
    for x_i in x:
        y = (y * k + x_i) % N
    return y

class BloomFilter:
    def __init__(self, s):
        self.s = s
        self.bit_array = [0] * (N // 64)
        self.keys = []
        while len(self.keys) < s:
            k = random.randint(1, N - 1)
            if k not in self.keys:
                self.keys.append(k)

    def add(self, string_data):
        s_bytes = string_data.encode('utf-8') if isinstance(string_data, str) else string_data
        for k in self.keys:
            idx = h_poly(k, s_bytes)
            self.bit_array[idx // 64] |= (1 << (idx % 64))

    def contains(self, string_data):
        s_bytes = string_data.encode('utf-8') if isinstance(string_data, str) else string_data
        for k in self.keys:
            idx = h_poly(k, s_bytes)
            if not (self.bit_array[idx // 64] & (1 << (idx % 64))):
                return False
        return True

def gen_random_str():
    return "".join(chr(random.randint(97, 122)) for _ in range(random.randint(5, 30)))

print(f"{'alpha':<10}{'s':<10}{'Theoretical P':<15}{'Experimental P':<15}")
print("-" * 50)

alphas = [round(0.05 * i, 2) for i in range(1, 11)]

for alpha in alphas:
    n = int(alpha * N)
    s0 = max(1, int((1 / alpha) * math.log(2)))
    
    for s in sorted(list(set([max(1, s0 - 1), s0, s0 + 1]))):
        bf = BloomFilter(s)
        added_set = set()
        
        while len(added_set) < n:
            r_str = gen_random_str()
            if r_str not in added_set:
                bf.add(r_str)
                added_set.add(r_str)
        
        M = 0
        while True:
            test_str = gen_random_str()
            if test_str in added_set:
                continue
            M += 1
            if bf.contains(test_str):
                break
        
        avg_p_err = 1.0 / M
        p_theory = (1 - math.exp(-alpha * s)) ** s
        print(f"{alpha:<10.2f}{s:<10}{p_theory:<15.4f}{avg_p_err:<15.4f}")

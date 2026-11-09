import sys
import time


def smallest_prime_factors(limit):
    spf = list(range(limit + 1))
    i = 2
    while i * i <= limit:
        if spf[i] == i:
            for j in range(i * i, limit + 1, i):
                if spf[j] == j:
                    spf[j] = i
        i += 1
    return spf


def scan(n):
    spf = smallest_prime_factors(max(n, 2))
    exponent = {}
    divisions = bytearray(n + 1)
    for m in range(1, n):
        x = m
        factors = []
        while x > 1:
            p = spf[x]
            c = 0
            while x % p == 0:
                x //= p
                c += 1
            factors.append((p, c))
        divide = all(exponent.get(p, 0) >= c for p, c in factors)
        sign = -1 if divide else 1
        if divide:
            divisions[m] = 1
        for p, c in factors:
            exponent[p] = exponent.get(p, 0) + sign * c
    return exponent, divisions


def factorization(n):
    exponent, _ = scan(n)
    return {p: c for p, c in exponent.items() if c}


def value(n):
    result = 1
    for p, c in factorization(n).items():
        result *= p ** c
    return result


def division_set(n):
    _, divisions = scan(n)
    return [m for m in range(1, n) if divisions[m]]


def direct(n):
    a = 1
    for m in range(1, n):
        a = a // m if a % m == 0 else a * m
    return a


def check(limit):
    for n in range(1, limit + 1):
        assert value(n) == direct(n), n


if __name__ == "__main__":
    check(300)
    print("verified n<=300")
    for n in (10**4, 4 * 10**4, 10**5):
        t = time.perf_counter()
        direct(n)
        t1 = time.perf_counter() - t
        t = time.perf_counter()
        value(n)
        t2 = time.perf_counter() - t
        print(n, round(t1, 3), round(t2, 3))

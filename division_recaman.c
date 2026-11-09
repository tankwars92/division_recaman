#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    uint32_t n = (uint32_t)strtoul(argv[1], 0, 10);
    const char *out = argc > 2 ? argv[2] : 0;
    uint32_t half = n / 2;
    uint32_t *spf = calloc((size_t)n / 2 + 2, sizeof(uint32_t));
    int32_t *e = calloc((size_t)half + 2, sizeof(int32_t));
    if (!spf || !e)
        return 2;
    struct timespec t0, t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (uint32_t i = 3; i <= n; i += 2) {
        if (spf[i >> 1])
            continue;
        spf[i >> 1] = i;
        if ((uint64_t)i * i <= n)
            for (uint32_t j = i * i; j <= n; j += 2 * i)
                if (!spf[j >> 1])
                    spf[j >> 1] = i;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    uint32_t pr[16];
    int32_t ex[16];
    uint64_t divisions = 0;
    for (uint32_t m = 1; m < n; m++) {
        int a = __builtin_ctz(m);
        uint32_t x = m >> a;
        if (a == 0 && x > 1 && spf[x >> 1] == x) {
            if (m <= half)
                e[m] = 1;
            continue;
        }
        int k = 0;
        int ok = 1;
        if (a) {
            pr[0] = 2;
            ex[0] = a;
            k = 1;
            if (e[2] < a)
                ok = 0;
        }
        while (x > 1) {
            uint32_t p = spf[x >> 1];
            int32_t c = 0;
            do {
                x /= p;
                c++;
            } while (x % p == 0);
            pr[k] = p;
            ex[k] = c;
            if (e[p] < c)
                ok = 0;
            k++;
        }
        if (ok) {
            divisions++;
            for (int i = 0; i < k; i++)
                e[pr[i]] -= ex[i];
        } else {
            for (int i = 0; i < k; i++)
                e[pr[i]] += ex[i];
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
    double ts = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double tl = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) * 1e-9;
    printf("n=%u divisions=%llu sieve=%.3fs scan=%.3fs total=%.3fs\n", n, (unsigned long long)divisions, ts, tl, ts + tl);
    if (out) {
        FILE *f = fopen(out, "w");
        for (uint32_t p = 2; p < n; p++) {
            int prime = p == 2 || (p & 1 && spf[p >> 1] == p);
            if (!prime)
                continue;
            int32_t c = p <= half ? e[p] : 1;
            if (c)
                fprintf(f, "%u %d\n", p, c);
        }
        fclose(f);
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

static inline double now(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    uint64_t n = strtoull(argv[1], 0, 10);
    const char *out = argc > 2 ? argv[2] : 0;
    uint64_t half = n / 2;
    uint64_t root = 1;
    while (root * root <= n)
        root++;
    root--;
    uint32_t *pr = malloc((root + 2) * sizeof(uint32_t));
    size_t pc = 0;
    {
        char *s = calloc(root + 2, 1);
        for (uint64_t i = 3; i <= root; i += 2) {
            if (s[i])
                continue;
            pr[pc++] = (uint32_t)i;
            for (uint64_t j = i * i; j <= root; j += 2 * i)
                s[j] = 1;
        }
        free(s);
    }
    if (pc >= 65535)
        return 3;
    uint64_t size = n / 2 + 2;
    uint16_t *lp = calloc(size, sizeof(uint16_t));
    uint64_t thr = n / 65534 + 3;
    int32_t *es = calloc(thr + 1, sizeof(int32_t));
    uint16_t *el = calloc(size, sizeof(uint16_t));
    if (!lp || !es || !el)
        return 2;
    double t0 = now();
    for (size_t k = 0; k < pc; k++) {
        uint64_t p = pr[k];
        for (uint64_t j = p * p; j <= n; j += 2 * p)
            if (!lp[j >> 1])
                lp[j >> 1] = (uint16_t)(k + 1);
    }
    double t1 = now();
    uint32_t fp[24];
    int32_t fe[24];
    uint64_t divisions = 0;
#define GET(p) ((p) < thr ? es[p] : (int32_t)el[(p) >> 1])
#define ADD(p, d) do { if ((p) < thr) es[p] += (d); else el[(p) >> 1] = (uint16_t)(el[(p) >> 1] + (d)); } while (0)
    for (uint64_t m = 1; m < n; m++) {
        int a = __builtin_ctzll(m);
        uint64_t x = m >> a;
        if (a == 0 && x > 1 && lp[x >> 1] == 0) {
            if (m <= half) {
                if (m < thr)
                    es[m] = 1;
                else
                    el[m >> 1] = 1;
            }
            continue;
        }
        int k = 0;
        int ok = 1;
        if (a) {
            fp[0] = 2;
            fe[0] = a;
            k = 1;
            if (es[2] < a)
                ok = 0;
        }
        while (x > 1) {
            uint32_t idx = lp[x >> 1];
            uint64_t p;
            int32_t c = 0;
            if (!idx) {
                p = x;
                c = 1;
                x = 1;
            } else {
                p = pr[idx - 1];
                do {
                    x /= p;
                    c++;
                } while (x % p == 0);
            }
            fp[k] = (uint32_t)p;
            fe[k] = c;
            if (GET(p) < c)
                ok = 0;
            k++;
        }
        if (ok) {
            divisions++;
            for (int i = 0; i < k; i++)
                ADD(fp[i], -fe[i]);
        } else {
            for (int i = 0; i < k; i++)
                ADD(fp[i], fe[i]);
        }
    }
    double t2 = now();
    printf("n=%llu divisions=%llu sieve=%.3fs scan=%.3fs total=%.3fs\n", (unsigned long long)n, (unsigned long long)divisions, t1 - t0, t2 - t1, t2 - t0);
    if (out) {
        FILE *f = fopen(out, "w");
        for (uint64_t p = 2; p < n; p++) {
            int prime = p == 2 || ((p & 1) && lp[p >> 1] == 0);
            if (!prime)
                continue;
            int32_t c = p <= half ? GET(p) : 1;
            if (c)
                fprintf(f, "%llu %d\n", (unsigned long long)p, c);
        }
        fclose(f);
    }
    return 0;
}

// program to explore the Gemenid Uniquely Contravariant Conjecture (GUCC)

#include <stdio.h>

#define MAX 10000000
#define MAX_PFACTORS 100

// least_factor[i] is the smallest factor of i in [2,i-1]
// and 0 if none exists (i.e. i is prime)
//
int least_factor[MAX];

inline int power(int base, int exp) {
    int res = 1;
    while (exp > 0) {
        if (exp % 2) {
            res *= base;
        }
        base *= base;
        exp /= 2;
    }
    return res;
}

// a prime factorization
struct FACTORIZATION {
    int p[MAX_PFACTORS];    // list of primes
    int a[MAX_PFACTORS];    // list of multiplicities
    int nfact;
    int cur_fac;
    inline void add_fact(int fac) {
        if (fac == cur_fac) {
            a[nfact]++;
        } else {
            nfact++;
            p[nfact] = fac;
            a[nfact] = 1;
            cur_fac = fac;
        }
    }
    inline void get_factors(int n) {
        nfact = -1;
        cur_fac = 0;
        while (true) {
            if (least_factor[n]) {
                add_fact(least_factor[n]);
                n /= least_factor[n];
            } else {
                add_fact(n);
                break;
            }
        }
    }
    inline void show_factorization() {
        for (int i=0; i<=nfact; i++) {
            printf(" %d^%d", p[i], a[i]);
        }
    }
    inline int phi() {
        int res = 1;
        for (int i=0; i<=nfact; i++) {
            res *= (p[i] - 1)*(power(p[i], a[i]-1));
        }
        return res;
    }
};

// print prime factors in increasing order (without multiplicities)
//
void print_factors(int n) {
    while (true) {
        if (least_factor[n]) {
            int fac = least_factor[n];
            printf(" %d", fac);
            n /= least_factor[n];
        } else {
            printf(" %d", n);
            break;
        }
    }
}

void compute_least_factors() {
    for (int i=2; i<MAX/2; i++) {
        if (least_factor[i]) continue;
        for (int j=i+i; j<MAX; j+=i) {
            if (least_factor[j]) continue;
            least_factor[j] = i;
        }
    }
}

void test_output() {
    FACTORIZATION f;
    for (int i=2; i<100; i++) {
        printf("%d", i);
        f.get_factors(i);
        f.show_factorization();
        printf(" phi: %d", f.phi());
        printf("\n");
    }
}

void gucc() {
    FACTORIZATION f;
    for (int i=2; i<MAX-2; i++) {
        if (least_factor[i]) continue;
        int j = i+2;
        if (least_factor[j]) continue;
        f.get_factors(i-1);
        int phi1 = f.phi();
        f.get_factors(j-1);
        int phi2 = f.phi();
        if (phi1 < phi2) {
            printf("%d %d %d %d\n", i, j, phi1, phi2);
        }
    }
}

int main() {
    printf("Testing up to %d\n", MAX);
    compute_least_factors();
    //test_output();
    gucc();
}

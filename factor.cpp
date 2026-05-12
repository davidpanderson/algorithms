// deprecated; see gucc.cpp

#include <stdio.h>

#define MAX 1000000

int x[MAX];

void print_factors(int n) {
    while (true) {
        if (x[n]) {
            int fac = n/x[n];
            printf(" %d", fac);
            n = x[n];
        } else {
            printf(" %d", n);
            break;
        }
    }
}

int main() {
    int i, j, k;
    for (i=2; i<MAX; i++) {
        for (j=2; j<=i; j++) {
            if (x[j]) continue;
            k = i*j;
            if (k >= MAX) {
                break;
            }
            x[k] = i;
        }
    }
    return 0;
    for (i=2; i<MAX; i++) {
        printf("%d %d", i, x[i]);
        print_factors(i);
        printf("\n");
    }
}

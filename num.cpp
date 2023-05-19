// performance with -O4:
// n=10 (231051): time 7.76s, RSS 21MB
// N=11 (1765186): time 108 sec, RSS 162 MB
// N=12 (10539427) 1550 sec, 1.6 GB
// N=13 ?

// see https://oeis.org/A060315

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <vector>
#include <unordered_map>
using std::vector;
using std::unordered_map;

//#define DEBUG

// run-length encoded set of numbers
// encoding: -n m means n..m
struct RLES {
    vector<long> nums;
    long last_ind;
    long last_num;
    RLES() {
        last_ind = -1;
        last_num = 99999;
    }
    inline void add(long n) {
        nums.push_back(n);
        last_ind++;
        last_num = n;
    }
    inline void negate_last() {
        nums[last_ind] = -nums[last_ind];
        last_num = nums[last_ind];
    }
    long first_missing() {
        if (nums[0] > 0) return nums[0]+1;
        return nums[1]+1;
    }
    void print(const char* indent, const char* title) {
        printf("%sRLES %s:\n", indent, title);
        for (long n: nums) {
            printf("%s%ld\n", indent, n);
        }
    }

    // enumeration stuff

    bool in_run;
    ulong ind;
    long run_val, run_end;

    // set up for enumeration
    //
    inline void enum_init() {
        ind = 0;
        in_run = false;
    }
    // get next time; 0 means end of enumeration
    inline long enum_next() {
        if (in_run) {
            run_val++;
            if (run_val == run_end) {
                in_run = false;
            }
            return run_val;
        }
        if (ind == nums.size()) return 0;
        if (nums[ind] < 0) {
            in_run = true;
            run_val = -nums[ind];
            run_end = nums[++ind];
            ind++;
            return run_val;
        }
        return nums[ind++];
    }
};

// bitmap unit; choose one.
// This doesn't seem to make a big difference.
// char is slightly faster than long (18.4 vs 18.6)

//#define WORD        long
//#define WORD_LEN    64

//#define WORD        int
//#define WORD_LEN    32

//#define WORD        short
//#define WORD_LEN    16

#define WORD        char
#define WORD_LEN    8

WORD wbits[WORD_LEN];
long lbits[64];

// dynamically-sized bitmap.
// we don't know in advance how big products can be
//
struct BITMAP {
    vector<WORD> words;

    inline void set(long n) {
        long iw = n/WORD_LEN;
        if (iw+1 > words.size()) {
            words.resize(iw+1);
        }
        words[iw] |= wbits[n%WORD_LEN];
    }
    inline bool is_set(long n) {
        return (words[n/WORD_LEN] & wbits[n%WORD_LEN]) != 0;
    }
    void print(const char *indent, const char* title) {
        printf("%sBITMAP %s:\n", indent, title);
        for (long i=0; i<words.size(); i++) {
            for (int j=0; j<WORD_LEN; j++) {
                if (is_set(i*WORD_LEN+j)) {
                    printf("%s%ld\n", indent, i*WORD_LEN+j);
                }
            }
        }
    }
};

// convert a bitmap to a RLES
// Identify runs in the bitmap
//
void bitmap_to_rles(BITMAP &bm, RLES &r) {
    long bm_pos = 0;
    for (int i=0; i<bm.words.size(); i++) {
        WORD w = bm.words[i];
        if (w == -1) {
            // bitmap word is all ones.  Start or extend a run
            //
            if (r.last_num<0)  {
                // do nothing - stay in run
            } else {
                if (r.last_num == bm_pos-1) {
                    r.negate_last();
                } else {
                    r.add(-bm_pos);
                }
            }
        } else {
            for (int j=0; j<WORD_LEN; j++) {
                if (w & wbits[j]) {
                    if (r.last_num < 0) {
                        // keep run going
                    } else {
                        if (r.last_num == bm_pos+j-1) {
                            // start a new run
                            r.negate_last();
                        } else {
                            r.add(bm_pos+j);
                        }
                    }
                } else {
                    if (r.last_num < 0) {
                        // end the run
                        r.add(bm_pos+j-1);
                    }
                }
            }
        }
        bm_pos += WORD_LEN;
    }
    // handle case where bitmap ends with a run
    if (r.last_num<0) {
        r.add(bm_pos-1);
    }
}

// In the following, "small number" means 1..63

// convert a set of small numbers to a bitmask
// Use as key for memoization map
//
inline long mask(vector<int> &nums) {
    long m=0;
    for (int i: nums) {
        m |= lbits[i];
    }
    return m;
}

// given a vector X of numbers and a bitmask,
// return the subset of X selected by the bitmask
//
inline void bits_to_vec(vector<int> &nums, long bits, vector<int> &out) {
    for (unsigned int i=0; i<nums.size(); i++) {
        if (bits&lbits[i]) out.push_back(nums[i]);
    }
}

// return the set of products of the given small numbers as a RLES
//
RLES *products(vector<int> &nums) {
    static unordered_map<int, RLES*> memo;
#ifdef DEBUG
    static int level=0;
    char indent[256];
    strcpy(indent, "");
    for (int i=0; i<level; i++) {
        strcat(indent, "    ");
    }
    printf("%sentering products(", indent);
    for (int i: nums) {
        printf("%d ", i);
    }
    printf(")\n");
    strcat(indent, "    ");
    level++;
#endif

    // see if the result is memoized
    //
    long nums_mask = mask(nums);
    if (memo.count(nums_mask) == 1) {
#ifdef DEBUG
        printf("%sreturning memoized result\n", indent);
        level--;
#endif
        return memo[nums_mask];
    }

    // handle the singleton case
    //
    int n = nums.size();
    if (n == 1) {
        RLES *r = new RLES;
        r->add(nums[0]);
        memo[nums_mask] = r;
#ifdef DEBUG
        printf("%sreturning singleton result\n", indent);
        level--;
#endif
        return r;
    }

    BITMAP bm;
    long m = 1<<n;
    for (int x=1; x<m/2; x++) {
        int y = m-x-1;  // complement of y
        vector<int> xvec, yvec;
        bits_to_vec(nums, x, xvec);
        bits_to_vec(nums, y, yvec);
        RLES *xres = products(xvec);
        RLES *yres = products(yvec);
#ifdef DEBUG
        xres->print(indent, "xres");
        yres->print(indent, "yres");
#endif
        xres->enum_init();
        while (long a = xres->enum_next()) {
            yres->enum_init();
            while (long b = yres->enum_next()) {
#ifdef DEBUG
                printf("%sa %ld b %ld\n", indent, a, b);
#endif
                bm.set(a+b);
                bm.set(a*b);
                if (a>b) bm.set(a-b);
                if (b>a) bm.set(b-a);
                if (a%b==0) bm.set(a/b);
                if (b%a==0) bm.set(b/a);
            }
        }
    }
    RLES *r = new RLES;
    bitmap_to_rles(bm, *r);
#ifdef DEBUG
    bm.print(indent, "products");
    r->print(indent, "products");
    printf("%sleaving product()\n", indent);
    level--;
#endif
    memo[nums_mask] = r;
    return r;
}

void test1() {
    BITMAP bm;
    bm.set(1);
    bm.set(3);
    for (int i=5; i<30; i++) {
        bm.set(i);
    }
    bm.set(44);
    for (int i=80; i<100; i++) {
        bm.set(i);
    }
    bm.print("", "");

    RLES r;
    bitmap_to_rles(bm, r);
    r.print("", "");
}

void test2() {
    RLES r;
    r.add(1);
    r.negate_last();
    r.add(3);
    r.enum_init();
    while(long a = r.enum_next()) {
        printf("%ld\n", a);
    }
}

double cpu_time() {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    double cpu_t = (double)ru.ru_utime.tv_sec + ((double)ru.ru_utime.tv_usec) / 1e6;
    cpu_t += (double)ru.ru_stime.tv_sec + ((double)ru.ru_stime.tv_usec) / 1e6;
    return cpu_t;
}

int main(int, char**) {
    for (int i=0; i<WORD_LEN; i++) {
        wbits[i] = (long)1<<i;
    }
    for (int i=0; i<64; i++) {
        lbits[i] = (long)1<<i;
    }
    //test1(); return 0;
    vector<int> nums;
    double last_cpu_time = 0;
    for (int i=1; i<14; i++) {
        nums.push_back(i);
        RLES *r = products(nums);
        printf("first missing for %d: %ld\n", i, r->first_missing());
        double t = cpu_time();
        printf("CPU time: %f\n", t-last_cpu_time);
        last_cpu_time = t;
        printf("---------------------\n");
    }
}

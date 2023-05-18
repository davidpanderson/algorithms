# this version did n=10 in 153 CPU sec
# max mem usage was 451000

from itertools import count, filterfalse
import time

bits = [1,2,4,8,16,32,64,128,256,512,1024,2048,4096]

# return sublist of list_in for which corresponding bitmask bit is set
def elements(list_in, bm, n):
    out = []
    for i in range(n):
        if bits[i]&bm:
            out.append(list_in[i])
    return out
  
# return bitmask of list of numbers
def mask(nums):
    x = 0
    for i in nums:
        x |= bits[i]
    return x

# Return the set of values of arithmetic expressions involving the
# given numbers.  Memoize for efficiency.

memo = {}
def products(nums):
    global memo
    n = len(nums)
    if n == 1: return {nums[0]}

    bm = mask(nums)
    if bm in memo: return memo[bm]

    # Loop over partitions of nums into nonempty X and Y,
    # represented as bitmasks.
    # Compute products(X) and products(Y)
    # Then add results of a+b, a-b, b-a, a*b, a/b, b/a to output

    result = set()
    m = 2**n
    for x in range(1, int(m/2)):
        y = m-x-1   # y is the complement of x
        xlist = products(elements(nums, x, n))
        ylist = products(elements(nums, y, n))
        for a in xlist:
            for b in ylist:
                result.add(a+b)
                result.add(a*b)
                if a>b: result.add(a-b)
                if b>a: result.add(b-a)
                if a%b == 0: result.add(a//b)
                if b%a == 0: result.add(b//a)
    memo[bm] = result
    return result

# compute the least number not in products(1..n)
#
for i in range(1, 11):
    x = 0
    p = products(list(range(1,i+1)))
    print('answer', i, next(filterfalse(p.__contains__, count(1))))
    y = time.process_time()
    print('CPU time:', y-x)
    x = y

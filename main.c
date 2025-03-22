#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>

int main() {
    printf("Hello World\n");

    const char *str = "67";
    char *endptr;
    uint64_t num = strtoull(str, &endptr, 10);
    printf("Num: %"PRIu64"\n", num);

    long long num2 = strtoll(str, &endptr, 10);
    printf("Num2: %lld\n", num2);
    return 0;
}

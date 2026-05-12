#include <stdio.h>

int is_case_bad[15];

int func4(int a, int b, int c) {
    // looks like the distance to the average value between c and b (?)
    // nevermind. dont care what that means.
    int avg_distance;
    if (c > b) {
        avg_distance = c - b;
    } else {
        avg_distance = c - b + 1;
    }
    avg_distance /= 2;

    int d = avg_distance + b;
    if (d > a) {
        return func4(a, b, d - 1) * 2;
    }

    int result = 0;
    if (d < a) {
        is_case_bad[a] = 1;
        return 2 * func4(a, d + 1, c) + 1;
    }

    return 0;
}

int
main(
        void
)
{
        for (int i = 0; i < 15; i++) {
                is_case_bad[i] = 0;
        }
        for (int a = 0; a <= 14; a++) {
                func4(a, 0, 14);
        }
        for (int i = 0; i <= 14; i++) {
                if (is_case_bad[i]) {
                        printf("a=%d is a bad case.\n", i);
                }
        }
        return 0;
}
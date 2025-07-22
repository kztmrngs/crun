#include <stdio.h>
#include <immintrin.h> // For AVX

int main() {
    __m256d a = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    __m256d b = _mm256_set_pd(5.0, 6.0, 7.0, 8.0);
    __m256d c = _mm256_add_pd(a, b);

    double d[4];
    _mm256_storeu_pd(d, c);

    printf("AVX test successful:\n");
    printf("  %f + %f = %f\n", 1.0, 5.0, d[3]);
    printf("  %f + %f = %f\n", 2.0, 6.0, d[2]);
    printf("  %f + %f = %f\n", 3.0, 7.0, d[1]);
    printf("  %f + %f = %f\n", 4.0, 8.0, d[0]);

    // Check if the result is correct
    if (d[3] == 6.0 && d[2] == 8.0 && d[1] == 10.0 && d[0] == 12.0) {
        printf("\nResult is correct.\n");
        return 0;
    } else {
        printf("\nResult is incorrect.\n");
        return 1;
    }
}

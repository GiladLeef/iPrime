#include <stdbool.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void displayProgressBar(float progress) {
    int barWidth = 40;
    int pos = barWidth * progress;

    printf("[");
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %.2f%%\r", progress * 100);
    fflush(stdout);
}

void modularSquareAndMultiply(mpz_t base, mpz_t exponent, mpz_t mod, mpz_t result) {
    mpz_set_ui(result, 1);
    mpz_t temp;
    mpz_init(temp);

    while (mpz_sgn(exponent) > 0) {
        if (mpz_tstbit(exponent, 0)) {
            mpz_mul(temp, result, base);
            mpz_mod(result, temp, mod);
        }
        mpz_mul(temp, base, base);
        mpz_mod(base, temp, mod);

        mpz_fdiv_q_2exp(exponent, exponent, 1);
    }

    mpz_clear(temp);
}

bool findLucasLehmerNumber(int exp, mpz_t mod, int updateFrequency) {
    mpz_t s;
    mpz_init(s);
    mpz_set_ui(s, 4);

    int progressBarUpdate = updateFrequency;
    for (int i = 0; i < exp - 2; i++) {
        mpz_t temp;
        mpz_init(temp);

        mpz_mul(temp, s, s);
        mpz_sub_ui(temp, temp, 2);
        mpz_mod(s, temp, mod);

        mpz_clear(temp);

        if (--progressBarUpdate == 0) {
            float progress = (float)i / (exp - 2);
            displayProgressBar(progress);
            progressBarUpdate = updateFrequency;
        }
    }
    bool isPrime = mpz_cmp_ui(s, 0) == 0;

    mpz_clear(s);

    return isPrime;
}

bool isPrime(unsigned int num) {
    if (num <= 1) return false;
    if (num <= 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;

    for (unsigned int i = 5; i * i <= num; i += 6) {
        unsigned int squared_i = i * i;
        if (num % squared_i == 0 || num % (squared_i + 2 * i) == 0) return false;
    }
    return true;
}

void lucasLehmer(int exp, mpz_t mod) {
    size_t size = mpz_sizeinbase(mod, 10);
    int updateFrequency = (exp - 2) / 100;  // Calculate updateFrequency once

    clock_t start = clock();
    bool lucasNum = findLucasLehmerNumber(exp, mod, updateFrequency);
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    if (lucasNum) {
        printf("2^%d - 1 is a Prime number!\n", exp);
        printf("The length of this prime is %zu digits!\n", size);
    } else {
        printf("2^%d - 1 is definitely not prime\n", exp);
    }

    printf("That calculation took %f seconds\n", cpu_time);
}

void lucasSetup(int expNum) {
    mpz_t mod;
    mpz_init(mod);

    mpz_ui_pow_ui(mod, 2, expNum);
    mpz_sub_ui(mod, mod, 1);

    lucasLehmer(expNum, mod);

    mpz_clear(mod);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s N\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);

    printf("Generating prime numbers between %d and %d...\n", N, 2 * N);

    FILE *file = fopen("primes.txt", "w");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    for (int num = N; num <= 2 * N; num++) {
        if (isPrime(num)) {
            fprintf(file, "%d\n", num);
        }
    }

    fclose(file);
    printf("Prime numbers saved in primes.txt\n");

    FILE *primeFile = fopen("primes.txt", "r");
    if (primeFile == NULL) {
        printf("Error opening primes.txt\n");
        return 1;
    }

    FILE *foundFile = fopen("found.txt", "w");
    if (foundFile == NULL) {
        printf("Error opening found.txt\n");
        fclose(primeFile);
        return 1;
    }

    char expStr[32];
    while (fgets(expStr, sizeof(expStr), primeFile)) {
        int expNum = atoi(expStr);
        if (isPrime(expNum)) {
            mpz_t mod;
            mpz_init(mod);

            mpz_ui_pow_ui(mod, 2, expNum);
            mpz_sub_ui(mod, mod, 1);

            if (findLucasLehmerNumber(expNum, mod, (expNum - 2) / 100)) {
                fprintf(foundFile, "%s", expStr);
                lucasSetup(expNum);
            }

            mpz_clear(mod);
        }
    }

    fclose(primeFile);
    fclose(foundFile);

    return 0;
}
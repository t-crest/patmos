#include <stdio.h>

int isPrime(int z)
{
    int i = 1;
    while (i < z/2)
    {
        if (z % i == 0)
            return 0;
        i++;
    }
    return 1;
}

int main(int argc, char** argv) {
    int numberOfPrimes = 0;

    numberOfPrimes += isPrime(1);
    numberOfPrimes += isPrime(2);
    numberOfPrimes += isPrime(3);
    numberOfPrimes += isPrime(4);
    numberOfPrimes += isPrime(5);
    numberOfPrimes += isPrime(6);

    printf("Number of primes: %d\n", numberOfPrimes);

    return 0;
}

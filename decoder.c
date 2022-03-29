#include <stdio.h>
#include <math.h>

/**
 * Converts the given fractional representation
 *  to a floating point number.
 */
double convert(int x) {
    double num = 0.0;
    for(int i = 0; i < 32; i++) {
        if(((x >> (31 - i)) & 1) == 1) {
            num += 1.0 / pow(2, i + 1);
        }
    }
    return num;
}

void test(int num1, int num2) {
    double d1 = convert(num1),
           d2 = convert(num2);

    printf("%x decoded to %f\n", num1, d1);
    printf("%x decoded to %f\n", num2, d2);

    printf("diff = %f seconds\n", d2 - d1);
}

int main() {

    test(0x07ae1472, 0x5ef9dab5);
    test(0xf2f1aa0b, 0x5a1caba0);
    test(0x1e353f5a, 0x628f5bb7);

    return 0;
}

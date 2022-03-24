#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define WAIT_TIME 1000000
#define WAIT_EPSILON 500
#define WAIT_UNTIL_TIME 5
#define TESTS 1000

long long diff_time(struct timeval end, struct timeval start) {
    return (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
}

/**
 * Waits for WAIT_TIME seconds using a dumb approach.
 */
long long busy_wait() {
    struct timeval start, end;
    long long diff;

    gettimeofday(&start, NULL);
    do {
        gettimeofday(&end, NULL);
        diff = diff_time(end, start);
    } while(diff < WAIT_TIME);

    return diff - WAIT_TIME;
}

/**
 * Waits for WAIT_TIME seconds using sleep.
 */
long long sleep_wait() {
    struct timeval start, end;
    long long diff;

    gettimeofday(&start, NULL);
    sleep(WAIT_TIME / 1000000);
    gettimeofday(&end, NULL);
    diff = diff_time(end, start);

    return diff - WAIT_TIME;
}

/**
 * First waits using sleep and then finishes off using a busy wait
 *  in an attempt to get very low time difference.
 */
long long custom_wait() {
    struct timeval start, middle, end;
    long long busy_time, diff;

    // first use sleep to wait using interrupts
    gettimeofday(&start, NULL);
    sleep((WAIT_TIME - WAIT_EPSILON) / 1000000);
    gettimeofday(&middle, NULL);

    // now busy wait for the remaining time
    busy_time = WAIT_EPSILON - diff_time(middle, start);
    printf("busy time = %lld\n", busy_time);
    do {
        gettimeofday(&end, NULL);
        diff = diff_time(end, start);
    } while(diff < WAIT_TIME);

    return diff - WAIT_TIME;
}

/**
 * Waits until time modulo WAIT_UNTIL_TIME is 0.
 */
long long wait_until() {
    time_t current_time;
    struct timeval start, end;
    long long diff, diff2;

    gettimeofday(&start, NULL);
    time(&current_time);
    diff2 = WAIT_UNTIL_TIME - (current_time % WAIT_UNTIL_TIME);
    sleep(diff2);
    gettimeofday(&end, NULL);
    diff = diff_time(end, start);

    return diff - diff2 * 1000000;
}

/**
 * Runs each wait function.
 */
int main(int argc, char **argv) {
    long long diff, sum, max, min;

    printf("Running %d wait tests with %d seconds of wait...\n", TESTS, WAIT_TIME / 1000000);

    char *func_names[4] = {"Busy wait", "Sleep wait", "Custom wait", "Wait until"};
    long long (*funcs[4])() = {&busy_wait, &sleep_wait, &custom_wait, &wait_until};
    for(int i = 2; i < 3; i++) {
        printf("Running %s tests...\n", func_names[i]);
        // get initial value for sum, max, min
        sum = max = min = funcs[i]();
        for(int j = 1; j < TESTS; j++) {
            diff = funcs[i]();
            if(diff > max) {
                max = diff;
            } else if(diff < min) {
                min = diff;
            }
        }

        // print average
        printf("%s test:\n", func_names[i]);
        printf("  Time loss = %lld\n", sum);
        printf("  Max loss  = %lld\n", max);
        printf("  Min loss  = %lld\n", min);
        printf("  Avg. loss = %lld\n", sum / TESTS);
    }

    return 0;
}

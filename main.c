#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define WAIT_TIME 1000000
#define WAIT_EPSILON 500
#define WAIT_UNTIL_TIME 5
#define TESTS 10

typedef long long (*wait_func)(long, long);

long long diff_time(struct timeval end, struct timeval start) {
    return (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
}

/**
 * Waits for wait_time seconds using a dumb approach.
 */
long long busy_wait(long wait_time, long _dummy) {
    struct timeval start, end;
    long long diff;

    gettimeofday(&start, NULL);
    do {
        gettimeofday(&end, NULL);
        diff = diff_time(end, start);
    } while(diff < wait_time);

    return diff - wait_time;
}

/**
 * Waits for wait_time seconds using sleep.
 */
long long sleep_wait(long wait_time, long _dummy) {
    struct timeval start, end;
    long long diff;

    gettimeofday(&start, NULL);
    sleep(wait_time / 1000000);
    gettimeofday(&end, NULL);
    diff = diff_time(end, start);

    return diff - wait_time;
}

/**
 * First waits using sleep and then finishes off using a busy wait
 *  in an attempt to get very low time difference.
 */
long long custom_wait(long wait_time, long wait_epsilon) {
    struct timeval start, middle, end;
    long long busy_time, diff;

    // first use sleep to wait using interrupts
    gettimeofday(&start, NULL);
    sleep((wait_time - wait_epsilon) / 1000000);
    gettimeofday(&middle, NULL);

    // now busy wait for the remaining time
    busy_time = wait_epsilon - diff_time(middle, start);
    do {
        gettimeofday(&end, NULL);
        diff = diff_time(end, start);
    } while(diff < wait_time);

    return diff - wait_time;
}

/**
 * Waits until time modulo modulo_time is 0.
 */
long long wait_until(long modulo_time, long _dummy) {
    time_t current_time;
    long long diff2;

    time(&current_time);
    diff2 = modulo_time - (current_time % modulo_time);

    // use different wait impl.
    return sleep_wait(diff2 * 1000000, 0);
}

// TODO: sleep weighted, custom weighted

void run_test(wait_func func, char *func_name, long param1, long param2) {
    long long diff, sum, max, min;
    // TODO: long weighted_avg = 0.0;

    printf("Running %s tests...\n", func_name);
    // set initial value for sum, max, min
    sum = min = func(param1, param2);
    max = 0;
    for(int j = 1; j < TESTS; j++) {
        diff = func(param1, param2);
        sum += diff;
        if(diff > max) {
            max = diff;
        } else if(diff < min) {
            min = diff;
        }
    }
    // print average
    printf("%s test:\n", func_name);
    printf("  Time loss = %lld\n", sum);
    printf("  Max loss  = %lld\n", max);
    printf("  Min loss  = %lld\n", min);
    printf("  Avg. loss = %f\n", ((double) sum) / TESTS);
}

void full_test(long wait_time, long wait_epsilon, long modulo_time) {
    run_test(&busy_wait, "busy wait", wait_time, -1);
    run_test(&sleep_wait, "sleep wait", wait_time, -1);
    run_test(&custom_wait, "custom wait", wait_time, wait_epsilon);
    run_test(&wait_until, "wait until time", modulo_time, -1);
}

/**
 * Runs each wait function.
 */
int main(int argc, char **argv) {

    // TODO: argument parsing

    printf("Running %d wait tests with %d seconds of wait...\n", TESTS, WAIT_TIME / 1000000);
    full_test(WAIT_TIME, WAIT_EPSILON, WAIT_UNTIL_TIME);

    return 0;
}

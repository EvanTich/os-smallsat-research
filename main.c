#include <stdio.h>      /* printf */
#include <time.h>       /* clock */
#include <sys/time.h>   /* gettimeofday */
#include <string.h>     /* strcmp */
#include <stdlib.h>     /* atoi */

#define WAIT_TIME 1000000
#define WAIT_EPSILON 500
#define WAIT_MODULO 5
#define TESTS 100

#define VERBOSE 1

typedef long long (*wait_func)(long, long*);

int sleep(long microseconds) {
    struct timespec ts;

    if(microseconds < 0) {
        return -1;
    }

    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;

    return nanosleep(&ts, &ts);
}

long long diff_time(struct timeval end, struct timeval start) {
    return (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
}

/**
 * Waits for wait_time seconds using a dumb approach.
 */
long long busy_wait(long wait_time, long* _) {
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
long long sleep_wait(long wait_time, long* _) {
    struct timeval start, end;
    long long diff;

    gettimeofday(&start, NULL);
    sleep(wait_time);
    gettimeofday(&end, NULL);
    diff = diff_time(end, start);

    return diff - wait_time;
}

/**
 * First waits using sleep and then finishes off using a busy wait
 *  in an attempt to get very low time difference.
 */
long long custom_wait(long wait_time, long* wait_epsilon) {
    struct timeval start, end;
    long long diff;

    // first use sleep to wait using interrupts
    gettimeofday(&start, NULL);
    sleep(wait_time - *wait_epsilon);

    // now busy wait for the remaining time
    do {
        gettimeofday(&end, NULL);
        diff = diff_time(end, start);
    } while(diff < wait_time);

    return diff - wait_time;
}

/**
 * Waits until time modulo modulo_time is 0.
 * Not included in the full test run due to it being
 *  functionally different from the other wait functions.
 */
long long wait_until(long modulo_time, long* _) {
    struct timeval current_time;
    long long diff;

    gettimeofday(&current_time, NULL);
    diff = modulo_time - (current_time.tv_sec % modulo_time);

    // use different wait impl.
    return sleep_wait(diff * 1000000, 0);
}

/**
 * The goal of this wait function is to reduce the amount of CPU cycles
 *  from the busy wait after sleeping.
 * Uses the weighted average of the amount of time it should take to
 *  sleep and then busy wait.
 */
long long custom_weighted_wait(long wait_time, long* wait_epsilon) {
    struct timeval start, middle, end;
    long long busy_time, diff;

    if(*wait_epsilon < 0) {
        *wait_epsilon = WAIT_EPSILON;
    }

    // first use sleep to wait using interrupts
    gettimeofday(&start, NULL);
    sleep(wait_time - *wait_epsilon);
    gettimeofday(&middle, NULL);

    // now busy wait for the remaining time
    busy_time = diff_time(middle, start) - wait_time + *wait_epsilon ;
    do {
        gettimeofday(&end, NULL);
        diff = diff_time(end, start);
    } while(diff < wait_time);

#if 0
    printf("epsilon = %ld\n", *wait_epsilon);
    printf("busy time = %lld\n", busy_time);
#endif

    // 90% of wait is kept, current wait is more important
    *wait_epsilon = (9 * *wait_epsilon + busy_time) / 10;

    return diff - wait_time;
}

/**
 * Runs tests on the given function using the given parameters.
 *  Handles weighted averages for
 */
void run_test(wait_func func, char *func_name, long wait_time, long epsilon, int tests) {
    long long diff, sum, max, min;
    clock_t clock_time;
    unsigned long long clock_sum;
    char epsilon_enabled = epsilon != -1;

    printf("Running %s tests...\n", func_name);
    // set initial value for sum, max, min
    sum = max = min = func(wait_time, &epsilon);
    for(int j = 1; j < tests; j++) {
        clock_time = clock();

        diff = func(wait_time, &epsilon);
        clock_time = clock() - clock_time;

        clock_sum += clock_time;
        sum += diff;
        if(diff > max) {
            max = diff;
        } else if(diff < min) {
            min = diff;
        }

#if VERBOSE
        printf("diff = %lld\n", diff);
        printf("clock cycles = %ld\n", clock_time);

        if(epsilon_enabled && epsilon < 0) {
            printf("Epsilon is negative! %ld\n", epsilon);
        }

        if(j % 100 == 0) {
            printf("...iteration %d epsilon = %ld\n", j, epsilon);
        }
#endif
    }

    // print stats
    printf("%s test:\n", func_name);
    printf("  CPU Time used = %lld cycles\n", clock_sum);
    printf("      Time loss = %lld microseconds\n", sum);
    printf("       Max loss = %lld\n", max);
    printf("       Min loss = %lld\n", min);
    printf("      Avg. loss = %f\n", ((double) sum) / tests);
    if(epsilon_enabled) {
        printf("        Epsilon = %ld\n", epsilon);
    }
}

void full_test(long wait_time, long wait_epsilon, long modulo_time, int tests) {
    run_test(&busy_wait, "busy wait", wait_time, -1, tests);
    run_test(&sleep_wait, "sleep wait", wait_time, -1, tests);
    run_test(&custom_wait, "custom wait", wait_time, wait_epsilon, tests);
    run_test(&custom_weighted_wait, "custom weighted wait", wait_time, wait_epsilon, tests);
}

/**
 * Runs each wait function.
 */
int main(int argc, char **argv) {
    int wait_time = WAIT_TIME,
        wait_epsilon = WAIT_EPSILON,
        modulo_time = WAIT_MODULO,
        tests = TESTS,
        test = 0;

    for(int i = 1; i < argc - 1; i++) {
        if(strcmp(argv[i], "--wait-time") == 0) {
            wait_time = atoi(argv[i + 1]);
            if(wait_time <= 0) {
                printf("Invalid wait time! Reset to %d.\n", WAIT_TIME);
                wait_time = WAIT_TIME;
            }
            i++;
        } else if(strcmp(argv[i], "--epsilon") == 0) {
            wait_epsilon = atoi(argv[i + 1]);
            if(wait_epsilon < -1) {
                printf("Invalid epsilon time! Reset to %d.\n", WAIT_EPSILON);
                wait_epsilon = WAIT_EPSILON;
            }
            i++;
        } else if(strcmp(argv[i], "--modulo-time") == 0) {
            modulo_time = atoi(argv[i + 1]);
            if(modulo_time <= 0) {
                printf("Invalid modulo time! Reset to %d.\n", WAIT_MODULO);
                modulo_time = WAIT_MODULO;
            }
            i++;
        } else if(strcmp(argv[i], "--iterations") == 0) {
            tests = atoi(argv[i + 1]);
            if(tests <= 0) {
                printf("Invalid number of tests! Reset to %d.\n", TESTS);
                tests = TESTS;
            }
            i++;
        } else if(strcmp(argv[i], "--test") == 0) {
            test = atoi(argv[i + 1]);
            if(test < 0 || test > 5) {
                printf("Invalid test number! Reset to test 0.\n");
                test = 0;
            }
            i++;
        }
    }

    printf("Running %d wait tests with %f seconds of wait...\n", tests, wait_time / 1000000.0);
    switch(test) {
        case 0:
            run_test(&busy_wait, "busy wait", wait_time, -1, tests);
            break;
        case 1:
            run_test(&sleep_wait, "sleep wait", wait_time, -1, tests);
            break;
        case 2:
            run_test(&custom_wait, "custom wait", wait_time, wait_epsilon, tests);
            break;
        case 3:
            run_test(&custom_weighted_wait, "custom weighted wait", wait_time, wait_epsilon, tests);
            break;
        case 4:
            run_test(&wait_until, "wait until time", modulo_time, -1, tests);
            break;
        case 5:
            full_test(wait_time, wait_epsilon, modulo_time, tests);
            break;
    }
    printf("Reminder: Number of clock cycles per second = %ld\n", CLOCKS_PER_SEC);

    return 0;
}

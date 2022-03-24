#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define WAIT_TIME 60000000
#define WAIT_UNTIL_TIME 300

long long diff_time(struct timeval end, struct timeval start) {
    return (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
}

/**
 * Waits for WAIT_TIME seconds using a dumb approach.
 */
void busy_wait() {
    struct timeval start, end;
    long long diff;

    gettimeofday(&start, NULL);
    do {
        gettimeofday(&end, NULL);
        diff = diff_time(end, start);
    } while(diff < WAIT_TIME);

    printf("Execution time = %lld us\n", diff);
    printf("Difference = %lld us\n", diff - WAIT_TIME);
}

/**
 * Waits for WAIT_TIME seconds using sleep.
 */
void sleep_wait() {
    struct timeval start, end;
    long long diff;

    gettimeofday(&start, NULL);
    sleep(WAIT_TIME / 1000000);
    gettimeofday(&end, NULL);
    diff = diff_time(end, start);

    printf("Execution time = %lld us\n", diff);
    printf("Difference = %lld us\n", diff - WAIT_TIME);
}

/**
 * Waits until time modulo WAIT_UNTIL_TIME is 0.
 */
void wait_until() {
    time_t current_time;
    struct timeval start, end;
    long long diff, diff2;

    gettimeofday(&start, NULL);
    time(&current_time);
    diff2 = WAIT_UNTIL_TIME - (current_time % WAIT_UNTIL_TIME);
    sleep(diff2);
    gettimeofday(&end, NULL);
    diff = diff_time(end, start);

    printf("Execution time = %lld us\n", diff);
    printf("Difference = %lld us\n", diff - diff2 * 1000000);
}

/**
 * Runs each wait function.
 */
int main(int argc, char **argv) {
    busy_wait();
    sleep_wait();
    wait_until();
}

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX         102400
#define NUM_THREADS 20

int n1, n2;
char *s1, *s2;
int countArray[NUM_THREADS] = {0};

// Read inputs from emoji.txt
int readf(void)
{
    FILE *fp = fopen("emoji.txt", "r");
    if (fp == NULL) {
        printf("ERROR: can't open emoji.txt!\n");
        return -1;
    }

    s1 = (char *)malloc(MAX);
    s2 = (char *)malloc(MAX);
    if (!s1 || !s2) {
        printf("ERROR: Out of memory!\n");
        fclose(fp);
        return -1;
    }

    if (!fgets(s1, MAX, fp) || !fgets(s2, MAX, fp)) {
        printf("ERROR: failed to read lines from emoji.txt\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    // lengths; 
    n1 = (int)strlen(s1);
    n2 = (int)strlen(s2);
    if (n2 > 0 && s2[n2 - 1] == '\n') {
        s2[n2 - 1] = '\0';
        n2 -= 1;
    }

    if (n1 < n2 || n2 <= 0) {
        printf("ERROR: invalid lengths n1=%d, n2=%d\n", n1, n2);
        return -1;
    }
    return 0;
}

// Per-thread substring count
int num_substring(int t)
{
    int local = 0;

    // assignment assumes divisible
    int chunk       = n1 / NUM_THREADS;
    int owned_start = t * chunk;             // inclusive
    int owned_end   = (t + 1) * chunk - 1;   // inclusive

    // last legal start index 
    int max_start = n1 - n2;
    if (owned_end > max_start) owned_end = max_start;

    for (int i = owned_start; i <= owned_end; ++i) {
        if (memcmp(s1 + i, s2, (size_t)n2) == 0) {
            ++local;
        }
    }

    countArray[t] = local;  // store for reduction in main
    return local;
}

void *calSubStringThread(void *threadid)
{
    long tid = (long)threadid;
    int num = num_substring((int)tid);
    printf("This is thread %ld, num of substring \"%s\" is %d\n", tid, s2, num);
    return NULL;
}

int main(void)
{
    if (readf() != 0) {
        return 1;
    }

    pthread_t threads[NUM_THREADS];
    int rc;

    // launch
    for (long t = 0; t < NUM_THREADS; t++) {
        rc = pthread_create(&threads[t], NULL, calSubStringThread, (void *)t);
        if (rc) {
            printf("ERROR; pthread_create() returned %d\n", rc);
            return 1;
        }
    }

    // join
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // reduction
    int totalNum = 0;
    for (int t = 0; t < NUM_THREADS; t++) {
        totalNum += countArray[t];
    }

    printf("The number of substrings is: %d\n", totalNum);
    return 0;
}

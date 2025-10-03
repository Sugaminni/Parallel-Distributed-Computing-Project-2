#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX 102400
#define NUM_THREADS 20

int n1, n2;
char *s1, *s2;
FILE *fp;
int countArray[NUM_THREADS] = {0};

// read two strings from emoji.txt
int readf(FILE *fp)
{
    fp = fopen("emoji.txt", "r");
    if (fp == NULL) {
        printf("ERROR: can't open emoji.txt!\n");
        return -1;
    }

    s1 = (char *)malloc(MAX);
    s2 = (char *)malloc(MAX);
    if (!s1 || !s2) { printf("ERROR: Out of memory!\n"); fclose(fp); return -1; }

    if (!fgets(s1, MAX, fp) || !fgets(s2, MAX, fp)) {
        printf("ERROR: failed to read file\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    n1 = (int)strlen(s1);
    n2 = (int)strlen(s2);
    if (n2 > 0 && s2[n2 - 1] == '\n') { s2[n2 - 1] = '\0'; n2--; }

    if (n1 < n2 || n2 <= 0) return -1;
    return 0;
}

// count matches for thread t
int num_substring(int t)
{
    int local = 0;
    int chunk = n1 / NUM_THREADS;
    int start = t * chunk;
    int end   = (t + 1) * chunk - 1;

    int max_start = n1 - n2;
    if (end > max_start) end = max_start;

    for (int i = start; i <= end; i++) {
        if (memcmp(s1 + i, s2, (size_t)n2) == 0) {
            local++;
        }
    }

    countArray[t] = local;
    return local;
}

// work done by each thread
void *calSubStringThread(void *threadid){
    long tid = (long)threadid;
    int num = num_substring((int)tid);
    printf("Thread %ld found %d matches\n", tid, num);
    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[NUM_THREADS];
    int totalNum = 0;

    if (readf(fp) != 0) return 1;

    // create threads
    for (int t = 0; t < NUM_THREADS; t++) {
        if (pthread_create(&threads[t], NULL, calSubStringThread, (void *)(size_t)t)) {
            printf("ERROR creating thread %d\n", t);
            return 1;
        }
    }

    // wait for threads to finish
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // sum results
    for (int t = 0; t < NUM_THREADS; t++) {
        totalNum += countArray[t];
    }

    printf("The number of substrings is: %d\n", totalNum);
    return 0;
}

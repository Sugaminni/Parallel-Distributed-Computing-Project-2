#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX 102400
#define NUM_THREADS 20

// globals (simple for this assignment)
int n1, n2;
char *s1, *s2;
int countArray[NUM_THREADS] = {0};

int readf(void) {
    FILE *fp = fopen("emoji.txt", "r");
    if (!fp) { printf("ERROR: can't open emoji.txt!\n"); return -1; }
    s1 = (char*)malloc(MAX); s2 = (char*)malloc(MAX);
    if (!s1 || !s2) { printf("ERROR: OOM\n"); fclose(fp); return -1; }
    if (!fgets(s1, MAX, fp) || !fgets(s2, MAX, fp)) { printf("ERROR: read\n"); fclose(fp); return -1; }
    fclose(fp);
    n1 = (int)strlen(s1);
    n2 = (int)strlen(s2);
    if (n2 > 0 && s2[n2-1] == '\n') { s2[n2-1] = '\0'; --n2; }
    if (n1 < n2 || n2 <= 0) return -1;
    return 0;
}

int num_substring(int t) {
    int local = 0;
    int chunk = n1 / NUM_THREADS;
    int owned_start = t * chunk;                 // inclusive
    int owned_end   = (t + 1) * chunk - 1;       // inclusive
    int max_start   = n1 - n2;                   // last valid start
    if (owned_end > max_start) owned_end = max_start;

    for (int i = owned_start; i <= owned_end; ++i)
        if (memcmp(s1 + i, s2, (size_t)n2) == 0) ++local;

    countArray[t] = local;
    return local;
}

void* worker(void* vp) {
    long tid = (long)vp;
    int num = num_substring((int)tid);
    printf("Thread %ld local = %d\n", tid, num);
    return NULL;
}

int main(void) {
    if (readf() != 0) return 1;

    pthread_t th[NUM_THREADS];
    for (long t = 0; t < NUM_THREADS; ++t)
        if (pthread_create(&th[t], NULL, worker, (void*)t)) { puts("pthread_create err"); return 1; }

    for (int t = 0; t < NUM_THREADS; ++t) pthread_join(th[t], NULL);

    int total = 0; for (int t = 0; t < NUM_THREADS; ++t) total += countArray[t];
    printf("The number of substrings is: %d\n", total);

    free(s1); free(s2);
    return 0;
}

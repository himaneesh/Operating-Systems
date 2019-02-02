#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

#define NUM_BUCKETS 5 
#define NUM_KEYS 100000   // Number of keys inserted per thread
int num_threads = 1;      // Number of threads (configurable)
int keys[NUM_KEYS];


/*
Create locks for each of the 5 buckets
*/
//pthread_mutex_t lock;//comment for part 4
pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
pthread_mutex_t lock4;
pthread_mutex_t lock5;



/*
PART 1: 

Find out under what circumstances entries can get lost?
The keys are getting lost because the buckets are being modified in parallel,
without using a locking system. 
Many of the keys are contending for the same bucket and as a result the memory 
requests are not getting fulfilled


Without Mutex:
[main] Inserted 100000 keys in  0.0517 seconds
[main] Retrieved 96598/100000 keys in 2.0234 seconds

With Mutex:
[main] Inserted 100000 keys in 0.01345 seconds
[main] Retrieved 100000/100000 keys in 8.6386 seconds

There is an overhead of adding a mutex



PART 3:
Does retrieving an item from the hash table require
a lock?
No retrieving doesn't require a lock 
Multiple retrieves can run in parallel.
*/


typedef struct _bucket_entry {
    int key;
    int val;
    struct _bucket_entry *next;
} bucket_entry;

bucket_entry *table[NUM_BUCKETS];


void panic(char *msg) {
    printf("%s\n", msg);
    exit(1);
}

double now() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Inserts a key-value pair into the table
void insert(int key, int val) {
    int i = key % NUM_BUCKETS;
if(i==1)
pthread_mutex_lock(&lock1);
if(i==2)
pthread_mutex_lock(&lock2);
if(i==3)
pthread_mutex_lock(&lock3);
if(i==4)
pthread_mutex_lock(&lock4);
if(i==0)
pthread_mutex_lock(&lock5);

    bucket_entry *e = (bucket_entry *) malloc(sizeof(bucket_entry));

    if (!e) panic("No memory to allocate bucket!");
    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;

if(i==1)
pthread_mutex_unlock(&lock1);
if(i==2)
pthread_mutex_unlock(&lock2);
if(i==3)
pthread_mutex_unlock(&lock3);
if(i==4)
pthread_mutex_unlock(&lock4);
if(i==0)
pthread_mutex_unlock(&lock5);

}

// Retrieves an entry from the hash table by key
// Returns NULL if the key isn't found in the table
bucket_entry * retrieve(int key) {
    bucket_entry *b;
    for (b = table[key % NUM_BUCKETS]; b != NULL; b = b->next) {
        if (b->key == key) return b;
    }
    return NULL;
}

void * put_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;

    // If there are k threads, thread i inserts
    //      (i, i), (i+k, i), (i+k*2)
    for (key = tid ; key < NUM_KEYS; key += num_threads) {
//pthread_mutex_lock(&lock);// We use this locking mechanism for part 1 and part 3 but not for part 4
        insert(keys[key], tid);
//pthread_mutex_unlock(&lock);//Commented to run insert operation in parallel.
    }

    pthread_exit(NULL);
}

void * get_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;
    long lost = 0;

    for (key = tid ; key < NUM_KEYS; key += num_threads) {
//pthread_mutex_lock(&lock); //commented for part 3
        if (retrieve(keys[key]) == NULL) lost++;
//pthread_mutex_unlock(&lock);// commented for part 3
    }
    printf("[thread %ld] %ld keys lost!\n", tid, lost);

    pthread_exit((void *)lost);
}

int main(int argc, char **argv) {
    long i;
    pthread_t *threads;
    double start, end;

    if (argc != 2) {
        panic("usage: ./parallel_hashtable <num_threads>");
    }
    if ((num_threads = atoi(argv[1])) <= 0) {
        panic("must enter a valid number of threads to run");
    }

    srandom(time(NULL));
    for (i = 0; i < NUM_KEYS; i++)
        keys[i] = random();

    threads = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);
    if (!threads) {
        panic("out of memory allocating thread handles");
    }
    pthread_mutex_init(&lock1, NULL);
pthread_mutex_init(&lock2, NULL);
pthread_mutex_init(&lock3, NULL);
pthread_mutex_init(&lock4, NULL);
pthread_mutex_init(&lock5, NULL);
    // Insert keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, put_phase, (void *)i);
    }
    
    // Barrier
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end = now();
    
    printf("[main] Inserted %d keys in %f seconds\n", NUM_KEYS, end - start);
    
    // Reset the thread array
    memset(threads, 0, sizeof(pthread_t)*num_threads);

    // Retrieve keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, get_phase, (void *)i);
    }

    // Collect count of lost keys
    long total_lost = 0;
    long *lost_keys = (long *) malloc(sizeof(long) * num_threads);
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], (void **)&lost_keys[i]);
        total_lost += lost_keys[i];
    }
    end = now();

    printf("[main] Retrieved %ld/%d keys in %f seconds\n", NUM_KEYS - total_lost, NUM_KEYS, end - start);

    return 0;
}

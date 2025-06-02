#include "simulator.h"
#include "common.h"

#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define THINKING        0                   /* philosopher is thinking */
#define HUNGRY          1                   /* philosopher is trying to get forks */
#define EATING          2                   /* philosopher is eating */

typedef sem_t semaphore;
typedef struct {
    int id;
    int total_eats;
    double total_wait;
} philosopher_stat;

semaphore state_mtx;                        /* mutual exclusion for state data*/
semaphore print_mtx;                        /* printer access mutual exclusion */

int n_philosophers;
int* state;                               /* array to keep track of everyone’s state */
semaphore* wait;                          /* one semaphore per philosopher */

bool keep_running = true;                          /* flag to exit thread */

/* forward declarations */
void acquire_forks(int id);
void release_forks(int id);
void try_eat(int id);
void* philosopher(void *v);

static inline int left_philosopher(int i) {
    return (i+n_philosophers-1)%n_philosophers;
}

static inline int right_philosopher(int i) {
    return (i+1)%n_philosophers;
}

void print_message(int id, const char* message, int r) {
    sem_wait(&print_mtx);
    printf("philosopher [%d]: %s (%d) ...\n", id, message, r);
    fflush(stdout);
    sem_post(&print_mtx);
}

void acquire_forks(int id) {
    sem_wait(&state_mtx);
    state[id] = HUNGRY;
    try_eat(id);
    sem_post(&state_mtx);
    sem_wait(&wait[id]);
}

void release_forks(int id) {
    sem_wait(&state_mtx);
    state[id] = THINKING;
    try_eat(left_philosopher(id));
    try_eat(right_philosopher(id));
    sem_post(&state_mtx);
}

void try_eat(int id) {
    if(state[id] == HUNGRY && state[left_philosopher(id)] != EATING && state[right_philosopher(id)] != EATING) {
        state[id] = EATING;
        sem_post(&wait[id]);
    }
}

void* philosopher(void *v) {
    int id = *(int*)v;
    print_message(id, "entering the dinning room", 0);
    philosopher_stat *stat = (philosopher_stat*)malloc(sizeof(philosopher_stat));
    *stat = (philosopher_stat) {
        .id = id,
        .total_wait = 0,
        .total_eats = 0
    };

    int r = 0;
    while (keep_running) {
        // think
        r = random_number() % 7;
        print_message(id, "thinking", r);
        sleep(r);

        // pick forks
        print_message(id, "hungry and waiting", 0);
        time_t start = time(nullptr);
        acquire_forks(id);
        stat->total_wait += difftime(time(nullptr), start);
        //eat
        r = (random_number() % 7) + 10;
        print_message(id, "eating", r);
        ++stat->total_eats;
        sleep(r);

        // put down forks
        release_forks(id);
    }

    print_message(id, "leaving the dinning room", 0);
    return stat;
}

void interupt_signal_handler(int sig) {
    keep_running = false;
}

void dinning_philosophers_simulator(int nthreads) {
    printf("Starting dinning philosophers simulator with %d philosophers ... \n\n", nthreads);
    n_philosophers = nthreads;
    state = (int*) malloc(sizeof(int) * n_philosophers);
    wait = (semaphore *) malloc(sizeof(semaphore) * n_philosophers);

    // initialize semaphores
    sem_init(&state_mtx, 0, 1);
    sem_init(&print_mtx, 0, 1);
    for(int i = 0; i<n_philosophers; ++i) {
        sem_init(&wait[i], 0, 1);
    }

    pthread_t threads[n_philosophers];
    int thread_ids[n_philosophers];
    for(int i = 0; i<n_philosophers; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, philosopher,  &thread_ids[i]);
    }

    signal(SIGINT, interupt_signal_handler);
    void *stats[n_philosophers];
    for(int i = 0; i<n_philosophers; ++i) {
        pthread_join(threads[i], &stats[i]);
    }

    printf("\n\nStats\n\n");
    for(int i = 0; i<n_philosophers; ++i) {
        philosopher_stat *ph = (philosopher_stat*)stats[i];
        printf("philosopher [%d]: wait: %ds eat: %d wait per eat: %.2fs\n",
               ph->id, (int)ph->total_wait, ph->total_eats, ph->total_wait/ph->total_eats);
        free(stats[i]);
    }

    sem_destroy(&state_mtx);
    sem_destroy(&print_mtx);
    for(int i = 0; i<n_philosophers; ++i) {
        sem_destroy(&wait[i]);
    }

    free(state);
    free(wait);
}

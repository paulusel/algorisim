#include "philosophers.h"

#include <pthread.h>
#include <stdlib.h>
#include <threads.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define THINKING        0                   /* philosopher is thinking */
#define HUNGRY          1                   /* philosopher is trying to get forks */
#define EATING          2                   /* philosopher is eating */

typedef sem_t semaphore;

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
int random_number();

static inline int left_philosopher(int i) {
    return (i+n_philosophers-1)%n_philosophers;
}

static inline int right_philosopher(int i) {
    return (i+1)%n_philosophers;
}

int random_number() {
    int random_num;
    int fd = open("/dev/urandom", O_RDONLY);
    read(fd, &random_num, sizeof(random_num));
    close(fd);
    return abs(random_num) % 7;
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

    int r = 0;
    while (keep_running) {
        // think
        r = random_number();
        print_message(id, "thinking", r);
        sleep(r);

        // pick forks
        print_message(id, "hungry and waiting", 0);
        acquire_forks(id);
        //eat
        r = random_number() + 3;
        print_message(id, "eating", r);
        sleep(r);

        // put down forks
        release_forks(id);
    }

    print_message(id, "leaving the dinning room", 0);
    return nullptr;
}

void interupt_signal_handler(int sig) {
    keep_running = false;
}

void philosophers_start_dinning(int nthreads) {
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
    for(int i = 0; i<n_philosophers; ++i) {
        pthread_join(threads[i], nullptr);
    }

    sem_destroy(&state_mtx);
    sem_destroy(&print_mtx);
    for(int i = 0; i<n_philosophers; ++i) {
        sem_destroy(&wait[i]);
    }

    free(state);
    free(wait);
}

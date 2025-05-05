#include "simulator.h"
#include "common.h"

#include "dstruct/vector.h"
#include "dstruct/dlinked_list.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define CHUNK_OCCUPIED      1
#define CHUNK_FREE          0

typedef struct {
    int status;             /* 0 - free, 1 - occupied */
    int address;                /* begin position as offset from 0 */
    int size;
} chunk;

typedef node* (*fit_finder)(dlist*, int);

node* search_best_fit(dlist *list, int size) {
    // search
    int diff = INT_MAX;
    node* nd = list->end;

    node *tip = list->end->next;
    while(tip != list->end) {          /* search the entire memory */
        chunk *slab = (chunk*)tip->data;
        if(slab->status == CHUNK_FREE && slab->size >= size) {
            if(slab->size - size < diff) {
                diff = slab->size - size;
                nd = tip;
            }
        }
        tip = tip->next;
    }

    return nd;
}

node* search_worst_fit(dlist *list, int size) {
    // search
    int diff = INT_MIN;
    node* nd = list->end;

    node *tip = list->end->next;
    while(tip != list->end) {                                      /* search the entire memory */
        chunk *slab = (chunk*)tip->data;
        if(slab->status == CHUNK_FREE && slab->size >= size) {
            if(slab->size - size > diff) {
                diff = slab->size - size;
                nd = tip;
            }
        }
        tip = tip->next;
    }

    return nd;
}

int fit_allocate(dlist *list, int size, fit_finder find){
    node* nd = find(list, size);

    if(nd == list->end) return -1; // no space

    chunk *slab = (chunk*)nd->data;
    int address = slab->address;

    if(slab->size == size) {
        slab->status = CHUNK_OCCUPIED;
    }
    else {
        chunk *new_slab = (chunk*)malloc(sizeof(chunk));

        // allocate new area
        new_slab->status = CHUNK_OCCUPIED;
        new_slab->address = slab->address;
        new_slab->size = size;

        // shrink the free space
        slab->address += size;
        slab->size -= size;

        dlist_insert(list, nd, new_slab);
    }

    return address;
}

void fit_free(dlist *list, int pos) {
    //search for pos
    node *nd = list->end->next;
    while(nd != list->end) {
        chunk *slab = (chunk*)nd->data;
        if(slab->address == pos) break;
        nd = nd->next;
    }
    if(nd == list->end) return;        /* pos not found */

    chunk *slab = (chunk*)nd->data;
    slab->status = CHUNK_FREE;                   /* mark it free */

    if(nd->prev != list->end) {
        chunk *left_slab = (chunk*)nd->prev->data;
        if(left_slab->status == CHUNK_FREE) {
            slab->address = left_slab->address;
            slab->size += left_slab->size;

            free(nd->prev->data);
            dlist_remove(list, nd->prev);
        }
    }

    if(nd->next != list->end) {
        chunk *right_slab = (chunk*)nd->next->data;
        if(right_slab->status == CHUNK_FREE) {
            slab->size += right_slab->size;

            free(nd->next->data);
            dlist_remove(list, nd->next);
        }
    }
}

static inline void print_list(dlist* list) {
    node* nd = list->end->next;
    printf("\n");
    while(nd != list->end) {
        chunk *slab = (chunk*)nd->data;
        printf("%c %d %d --> ", slab->status == CHUNK_FREE ? 'H' : 'P', slab->address, slab->size);
        nd = nd->next;
    }
}

static inline void simulate_activity(dlist *list, vector *v, fit_finder find) {
    for(int i = 0; i<1000; ++i) {
        int random = random_number();
        if(random % 3 == 0) {
            // free one of the existing
            if(v->size > 0) {
                int rand_indx = random_number() % v->size;
                int pos = *(int*)vector_at(v, rand_indx);
                vector_remove_unordered(v, rand_indx);
                fit_free(list, pos);
            }
        }
        else {
            // request new memory
            int rand_size = (random % 7) + 1;
            if(random % 23) rand_size *= 2;
            if(random % 53) rand_size *= 5;
            int addr = fit_allocate(list, rand_size, find);
            if(addr > -1) vector_push_back(v, &addr);
        }
    }
}

static inline void initialize_simulator(dlist **list, vector **v) {
        size_t mem_size = 1024; // 1KB

        chunk *total = (chunk*)malloc(sizeof(chunk));
        total->address = 0;
        total->status = CHUNK_FREE;
        total->size = mem_size;

        dlist *lst = dlist_create();
        dlist_insert(lst, lst->end, total);

        *v = vector_create(sizeof(int), mem_size/10);
        *list = lst;
}

static inline double inspect_results(dlist *list) {
    // inspect the result
    int free_chunk_count = 0, total_free = 0;
    node* nd = list->end->next;
    while(nd != list->end) {
        chunk *slab = (chunk*)nd->data;
        if(slab->status == CHUNK_FREE) {
            ++free_chunk_count;
            total_free += slab->size;
            printf("%d ", slab->size);
        }
        nd = nd->next;
    }
    double average = (double)total_free/free_chunk_count;
    printf("\nFree: %d, Av. Size: %.2f\n", free_chunk_count, average);
    return average;
}

static inline void release_resources(dlist *list, vector *v) {
    node *nd = list->end->next;
    while(nd != list->end) {
        free(nd->data);
        nd = nd->next;
    }
    dlist_destrory(list);
    vector_destroy(v);
}

void launch_simulator(fit_finder find) {
    dlist *list;
    vector *v;

    double average = 0;
    for(int i = 0; i<10; ++i) {
        initialize_simulator(&list, &v);
        simulate_activity(list, v, find);
        average += inspect_results(list);
        release_resources(list, v);
    }
    printf("\nAverage Total: %.2f\n", average/10);
}

void best_fit_simulator() {
    printf("Launching best fit simulator ... \n\n");
    launch_simulator(search_best_fit);
}

void worst_fit_simulator() {
    printf("Launching worst fit simulator ... \n\n");
    launch_simulator(search_worst_fit);
}

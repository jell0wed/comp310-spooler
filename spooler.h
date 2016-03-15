//
// Created by Jérémie Poisson on 2016-03-08.
//

#ifndef CODE_SPOOLER_OBJECTS_H
#define CODE_SPOOLER_OBJECTS_H

#include "assert.h"
#include <semaphore.h>

typedef int bool;
#define true 1
#define false 0

#define SPOOLER_MAX_JOBS_COUNT 2

struct print_client {
    int id;
};
typedef struct print_client print_client;

typedef struct {
    int page_count;
    int position;
    print_client submitted_by;
} print_job;

typedef struct {
    print_job active_jobs[SPOOLER_MAX_JOBS_COUNT];
    int active_jobs_count;
    sem_t mutex;
    sem_t fill_count; // job produced
    sem_t empty_count; // remaining space left
} shared_spooler_data;

void enqueue_job(shared_spooler_data*, print_job* );
void dequeue_job(shared_spooler_data*, print_job* );
bool is_spooler_full(shared_spooler_data* );
bool is_spooler_empty(shared_spooler_data* );

#endif //CODE_SPOOLER_OBJECTS_H

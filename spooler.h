//
// Created by Jérémie Poisson on 2016-03-08.
//

#ifndef CODE_SPOOLER_OBJECTS_H
#define CODE_SPOOLER_OBJECTS_H

#include <semaphore.h>
#include "assert.h"

#define JOBS_BUFFER_CAPACITY    2

typedef int bool;
#define true 1
#define false 0

typedef struct {
    int id;
} print_client;

typedef struct {
    int page_count;
    int position;
    int duration;
    print_client submitted_by;
} print_job;

typedef struct {
    sem_t mutex;
    sem_t fill_count; // job produced
    sem_t empty_count; // remaining space left
    int active_jobs_count; // dynamic capacity of the spooler
    print_job active_jobs[JOBS_BUFFER_CAPACITY]; // jobs queue
} shared_spooler_data;

void enqueue_job(shared_spooler_data*, print_job* );
void dequeue_job(shared_spooler_data*, print_job* );

#endif //CODE_SPOOLER_OBJECTS_H

//
// Created by Jérémie Poisson on 2016-03-08.
//

#include <string.h>
#include "spooler.h"

/** void enqueue_job(shared_spooler_data* spooler, print_job* job)
 * enqueue a job at the end of the array
 */
void enqueue_job(shared_spooler_data* spooler, print_job* job) {
    sem_wait(&(spooler->empty_count));
    sem_wait(&(spooler->mutex));

    assert(spooler->active_jobs_count + 1 <= SPOOLER_MAX_JOBS_COUNT);

    int nextPos = spooler->active_jobs_count;
    job->position = nextPos;

    memcpy(&(spooler->active_jobs[nextPos]), job, sizeof(print_job));
    spooler->active_jobs_count = spooler->active_jobs_count + 1;

    sem_post(&(spooler->mutex));
    sem_post(&(spooler->fill_count));
}

/** void dequeue_job(shared_spooler_data* spooler, print_job* job)
 * dequeues a job at the begining of the array then shifts the array
 */
void dequeue_job(shared_spooler_data* spooler, print_job* job) {
    sem_wait(&(spooler->fill_count));
    sem_wait(&(spooler->mutex));

    assert(spooler->active_jobs_count >= 1);
    memcpy(job, &(spooler->active_jobs[0]), sizeof(print_job));
    // shift every active jobs one spot
    int i;
    for(i = 1; i < spooler->active_jobs_count; i++) {
        //spooler->active_jobs[i - 1] = spooler->active_jobs[i];
        memcpy(&(spooler->active_jobs[i-1]), &(spooler->active_jobs[i]), sizeof(print_job));
    }
    spooler->active_jobs_count = spooler->active_jobs_count - 1;

    sem_post(&(spooler->mutex));
    sem_post(&(spooler->empty_count));
}

bool is_spooler_full(shared_spooler_data* spooler) {
    sem_wait(&(spooler->mutex));
    bool full = spooler->active_jobs_count >= SPOOLER_MAX_JOBS_COUNT;
    sem_post(&(spooler->mutex));
    return full;
}

bool is_spooler_empty(shared_spooler_data* spooler) {
    sem_wait(&(spooler->mutex));
    bool empty = spooler->active_jobs_count == 0;
    sem_post(&(spooler->mutex));
    return empty;
}
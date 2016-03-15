#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "spooler.h"

const char* KEY_FILE = "keyfile";
const size_t BUFF_SIZE = sizeof(shared_spooler_data);
int errno;

int setup_shared_mem();
shared_spooler_data* attach_shared_mem(int fd);
void initialized_shared_spooler(shared_spooler_data*);
void detatch_shared_mem(shared_spooler_data*);

void take_a_job(shared_spooler_data*, print_job* );
void output_job(print_job* );
void go_sleep(print_job* );

int main(int argc, const char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    int fd = setup_shared_mem();
    shared_spooler_data* spooler = attach_shared_mem(fd);
    initialized_shared_spooler(spooler);

    while(true) {
        print_job job;
        take_a_job(spooler, &job);

        output_job(&job);

        go_sleep(&job);
    }

    detatch_shared_mem(spooler);
    return 0;
}

/** setup_shared_mem()
 * Sets up the shared memory shared with the clients that contains the jobs buffer
 * for the pinter spooler
 */

int setup_shared_mem() {
    int fd = shm_open(KEY_FILE, O_CREAT | O_RDWR, 0666);
    if(fd == -1) {
        printf("%d", errno);
        perror("Failed to create nmap from appropriate key for shared memory");
        exit(1);
    }

    ftruncate(fd, BUFF_SIZE);
    return fd;
}

shared_spooler_data* attach_shared_mem(int fd) {
    shared_spooler_data* smemPtr = (shared_spooler_data*) mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(smemPtr == MAP_FAILED) {
        perror("Failed to attach to shared memory");
        exit(1);
    }

    return smemPtr;
}

/** void initialized_shared_spooler(shared_spooler_data*)
 * Initializes the shared spooler region to initial values
 */
void initialized_shared_spooler(shared_spooler_data* spooler) {
    spooler->active_jobs_count = 0;
    // initialize the semaphore
    sem_init(&(spooler->mutex), 1, 1);
    sem_init(&(spooler->empty_count), 1, SPOOLER_MAX_JOBS_COUNT);
    sem_init(&(spooler->fill_count), 1, 0);
    return;
}

/** detatch_shared_mem(shared_spooler_data*)
 * Detach the current process from the shared memory instance pointed by a shared memory instance passed
 * as a parameter
 */
void detatch_shared_mem(shared_spooler_data* smemPtr) {
    /*if(shmdt(smemPtr) == -1) {
        perror("Failed to detatch from shared memory instance (shmdt)");
        exit(1);
    }*/
    return;
}

/** void take_a_job(shared_spooler_data*, print_job* );
 * Takes a job from the spooler
 * */
void take_a_job(shared_spooler_data* spooler, print_job* job) {
    printf("Waiting for job; buffer is empty.\n");
    dequeue_job(spooler, job);
    return;
}

/** void output_job(shared_spooler_data* , print_job* )
 * Output job information on the console
 */
void output_job(print_job* job) {
    printf("Now processing job Buffer[%d] submitted by client id %d. Total of %d page(s) to be printed (%d seconds). \n",
            job->position,
            job->submitted_by.id,
            job->page_count,
            job->page_count);
}

/** void go_sleep(print_job* )
 * Make the server sleeps while processing a certain job
 */
void go_sleep(print_job* job) {
    int i;
    for(i = 0; i < job->page_count; i++) {
        printf(".");
        sleep(1);
    }
    printf("\n");
    printf("Finished processing job Buffer[%d] submitted by client id %d. \n", job->position, job->submitted_by.id);
}

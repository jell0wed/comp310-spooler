#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>

#include "spooler.h"

const char* KEY_FILE = "keyfile";
const size_t BUFF_BASE_SIZE = sizeof(shared_spooler_data);

void setup_shared_mem();
void initialized_shared_spooler();
void detatch_shared_mem();

void take_a_job(print_job* );
void output_job(print_job* );
void go_sleep(print_job* );

void server_spooler_cleanup(int );

// gobals
int fd;
shared_spooler_data* spooler = 0;

int main(int argc, const char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    signal(SIGINT, server_spooler_cleanup);
    setup_shared_mem();

    while(true) {
        print_job job;
        take_a_job(&job);

        output_job(&job);
        go_sleep(&job);
    }

    detatch_shared_mem();
    return 0;
}

/** setup_shared_mem(int )
 * Sets up the IPC shared memory with the appropriate key file. The dynamic job buffer size is passed as parameter.
 */

void setup_shared_mem() {
    fd = shm_open(KEY_FILE, O_CREAT | O_RDWR, 0666);
    if(fd == -1) {
        perror("shm_open failed(). Unable to create nmap from appropriate key for shared memory");
        exit(1);
    }

    ftruncate(fd, BUFF_BASE_SIZE);

    spooler = (shared_spooler_data*) mmap(NULL, BUFF_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(spooler == MAP_FAILED) {
        perror("mmap() failed. Unable to attach to shared memory");
        exit(1);
    }

    initialized_shared_spooler();
    return;
}


/** void initialized_shared_spooler(shared_spooler_data*)
 * Initializes the shared spooler region to initial values
 */
void initialized_shared_spooler() {
    spooler->active_jobs_count = 0;
    // semaphores setup
    sem_init(&(spooler->mutex), 1, 1);
    sem_init(&(spooler->empty_count), 1, JOBS_BUFFER_CAPACITY);
    sem_init(&(spooler->fill_count), 1, 0);

    printf("Initialized spooler with capacity of %d jobs.", JOBS_BUFFER_CAPACITY);
    return;
}

/** detatch_shared_mem(shared_spooler_data*)
 * Detach the current process from the shared memory instance pointed by a shared memory instance passed
 * as a parameter
 */
void detatch_shared_mem() {
    if(munmap(spooler, BUFF_BASE_SIZE) == -1) {
        perror("munmap() failed. Unable to free mmapped memory region.");
        exit(1);
    }

    close(fd);
    shm_unlink(KEY_FILE);
    return;
}

/** void take_a_job(shared_spooler_data*, print_job* );
 * Takes a job from the spooler
 * */
void take_a_job(print_job* job) {
    printf("Waiting for job-- print job buffer is empty.\n");
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
            job->duration);
}

/** void go_sleep(print_job* )
 * "Process" the actual job. Puts the spooler to sleep for a specific amount of time.
 */
void go_sleep(print_job* job) {
    int i;
    for(i = 0; i < job->duration; i++) {
        printf(".");
        sleep(1);
    }
    printf("\n");
    printf("Finished processing job Buffer[%d] submitted by client id %d. \n", job->position, job->submitted_by.id);
}

void server_spooler_cleanup(int sig) {
    printf("Cleaning up and releasing the shared memory.");
    if(spooler != 0) {
        detatch_shared_mem();
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "spooler.h"

const char* KEY_FILE = "keyfile";
const size_t BUFF_BASE_SIZE = sizeof(shared_spooler_data);

int setup_shared_mem();
shared_spooler_data* attach_share_mem(int fd);
print_job create_print_job(print_client*, int , int);
void put_job(print_client*, shared_spooler_data*, print_job* );
void release_shared_mem();

int fd;
shared_spooler_data* spooler;

int main(int argc, const char* argv[]) {
    int client_id;
    int num_pages;
    int duration;

    if(argc < 4) {
        perror("Invalid number of argument. Usage : ./client <client_id> <pages to print> <duration_in_sec>");
        exit(1);
    }

    client_id = atoi(argv[1]);
    num_pages = atoi(argv[2]);
    duration = atoi(argv[3]);

    if(num_pages <= 0) {
        printf("You must specify a positive number of pages to print.");
        exit(1);
    }

    if(duration <= 0) {
        printf("You must specify a positive number for the duration.");
    }

    print_client client;
    client.id = client_id;

    fd = setup_shared_mem();
    spooler = attach_share_mem(fd);
    print_job job = create_print_job(&client, num_pages, duration);
    put_job(&client, spooler, &job);

    release_shared_mem();

    return 0;
}

/** void setup_shared_mem()
 * Sets up the IPC shared memory region with the appropriate key.
 */
int setup_shared_mem() {
    int fd = shm_open(KEY_FILE, O_RDWR, 0666);
    if(fd == -1) {
        perror("shm_open() failed. Unable to load appropriate key for shared memory");
        exit(1);
    }

    return fd;
}

/** shared_spooler_data* attach_share_mem();
 * Attach to shared server memory through IPC and valid keyfile
 */
shared_spooler_data* attach_share_mem(int fd) {
    shared_spooler_data* smemPtr = (shared_spooler_data*) mmap(NULL, BUFF_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(smemPtr == MAP_FAILED) {
        perror("mmap() failed. Unable to attach to the shared memory region.");
        exit(1);
    }

    return smemPtr;
}

/** print_job create_print_job(print_client*, int)
 * Create a print_job with the specified parameters.
 */
print_job create_print_job(print_client* client, int pagec, int duration) {
    print_job j;
    j.page_count = pagec;
    j.submitted_by = *client;
    j.duration = duration;

    return j;
}

/** void put_job(print_client*, shared_spooler_data*, print_job* )
 * Enqueue the given job in the spooler buffer.
 */
void put_job(print_client* client, shared_spooler_data* spooler, print_job* job) {
    printf("Waiting for server; queue is full. \n");
    enqueue_job(spooler, job);

    printf("Client %d has %d pages to print. Puts request in Buffer[%d]\n",
           client->id,
           job->page_count,
           job->position);
}

void release_shared_mem()  {
    if(munmap(spooler, BUFF_BASE_SIZE) == -1) {
        perror("munmap() failed. Unable to detatch shared memory region.");
    }

    close(fd);

}
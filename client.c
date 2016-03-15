#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "spooler.h"

const char* KEY_FILE = "keyfile";
const size_t BUFF_SIZE = sizeof(shared_spooler_data);

int setup_shared_mem();
shared_spooler_data* attach_share_mem(int fd);
print_job create_print_job(print_client*, int );
void put_job(print_client*, shared_spooler_data*, print_job* );


int main(int argc, const char* argv[]) {
    int client_id;
    int num_pages;

    if(argc < 3) {
        perror("Invalid number of argument. Usage : ./client <client_id> <pages to print>");
        exit(1);
    }

    client_id = atoi(argv[1]);
    num_pages = atoi(argv[2]);

    print_client client;
    client.id = client_id;

    int fd = setup_shared_mem();
    shared_spooler_data* spooler = attach_share_mem(fd);
    print_job job = create_print_job(&client, num_pages);
    put_job(&client, spooler, &job);

    return 0;
}

int setup_shared_mem() {
    int fd = shm_open(KEY_FILE, O_RDWR, 0666);
    if(fd == -1) {
        perror("Failed to load appropriate key for shared memory");
        exit(1);
    }

    return fd;
}

/** shared_spooler_data* attach_share_mem();
 * Attach to shared server memory through IPC and valid keyfole
 */
shared_spooler_data* attach_share_mem(int fd) {
    shared_spooler_data* smemPtr = (shared_spooler_data*) mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(smemPtr == MAP_FAILED) {
        perror("Failed to attach to the shared memory region.");
        exit(1);
    }

    return smemPtr;
}

print_job create_print_job(print_client* client, int pagec) {
    print_job j;
    j.page_count = pagec;
    j.submitted_by = *client;

    return j;
}

void put_job(print_client* client, shared_spooler_data* spooler, print_job* job) {
    printf("Waiting for server; queue is full.");
    enqueue_job(spooler, job);

    printf("Client %d has %d pages to print. Puts request in Buffer[%d]\n",
           client->id,
           job->page_count,
           job->position);
}
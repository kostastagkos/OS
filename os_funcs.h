#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/time.h>


struct timeval start, stop;
/* Union semun */
union semun {
    int val;                  /* value for SETVAL */
    struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array;    /* array for GETALL, SETALL */
};

struct SHM 		// domh koinhs mnhmhs
{
    int seg; 
    int N_C;
    char shared_mem[100];

}shm;

struct write_file{
    int segment;
    int line;
    char* wanted_line;
};

int sem_Init(int sem_id, int val);

int createSem(int a ,int b);


/* Semaphore P - down operation, using semop */
int sem_P(int sem_id);

/* Semaphore V - up operation, using semop */
int sem_V(int sem_id);
void removeSemaphores(int sem_id);


void deleteSharedMemory(int memId);

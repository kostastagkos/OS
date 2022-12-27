#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/time.h>
#include "os_funcs.h"


int sem_Init(int sem_id, int val) {
    union semun arg;

    arg.val = val;
    if (semctl(sem_id, 0, SETVAL, arg) == -1) {
        perror("# Semaphore setting value ");
        return -1;
    }
    return 0;
}

int createSem(int a ,int b){ //create and set the value in new semaphore

    /* Create a new semaphore id */
    int SemId = semget(IPC_PRIVATE, 1, IPC_CREAT | 0660);
    if (SemId == -1) {
        perror("Semaphore creation ");
        shmctl(b, IPC_RMID, (struct shmid_ds *) NULL);
        exit(EXIT_FAILURE);
    }

    /* Set the value of the semaphore */
    if (sem_Init(SemId, a) == -1) { 
        printf("error");
        exit(EXIT_FAILURE);
    }

    return SemId;
}


/* Semaphore P - down operation, using semop */
int sem_P(int sem_id) {
    struct sembuf sem_d;

    sem_d.sem_num = 0;
    sem_d.sem_op = -1;
    sem_d.sem_flg = 0;
    if (semop(sem_id, &sem_d, 1) == -1) {
        perror("# Semaphore down (P) operation ");
        return -1;
    }
    return 0;
}

/* Semaphore V - up operation, using semop */
int sem_V(int sem_id) {
    struct sembuf sem_d;

    sem_d.sem_num = 0;
    sem_d.sem_op = 1;
    sem_d.sem_flg = 0;
    if (semop(sem_id, &sem_d, 1) == -1) {
        perror("# Semaphore up (V) operation ");
        return -1;
    }
    return 0;
}

void removeSemaphores(int sem_id){  //remove semaphores from the system
  int rem = semctl(sem_id,0,IPC_RMID);
  if (rem < 0)
   {  perror ("error in removing semaphore from the system");
      exit (1);
   }
}


void deleteSharedMemory(int memId){ //delete shared memory from the system
  int err = shmctl(memId,IPC_RMID,NULL);
  if (err == -1){
    perror("Detached");
    exit(1);
  }
}

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

#define SHM_SIZE 2048
#define MAX_LINE_LENGTH 150

int main(int argc , char* argv[]){
	
	int child_status;
    int lines_of_every_segment;
    pid_t  pid;
    char fname[20];
    char* input_file = malloc(sizeof(char*) + 1);

    input_file = argv[1];
    int num_of_childs = atoi(argv[2]); // #αριθμος των παιδιων-διεργασιων που παραγουμε
    int num_of_requests = atoi(argv[3]); // #αριθμος των αιτησεων του κειμενου
    int num_of_segments = atoi(argv[4]);

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    if(num_of_requests == 0 || num_of_childs == 0){
        printf("The process has stopped because the arguments are not right\n");
        return 0;
    }

    printf("Ναme of file is:%s\n",input_file);
    printf("Νumber of childs=%d\n",num_of_childs);
    printf("Number of requests for every child=%d\n",num_of_requests);
    printf("Number of segments=%d\n",num_of_segments);

    FILE *input_file_ptr;
    FILE *child_ptr;

    input_file_ptr = fopen(input_file, "r");    // open input file
    if(input_file_ptr == NULL)
    {
        perror("Error\n");
        exit(1);
    }

    char c;
    int input_items_counter = 0;
    for (c = getc(input_file_ptr); c != EOF; c = getc(input_file_ptr))  // count the items of the file input(axis x)
    {
        if (c == '\n')
            input_items_counter = input_items_counter + 1;
    }
    if(num_of_segments > input_items_counter || num_of_segments < 0){
        printf("\nThe number of segments are more than the lines of the text file\n");
        exit(0);
    }
    rewind(input_file_ptr);     //rewind wste na mporv na to diatreksw thn epomenh fora apthn arxh
    printf("Number of lines=%d\n",input_items_counter);

    if(input_items_counter >= num_of_segments){
        lines_of_every_segment = input_items_counter / num_of_segments;
        printf("Every segment has:%d lines\n",lines_of_every_segment);
    }else{
        printf("Error:The number of segments are more than the lines of the text\n");
    }

    int shmid;
    shmid=shmget(IPC_PRIVATE,sizeof(struct SHM),0600);
    if (shmid == -1){
         perror("allocation");
    }
    // create and value 3 new semaphore
    int childSemId=createSem(0,shmid);
    int fatherSemId=createSem(0,shmid);
    int sharedMemorySemId = createSem(1,shmid);

    /* Attach the shared memory segment */
    struct SHM* ptr;
    ptr=(struct SHM*)shmat(shmid,NULL,0);

    for (int i = 0;i < num_of_childs;i++){
        pid = fork();
        
        int pid_seg[num_of_requests];
        
        if(pid == 0){
            srand(getpid());
            sprintf(fname, "output%d.txt", getpid());
            child_ptr = fopen(fname,"w");
            if(child_ptr==NULL)
            {
                printf(" Error in opening file!");
                exit(1);
            }else{
                fprintf(child_ptr,"%s","SUBMISSION TIME|ANSWER TIME|<SEGMENT,LINE>|WANTED LINE\n");
            }
            int segment = (rand()%num_of_segments)+1;        
            struct write_file doc;

            for(int j = 0;j < num_of_requests;j++){
                sem_P(sharedMemorySemId);
                printf("\nHello from Child!\n"); 
                printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid());
                clock_t begin = clock();

                int randomnumber = rand() % 10;
                if(randomnumber >= 7)
                    segment = (rand()%num_of_segments)+1;   
                int line_of_segment = (rand()%lines_of_every_segment)+1;

                pid_seg[j] = segment;
                ptr->seg = pid_seg[j];
                
                doc.segment = segment;
                doc.line = line_of_segment;
                int wanted_line;
                printf("I want from the segment %d the line %d\n",doc.segment,doc.line);
                wanted_line = ((doc.segment*lines_of_every_segment) + doc.line) - lines_of_every_segment;               
                ptr->N_C = wanted_line; 
                 
                sem_V(fatherSemId);  // unblock father to give us the line
                sem_P(childSemId); // block until father responds
                doc.wanted_line = ptr->shared_mem;
                printf("Hello from Child again!\n"); 
                printf("My parent gave me the line ----> %s",doc.wanted_line);
                clock_t answer = clock();
                usleep(200000);
                fprintf(child_ptr,"\t%lu\t\t%lu\t<%d,%d>\t-->%s\n",begin,answer,doc.segment,line_of_segment,doc.wanted_line);
                sem_V(fatherSemId);//keskleidwnw ton patera efoson teleiwsa                                                
            }
            printf("The segments for the %d child are:\n",getpid());
            for(int i = 0;i < num_of_requests;++i){
                printf("%d\t",pid_seg[i]);
            }
            printf("\n");
            fclose(child_ptr);
            exit(0);
        }
    }
    
    for(int j = 0;j < num_of_childs*num_of_requests;j++){
        sem_P(fatherSemId);   //kleidwnw ton eayto moy gia na ksekinhsei to paidi
        printf("Hello from parent!\n");
        printf("Child need line %d\n",ptr->N_C);
        int temp=0;
        // diatrexw to arxeio kai ekxwrw th swsth grammh sth shared memory 
        while ((nread = getline(&line, &len, input_file_ptr)) != -1) {
            temp++;
            if (temp == ptr->N_C){                        
                strcpy(ptr->shared_mem, line );  
            }
        }
        rewind( input_file_ptr);
        sem_V(childSemId); //   unblock the child
  
        sem_P(fatherSemId); //block myself
        sem_V(sharedMemorySemId); // unblock shared memory for childs
        
    }

    while (wait(&child_status) > 0);// Wait all chlidren to finish
    wait(&child_status); 

    fclose (input_file_ptr);
    
    // remove semaphores and shared memory//
    removeSemaphores(childSemId);
    removeSemaphores(fatherSemId);
    removeSemaphores(sharedMemorySemId);
    deleteSharedMemory(shmid);

    //free memory
    free(line);   
        
    return 0;

}
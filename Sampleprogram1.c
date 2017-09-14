#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

#define SIZE 16
//Matt Noblett
//Ben Commet
//Semaphore Lab -


typedef union semun{
    int val;
    struct semid_ds *buf;
    ushort * array;
} semun_t;


int main (int argc, char **argv) 
{ 
   int status; 
   long int i, loop, temp, *shmPtr; 
   int shmId; 
   pid_t pid;

   //Things for semaphores
   int semid;
   //Key to pass to semget()
   key_t key = 1234;
   struct sembuf op;
   semun_t arg;

   if((semid = semget(key, 1, IPC_CREAT | 0666)) == -1){
       perror("semget - semget failed\n");
       exit(1);
   }

   //initaializes the semaphore in the set to a value of 1
   arg.val = 1;
   if(semctl(semid, 0, SETVAL, arg) < 0){
       perror("semctl failed\n");
       exit(-1);
   }

      // get value of loop variable (from command-line argument)
      loop = atoi(argv[1]);
      
   if ((shmId = shmget (IPC_PRIVATE, SIZE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0) {
      perror ("i can't get no..\n"); 
      exit (1); 
   } 
   if ((shmPtr = shmat (shmId, 0, 0)) == (void*) -1) { 
      perror ("can't attach\n"); 
      exit (1); 
   }

   shmPtr[0] = 0; 
   shmPtr[1] = 1;

   //Signifies semaphore
   op.sem_num = 0;
   //Wait until there is a lock on semaphore
   op.sem_flg = 0;

   if (!(pid = fork())) { 
      for (i=0; i<loop; i++) { 

         //Operation for the semaphore
         //This reduces the semaphore count to lock
          op.sem_op = -1; 

          if(semop(semid, &op, 1) == -1){
              perror("semop failure\n");
              exit(-1);
          }
          // swap the contents of shmPtr[0] and shmPtr[1] 
          temp = shmPtr[0];
          shmPtr[0] = shmPtr[1];
          shmPtr[1] = temp;

          //release the semaphore
          op.sem_op = 1;

          if(semop(semid, &op,1) == -1){
              perror("semop failure\n");
              exit(-1);
          }

      } 
      if (shmdt (shmPtr) < 0) { 
         perror ("just can't let go\n"); 
         exit (1); 
      } 
      exit(0); 
   } 
   else 
      for (i=0; i<loop; i++) { 
          op.sem_op = -1;
          if(semop(semid, &op, 1) == -1){
              perror("semop failure\n");
              exit(-1);
            }

          //Swaps the contents of shmPtr[0] and shmPtr[1]
          temp = shmPtr[0];
          shmPtr[0] = shmPtr[1];
          shmPtr[1] = temp;

          op.sem_op = 1;
          if(semop(semid, &op,1) == -1){
              perror("Thread1: semop failure\n");
              exit(-1);
          }
      }

   wait (&status); 
   printf ("values: %li\t%li\n", shmPtr[0], shmPtr[1]);

   //Clearing the semaphore set 
   if(semctl(semid, 1, IPC_RMID) == -1){
       perror("semctl failure while clearing");
       exit(-1);
   }
   if (shmdt (shmPtr) < 0) { 
      perror ("just can't let go\n"); 
      exit (1); 
   } 
   if (shmctl (shmId, IPC_RMID, 0) < 0) { 
      perror ("can't deallocate\n"); 
      exit(1); 
   }

   return 0; 
} 

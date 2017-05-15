#include <stdio.h>

#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

#define SHARED_MEMORY_OBJECT_NAME "my_shared_memory"
#define SHARED_MEMORY_OBJECT_SIZE 50


int value;		
char *addr;
int shm;

void print_value()
{
	printf("value = %d\n", value);
}



void * thread_func( void *arg)
{
	signal (SIGUSR2, print_value);

    while(1){
    	
    	printf("I am alive\n");
        sleep(1);
	       
    }
            
}




void exit_all()
{
	printf("EXIT NOW\n");
	//close memmory
	munmap(addr, SHARED_MEMORY_OBJECT_SIZE);
	close(shm);
	shm_unlink(SHARED_MEMORY_OBJECT_NAME);
	//close memmory
	kill(0, SIGTERM);
}

int main()
{
	int processA;
	int processC;
	unsigned int number=0;
	int pipefd[2];
	
	
	int result_thread2;
    pthread_t thread2;

   
    //-------------------shared memmory init----------
   
   

	if ( (shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_CREAT|O_RDWR, 0777)) == -1 ) {
		perror("shm_open");
		return 1;
	}


    if ( ftruncate(shm, SHARED_MEMORY_OBJECT_SIZE+1) == -1 ) {
        perror("ftruncate");
        return 1;
    }


	addr = mmap(0, SHARED_MEMORY_OBJECT_SIZE+1, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
	if ( addr == (char*)-1 ) {
		perror("mmap");
		return 1;
	}

		//-------------------shared memmory init----------	
   
        
     

    //------------semaphores init -----------------
	sem_t *semaphore = NULL;
	sem_unlink ("semaphore");
	semaphore = sem_open ("semaphore", O_CREAT, 0, 0);

	sem_t *o_semaphore = NULL;
	sem_unlink ("o_semaphore");
	o_semaphore = sem_open ("o_semaphore", O_CREAT, 0, 0);
	//------------semaphores init -----------------


	pipe(pipefd); 						//create new pipe

	processA = fork();					//create child A

	if (processA != 0){					//Parent (Process B)
		signal (SIGUSR1, exit_all);		
		close(pipefd[1]); 				//Block write mode.
			char tg[10];
			while(1){

				read (pipefd[0], &number, sizeof (number));
				
				number*=number;
				sprintf(tg, "%d", number);
				memcpy(addr, tg, 10);
				if(number == 100)
				{
					printf("value = %d\n", number);
					raise(SIGUSR1);
				}
				
				sem_post (o_semaphore);
			}
	}
	else{				

		processC = fork();	//create child C

		if (processC != 0)	//---------------------child A
		{
					
				
			close(pipefd[0]); //Block read mode.
			
			while(1)
			{
			
				printf("%s\n", "Please enter a number:");
				scanf("%d", &number);
				write(pipefd[1], &number, sizeof (number));		
				sem_wait (semaphore);
			}
				
			
		
		}
		if (processC == 0) 	//---------------------child C
		{

			result_thread2 = pthread_create(&thread2, NULL, thread_func, &value);	//---Create new thread
			if (result_thread2 != 0) {
		        perror("Create new thread");
		        return 1;
	    	}
    	
	    	while(1)
	    	{
	    		sem_wait (o_semaphore);
	    		value = atoi(addr);
	    		raise(SIGUSR2);
	    		sem_post (semaphore);	
	    		
	    	}

		}
	}	
	
	return 0;
}


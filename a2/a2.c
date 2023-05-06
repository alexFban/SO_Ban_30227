#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include "a2_helper.h"

typedef struct{
	sem_t* sem1;
	sem_t* sem2;
	int id;
}TH_STRUCT;

sem_t* sem75 = NULL;
sem_t* sem75_2 = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond14 = PTHREAD_COND_INITIALIZER;

void* process5Thread(void* unused){
	TH_STRUCT* data = (TH_STRUCT*)unused;
	
	
	if(4 == data->id){
		sem_wait(data->sem2);
	}

	info(BEGIN, 5, data->id);
	
	if(3 == data->id){
		sem_post(data->sem2);
		sem_wait(data->sem1);
	}
	
	info(END, 5, data->id);
	
	if(4 == data->id){
		sem_post(data->sem1);
	}
	
	if(1 == data->id){
		sem_post(sem75_2);
	}
	
	return NULL;
}

int nrOfThreadsLeft = 47;
int nrOfCurrentThreads = 0;

void* process2Thread(void* unused){
	TH_STRUCT* data = (TH_STRUCT*)unused;
	
	sem_wait(data->sem1);
	info(BEGIN, 2, data->id);
	nrOfCurrentThreads++;
	
	
	pthread_mutex_lock(&lock);
	if(data->id == 14){
		if(nrOfThreadsLeft != 4){
			pthread_cond_wait(&cond14, &lock);
		}
	}else if(nrOfThreadsLeft == 4){
		if(nrOfCurrentThreads == 4){
			pthread_cond_signal(&cond14);
		}

		pthread_cond_wait(&cond, &lock);
	}
	nrOfThreadsLeft--;
	info(END, 2, data->id);
	nrOfCurrentThreads--;
	if(data->id == 14){
		pthread_cond_broadcast(&cond);
	}
	pthread_mutex_unlock(&lock);
	
	sem_post(data->sem1);
	
	return NULL;
}

void* process7Thread(void* unused){

	if(4 == (int)(long)unused){
		sem_wait(sem75_2);
	}

	info(BEGIN, 7, (int)(long)unused);									
	info(END, 7, (int)(long)unused);
	if(1 == (int)(long)unused){
		sem_post(sem75);
	}
	return NULL;
}

int main(){
	sem_t* semafor1 = sem_open("/a2SEM1", O_CREAT, 0644, 0);
	sem_t* semafor2 = sem_open("/a2SEM2", O_CREAT, 0644, 0);
	sem_t* semafor3 = sem_open("/a2SEM3", O_CREAT, 0644, 0);
	sem_t* semafor4 = sem_open("/a2SEM4", O_CREAT, 0644, 0);
	sem_t* semafor5 = sem_open("/a2SEM5", O_CREAT, 0644, 0);
	sem_t* semafor6 = sem_open("/a2SEM6", O_CREAT, 0644, 0);
	sem_t* semafor7 = sem_open("/a2SEM7", O_CREAT, 0644, 0);
	sem_t* semafor8 = sem_open("/a2SEM8", O_CREAT, 0644, 0);
	sem_t* semafor9 = sem_open("/a2SEM9", O_CREAT, 0644, 0);
	
	sem75 = sem_open("/a2SEM75", O_CREAT, 0644, 0);
	sem75_2 = sem_open("/a2SEM75_2", O_CREAT, 0644, 0);
	
	init();

	info(BEGIN, 1, 0);
	
	if(fork() == 0){
		if(fork() == 0){
		
		}
		else{
		    	info(BEGIN, 2, 0);
		    	sem_post(semafor2);
		    	
		    	TH_STRUCT paramsP2[47];
		    	
		    	pthread_t p2tid[47];
		    	
		    	sem_t p2sem;
		    	sem_init(&p2sem, 0, 4);
		    	for(int i=0;i<47;i++){
		    		paramsP2[i].sem1 = &p2sem;
		    		paramsP2[i].id = (i+1);
		    		pthread_create(&p2tid[i], NULL, process2Thread, &paramsP2[i]);
		    	}
		    	
		    	for(int i=0;i<47;i++){
				pthread_join(p2tid[i], NULL);
			}
		    	
		    	sem_wait(semafor3);
		    	
		    	if(fork() == 0){
		    		info(BEGIN, 4, 0);
		    		
		    		if(fork() == 0){
		    			info(BEGIN, 5, 0);
		    			
		    			pthread_t p5tid[4];
		    			
		    			TH_STRUCT params[4];
		    			
		    			sem_t psem3;
		    			sem_init(&psem3, 0, 0);
		    			
		    			sem_t psem4;
		    			sem_init(&psem4, 0, 0);
		    			
		    			sem_post(semafor6);
		    			
		    			sem_wait(sem75);
		    			
		    			for(int i=0;i<4;i++){
		    				params[i].sem1 = &psem3;
		    				params[i].sem2 = &psem4;
		    				params[i].id = (i+1);
		    				pthread_create(&p5tid[i], NULL, process5Thread, &params[i]); 
		    			}
		    			
		    			
		    			if(fork() == 0){
		    				sem_wait(semafor8);
		    				info(BEGIN, 9, 0);
		    				info(END, 9, 0);
		    				sem_post(semafor9);
		    			}else{
			    			sem_wait(semafor9);
			    			
			    			for(int i=0;i<4;i++){
			    				pthread_join(p5tid[i], NULL);
			    			}
			    			
			    			info(END, 5, 0);
			    			sem_post(semafor5);
			    		}
			    		sem_close(&psem3);
			    		sem_close(&psem4);
		    		}else{
		    			if(fork() == 0){
		    				sem_wait(semafor6);
		    				info(BEGIN, 6, 0);
		    				
		    				if(fork() == 0){
		    					info(BEGIN, 7, 0);
		    					
		    					pthread_t p7tid[5];
		    					
		    					for(int i=0;i<5;i++){
		    						pthread_create(&p7tid[i], NULL, process7Thread, (void*)(long)(i+1));
		    					}
		    					
		    					for(int i=0;i<5;i++){
		    						pthread_join(p7tid[i], NULL);
		    					}
		    					
		    					info(END, 7, 0);
		    					sem_post(semafor7);
		    					sem_post(semafor7);
		    				}else{
			    				sem_wait(semafor7);
			    				info(END, 6, 0);
			    			}
		    			}
		    			else{
		    				if(fork() == 0){
		    					sem_wait(semafor7);
		    					info(BEGIN, 8, 0);
		    					info(END, 8, 0);
		    					sem_post(semafor8);
		    				}else{
		    					sem_wait(semafor5);
		    					info(END, 4, 0);
		    					sem_post(semafor4);
		    				}
		    			}
		    		}
		    	}
		    	else{
			    	sem_wait(semafor4);
			    	info(END, 2, 0);
			    	sem_post(semafor1);
			}
			sem_close(&p2sem);
	    	}
    	}
    	else{
    		if(fork() == 0){
	    		sem_wait(semafor2);
		    	info(BEGIN, 3, 0);
		    	sem_post(semafor3);
		    	info(END, 3, 0);
	    	}
	    	else{
	    		sem_wait(semafor1);
	    		info(END, 1, 0);
	    	}
    	}
    	
    	sem_unlink("/a2SEM1");
    	sem_unlink("/a2SEM2");
    	sem_unlink("/a2SEM3");
    	sem_unlink("/a2SEM4");
    	sem_unlink("/a2SEM5");
    	sem_unlink("/a2SEM6");
    	sem_unlink("/a2SEM7");
    	sem_unlink("/a2SEM8");
    	sem_unlink("/a2SEM9");
    	sem_unlink("/a2SEM75");
    	sem_unlink("/a2SEM75_2");
    	
    	pthread_mutex_destroy(&lock);
    	pthread_cond_destroy(&cond);
    	
    return 0;
}

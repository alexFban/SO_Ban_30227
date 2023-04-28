#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include "a2_helper.h"

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
	init();

	info(BEGIN, 1, 0);
	
	if(fork() == 0){
		if(fork() == 0){
		
		}
		else{
		    	info(BEGIN, 2, 0);
		    	sem_post(semafor2);
		    	sem_wait(semafor3);
		    	if(fork() == 0){
		    		info(BEGIN, 4, 0);
		    		
		    		if(fork() == 0){
		    			info(BEGIN, 5, 0);
		    			
		    			sem_post(semafor6);
		    			
		    			if(fork() == 0){
		    				sem_wait(semafor8);
		    				info(BEGIN, 9, 0);
		    				info(END, 9, 0);
		    				sem_post(semafor9);
		    			}else{
			    			sem_wait(semafor9);
			    			info(END, 5, 0);
			    			sem_post(semafor5);
			    		}
		    		}else{
		    			if(fork() == 0){
		    				sem_wait(semafor6);
		    				info(BEGIN, 6, 0);
		    				
		    				if(fork() == 0){
		    					info(BEGIN, 7, 0);
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
    return 0;
}

#include "types.h"
#include "stat.h"
#include "x86.h"
#include "uthreads.h"
#include "user.h"

static b_sem b_semaphores[MAX_BSEM];

/***********************************************************************/
/************************ BINARY SEMAPHORE ****************************/

void bsem_init(){
	alarm(0);
	int i;
	for (i=0; i<MAX_BSEM; i++){
		b_semaphores[i].lock= UNLOCKED;
		b_semaphores[i].used = 0;
		int j;
		for (j=0; j<MAX_UTHREADS; j++)
			b_semaphores[i].waiting[j]=0;
	}
	alarm(UTHREAD_QUANTA);
}

int bsem_alloc(){
	int i;
	alarm(0);
	for (i=0; i<MAX_BSEM; i++){
		if (b_semaphores[i].used==0){
			b_semaphores[i].used=1;
			alarm(UTHREAD_QUANTA);
			return i;
		}
	}
	printf(1,"all semaphores are used\n");
	alarm(UTHREAD_QUANTA);
	return -1;
}


int bsem_free(int bs){
	alarm(0);
	int i;
	int waiting=0;
	for (i=0; i<MAX_UTHREADS; i++){
		if (b_semaphores[bs].waiting[i]==1){
			waiting=1;
			break;
		}
	}
	if (waiting){
		printf(1,"threads are blocked on the semaphore, cannot free \n");
		alarm(UTHREAD_QUANTA);
		return -1;
	}
	b_semaphores[bs].lock = UNLOCKED;
	b_semaphores[bs].used = 0;
	printf(1,"semaphore is free\n");
	alarm(UTHREAD_QUANTA);	
	return 1;
}



void bsem_down(int bs){
	alarm(0);
	int id = uthread_self();
	while (b_semaphores[bs].lock==LOCKED){
		b_semaphores[bs].waiting[id]=1;
		uthread_wait_sem(id);
		sigsend(getpid(),SIGALARM);
		alarm(0);
	}
	alarm(UTHREAD_QUANTA);
} 


void bsem_up(int bs){
	if (b_semaphores[bs].lock == UNLOCKED)
		return;
	alarm(0);
	int i;
	for (i=0; i<MAX_UTHREADS; i++){
		if(b_semaphores[bs].waiting[i]==1){
			b_semaphores[bs].waiting[i]=0;
			uthread_wakeup_sem(i);
			break;
		}
	}
	b_semaphores[bs].lock = UNLOCKED;
	alarm(UTHREAD_QUANTA);
}

/**************************************************************/
/**************************************************************/

c_sem* csem_alloc(int val){
	c_sem* sem = (c_sem*)malloc(sizeof(c_sem)+1);
	sem->value = val;
	sem->sem1 = bsem_alloc();
	if (sem->sem1 == -1){
		printf(1,"failed creating counting semaphore\n");
		free(sem);
		return 0;
	}
	sem->sem2 = bsem_alloc();
	if (sem->sem2 == -1){
		printf(1,"failed creating counting semaphore\n");
		free(sem);
		bsem_free(sem->sem1);
		return 0;
	}
	return sem;
}

void up(c_sem *sem){
	bsem_down(sem->sem1);
	sem->value++;
	if (sem->value == UNLOCKED)
		bsem_up(sem->sem2);
	bsem_up(sem->sem1);
}


void down(c_sem *sem){
	bsem_down(sem->sem2);
	bsem_down(sem->sem1);
	sem->value--;
	if (sem->value<0)
		bsem_up(sem->sem2);
	bsem_up(sem->sem1);
}

int csem_free(c_sem *sem){
	alarm(0);
	int waiting=0;
	int i;

	for (i=0; i<MAX_UTHREADS; i++){
		if (b_semaphores[sem->sem1].waiting[i]==1 || b_semaphores[sem->sem2].waiting[i]==1)
			waiting=1;
			break;
	}
	if (waiting){
		printf(1,"cannot free bin sems\n");
		alarm(UTHREAD_QUANTA);
		return -1;
	}
	bsem_free(sem->sem1);
	bsem_free(sem->sem2);
	free(sem);
	sem=0;
	alarm(UTHREAD_QUANTA);
	return 1;
}





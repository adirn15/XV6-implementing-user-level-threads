#include "types.h"
#include "stat.h"
#include "x86.h"
#include "user.h"
#include "uthreads.h"

void producer();
void consumer();
int find_empty_slot();
int find_min_item_slot();

int queue [100];
c_sem* empty=0;
c_sem* full=0;
int mutex;

int main(int argc, char** argv){

	int i;
	for (i=0;i<100;i++)
		queue[i]=-1;

	uthread_init();	
	bsem_init();
	mutex = bsem_alloc();
	empty = csem_alloc(100);
	full = csem_alloc(0);

	int t1=uthread_create(producer,0);		
	int t2=uthread_create(consumer,0);
	int t3=uthread_create(consumer,0);
	int t4=uthread_create(consumer,0);

	uthread_join(t1);
	uthread_join(t2);
	uthread_join(t3);
	uthread_join(t4);

	bsem_free(mutex);
	csem_free(full);
	csem_free(empty);

	uthread_exit();
	exit();
}

void producer(){
	int counter=1;
	while(1){
		down(empty);
		bsem_down(mutex);
		int next_place= find_empty_slot();
		queue[next_place]=counter++;
		printf(1,"producer created item %d\n",counter);
		bsem_up(mutex);
		up(full);
		if (counter==1001)
			return;
	}
} 

void consumer(){
	while(1){
		down(full);
		bsem_down(mutex);
		int index_to_consume= find_min_item_slot();
		int sleep_time=queue[index_to_consume];
		queue[index_to_consume]=-1;
		bsem_up(mutex);
		up(empty);
		if (sleep_time<=1000){
			uthread_sleep(sleep_time);
			printf(1,"thread %d  slept %d ticks \n",uthread_self(),sleep_time);
			if (sleep_time==1000)
				return;
		}
		else
			return;
	}
} 

int find_empty_slot(){
	int j;
	for(j=0;j<100;j++){
		if(queue[j]==-1)
			return j;
	}
	return -1;
}
int find_min_item_slot(){
	int j;
	int min_item=1001;
	int min_index=-1;
	for(j=0;j<100;j++){
		if(queue[j]!=-1 && queue[j]<min_item){
			min_item=queue[j];
			min_index=j;
		}
	}
	return min_index;
}
#include <stdio.h>
#include <unistd.h>

#define PAGESIZE 4096

typedef unsigned char byte;


typedef struct {
	unsigned int size;
	unsigned int info;
	unsigned char *next, *prev;
} chunkhead;



unsigned int heapSize = 0;
void *startOfHeap = NULL;  

unsigned char *mymalloc(unsigned int size){
	chunkhead *chunkiter = (chunkhead *)startOfHeap, *chunknext = NULL, *bestFit = NULL;
	unsigned char *charnext;
	int prevSize, a = (size + sizeof(chunkhead)), b = PAGESIZE - ((size + sizeof(chunkhead)) % PAGESIZE);
	
	if (b == PAGESIZE)
		b = 0;
	size = a + b;

	while(chunkiter != NULL && chunkiter->next != NULL){
		if (chunkiter->info == 0 && chunkiter->size >= size){
			if (bestFit == NULL || chunkiter->size < bestFit->size)
				bestFit = chunkiter;
		}
		chunkiter = (chunkhead *)(chunkiter->next);
	}
	if (bestFit != NULL)
		chunkiter = bestFit;
	if (heapSize == 0){
		startOfHeap = sbrk(size);
		chunkiter = startOfHeap;
		chunkiter->next = chunkiter->prev = NULL;
		chunkiter->size = size;
		chunkiter->info = 1; 
		heapSize += size;
	}else if (chunkiter->next == NULL){
		if (chunkiter->info == 1){
			chunknext = (chunkhead *)sbrk(size); //I added the pagesize so I had space to put chunknext
			chunknext->prev = (unsigned char *)chunkiter;
			chunknext->next = NULL;
			chunkiter->next = (unsigned char *)chunknext;
			chunkiter = chunknext;
		}
		else{
			if (chunkiter->size < size)
				sbrk(size - chunkiter->size);
			else if (chunkiter->size > size){
				charnext = ((unsigned char *)chunkiter) + size;
				chunknext = (chunkhead *)charnext;
				chunknext->prev = (unsigned char *)chunkiter;
				chunknext->next = chunkiter->next;
				chunknext->size = chunkiter->size - size;
				chunknext->info = 0;
				chunkiter->next = charnext;
			}	
		}
	}else{
		if (chunkiter->size > size){
			charnext = ((unsigned char *)chunkiter) + size;
			chunknext = (chunkhead *)charnext;
			chunknext->prev = (unsigned char *)chunkiter;
			chunknext->next = chunkiter->next;
			chunknext->size = chunkiter->size - size;
			chunknext->info = 0;
			chunkiter->next = charnext;
		}
	}
	chunkiter->info = 1;
	chunkiter->size = size;
	return (unsigned char *)chunkiter + sizeof(chunkhead);
}


void myfree(unsigned char *address){
	chunkhead *chunkPointer = (chunkhead *)(address - sizeof(chunkhead)),
		*theNext = ((chunkhead *)(chunkPointer->next)), 
		*thePrev = ((chunkhead *)(chunkPointer->prev));
	int currentsize =  chunkPointer->size;
	chunkPointer->info = 0;
	if (theNext != NULL && theNext->info == 0){
		chunkPointer->size += theNext->size;
		chunkPointer->next = theNext->next;
		theNext = ((chunkhead *)(theNext->next));
		if (theNext != NULL)
			theNext->prev = (unsigned char *)chunkPointer;
		currentsize = chunkPointer->size;
	}
	if (thePrev != NULL && thePrev->info == 0){
		thePrev->next = chunkPointer->next;
		if (theNext != NULL)
			theNext->prev = chunkPointer->prev;
		thePrev->size += currentsize;
	}
	if(theNext == NULL){
		heapSize -= chunkPointer->size;
		brk(chunkPointer);
	}
	if (((chunkhead *)startOfHeap)->next == NULL && ((chunkhead *)startOfHeap)->prev == NULL){
			brk(startOfHeap);
			startOfHeap = NULL;	
	}
}
void analyze(){
	printf("\n----------------------------------------\n");
	if (startOfHeap == NULL){
		printf("no heap \n");
		return;
	}
	chunkhead* ch = (chunkhead*)startOfHeap;
	for (int no=0; ch; ch = (chunkhead*)ch->next,no++){
		printf("%d | current addr: %x |", no, ch);
		printf("size: %d | " , ch->size);
		printf("info: %d | ", ch->info);
		printf("next: %x | ", ch->next);
		printf("prev: %x | ", ch->prev);
		printf("\n");
	}
	printf("program break on address: %x\n",sbrk(0));
}


int main(){
	byte *a[100]; 
	analyze();
	for(int i=0;i<100;i++)
		a[i]= mymalloc(1000); 
	for(int i=0;i<90;i++)
		myfree(a[i]);
	analyze();
	myfree(a[95]);
	a[95] = mymalloc(1000);
	analyze();
	for(int i=90;i<100;i++)
        myfree(a[i]);
    analyze();
	return 0;
}


		



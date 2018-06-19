#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>
typedef struct node
{
	SM_PageHandle data;
	PageNumber pagenum;
	int dirtybit;
	int fixedcnt;
	int hitnum;	
	int numRef;
} Data_Structure;

int bufferSize = 0, rear = 0, writecnt = 0, hit = 0, clockptr = 0,lfuptr = 0;

void fifo(BM_BufferPool *const bm, Data_Structure *newnode) //Fifo function 
{
	//printf("\nInside fifo function");	
	Data_Structure *ds=(Data_Structure *) bm->mgmtData;
	
	int i,front = rear%bufferSize;	
	for(i=0;i<bufferSize;i++)
	{
		if(ds[front].fixedcnt == 0)
		{
			if(ds[front].dirtybit == 1)
			{
				SM_FileHandle fh;
				openPageFile (bm->pageFile, &fh);
				writeBlock (ds[front].pagenum, &fh, ds[front].data);
				writecnt++;
			}
			
			ds[front].data = newnode->data;
			ds[front].pagenum = newnode->pagenum;
			ds[front].dirtybit = newnode->dirtybit;
			ds[front].fixedcnt = newnode->fixedcnt;
			break;
		}
		else
		{
			front++;
			if(front%bufferSize == 0)
				front=0;
		}
	}
}

void LFU(BM_BufferPool *const bm, Data_Structure *newnode){

	Data_Structure *ds=(Data_Structure *) bm->mgmtData;
	
	int i,
	front = lfuptr;	
	//printf("front : %d \n", front);
	int min, count = 0;
	
	for(i=0;i<bufferSize;i++)
	{
		if(ds[front].fixedcnt == 0)
		{
			front = (front + i)%bufferSize;
		//	printf("front : %d \n", front);
			min = ds[front].numRef;
		//	printf("reference count: %d \n", min);
			break;
		}
	}
	i = (front+1)%bufferSize;
	for(count = 0;count < bufferSize;count++)
	{
		if(ds[i].numRef < min)
		{
			front = i;
			min = ds[i].numRef;
		}
		i = (i+1)%bufferSize;
		
	}
		
		
	if(ds[front].dirtybit == 1)
	{
		SM_FileHandle fh;
		openPageFile (bm->pageFile, &fh);
		writeBlock (ds[front].pagenum, &fh, ds[front].data);
		writecnt++;
	}
			
	ds[front].data = newnode->data;
	ds[front].pagenum = newnode->pagenum;
	ds[front].dirtybit = newnode->dirtybit;
	ds[front].fixedcnt = newnode->fixedcnt;
	lfuptr=front+1;
	
	}


void clock(BM_BufferPool *const bm,Data_Structure *newnode)
{
	//printf("\nInside clock function");	
	Data_Structure *ds=(Data_Structure *) bm->mgmtData;
	for(;;)
	{
		if(clockptr%bufferSize == 0)
			clockptr = 0;
		if(ds[clockptr].hitnum == 0)
		{
			if(ds[clockptr].dirtybit == 1)
			{
				SM_FileHandle fh;
				openPageFile (bm->pageFile, &fh);
				writeBlock (ds[clockptr].pagenum, &fh, ds[clockptr].data);
				writecnt++;
			}			
			ds[clockptr].data = newnode->data;
			ds[clockptr].pagenum = newnode->pagenum;
			ds[clockptr].dirtybit = newnode->dirtybit;
			ds[clockptr].fixedcnt = newnode->fixedcnt;
			ds[clockptr].hitnum = newnode->hitnum;
			clockptr++;
			break;	
		}
		else
		{
			ds[clockptr].hitnum = 0;
			clockptr++;			
		}
	}
}

void lru(BM_BufferPool *const bm, Data_Structure *newnode)
{
	//printf("\nInside lru function");	
	Data_Structure *ds=(Data_Structure *) bm->mgmtData;
	
	int i,front, min;
	for(i=0;i<bufferSize;i++)
	{
		if(ds[i].fixedcnt == 0)
		{
			front= i;
			min = ds[i].hitnum;
			break;
		}
	}	
	for(i=front+1;i<bufferSize;i++)
	{
		if(ds[i].hitnum < min)
		{
			front = i;
			min = ds[i].hitnum;
		}
	}
	if(ds[front].dirtybit == 1)
	{
		SM_FileHandle fh;
		openPageFile (bm->pageFile, &fh);
		writeBlock (ds[front].pagenum, &fh, ds[front].data);
		writecnt++;
	}
	
	ds[front].data = newnode->data;
	ds[front].pagenum = newnode->pagenum;
	ds[front].dirtybit = newnode->dirtybit;
	ds[front].fixedcnt = newnode->fixedcnt;
	ds[front].hitnum = newnode->hitnum;
}



RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData)
{
	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;
	Data_Structure *ds = malloc(sizeof(Data_Structure)*numPages);
		
	bufferSize =numPages;	
	int i;
	for(i=0;i<bufferSize;i++)
	{
		ds[i].data = NULL;
		ds[i].pagenum = -1;
		ds[i].dirtybit = 0;
		ds[i].fixedcnt = 0;
		ds[i].hitnum = 0;	
		ds[i].numRef = 0;
	}
	bm->mgmtData = ds;
	writecnt = 0;
	clockptr = 0;
	lfuptr = 0;
	return RC_OK;
		
}


RC shutdownBufferPool(BM_BufferPool *const bm)
{
	Data_Structure *ds = (Data_Structure *)bm->mgmtData;
	//printf("\nshutdownBufferPool");	
	forceFlushPool(bm);	
	int i;	
	for(i=0;i<bufferSize;i++)
	{
		if(ds[i].fixedcnt != 0)
			return RC_BUFFER_CONTAINS_PINNED_PAGES;
	}
	free(ds);
	bm->mgmtData = NULL;
	return RC_OK;
}
RC forceFlushPool(BM_BufferPool *const bm)
{
	Data_Structure *ds = (Data_Structure *)bm->mgmtData;
	//printf("\nforceFlushPool");	
	int i;	
	for(i=0;i<bufferSize;i++)
	{
		if(ds[i].dirtybit == 1 && ds[i].fixedcnt == 0)
		{
			SM_FileHandle fh;
			openPageFile (bm->pageFile, &fh);
			writeBlock (ds[i].pagenum, &fh, ds[i].data);
			ds[i].dirtybit = 0;
			writecnt++;
		}
	}	
	return RC_OK;
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	Data_Structure *ds = (Data_Structure *)bm->mgmtData;
	int i;	
	for(i=0;i<bufferSize;i++)
	{
		if(ds[i].pagenum == page->pageNum)
		{
			ds[i].dirtybit = 1;
			return RC_OK;		
		}			
	}		
	return RC_ERROR;
}
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
		
	Data_Structure *ds = (Data_Structure *)bm->mgmtData;
	int i;	
	for(i=0;i<bufferSize;i++)
	{
		if(ds[i].pagenum == page->pageNum)
		{
			ds[i].fixedcnt--;
			break;		
		}		
	}
	/*
	printf("\nAfter Unpinning: ");	
	for(i=0;i<bufferSize;i++)
		printf("\nPagenum: %d fixedcnt: %d dirty: %d",ds[i].pagenum,ds[i].fixedcnt,ds[i].dirtybit);
	*/
	return RC_OK;
}
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	Data_Structure *ds = (Data_Structure *)bm->mgmtData;
	//printf("\nforcePage");	
	int i;	
	for(i=0;i<bufferSize;i++)
	{
		if(ds[i].pagenum == page->pageNum)
		{		
			SM_FileHandle fh;
			openPageFile (bm->pageFile, &fh);
			writeBlock (ds[i].pagenum, &fh, ds[i].data);
			ds[i].dirtybit = 0;
			writecnt++;
		}
	}	
	return RC_OK;
}
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)
{
	Data_Structure *ds = (Data_Structure *)bm->mgmtData;
	
	if(ds[0].pagenum == -1)
	{
		//printf("\nINSIDE ds[0]->pagenum == -1\n");
		
		SM_FileHandle fh;
		openPageFile (bm->pageFile, &fh);
		ds[0].data = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageNum,&fh);
		readBlock(pageNum, &fh, ds[0].data);
		ds[0].pagenum = pageNum;
		ds[0].fixedcnt++;
		rear = 0;
		hit = 0;
		ds[0].hitnum = hit;	
		ds[0].numRef = 0;	
		page->pageNum = pageNum;
		page->data = ds[0].data;
		
		/*	
		printf("\nPinPage function buffer");
		int i;
		for(i=0;i<bufferSize; i++)
		{
			printf("\nPagenum: %d fixedcnt: %d dirty: %d",ds[i].pagenum,ds[i].fixedcnt,ds[i].dirtybit);	
		}*/
		
		return RC_OK;		
	}
	else
	{	
		//printf("\nINSIDE front != NULL\n");
		int i,check = 0;
		
		for(i=0;i<bufferSize;i++)
		{
			if(ds[i].pagenum != -1)
			{	
				if(ds[i].pagenum == pageNum)  //if Page already in memory
				{
					ds[i].fixedcnt++;
					check = 1;
					hit++;
					if(bm->strategy == RS_LRU)	
						ds[i].hitnum = hit;
					else if(bm->strategy == RS_CLOCK)		//if bm-> strategy is RS_CLOCK storing the USED bit in hitnum.
						ds[i].hitnum = 1;
					else if(bm->strategy == RS_LFU)
						{
						ds[i].numRef++;
						//printf("Page %d referenced again \n", pageNum);
						//rear = rear + 2;
						//printf(" rear : %d \n", rear);
						}
					
					
					page->pageNum = pageNum;
					page->data = ds[i].data;

					clockptr++;
					
					break;
				}				
			}
			else	//Condition when the buffer has space to add a page
			{
				SM_FileHandle fh;
				openPageFile (bm->pageFile, &fh);
				ds[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageNum, &fh, ds[i].data);
				ds[i].pagenum = pageNum;
				ds[i].fixedcnt = 1;
				ds[i].numRef = 0;
				rear++;	
				hit++;
				if(bm->strategy == RS_LRU)
					ds[i].hitnum = hit;				
				else if(bm->strategy == RS_CLOCK)		//if bm-> strategy is RS_CLOCK storing the USED bit in hitnum.
					ds[i].hitnum = 1;
				
								
				page->pageNum = pageNum;
				page->data = ds[i].data;
				
				check = 1;
				break;
			}
		}//end of for
		
		if(check == 0)//Condition when the buffer is full and we need to use a replacement strategy.
		{
			Data_Structure *temp = (Data_Structure *)malloc(sizeof(Data_Structure));		
						
			SM_FileHandle fh;
			openPageFile (bm->pageFile, &fh);
			temp->data = (SM_PageHandle) malloc(PAGE_SIZE);
			readBlock(pageNum, &fh, temp->data);
			temp->pagenum = pageNum;
			temp->dirtybit = 0;		
			temp->fixedcnt = 1;
			temp->numRef = 0;
			rear++;
			hit++;
			//printf("HIT : %d \n", hit); //test by Rakesh
			if(bm->strategy == RS_LRU )
				temp->hitnum = hit;
			else if(bm->strategy == RS_CLOCK)			//if bm-> strategy is RS_CLOCK storing the USED bit in hitnum.
				temp->hitnum = 1;

			page->pageNum = pageNum;
			page->data = temp->data;			

			switch(bm->strategy)
			{			
				case RS_FIFO:	//printf("\n Inside FIFO switch.");
						fifo(bm,temp);
						break;
				case RS_LRU:	//printf("\n Inside LRU switch.");
						lru(bm,temp);  						
						break;
				case RS_CLOCK:	//printf("\n Inside CLOCK switch");
						clock(bm,temp);						
						break;
  				case RS_LFU:	//printf("\n Inside LFU switch");
  						LFU(bm,temp);
						break;
  				case RS_LRU_K:	printf("\n Inside LRU_K switch");
						break;
				default:
								printf("\nAlgorithm Not Implemented\n");
						break;
			}//end of switch
						
		}//end of if(check = 0)
		
		/*
		printf("\nPinPage function buffer");
		for(i=0;i<bufferSize;i++)
			printf("\nPagenum: %d fixedcnt: %d dirty: %d",ds[i].pagenum,ds[i].fixedcnt,ds[i].dirtybit);
		*/		
		return RC_OK;
	}//end of else	
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	PageNumber *x = malloc(sizeof(PageNumber)*bufferSize);
	Data_Structure *ds= (Data_Structure *)bm->mgmtData;
	int i;	
	for(i=0;i<bufferSize;i++)
		x[i] = ds[i].pagenum;
	
	/*printf("\ngetFrameContents: ");
	for(i=0;i<bufferSize;i++)
		printf("%i ",x[i]);
	*/	
	return x;
}
bool *getDirtyFlags (BM_BufferPool *const bm)
{
	bool *x = malloc(sizeof(bool)*bufferSize);
	Data_Structure *ds= (Data_Structure *)bm->mgmtData;
	int i;	
	for(i=0;i<bufferSize;i++)
	{
		if(ds[i].dirtybit == 1)		
			x[i]= true;
		else
			x[i]=false;
	}	
	return x;
}
int *getFixCounts (BM_BufferPool *const bm)
{
	int *x = malloc(sizeof(int)*bufferSize);
	Data_Structure *ds= (Data_Structure *)bm->mgmtData;
	int i;	
	for(i=0;i<bufferSize;i++)
		x[i] = ds[i].fixedcnt;
		
	return x;
}

int getNumReadIO (BM_BufferPool *const bm)
{
	return (rear+1);
	
}
int getNumWriteIO (BM_BufferPool *const bm)
{
	return writecnt;
}


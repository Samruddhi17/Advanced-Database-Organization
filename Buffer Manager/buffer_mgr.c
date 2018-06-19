#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>

typedef struct page_structure
{
	SM_PageHandle data;
	PageNumber pno;
	int fixed_count;
	int hitrate;	
	int refnum;
	int dirty_bit;
} PS;

int buffer_sz = 0, rear = 0, write_count = 0;
int hit = 0, clock_pointer = 0,lfu_pointer = 0;
void clock(BM_BufferPool *const bm,PS *node);
void LFU(BM_BufferPool *const bm, PS *node);
void lru(BM_BufferPool *const bm, PS *node);
void fifo(BM_BufferPool *const bm, PS *node);

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData)    //creating new bufferpool with numpages pageframes
{
	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;
	PS *ds = malloc(sizeof(PS)*numPages);
		
	buffer_sz =numPages;	
	int i=0;
	while(i<buffer_sz)              //initializing values for bufferpool
	{
		ds[i].data = NULL;
		ds[i].pno = -1;
		ds[i].dirty_bit = 0;
		ds[i].fixed_count = 0;
		ds[i].hitrate = 0;	
		ds[i].refnum = 0;
		i++;
	}
	bm->mgmtData = ds;
	write_count = 0;
	clock_pointer = 0;
	lfu_pointer = 0;
	return RC_OK;
		
}


RC shutdownBufferPool(BM_BufferPool *const bm)   //destroying allocated memory for pageframes
  {
	PS *ds = (PS *)bm->mgmtData;	
	forceFlushPool(bm);	
	int i=0;	
	while(i<buffer_sz)
	{
		if(ds[i].fixed_count != 0)
			return RC_BUFFER_CONTAINS_PINNED_PAGES;
		i++;
	}
	free(ds);
	bm->mgmtData = NULL;
	return RC_OK;
}
RC forceFlushPool(BM_BufferPool *const bm)  //writing dirty pages from bufferpool to disk
{
	PS *ds = (PS *)bm->mgmtData;	
	int i=0;	
	while(i<buffer_sz)
	{
		if(ds[i].fixed_count == 0 && ds[i].dirty_bit == 1)
		{
			SM_FileHandle fh;
			openPageFile (bm->pageFile, &fh);
			writeBlock (ds[i].pno, &fh, ds[i].data);
			ds[i].dirty_bit = 0;
			write_count++;
		}
		i++;
	}	
	return RC_OK;
}

int getNumReadIO (BM_BufferPool *const bm)   //Initializing pages read
{
	return (rear+1);
	
}
int getNumWriteIO (BM_BufferPool *const bm)   //Initializing pages written to pagefile
{
	return write_count;
}




RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)  //unpins the requested page
{
		
	PS *ds = (PS *)bm->mgmtData;
	int i=0;	
	while(i<buffer_sz)
	{
		if(page->pageNum == ds[i].pno)  //find requested page in buffer
		{
			ds[i].fixed_count--;
			break;		
		}
		i++;		
	}
	
	return RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)   //Identify pages by their position in pagefile
{
	PS *ds = (PS *)bm->mgmtData;
	
	if(ds[0].pno == -1)
	{
		
		SM_FileHandle fh;
		openPageFile (bm->pageFile, &fh);
		ds[0].data = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageNum,&fh);
		readBlock(pageNum, &fh, ds[0].data);
		ds[0].pno = pageNum;
		ds[0].fixed_count++;
		rear = 0;
		hit = 0;
		ds[0].hitrate = hit;	
		ds[0].refnum = 0;	
		page->pageNum = pageNum;
		page->data = ds[0].data;
		return RC_OK;		
	}
	else
	{	
		int i=0,check = 0;
		
		while(i<buffer_sz)
		{
			if(ds[i].pno != -1)
			{	
				if(ds[i].pno == pageNum)  //To check if the requested page is in memory
				{
					ds[i].fixed_count++;
					check = 1;
					hit++;
					if(bm->strategy == RS_LRU)	
						ds[i].hitrate = hit;
					else if(bm->strategy == RS_CLOCK)  //To check if the page is used then modify hitrate.
						ds[i].hitrate = 1;
					else if(bm->strategy == RS_LFU)
						{
						ds[i].refnum++;
						
						}
					
					
					page->pageNum = pageNum;
					page->data = ds[i].data;

					clock_pointer++;
					
					break;
				}				
			}
			else	//To check if the buffer has space to add a page
			{
				SM_FileHandle fh;
				openPageFile (bm->pageFile, &fh);
				ds[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageNum, &fh, ds[i].data);
				ds[i].pno = pageNum;
				ds[i].fixed_count = 1;
				ds[i].refnum = 0;
				rear++;	
				hit++;
				if(bm->strategy == RS_LRU)
					ds[i].hitrate = hit;				
				else if(bm->strategy == RS_CLOCK)	//To check if the page is used then modify hitrate.
					ds[i].hitrate = 1;
				
								
				page->pageNum = pageNum;
				page->data = ds[i].data;
				
				check = 1;
				break;
			}

			i++;
		}
		
		if(check == 0)//To Check if the buffer is full and which replacement strategy to apply
		{
			PS *temp = (PS *)malloc(sizeof(PS));		
						
			SM_FileHandle fh;
			openPageFile (bm->pageFile, &fh);
			temp->data = (SM_PageHandle) malloc(PAGE_SIZE);
			readBlock(pageNum, &fh, temp->data);
			temp->pno = pageNum;
			temp->dirty_bit = 0;		
			temp->fixed_count = 1;
			temp->refnum = 0;
			rear++;
			hit++;
			
			if(bm->strategy == RS_LRU )
				temp->hitrate = hit;
			else if(bm->strategy == RS_CLOCK)	//To check if the page is used then modify hitrate.
				temp->hitrate = 1;

			page->pageNum = pageNum;
			page->data = temp->data;			

			switch(bm->strategy)
			{			
				case RS_FIFO:	fifo(bm,temp);
						break;
				case RS_LRU:	lru(bm,temp);  						
						break;
				case RS_CLOCK:	clock(bm,temp);						
						break;
  				case RS_LFU:	LFU(bm,temp);
						break;
  						
				default:        printf("\n Bad Request\n");
						break;
			}
						
		}	
		return RC_OK;
	}
}

void fifo(BM_BufferPool *const bm, PS *node) //Implementing Fifo replacement policy
{	
	PS *ds=(PS *) bm->mgmtData;
	
	int i,front = rear%buffer_sz;	
	for(i=0;i<buffer_sz;i++)
	{
		if(ds[front].fixed_count == 0)
		{
			if(ds[front].dirty_bit == 1)
			{
				SM_FileHandle fh;
				openPageFile (bm->pageFile, &fh);
				writeBlock (ds[front].pno, &fh, ds[front].data);
				write_count++;
			}
			
			ds[front].data = node->data;
			ds[front].pno = node->pno;
			ds[front].dirty_bit = node->dirty_bit;
			ds[front].fixed_count = node->fixed_count;
			break;
		}
		else
		{
			front++;
			if(front%buffer_sz == 0)
				front=0;
		}
	}
}


void lru(BM_BufferPool *const bm, PS *node)  //Implementing LRU replacement policy
{	
	PS *ds=(PS *) bm->mgmtData;
	
	int i,front, min;
	for(i=0;i<buffer_sz;i++)
	{
		if(ds[i].fixed_count == 0)
		{
			front= i;
			min = ds[i].hitrate;
			break;
		}
	}	
	for(i=front+1;i<buffer_sz;i++)
	{
		if(ds[i].hitrate < min)
		{
			front = i;
			min = ds[i].hitrate;
		}
	}
	if(ds[front].dirty_bit == 1)
	{
		SM_FileHandle fh;
		openPageFile (bm->pageFile, &fh);
		writeBlock (ds[front].pno, &fh, ds[front].data);
		write_count++;
	}
	
	ds[front].data = node->data;
	ds[front].pno = node->pno;
	ds[front].dirty_bit = node->dirty_bit;
	ds[front].fixed_count = node->fixed_count;
	ds[front].hitrate = node->hitrate;
}





RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)  //Writing current content of page to disk
{
	PS *ds = (PS *)bm->mgmtData;	
	int i;	
	for(i=0;i<buffer_sz;i++)
	{
		if(ds[i].pno == page->pageNum)
		{		
			SM_FileHandle fh;
			openPageFile (bm->pageFile, &fh);
			writeBlock (ds[i].pno, &fh, ds[i].data);
			ds[i].dirty_bit = 0;
			write_count++;
		}
	}	
	return RC_OK;
}

PageNumber *getFrameContents (BM_BufferPool *const bm)   //To check the position where the page are stored in frame 
{
	PageNumber *x = malloc(sizeof(PageNumber)*buffer_sz);
	PS *ds= (PS *)bm->mgmtData;
	int i;	
	for(i=0;i<buffer_sz;i++)
		x[i] = ds[i].pno;
		return x;
}
bool *getDirtyFlags (BM_BufferPool *const bm)  //To check the position where page contents are changed 
{
	bool *x = malloc(sizeof(bool)*buffer_sz);
	PS *ds= (PS *)bm->mgmtData;
	int i;	
	for(i=0;i<buffer_sz;i++)
	{
		if(ds[i].dirty_bit == 1)		
			x[i]= true;
		else
			x[i]=false;
	}	
	return x;
}

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)  //mark page as dirty
{
	PS *ds = (PS *)bm->mgmtData;
	int i;	
	for(i=0;i<buffer_sz;i++)
	{
		if(ds[i].pno == page->pageNum)
		{
			ds[i].dirty_bit = 1;
			return RC_OK;		
		}			
	}		
	return RC_ERROR;
}


int *getFixCounts (BM_BufferPool *const bm)  //get the fix count from pageframe
{
	int *x = malloc(sizeof(int)*buffer_sz);
	PS *ds= (PS *)bm->mgmtData;
	int i;	
	for(i=0;i<buffer_sz;i++)
		x[i] = ds[i].fixed_count;
		
	return x;
}

void LFU(BM_BufferPool *const bm, PS *node)   //Implementing LFU replacement policy
{

	PS *ds=(PS *) bm->mgmtData;
	
	int i,
	front = lfu_pointer;	
	int min, count = 0;
	
	for(i=0;i<buffer_sz;i++)
	{
		if(ds[front].fixed_count == 0)
		{
			front = (front + i)%buffer_sz;
			min = ds[front].refnum;
				break;
		}
	}
	i = (front+1)%buffer_sz;
	for(count = 0;count < buffer_sz;count++)
	{
		if(ds[i].refnum < min)
		{
			front = i;
			min = ds[i].refnum;
		}
		i = (i+1)%buffer_sz;
		
	}
		
		
	if(ds[front].dirty_bit == 1)
	{
		SM_FileHandle fh;
		openPageFile (bm->pageFile, &fh);
		writeBlock (ds[front].pno, &fh, ds[front].data);
		write_count++;
	}
			
	ds[front].data = node->data;
	ds[front].pno = node->pno;
	ds[front].dirty_bit = node->dirty_bit;
	ds[front].fixed_count = node->fixed_count;
	lfu_pointer=front+1;
	
	}

void clock(BM_BufferPool *const bm,PS *node)   //Implementing Clock replacement policy
{
	PS *ds=(PS *) bm->mgmtData;
	for(;;)
	{
		if(clock_pointer%buffer_sz == 0)
			clock_pointer = 0;
		if(ds[clock_pointer].hitrate == 0)
		{
			if(ds[clock_pointer].dirty_bit == 1)
			{
				SM_FileHandle fh;
				openPageFile (bm->pageFile, &fh);
				writeBlock (ds[clock_pointer].pno, &fh, ds[clock_pointer].data);
				write_count++;
			}			
			ds[clock_pointer].data = node->data;
			ds[clock_pointer].pno = node->pno;
			ds[clock_pointer].dirty_bit = node->dirty_bit;
			ds[clock_pointer].fixed_count = node->fixed_count;
			ds[clock_pointer].hitrate = node->hitrate;
			clock_pointer++;
			break;	
		}
		else
		{
			ds[clock_pointer].hitrate = 0;
			clock_pointer++;			
		}
	}
}






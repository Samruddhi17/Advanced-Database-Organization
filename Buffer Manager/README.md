                                                           
			                  ****************Assignment No. 2******************	

1] Personnel Information:

      Name				  	  CWID				     Email-id
1) Shraddha Vijay Mahadik(Team Leader) 		A20385273 			smahadik1@hawk.iit.edu
2) Amruta Ashok Pimple				A20380183 			apimple@hawk.iit.edu	
3) Samruddhi Sunil Naik				A20381084 			snaik6@hawk.iit.edu
4) Sumanth Gouru	 			A20379873 			sgouru@hawk.iit.edu

2] File List:

1) buffer_mgr.c
2) buffer_mgr.h
3) buffer_mgr_stat
4) buffer_mgr_stat.h
5) storage_mgr.c
6) storage_mgr.h
7) dberror.c
8) dberror.h
9) README.md
10) makefile
11) test_assign2_1.c
12) test_assign2_2.c
13) test_helper.h


3] Installation instructions:

1) Go to project root folder(Assignment2) from terminal.
2) Give "make clean" command to delete previous executed files.
3) Give "make" command to compile project files. 
4) Give "make run" command to run "test_assign2_1.c" file.
5) Give "make test2" command to compile Custom test file "test_assign2_2.c" additional replacement strategies testcases.
6) Give "make run2" command to run "test_assign2_2.c" file. 

-------------------------------------------------------------------------------------------------------------------------


Replacement alogorithms implemented:

1) FIFO :In this startegy we replace the page that has entered first with respect to others in the bufferpool.
2) LRU  :It finds the page which is not used recently and evicts that page.
3) CLOCK:It uses a special bit which is set and reset every time a page is referred and if the page is 0 then the page can be evicted.
4) LFU  :It removes the page that is not being used frequently.
 
i. Function Name: initBufferPool
		Version:1.0.0	
		Expected arguments: BM_BufferPool *const bm, const char *const pageFileName, 
		                    const int numPages, ReplacementStrategy strategy, 
		                    void *stratData

       1) Initializes the bufferpool with the inputs given as the arguments
       2) Initializes the the frames with the default values.
       3) Returns RC_OK after initializing the values.


ii.Function Name: shutdownBufferPool
		  Version:1.0.0	
		  Expected arguments: BM_BufferPool *const bm

       1) It checks whether if any clients are still using the pages.
       2) It calls the forceflush method to write the contents to the disk before shuttingdown.
       3) If there are some users still using it sends a message that it is still in use.
       4) If there are no current users then it frees the space allocated to the bufferpool.
       5) Returns Ok upon successful completion.



iii.Function Name: forceFlushPool 
		   Version:1.0.0	
		   Expected arguments: BM_BufferPool *const bm

       1)It checks for pages for which dirtybit is set and the fix count is zero i.e it is currently not being used.
       2) If it finds any page as mentioned above it writes the contents of that page to disk.
       3)It resets the dirty bit to zero after writing the page content into disk.


iv.Function Name: markDirty
		  Version:1.0.0	
		  Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page
       
       1) It sets the dirtybit for the page which has been changed.
       2) Returns Ok upon success.


v.Function Name: unpinPage  
		 Version:1.0.0
	         Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page
       
       1) When a user is done using the page then this method is called.
       2) It updates the fix count of the given page .	
       3) Returns OK upon completion.
       


vi.Function Name: forcePage 
		  Version:1.0.0
		  Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page
       
       1) Checks the bufferpool for the given page.
       2) After finding the page it will write the changes made in the page to the disk.
       3) Resets the dirtybit for that page and updates the write count.


vii.Function Name: pinPage 
		   Version:1.0.0
	           Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page, 
	                               const PageNumber pageNum
       
       1) When a user requests a specific page it checks whether the page is in memory.
       2) If it is present then it updates the hit count, fix count.
       3) Updates the values based on the page replacement strategy used.
       4) If the page is not in memory and the buffer has space then it allocates that space to the requested page.
       
       
viii.Function Name: getFrameContents 
		    Version:1.0.0	
       	 	    Expected arguments: BM_BufferPool *const bm

	   1) It scans through the bufferpool and stores all the pages in the bufferpool into an array.
           2) Returns the array of framecontents.

ix.Function Name: getDirtyFlags 
       	          Version:1.0.0	
	          Expected arguments: BM_BufferPool *const bm
     
	   1)Gets the list of pages whose contents has been changed.
	   2)Returns the array consisting of boolean values corresponding to the pages in bufferpool.
           3)Boolean true is used to mention page is dirty and false is used to mention page is not changed.



x.Function Name: getFixCounts 
       		 Version:1.0.0	
                 Expected arguments: BM_BufferPool *const bm

	   1)Returns the array consisting of fix counts for pages corresponding to positions in bufferpool.
           


xi.Function Name: getNumReadIO 
      		  Version:1.0.0	
                  Expected arguments: BM_BufferPool *const bm

	   1) Returns number of pages read from disk from the time of initialization.
	   


xii.Function Name: getNumWriteIO 
                   Version:1.0.0	
                   Expected arguments: BM_BufferPool *const bm

	    1) Returns the number of pages written to the disk from the time of initialization.
	   

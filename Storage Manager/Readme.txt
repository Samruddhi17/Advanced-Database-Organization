
			                  ****************Assignment No. 1******************	

1] Personnel Information:

      Name				  	  CWID				     Email-id
1) Pooja Dusane					A20380124                       pdusane@hawk.iit.edu
2) Samruddhi Sunil Naik				A20381084 			snaik6@hawk.iit.edu



2] File List:

1) storage_mgr.c
2) storage_mgr.h
3) dberror.c
4) dberror.h
5) Readme.txt
6) makefile
7) test_assign1_1.c
8) test_assign1_2.c
9) test_helper.h


3] Installation instruction:

1) Go to Project root folder (Storage_Mgr) using Linux Terminal.
2) Type ls and check for makefile. (To crosscheck if you are in right directory)
3) Type "make clean" to delete old compiled .o files.
4) Type "make -f makefile" to compile all project files including "test_assign1_1.c" file 
5) Type "make run" to run "test_assign1_1.c" file.
6) Type "make test2" to compile Custom test file "test_assign1_2.c"(added by the team).
7) Type "make run2" to run "test_assign1_2.c" file.


4] Function Description :


i. Function Name: CreatePageFile
		Version:1.0.0	
		Expected arguments: char *fileName

       1) Check if the file already exists
       2) If it is present, throw an error message that the file is already present.
       3) If it is not already present, create the file and allocate size of one PAGE to it.


ii.Function Name: OpenPageFile
		  Version:1.0.0	
		  Expected arguments: char *fileName, SM_FileHandle *fHandle

       1) Check if the file with the provided file name exists.
       2) If it does not exist, throw an error.
       3) If it exists, check for the total number of pages that the file has.
       4) After opening the file, initiate the structure elements needed.



iii.Function Name: ClosePageFile
		   Version:1.0.0	
		   Expected arguments: SM_FileHandle *fHandle

       1) Close the file and return a success message upon success.
       2) If the file could not be located, return an appropriate error message.


iv.Function Name: DestroyPageFile
		  Version:1.0.0	
		  Expected arguments: char *fileName
       
       1) Check if the file is present, and remove the file.
       2) Upon success, return a success message.
       3) Upon failure, return a failure message.


v.Function Name: writeBlock 
		 Version:1.0.0
	         Expected arguments: int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage
       
       1) Check if the file is present.
       2) Sets the file pointer from the current position to the specified page number.	
       3) Write Contents to the file at the specified page number.
       4) Close the file


vi.Function Name: appendEmptyBlock
		  Version:1.0.0
		  Expected arguments: SM_FileHandle *fHandle
       
       1) Opens the file in the append mode.
       2) Check for the total number of pages and moves the pointer to end of the file.
       3) Add one page and print'\0' in the new empty block.


vii.Function Name: writeCurrentBlock
		   Version:1.0.0
	           Expected arguments: SM_FileHandle *fHandle,SM_PageHandle memPage
       
       1) Opens the file in write mode.
       2) Get the current position of the file pointer and write a block of data from that position.
       
viii.Function Name: ensureCapacity
		    Version:1.0.0	
       	 	    Expected arguments: int numberOfPages, SM_FileHandle *fHandle

	   1) Try to locate the specified file, upon failure return an appropriate error message.
          Upon success, continue.
           2) Calculate the number of pages that the file could accommodate.
	   3) If the file's memory is insufficient, calculate the memory needed to make sure that the file has enough capacity and allocate the same.
	   4) If the file's memory is sufficient, provide an appropriate message.


ix.Function Name: readBlock
       	          Version:1.0.0	
	          Expected arguments: int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage
     
	   1)Check if the file already exists with the help of file descriptor in file handler
	   2)Move the file descriptors position to the page requested in pageNum.
	   3)Read the content of length 4096 bytes and load to the memory specified in mempage


x.Function Name: getBlockPos
       		 Version:1.0.0	
                 Expected arguments: SM_FileHandle *fHandle

	   1)Check if the file already exists with the help of file descriptor in file handler
           2)Get the current page position with the help of file handler


xi.Function Name: readFirstBlock
      		  Version:1.0.0	
                  Expected arguments: SM_FileHandle *fHandle, SM_PageHandle memPage

	   1)Check if the file already exists with the help of file descriptor in file handler
	   2)Move the file descriptor in the file handler to the first page of the file
           3)Read the content to mempage.


xii.Function Name: readLastBlock
                   Version:1.0.0	
                   Expected arguments: SM_FileHandle *fHandle, SM_PageHandle memPage

	    1)Check if the file already exists with the help of file descriptor in file handler
	    2)Move the file descriptor in the file handler to the last page of the file
            3)Read the content to mempage.


xiii.Function Name: readCurrentBlock
                    Version:1.0.0	
	            Expected arguments: SM_FileHandle *fHandle, SM_PageHandle memPage

	    1)Check if the file already exists with the help of file descriptor in file handler
	    2)Read the content to mempage of the current page position in the file handler with the help of file descriptor present in the file handler.

xiv.Function Name: readPreviousBlock
                   Version:1.0.0	
	           Expected arguments: SM_FileHandle *fHandle, SM_PageHandle memPage

	   1)Check if the file already exists and trying to access non existing page in the file  with the help of file descriptor and page position  in file handler
	   2)Read the content to mempage of the previuous page position in the file handler with the help of file descriptor present in the file handler.


xv.Function Name: readNextBlock
                  Version:1.0.0	
	          Expected arguments: SM_FileHandle *fHandle, SM_PageHandle memPage

	   1)Check if the file already exists or trying to access non existing page in the file  with the help of file descriptor and page position in file handler
	   2)Read the content to mempage of the next page position in the file handler with the help of file descriptor present in the file handler.


5] Additional Error Codes:
New error code for insuffient disk space.    
	1) RC_INSUFFICIENT_MEMORY 5


6] Additional Files
We have added a new test case file.
	1) test_assign1_2.c

7] Test Cases:
We have added new test case for the following functions:
1) ReadFirstBlock
2) WriteCurrentBlock
3) WriteBlock
4) ReadLastBlock
5) ReadPreviousBlock
6) ReadCurrentBlock
7) EnsureCapacity




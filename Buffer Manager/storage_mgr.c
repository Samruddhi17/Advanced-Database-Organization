#include<stdio.h>
#include<stdlib.h>
#include "storage_mgr.h"
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

FILE *file_pointer;//Global variable which points to current page pointer
extern void initStorageManager (void){

	file_pointer=NULL;

}
//Creating a new page
extern RC createPageFile (char *fileName){
	file_pointer = fopen(fileName, "w+");//creating and opening a file in read write mode

	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND;
	else{
	

		SM_PageHandle EmptyPage = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));  //create Empty Page
		if(fwrite(EmptyPage,sizeof(char),PAGE_SIZE,file_pointer) < PAGE_SIZE)
               {  //write Empty page to a file
		printf("fwrite wrong \n");
		}else{
			printf("fwrite correct \n");
		}
		
		fclose(file_pointer);  //Closing the file
		free(EmptyPage);// deallocates the memory allocated to Empty Page
		return RC_OK;
	}
}
//Opening a file and updating the file values in file structure
extern RC openPageFile (char *fileName, SM_FileHandle *fHandle){
	file_pointer = fopen(fileName, "r+");  //open file in read write mode

	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
	else{ 
		
		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;	
	
	               //For retrieving size of file and number of pages
			struct stat file_statistics;
			if(fstat(fileno(file_pointer),&file_statistics) < 0)    
	            	return RC_ERROR;
			fHandle->totalNumPages = file_statistics.st_size/ PAGE_SIZE;
		
			fclose(file_pointer); //Closing the file
			return RC_OK;
		
	}
}
//Function to close a pagefile
extern RC closePageFile (SM_FileHandle *fHandle){
	
	if(file_pointer!=NULL)
		file_pointer=NULL;	
	return RC_OK; 
}

//Removing the file
extern RC destroyPageFile (char *fileName){
	file_pointer = fopen(fileName, "r"); //open file with the paramter file name
	
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
		
	remove(fileName); //Remove file from memory
	return RC_OK;
}

/*To Read and store the content of a Block in the pointer pointed by memPage */
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	file_pointer = fopen(fHandle->fileName, "r"); //opens the file in read mode
	if(file_pointer == NULL){
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
	}
	if (pageNum > fHandle->totalNumPages || pageNum < 0) {
	memPage = NULL;				
        fclose(file_pointer);
        return RC_READ_NON_EXISTING_PAGE; //returns an Error message if the Page Number is not in range of total number of pages i.e 0 to totalNumPages
    	}
	fseek(file_pointer, (pageNum * PAGE_SIZE), SEEK_SET);//Setting the file pointer to the start of the file
	if(fread(memPage, 1, PAGE_SIZE, file_pointer) < PAGE_SIZE)
		return RC_INSUFFICIENT_MEMORY;   //returns an Error message if the size of mempage is less than the size of the page to which file pointer points.
    	fHandle->curPagePos = ftell(file_pointer);  //update the position of the file pointer
    	fclose(file_pointer); //closing the file
	
    	return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle){
	return fHandle->curPagePos;
}

/*To Read First Block of a File*/
extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	file_pointer = fopen(fHandle->fileName, "r+");	//Open file in read write mode	
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND;	//returns an Error message if the file pointer points to Null Value
	int i;
	for(i=0;i<PAGE_SIZE; i++){
		char c = fgetc(file_pointer); // Reading one char from the file.		
		if(feof(file_pointer)){	//Break when pointer reaches end of file.
			break;
		}
		else
			memPage[i] = c;
	}
	fHandle->curPagePos = ftell(file_pointer);//Assigning current page position to the first block
	fclose(file_pointer);	//closing the file
	return RC_OK;
}

/*To Read Previous Block of a File*/
extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	if(fHandle->curPagePos <= PAGE_SIZE){	//condition to check if current position of file is on first page.
		printf("File pointer on the first block. no previous block to read.");
		return RC_READ_NON_EXISTING_PAGE;	
	}	
	else{
		int current_page = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.
		int start= (PAGE_SIZE*(current_page - 2)); // storing the  start position of previous page
		file_pointer=fopen(fHandle->fileName,"r+"); //Open file in read write mode
		if(file_pointer == NULL)
			return RC_FILE_NOT_FOUND;  //returns an Error message if the file pointer points to Null Value
		fseek(file_pointer,start,SEEK_SET);
		int i;
		for(i=0;i<PAGE_SIZE;i++){ //reading previous block character by character and storing in memPage
			memPage[i] = fgetc(file_pointer);
		}
		fHandle->curPagePos = ftell(file_pointer);// assigning current file position to curPagePos
		fclose(file_pointer); //closing the file
		return RC_OK;				
	}
}
/*To Read Current Block of a File*/
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	int current_page = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.
	int start= (PAGE_SIZE*(current_page - 1)); // storing the current page start position
	
	file_pointer=fopen(fHandle->fileName,"r+"); //Open file in read write mode
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value	
	fseek(file_pointer,start,SEEK_SET);
	int i;
	for(i=0;i<PAGE_SIZE;i++){ //reading current block character by character and storing in memPage
		char c = fgetc(file_pointer);		
		if(feof(file_pointer))
			break;
		memPage[i] = c;
	}
	fHandle->curPagePos = ftell(file_pointer);// assigning current file position to curPagePos	
	fclose(file_pointer); //closing the file
	return RC_OK;	
}
/*To Read Next Block of a File*/
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	int current_page = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.	
	if(fHandle->totalNumPages == current_page){ //condition to check if current position of file is on last page.
		printf("File pointer on the last block. No next block to read.");
		return RC_READ_NON_EXISTING_PAGE;	
	}	
	else{
		int start= (PAGE_SIZE*current_page); // storing the start position of next page 
		
		file_pointer=fopen(fHandle->fileName,"r+"); //Open file in read write mode
		if(file_pointer == NULL)
			return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
		fseek(file_pointer,start,SEEK_SET);
		int i;
		for(i=0;i<PAGE_SIZE;i++){ //reading next block character by character and storing in memPage
			char c = fgetc(file_pointer);
			if(feof(file_pointer))
				break;
			memPage[i] = c;
		}
		fHandle->curPagePos = ftell(file_pointer);// assigning current file position to curPagePos
		fclose(file_pointer); //closing the file
		return RC_OK;							
	}	
}
/*To Read Last Block of a File*/
extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	file_pointer = fopen(fHandle->fileName, "r+"); //Open file in read write mode	
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
	
	int start = (fHandle->totalNumPages - 1) * PAGE_SIZE; // storing the start position of last page 	
	fseek(file_pointer,start,SEEK_SET);
	int i;
	for(i=0;i<PAGE_SIZE; i++){ //reading block character by character and storing in memPage
		char c = fgetc(file_pointer);
		if(feof(file_pointer))
			break;
		memPage[i] = c;
	}
	fHandle->curPagePos = ftell(file_pointer);// assigning current file position to curPagePos
	fclose(file_pointer); //closing the file		
	return RC_OK;
}


/* Write a page to disk at a specific position.*/
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	file_pointer = fopen(fHandle->fileName, "r+"); // open file in read write mode.
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
	int page_pos = (pageNum)*PAGE_SIZE; // starting position of pageNum
	
	if(pageNum!=0){ //Current page position is other than First page 
		fHandle->curPagePos = page_pos;
		fclose(file_pointer);
		writeCurrentBlock(fHandle,memPage);		
	}
	else{	//write content to the first page
		fseek(file_pointer,page_pos,SEEK_SET);	
		int i;
		for(i=0;i<PAGE_SIZE;i++) 
		{
			if(feof(file_pointer)) // check for End of File
			{
				 appendEmptyBlock(fHandle); // append empty block at the end of file
			}
			fputc(memPage[i],file_pointer);// write content to file
		}
		fHandle->curPagePos = ftell(file_pointer);// assigning current file position to curPagePos 
		fclose(file_pointer); //closing the file	
	}	
	return RC_OK;
}

// write block to current Page 
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	file_pointer = fopen(fHandle->fileName, "r+");// open file in read write mode.
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND;//returns an Error message if the file pointer points to Null Value
	long int curPosition = fHandle->curPagePos; //Storing current position of file
	
	appendEmptyBlock(fHandle); //Appending an empty block to accomodate new content

	fseek(file_pointer,curPosition,SEEK_SET); //Seek to the current position.
	
	int ctr=0;
	while(fgetc(file_pointer)!= EOF) //Calculating the total number of character after the point where we need to insert new data.
		ctr++;
	fseek(file_pointer,curPosition,SEEK_SET);
	
	fseek(file_pointer,curPosition,SEEK_SET);

	fwrite(memPage,1,strlen(memPage),file_pointer);//Writing the memPage to our file.
	
	fHandle->curPagePos = ftell(file_pointer); // assigning current file position to curPagePos
	
	fclose(file_pointer); //closing the file
	return RC_OK;
}



extern RC appendEmptyBlock (SM_FileHandle *fHandle){
	SM_PageHandle EmptyPage = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char)); //creating empty page of PAGE_SIZE bytes 
	fseek(file_pointer, 0, SEEK_END);
	fwrite(EmptyPage,sizeof(char),PAGE_SIZE,file_pointer); //Writing Empty page to the file.
	free(EmptyPage); //free memory 
	fHandle->totalNumPages++; //Incrementing total number of pages.
	return RC_OK;
}


extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
	file_pointer=fopen(fHandle->fileName,"a");
	if(file_pointer==NULL)
		return RC_FILE_NOT_FOUND;
	while(numberOfPages > fHandle->totalNumPages) //add emptly pages If numberOfPages is greater than totalNumPages .
		appendEmptyBlock(fHandle);

	fclose(file_pointer);//closing the file
	return RC_OK;
}

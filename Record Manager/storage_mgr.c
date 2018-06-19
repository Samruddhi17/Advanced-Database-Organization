#include<stdio.h>
#include<stdlib.h>
#include "storage_mgr.h"
#include <math.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

FILE *file_pointer;
extern void initStorageManager (void){

	file_pointer=NULL;

}
extern RC createPageFile (char *fileName){
	file_pointer = fopen(fileName, "w+");

	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND;
	else{
	
		SM_PageHandle EmptyPage = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));  //create an Empty Page
		if(fwrite(EmptyPage,sizeof(char),PAGE_SIZE,file_pointer) < PAGE_SIZE){
	
		}
		fclose(file_pointer);  //always close file
		free(EmptyPage); // de-allocate memory
		return RC_OK;
	}
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle){
	file_pointer = fopen(fileName, "r+");  //open file in read mode

	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND;
	else{ 
		//updating the file handle
		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;	
				//get file size and number of pages
			struct stat fileStat;
			if(fstat(fileno(file_pointer),&fileStat) < 0)    
		return RC_ERROR;
			fHandle->totalNumPages = fileStat.st_size/ PAGE_SIZE;
		
			fclose(file_pointer);
			return RC_OK;
		
	}
}
extern RC closePageFile (SM_FileHandle *fHandle){
	
	if(file_pointer!=NULL)
		file_pointer=NULL;	
	return RC_OK; 
}


extern RC destroyPageFile (char *fileName){
	file_pointer = fopen(fileName, "r"); //open the given file.
	
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; 
		
	remove(fileName); //remove the file.
	return RC_OK;
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	file_pointer = fopen(fHandle->fileName, "r"); //open file in read mode
	if(file_pointer == NULL){
		return RC_FILE_NOT_FOUND; // error
	}
	if (pageNum > fHandle->totalNumPages || pageNum < 0) {
	memPage = NULL;				
        fclose(file_pointer);
        return RC_READ_NON_EXISTING_PAGE; 
    	}
	fseek(file_pointer, (pageNum * PAGE_SIZE), SEEK_SET);
	if(fread(memPage, 1, PAGE_SIZE, file_pointer) < PAGE_SIZE)
		return RC_ERROR;
    	fHandle->curPagePos = ftell(file_pointer); //update the fhandle.
    	fclose(file_pointer); 
	
    	return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle){
	return fHandle->curPagePos;
}


extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	file_pointer = fopen(fHandle->fileName, "r+");	//Open file in read mode.	
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND;	//Error if filepointer is null
	int x=0;
	while(x<PAGE_SIZE){
		char ch = fgetc(file_pointer); // Read only one char from the file.		
		if(feof(file_pointer)){	
			break;
		}
		else
			memPage[x] = ch;
			
			x++;
	}
	fHandle->curPagePos = ftell(file_pointer);// set current file position to curPagePos
	fclose(file_pointer);	//closing filepointer
	return RC_OK;
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	if(fHandle->curPagePos <= PAGE_SIZE){	//Check if current position of file is on first page.
		printf("File pointer on the first block. no previous block to read.");
		return RC_READ_NON_EXISTING_PAGE;	
	}	
	else{
		int current_pagenum = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //is page number current?
		int start_position= (PAGE_SIZE*(current_pagenum - 2)); // store position of the previous page 
		file_pointer=fopen(fHandle->fileName,"r+"); 
		if(file_pointer == NULL)
			return RC_FILE_NOT_FOUND;  //Throw error if filepointer is null	
		fseek(file_pointer,start_position,SEEK_SET);
		int x=0;
		while(x<PAGE_SIZE){ //read block and store in memory
			memPage[x] = fgetc(file_pointer);
			x++;
		}
		fHandle->curPagePos = ftell(file_pointer);// set the current file position
		fclose(file_pointer); 
		return RC_OK;				
	}
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	int current_pagenum = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Find the current Page number.
	int start_position = (PAGE_SIZE*(current_pagenum - 1)); // storing the start position.
	
	file_pointer=fopen(fHandle->fileName,"r+"); //Open file in read mode
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; //Error if null.		
	fseek(file_pointer,start_position,SEEK_SET);
	int x;
	for(x<PAGE_SIZE){ 
		char ch = fgetc(file_pointer);  // read character by character.		
		if(feof(file_pointer))
			break;
		memPage[x] = ch;
		
		x++;
	}
	fHandle->curPagePos = ftell(file_pointer);// set file's current	position.
	fclose(file_pointer); 
	return RC_OK;	
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	int curpagenum = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.	
	if(fHandle->totalNumPages == curpagenum){ //condition to check if current position of file is on last page.
		printf("File pointer on the last block. No next block to read.");
		return RC_READ_NON_EXISTING_PAGE;	
	}	
	else{
		int startpos= (PAGE_SIZE*curpagenum); // storing the next page start position
		
		file_pointer=fopen(fHandle->fileName,"r+"); //Open file in read mode
		if(file_pointer == NULL)
			return RC_FILE_NOT_FOUND; //Throw error if filepointer is null
		fseek(file_pointer,startpos,SEEK_SET);
		int i;
		for(i=0;i<PAGE_SIZE;i++){ //reading next block character by character and storing in memPage
			char c = fgetc(file_pointer);
			if(feof(file_pointer))
				break;
			memPage[i] = c;
		}
		fHandle->curPagePos = ftell(file_pointer);// set current file position to curPagePos
		fclose(file_pointer); //closing filepointer
		return RC_OK;							
	}	
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	file_pointer = fopen(fHandle->fileName, "r+"); //Open file in read mode	
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; //Throw error if filepointer is null
	
	int startpos = (fHandle->totalNumPages - 1) * PAGE_SIZE; // storing the last page start position	
	fseek(file_pointer,startpos,SEEK_SET);
	int i;
	for(i=0;i<PAGE_SIZE; i++){ //reading last block character by character and storing in memPage
		char c = fgetc(file_pointer);
		if(feof(file_pointer))
			break;
		memPage[i] = c;
	}
	fHandle->curPagePos = ftell(file_pointer);// set current file position to curPagePos
	fclose(file_pointer); //closing filepointer		
	return RC_OK;
}

/////////////////////////////////////////

//Write block to Absolute Position (pageNum)  
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	file_pointer = fopen(fHandle->fileName, "r+"); // open file in read+write mode.
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; // Throw an error file not found
	int spec_pos = (pageNum)*PAGE_SIZE; //storing starting position of pageNum
	
	if(pageNum!=0){ // Write content to a non first page.
		fHandle->curPagePos = spec_pos;
		fclose(file_pointer);
		writeCurrentBlock(fHandle,memPage);		
	}
	else{	//write content to the first page
		fseek(file_pointer,spec_pos,SEEK_SET);	
		int i;
		for(i=0;i<PAGE_SIZE;i++) 
		{
			if(feof(file_pointer)) // check file is ending in between writing
			{
				 appendEmptyBlock(fHandle); // append empty block at the end of file
			}
			fputc(memPage[i],file_pointer);// write content to file
		}
		fHandle->curPagePos = ftell(file_pointer);// set current file position to curPagePos 
		fclose(file_pointer); //closing filepointer	
	}	
	return RC_OK;
}

// write block to current Page 
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	file_pointer = fopen(fHandle->fileName, "r+");// open file in read+write mode.
	if(file_pointer == NULL)
		return RC_FILE_NOT_FOUND; // Throw an error file not found.
	long int curPosition = fHandle->curPagePos; //Storing current file position.
	
	appendEmptyBlock(fHandle); //Appending an empty block to make space for the new content.

	fseek(file_pointer,curPosition,SEEK_SET); //Seek to the current position.
	
	int ctr=0;
	while(fgetc(file_pointer)!= EOF) //Calculating the total number of character after the point where we need to insert new data.
		ctr++;
	fseek(file_pointer,curPosition,SEEK_SET);
	//char *string1 = malloc(PAGE_SIZE+1);
	//fread(string1,1,ctr-PAGE_SIZE,file_pointer); //Storing in string1 the content after the current position.
	
	fseek(file_pointer,curPosition,SEEK_SET);

	fwrite(memPage,1,strlen(memPage),file_pointer);//Writing the memPage to our file.
	//fwrite(string1,1,strlen(string1),file_pointer);//Writing the string1 to our file.
	
	fHandle->curPagePos = ftell(file_pointer); // set current file position to curPagePos
	//free(string1);	//free string memory
	fclose(file_pointer); //closing filepointer
	return RC_OK;
}

////////////////////////////////////////////////////////

extern RC appendEmptyBlock (SM_FileHandle *fHandle){
	SM_PageHandle EmptyPage = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char)); //creating empty page of PAGE_SIZE bytes 
	fseek(file_pointer, 0, SEEK_END);
	fwrite(EmptyPage,sizeof(char),PAGE_SIZE,file_pointer); //Writing Empty page to the file.
	free(EmptyPage); //free memory from EmptyPage.
	fHandle->totalNumPages++; //Increasing total number of pages.
	return RC_OK;
}


extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
	file_pointer=fopen(fHandle->fileName,"a");
	if(file_pointer==NULL)
		return RC_FILE_NOT_FOUND;
	while(numberOfPages > fHandle->totalNumPages) //If numberOfPages is greater than totalNumPages then add emptly pages.
		appendEmptyBlock(fHandle);

	fclose(file_pointer);
	return RC_OK;
}

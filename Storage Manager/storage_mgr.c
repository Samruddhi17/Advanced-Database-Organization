#include<stdio.h>
#include "dberror.h"
#include "storage_mgr.h"
#include "test_helper.h"
#include <string.h>

FILE *file_pointer;  //Global variable which points to current page pointer
extern void initStorageManager (void){

	file_pointer=NULL;

}
//Creating a new page
extern RC createPageFile(char *fileName)
{
int i=0;        //For Iteration purpose  

file_pointer=fopen(fileName,"wb"); //creating and opening a file in write mode

if(file_pointer==NULL)
{

return RC_FILE_NOT_FOUND;
}
else
{
//
while(i<PAGE_SIZE)
{
fputc(0,file_pointer);  //Writes '0' till the End of page
i++;                    //Iterating to next character.
}
fclose(file_pointer);        //Closing the file
return RC_OK;         //returning Message that file created successfully.
}
}
//Opening a file and setting the default file values in file structure
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{

file_pointer=fopen(fileName,"r"); //Opens the file in read mode
if(file_pointer==NULL)
{
return RC_FILE_NOT_FOUND;    //returns an Error message if the file pointer points to Null Value
}
else
{
//setting the default file values in file structure
fseek(file_pointer,0L,SEEK_END);
long int curPos=ftell(file_pointer);
fHandle->fileName=fileName;
fHandle->totalNumPages=curPos/PAGE_SIZE;
fHandle->curPagePos=0;
fHandle->mgmtInfo=file_pointer;
fseek(file_pointer,0L,SEEK_SET);
return RC_OK;
}
}
//Function to close a pagefile
extern RC closePageFile(SM_FileHandle *fHandle)
{
if(file_pointer==NULL) 
{
return RC_FILE_NOT_FOUND;       //returns an Error message if the file pointer points to Null Value
}

int isclosed=fclose(file_pointer);  //Closing the file pointer
//Checking whether file is closed properly or not
if(isclosed == 0)
{
      
      file_pointer=NULL;  
      return RC_OK;
}
//If file is not closed then returning an error message
else
{
return RC_FILE_NOT_FOUND;   //returns an Error message if the file pointer points to Null Value
}
}
//Removing the file
extern RC destroyPageFile(char *fileName)
{
//Removing the file from memory
int isdestroyed=remove(fileName);
//Checking if file is removed successfully
if(isdestroyed == 0)
{
return RC_OK;
}
//If file removal is not successful returning an error message
else
{
return RC_FILE_NOT_FOUND;    //returns an Error message if the file pointer points to Null Value
}
}
/*To Read and store the content of a Block in the pointer pointed by memPage */

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
file_pointer = fopen(fHandle->fileName, "r"); //opens the file in read mode
	if(file_pointer == NULL)
        {
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
	}

	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        {
        fclose(file_pointer);  
        return RC_READ_NON_EXISTING_PAGE; //returns an Error message if the Page Number is not in range of total number of pages i.e 0 to totalNumPages
    	}

	fseek(file_pointer, (pageNum * PAGE_SIZE), SEEK_SET); //Setting the file pointer to the start of the file 

	if(fread(memPage, 1, PAGE_SIZE, file_pointer) < PAGE_SIZE)
		return RC_INSUFFICIENT_MEMORY;     //returns an Error message if the size of mempage is less than the size of the page to which file pointer points.

    	fHandle->curPagePos = ftell(file_pointer);  //update the position of the file pointer

    	fclose(file_pointer);   //closing the file
	
    	return RC_OK;
}
/*To get Block position*/
extern int getBlockPos (SM_FileHandle *fHandle)
{

return fHandle->curPagePos;  //Getting the current block position
}

/*To Read First Block of a File*/
extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

file_pointer = fopen(fHandle->fileName, "r");  //opens the file in read mode
	if(file_pointer == NULL)
       {
           
		return RC_FILE_NOT_FOUND;  //returns an Error message if the file pointer points to Null Value
	}

printf("Current Page position is :%d",fHandle->curPagePos);

if(fHandle->curPagePos!=0)
{
fHandle->curPagePos=0;  //Assigning current page position to the first block
}

fseek(file_pointer,0L,SEEK_SET);  //Setting the file pointer position to the beginning of a file
//Reading first block into mempage 
if(fread(memPage, 1, PAGE_SIZE, file_pointer) < PAGE_SIZE)
return RC_INSUFFICIENT_MEMORY;     //returns an Error message if the size of mempage is less than the size of the page to which file pointer points.

return RC_OK;   

}

/*To Read Previous Block of a File*/
extern RC readPreviousBlock (SM_FileHandle *fHandle,SM_PageHandle memPage)
{

file_pointer = fopen(fHandle->fileName, "r");  //opens the file in read mode
	if(file_pointer == NULL)
        {
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
	}

printf("Current Page position is :%d",fHandle->curPagePos);

if(fHandle->curPagePos==0)
return RC_READ_NON_EXISTING_PAGE;  //If pointer is pointing to first block then there wont be previos block, therefore returns error

else
fread(memPage, fHandle->curPagePos-PAGE_SIZE,1,file_pointer); //reading the previous block into mempage from the page pointed by file pointer.
  return RC_OK;
}

/*To Read Current Block of a File*/
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

file_pointer = fopen(fHandle->fileName, "r"); //opens the file in read mode
	if(file_pointer == NULL)
        {
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
}
printf("Current Page position is :%d",fHandle->curPagePos);

fread(memPage, PAGE_SIZE,1, file_pointer);  //reading current block into mempage

   return RC_OK;


}
/*To Read Next Block of a File*/
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
file_pointer = fopen(fHandle->fileName, "r"); //opens the file in read mode
	if(file_pointer == NULL)
        {
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
	}

printf("Current Page position is :%d",fHandle->curPagePos);
if(fHandle->curPagePos==fHandle->totalNumPages)
return RC_READ_NON_EXISTING_PAGE;
else
fread(memPage, fHandle->curPagePos+PAGE_SIZE,1, file_pointer);
return RC_OK;

}
/*To Read Last Block of a File*/
extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

file_pointer = fopen(fHandle->fileName, "r");  //opens the file in read mode
	if(file_pointer == NULL)
        {
		return RC_FILE_NOT_FOUND; //returns an Error message if the file pointer points to Null Value
	}

printf("Current Page position is :%d",fHandle->curPagePos);

fseek(file_pointer,0L,SEEK_END); //setting the position of the pointer to the end of the file
long int curPos=ftell(file_pointer);
fHandle->curPagePos=curPos-PAGE_SIZE;  //setting the position of the pointer to the beginning of the last block of a file
fread(memPage,PAGE_SIZE,1, file_pointer); //Reading the last block using the file pointer to the mempage
  return RC_OK;
}
/* Write a page to disk at a specific position.*/
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	FILE *file_pointer;
	int cur_pos, current_page, actual_pos;
	file_pointer = fopen(fHandle->fileName, "wb");  //Opens the file in write mode         
	cur_pos = ftell(file_pointer);   //Gets file pointer position
        current_page = cur_pos / 4096;  //Calculating the block position 
        			
	if (current_page<pageNum)   //Current page position is less than the specified page number 
	{       actual_pos=(current_page*4096) - (4096 *(current_page + pageNum));  //Actual position of file pointer  
		fseek(file_pointer, actual_pos, SEEK_SET);    //Setting the position of file pointer to the beginning of the block
		fwrite(memPage, 1, strlen(memPage), file_pointer);  //writing into mempage from the block pointed by file pointer 
		
	}
	else if (current_page>pageNum) //Current page position is greater than the specified page number 
	{
		actual_pos=(current_page*4096) - (4096 *(current_page - pageNum));     //Actual position of file pointer  
		fseek(file_pointer, actual_pos, SEEK_SET);              //Setting the position of file pointer to the beginning of the block
		fwrite(memPage, 1, strlen(memPage), file_pointer);  //writing into mempage from the block pointed by file pointer 
		

	}
        
	else    
	{   //Current page position is at the specified page number 
		fseek(file_pointer, pageNum * 4096, SEEK_SET);  //Setting the position of file pointer to the beginning of the block       
		fwrite(memPage, 1, strlen(memPage), file_pointer); //writing into mempage from the block pointed by file pointer
		
	}

	fclose(file_pointer);  
	return RC_OK;
}
/*Write a page to disk using the current position.*/
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	FILE *file_pointer;
	int cur_pos;
      
	file_pointer = fopen(fHandle->fileName, "a");  // Opens the file in write mode                    
	if (file_pointer != NULL)
{
              
		cur_pos = ftell(file_pointer);     // Getting the position of pointer                               
		printf("In write block %d", cur_pos);
             
		int Result = fseek(file_pointer, cur_pos, SEEK_SET);    // setting pointer to the current block position                    
		if (Result == 0)
		{
      
			fwrite(memPage, 1, strlen(memPage), file_pointer); //writing into mempage from the block pointed by file pointer           
			printf("Position of Pointer %ld", ftell(file_pointer)); 

			fHandle->curPagePos = (ftell(file_pointer) / PAGE_SIZE) - 1;  //sets the current page position
			fHandle->totalNumPages = fHandle->totalNumPages + 1;     //if file pointer position exceeds total no. of pages then add new page to the file.
			fclose(file_pointer);
			return RC_OK;
		}
else
	{
		return RC_FILE_NOT_FOUND;
	}
}
	
		else
		{
			return RC_FILE_NOT_FOUND;
		}
		
}
/*Appending and empty block to the file*/

extern RC appendEmptyBlock(SM_FileHandle *fHandle)
{
	FILE *file_pointer;
        
	file_pointer = fopen(fHandle->fileName, "a");  // opens the file in append mode               
        // setting pointer to end of the file        
	int Result = fseek(file_pointer, 0, SEEK_END);  // Setting the pointer to the last block position            
	if (Result!= 0)   
	{
		return RC_FILE_NOT_FOUND; 
	}
else
	{    

		fHandle->totalNumPages++;    // Adding the new empty block by increasing the total pages		  

		fclose(file_pointer);    // closing the file

		return RC_OK;

	}
}
/*If the file has less than numberOfPages then we increase the size.*/
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{
	FILE *file_pointer;
	file_pointer = fopen(fHandle->fileName, "a");    // opening the desired file in append mode                       
	if (file_pointer!= NULL)
{                  // Checking Capacity
		if (fHandle->totalNumPages < numberOfPages) 
                { 
                   fHandle->totalNumPages = numberOfPages;  // changing the file capacity to accomodate the specified number of pages
             
		}
		fclose(file_pointer); //Closing the File
		return RC_OK;
}
	
else
{
return RC_FILE_NOT_FOUND; 
}
	
}


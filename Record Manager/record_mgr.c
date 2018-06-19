#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>


//DataStructures
typedef struct Table_Info
{
    int no_of_rows;	// total number of tuples in the table
    int first_free_page;		// first free page which has empty slots in table
	BM_PageHandle page_hdl;	// Buffer Manager PageHandle 
    BM_BufferPool buffer_mgr;	// Buffer Manager Buffer Pool
	
} Table_Info;

// Structure for Scanning tuples in Table
typedef struct Scan
{
    BM_PageHandle page_hdl;
    RID curr_id; // current row that is being scanned 
    int cnt; // no. of tuples scanned till now
    Expr *cond; // expression to be checked
    
} Scan;
int cnt = 0;
int sz = 35;
Table_Info *t;
int flag;


extern RC initRecordManager (void *mgmtData){
       initStorageManager(); // initialize Storage Manager inside Record Manager
       return RC_OK;
}
extern RC shutdownRecordManager (){
       
       return RC_OK;
}



int getFreeSlot(char *data, int recordSize)
{
	
    int i=0;
    int total_No_Slots = floor(PAGE_SIZE/recordSize); 

    while (i < total_No_Slots)
    {
    	
        if (data[i * recordSize] != '#'){
            return i;
            
            }
	i++;
    }
    return -1;
}
 
// Create Table with filename "name"
extern RC createTable (char *name, Schema *schema){
    SM_FileHandle fh;
       
    t = (Table_Info*) malloc( sizeof(Table_Info) ); // Allocate memory for table management data
    initBufferPool(&t-> buffer_mgr, name, sz, RS_LFU, NULL);
 
    char data[PAGE_SIZE];
       char * page_hdl = data;
        
       RC rc;
       int i;
       
       memset(page_hdl, 0, PAGE_SIZE);
       *(int*) page_hdl = 0; // Number of tuples
    page_hdl+= sizeof(int); //increment char pointer
       
       *(int*)page_hdl = 1; // First free page is 1 because page 0 is for schema and other info
    page_hdl += sizeof(int); //increment char pointer
       
       *(int*) page_hdl = schema->numAttr; //set num of attributes
   page_hdl += sizeof(int); 
 
    *(int*)page_hdl = schema->keySize; // set keySize of Attributes
    page_hdl += sizeof(int);
       
       for(i=0; i<schema->numAttr; i++)
    {
       
       strncpy(page_hdl, schema->attrNames[i], 10);       // set Attribute Name
       page_hdl += 10;
 
       *(int*) page_hdl = (int)schema->dataTypes[i];      // Set the Data Types of Attribute
       page_hdl += sizeof(int);
 
       *(int*) page_hdl = (int) schema->typeLength[i];    // Set the typeLength of Attribute
       page_hdl += sizeof(int);
 
    }
              
       rc = createPageFile(name);          // Create file for table using Storage Manager Function
       if(rc != RC_OK)
        return rc;
              
       rc = openPageFile(name, &fh); // open Created File
       if(rc != RC_OK)
        return rc;
              
       rc=writeBlock(0, &fh, data); // Write Schema To 0th page of file
       if(rc != RC_OK)
        return rc;
              
       rc=closePageFile(&fh);                     // Closing File after writing
       if(rc != RC_OK)
        return rc;
 
 
    return RC_OK;
}
 
 
extern RC openTable (RM_TableData *rel, char *name){
  
    SM_PageHandle page_hdl;    
    int numAttrs, i;
       
    rel->mgmtData = t;              // Set mgmtData
    rel->name = strdup(name);               // Set mgmtData 
    
       pinPage(&t->buffer_mgr, &t-> page_hdl, (PageNumber)0); // pinning the 0th page
       
       page_hdl = (char*) t-> page_hdl.data;       // set char to pointer to 0th page data
       
       t->no_of_rows= *(int*) page_hdl; // retrieve the total number of tuples from file
       //printf("Num of Tuples: %d \n", t->no_of_rows );
    page_hdl += sizeof(int);
    
       t->first_free_page= *(int*) page_hdl;     // retrieve the free page
       //printf("First free page: %d \n", t->first_free_page );
    page_hdl += sizeof(int);
       
    numAttrs = *(int*) page_hdl;            // retrieve the number of Attributes 
       page_hdl += sizeof(int);
       
       // Set all values to Schema object
       
    Schema *schema;
    schema= (Schema*) malloc(sizeof(Schema));
    
    schema->numAttr= numAttrs;
    schema->attrNames= (char**) malloc( sizeof(char*)*numAttrs);
    schema->dataTypes= (DataType*) malloc( sizeof(DataType)*numAttrs);
    schema->typeLength= (int*) malloc( sizeof(int)*numAttrs);
 
    for(i=0; i < numAttrs; i++)
       schema->attrNames[i]= (char*) malloc(10); //10 is max field length
      
    
   for(i=0; i < schema->numAttr; i++)
    {
      
       strncpy(schema->attrNames[i], page_hdl, 10);
       page_hdl += 10;
          
          schema->dataTypes[i]= *(int*) page_hdl;
       page_hdl += sizeof(int);
 
       schema->typeLength[i]= *(int*) page_hdl;
       page_hdl +=sizeof(int);
    }
       
       rel->schema = schema; // set schema object to rel object
    // Unpin after reading
    unpinPage(&t-> buffer_mgr, &t-> page_hdl);
    forcePage(&t-> buffer_mgr, &t-> page_hdl);
    return RC_OK;    
   
}   
   
 
 
extern RC closeTable (RM_TableData *rel){
       Table_Info *t;;
       t = rel->mgmtData;    // set rel->mgmtData value to t before Closing
       shutdownBufferPool(&t-> buffer_mgr); //  Shutdown Buffer Pool
       rel->mgmtData = NULL; 
       return RC_OK;
}
 
// Delete the Table  file
extern RC deleteTable (char *name){
       //free(t);
       destroyPageFile(name);       // removing  file
       return RC_OK;
}
extern int getNumTuples (RM_TableData *rel){
              
              Table_Info *t; 
       t = rel->mgmtData;
       
       return t->no_of_rows;
}




// Function to Insert new record
extern RC insertRecord (RM_TableData *rel, Record *record){
    
    Table_Info *t;
    t = rel->mgmtData;    
    
    RID *curr_id = &record->id;                          // set current record id from current Record 
    
    
    char *data;
    char *Slot_Address;
    
    int Record_Size = getRecordSize(rel->schema);          // retrieves record size of particular Record 
    
    curr_id->page = t->first_free_page;          //sets First Free page to current page
    
    pinPage(&t->buffer_mgr, &t->page_hdl, curr_id->page);    // pinning first free page 
    
    
    data = t->page_hdl.data;                // setting character pointer to current page's data
    curr_id->slot = getFreeSlot(data, Record_Size);       // Gets the first free slot of current pinned page

    while(curr_id->slot == -1){
    unpinPage(&t->buffer_mgr, &t->page_hdl);    // if pinned page does not have free slot then unpin that page
    
    curr_id->page++;                              // Increment page by 1
    pinPage(&t->buffer_mgr, &t->page_hdl, curr_id->page);  //pinning the page pointed by curr_id
    data = t->page_hdl.data;                 //setting character pointer to current page's data
    curr_id->slot = getFreeSlot(data, Record_Size);  //Gets the free slot of current pinned page
    }
    
    
    Slot_Address = data;    //Update the slot address
    
    markDirty(&t->buffer_mgr,&t->page_hdl);  //mark page dirty in buffer pool
    
    Slot_Address += curr_id->slot * Record_Size;  //Adding the address of a current record to Slot address
    *Slot_Address = '#';    
    Slot_Address++;      
    
    memcpy(Slot_Address, record->data + 1, Record_Size - 1);   //copies recordSize - 1 characters from memory area record->data + 1 to memory area slot Address.

    
    unpinPage(&t->buffer_mgr, &t->page_hdl);           //Unpin the page
    
    t->no_of_rows++;                                  // increment number of rows by 1 and update number of rows
            
    pinPage(&t->buffer_mgr, &t->page_hdl, 0);        //Pin first page 
    data = t->page_hdl.data;                // setting character pointer to First page's data 
    
        
    return RC_OK;      
}


// Function to delete a record with a certain RID

extern RC deleteRecord (RM_TableData *rel, RID id){

    Table_Info *t = rel->mgmtData;
    pinPage(&t->buffer_mgr, &t->page_hdl, id.page); // pinning page which have record that needs to be deleted
    t->no_of_rows--;              //decrement number of rows by 1 and update number of rows
    t->first_free_page = id.page; //set first free page to the page of record id passed 
    
    char * data = t->page_hdl.data; // setting character pointer to a particular record id page's data
    *(int*)data =  t->no_of_rows; // retrieve number of rows

    int Record_Size = getRecordSize(rel->schema);  // retrieve size of record
    data += id.slot * Record_Size; // setting the pointer to the particular slot of record
    
    *data = '0'; // set tombstone '0' for deleted record
        
    markDirty(&t->buffer_mgr, &t->page_hdl); // As the record is deleted we mark page as Dirty 
    unpinPage(&t->buffer_mgr, &t->page_hdl); // unpin the page after deletion

    return RC_OK;
}

// Update particular Record of Table
extern RC updateRecord (RM_TableData *rel, Record *record){
        
    Table_Info *t = rel->mgmtData;
    pinPage(&t->buffer_mgr, &t->page_hdl, record->id.page); // Pin page for the record which is to be updated
    char * data;
    RID id = record->id;      //set id to the record id which is to be updated 
    
    data = t->page_hdl.data;  
    int Record_Size = getRecordSize(rel->schema); // retrieve Record size
    data += id.slot * Record_Size;   // setting the pointer to desired slot
    
    *data = '#';      // set tombstone as '#' 
//for non-empty record
    
    data++;          // increment data pointer by 1
    
    memcpy(data,record->data + 1, Record_Size - 1 );  //  update the old record with new record
    
    markDirty(&t->buffer_mgr, &t->page_hdl);         // As the record is updated we mark page as dirty 
    unpinPage(&t->buffer_mgr, &t->page_hdl);         // unpin the page after updating the record
    
    return RC_OK;    
}

// get particular record from Table
extern RC getRecord (RM_TableData *rel, RID id, Record *record){
    Table_Info *t = rel->mgmtData;
    
    pinPage(&t->buffer_mgr, &t->page_hdl, id.page); // pinning the page of the record which is to be retrieved         
    int Record_Size = getRecordSize(rel->schema); // get the Record Size
    char * Slot_Address = t->page_hdl.data;   //get the pointer to particular record data
    Slot_Address+=id.slot*Record_Size;        // setting the pointer to the particular slot of a record
    if(*Slot_Address != '#')
        return RC_RM_RID_NOT_FOUND; // return code for not mismatched record in the table
    else{
        char *tgt = record->data; // setting pointer to the data of records
        *tgt='0';
        tgt++;
        memcpy(tgt,Slot_Address+1,Record_Size-1); //copies recordSize - 1 characters from memory area Slot_Address+1 to memory area tgt
        record->id = id;                   // set Record ID
        
        char *Slot_Address1 = record->data;
        
        char *tmp1 = Slot_Address1+1; //  pointer points to next attribute
        
        tmp1 += sizeof(int); 
        char * str = (char *)malloc(5);
        strncpy(str,tmp1 , 4);  
        str[4] = '\0';
        printf("2nd ATTRIBUTE: %s \t", str);
        tmp1 += 4;
        printf("3RD ATTRIBUTE: %d \n", *(int*)tmp1);
        free(str);
        
    }
    unpinPage(&t->buffer_mgr, &t->page_hdl); // Unpin the page after getting record
    return RC_OK;
}






extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){
	
	
		closeTable(rel);
   		openTable(rel, "test_table_r");
 
    	Scan *scmgt;
    	scmgt = (Scan*) malloc( sizeof(Scan) ); //allocate memory for scanManagement Data
    	
		scmgt->cnt= 0;         // count the n number of scans
    	scan->mgmtData = scmgt;
    	scmgt->curr_id.slot= 0;      // start scanning from the first slot
    	scmgt->curr_id.page= 1;      // start scanning from the first page
    	scmgt->cond = cond; // set the condition for scanning
		flag = 0;
    	Table_Info *tbt; 
    	tbt = rel->mgmtData;
    	tbt->no_of_rows = 10;    // set ->number of tuples
    	scan->rel= rel;
	return RC_OK;
}

extern RC next (RM_ScanHandle *scan, Record *record){
	
	static char *new_data;
	int record_size = getRecordSize(scan->rel->schema);
     int total_no_slots = floor(PAGE_SIZE/record_size);
	Scan *sc; 
	sc = (Scan*) scan->mgmtData;
    Table_Info *tbt;
    tbt = (Table_Info*) scan->rel->mgmtData;	//t;
    
     Value *rlt = (Value *) malloc(sizeof(Value));
    	if (tbt->no_of_rows == 0)
    	    return RC_RM_NO_MORE_TUPLES;
  

	while(sc->cnt <= tbt->no_of_rows )
	{  //continue until all tuples are scanned 
		if (sc->cnt <= 0)
		{
			 sc->curr_id.slot = 0;
				sc->curr_id.page = 1;
				pinPage(&tbt->buffer_mgr, &sc->page_hdl, sc->curr_id.page);
				new_data = sc->page_hdl.data;
		
		
			}
		else{
			   



			sc->curr_id.slot++;
			if(sc->curr_id.slot >= total_no_slots)
			{
			sc->curr_id.slot = 0;
			sc->curr_id.page++;

			}
			
			pinPage(&tbt->buffer_mgr, &sc->page_hdl, sc->curr_id.page);
		    new_data = sc->page_hdl.data;
		}
        
		new_data = new_data + sc->curr_id.slot * record_size;
	
		record->id.page=sc->curr_id.page;
		record->id.slot=sc->curr_id.slot;
		sc->cnt++;
		
		char *tgt = record->data;
		*tgt='0';
		tgt++;
		
		memcpy(tgt,new_data+1,record_size-1);
		      
		if (sc->cond != NULL){
			evalExpr(record, (scan->rel)->schema, sc->cond, &rlt); 
			}
		
		if(rlt->v.boolV == TRUE){  //Check for condition
		    unpinPage(&tbt->buffer_mgr, &sc->page_hdl);
		    flag = 1;
			return RC_OK; 
		}
	}
    
	    unpinPage(&tbt->buffer_mgr, &sc->page_hdl);
	    sc->curr_id.page = 1;
	    sc->cnt = 0;
        sc->curr_id.slot = 0;
		
	return RC_RM_NO_MORE_TUPLES;
       
}

/// Close Scanning 
extern RC closeScan (RM_ScanHandle *scan){
	Scan *sc= (Scan*) scan->mgmtData;
	Table_Info *ti= (Table_Info*) scan->rel->mgmtData;
	
	//if the scan is incomplete
	if(sc->cnt > 0){
	unpinPage(&ti->buffer_mgr, &sc->page_hdl); // unpin the page
	 sc->curr_id.page= 1; // reset sc to 1st page
     sc->curr_id.slot= 0; // reset sc to 1st slot
     sc->cnt = 0; // reset count to 0
	}
	
        // Free space
        
    	scan->mgmtData= NULL;
    	free(scan->mgmtData);  
	return RC_OK;
}


extern int getRecordSize (Schema *schema){
	int offset_value = 0, i = 0; // offset_value set to zero
	while(i < schema->numAttr)
	{ 
		switch(schema->dataTypes[i])
		{  // check the data types of attributes
			 case DT_INT:
				offset_value += sizeof(int); // if Integer then increment offset_value to size of INTEGER
				break;
			 case DT_BOOL:
				offset_value += sizeof(bool); // if Bool then increment offset_value to size of BOOLEAN
				break;	
			 case DT_STRING:
				offset_value += schema->typeLength[i];  // if string then increment offset_value according to its typeLength
				break;
			  case DT_FLOAT:
				offset_value += sizeof(float); // if Float then increment offset_value to size of FLOAT
				break;
		}
		i++;
	}
	
	offset_value = offset_value + 1; // add 1 at the end of string
	return offset_value;
}

// Create the schema of Table
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){
	Schema *node = (Schema *)malloc(sizeof(Schema)); // allocate memory for the schema
	node->numAttr = numAttr; // set the no. of attributes
	node->attrNames = attrNames; // set names
	node->dataTypes =dataTypes; // set data types
	node->typeLength = typeLength; // set type length
	node->keySize = keySize;  // set size of Key
	node->keyAttrs = keys; // set attribute for key
	return node; 
}
// Free schema that was created
extern RC freeSchema (Schema *schema){
	free(schema); // de-allocate the memory assigned for schema.
	return RC_OK;
}




extern RC createRecord (Record **record, Schema *schema)
{
    Record *temporary_Record ;
     int Size_of_record;
   temporary_Record = (Record*) malloc( sizeof(Record) ); // allocation of memory to temporary record
	Size_of_record = getRecordSize(schema); // finding size of record
    
	temporary_Record ->data= (char*) malloc(Size_of_record); // Assigning the memory equivalent size of record 
    char * temporary = temporary_Record ->data; // Pointing to data in the record
	*temporary = '0'; // setting to zero as record is empty
	
    temporary += sizeof(char);
	*temporary = '\0'; //setting to null value
	
    temporary_Record ->id.page= -1; // page is not assigned for record
    temporary_Record ->id.slot= -1; // slot is not assigned for empty record

    *record = temporary_Record ; // Assign temporary record to the record pointer
    return RC_OK;
}

RC attrOffset (Schema *schema, int attrNum, int *result)
{
  int offset_value = 1;
  int attribute_Position = 0;
  
  for(attribute_Position = 0; attribute_Position < attrNum; attribute_Position++)  // Looping for number of attributes
    switch (schema->dataTypes[attribute_Position]) // Check the attributes datatypes
      {
      case DT_STRING:
	offset_value+= schema->typeLength[attribute_Position]; // Incrementing the offset value by its length for the string. 
	break;
      case DT_INT:
	offset_value+= sizeof(int); // Incrementing the offset value by size of integer
	break;
      case DT_FLOAT:
	offset_value+= sizeof(float); // Incrementing the offset value by size of float
	break;
      case DT_BOOL:
	offset_value+= sizeof(bool); // Incrementing the offset value by size of boolean
	break;
      }
  
  *result = offset_value;
  return RC_OK;
}

extern RC freeRecord (Record *record){
	free(record); // Free memory of record
	return RC_OK;
}


extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){

	int offset_value = 0; 
              char *string ;
            Value *temporary_value ;
 
	attrOffset(schema, attrNum, &offset_value); // Given attribute number offset
	
    temporary_value = (Value*) malloc(sizeof(Value)); // Allocating memory for object value

	string = record->data;
	
	string += offset_value;
	if(attrNum == 1){
	schema->dataTypes[attrNum] = 1;
	}
	
	 switch(schema->dataTypes[attrNum])
    {
    case DT_INT: // Getting integer type attribute value
      {
		int val = 0;
		memcpy(&val ,string, sizeof(int));
		temporary_value->v.intV = val ;
		temporary_value->dt = DT_INT;
      }
      break;
    case DT_STRING: // getting attribute from record type string
      {
     
    temporary_value->dt = DT_STRING;
	
	int length = schema->typeLength[attrNum];
	temporary_value->v.stringV = (char *) malloc(length + 1);
	strncpy(temporary_value->v.stringV, string, length );
	temporary_value->v.stringV[length ] = '\0';
	
      }
      break;
    case DT_FLOAT: // getting float attribute value from record
      {
      temporary_value->dt = DT_FLOAT;
	  float val;
	  memcpy(&val,string, sizeof(float));
	  temporary_value->v.floatV = val;
      }
      break;
    case DT_BOOL: // getting boolean type attribute from record
      {
	  temporary_value->dt = DT_BOOL;
	  bool val;
	  memcpy(&val,string, sizeof(bool));
	  temporary_value->v.boolV = val;
      }
      break;
    default:
      	printf("There is no serializer for the datatype\n\n\n\n");
    }	
 	*value = temporary_value  ;
	return RC_OK;
}


extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
		int offset_value = 0;
                            char *info; 
		attrOffset(schema, attrNum, &offset_value); // Attribute offset value
		 info = record->data;
		info+= offset_value;
		
		switch(schema->dataTypes[attrNum])
		{
		case DT_INT: // Set the interger type attribute value
			*(int *)info= value->v.intV;	  
			info+= sizeof(int);
		  	break;
		case DT_STRING: // Set the string type attibute value
		  {
			char *buf;
                        int length;
			length = schema->typeLength[attrNum]; // length of string
			buf = (char *) malloc(length+ 1); // allocate memory to buffer
			strncpy(buf, value->v.stringV, length); // copy string to buffer
			buf[length] = '\0';
			strncpy(info, buf, length); // copy data to buffer
			free(buf); // free memory of buffer
			info+= schema->typeLength[attrNum];
		  }
		  break;
		case DT_FLOAT: // Set the float type attribute value
		  {
			*(float *)info= value->v.floatV;	// set value of attribute
			info+= sizeof(float); //Incrementing data pointer
		  }
		  break;
		case DT_BOOL: // set the boolean type attribute value
		  {
			*(bool *)info= value->v.boolV;	// Assign the   boolean value
			info += sizeof(bool);
		  }
		  break;
		default:
		  printf("There is no serializer for the datatype");
		}
				
	return RC_OK;
}





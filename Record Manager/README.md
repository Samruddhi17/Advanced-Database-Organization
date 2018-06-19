# ADO_RecordManager
ADO assignment3 RecordManager CS525
-------------------------------------------------------------------------------------------------------------------------------------------------------------

Instructions to Run the Code:

1) Go to Project root (Assignment3) using Terminal 

2) Type "make clean" to delete old compiled .o files.

3) Type "make" to compile all project files including "test_assign3_1.c" file 

4) Type "make run" to run "test_assign3_1.c" file.

5) Type "make test_expr" to compile  test file "test_expr.c".

6) Type "make run_expr" to run "run_expr.c" file. 

------------------------------------------------------------------------------------------------------------------------------------------------------------

About the Code:

We have implemented the following:

1) Record Manager all function
2) Record with the Tombstone 

Brief ideas:

1) Functions use two Structures named RM_TableMgmtData and RM_ScanMgmtData. 
	
	RM_TableMgmtData : maintain all pointer of Buffer Pool and information of total number of tuples, First free page
	RM_ScanMgmtData  : maintain pointer of buffer pool page handle and scan information like scan count, condition expression.

2) Tombstone : Tombstone is an efficient implementation for lazy deletion of tuples. We used one byte character for it. Its the first byte of every tuple.
				
					'#' :- it represents that slot has a record meaning its a non-empty slot.
					'0' :- (zero) it represents that slot is empty or that a previous value has been deleted; ready for insertion.
					

3) Example of Record Data Structure: 
			----------------------------------------------------------------------------------------------------
			| Tombstone |	Attribute-1 (INT)  |	Attribute-2 (STRING)                 |	Attribute-3 (INT)  |
			----------------------------------------------------------------------------------------------------
			|	1 byte  | 		4 bytes		   |	  Length of String (eg : 4 bytes)    |        4 bytes      |
			----------------------------------------------------------------------------------------------------

				Total Record Size : 13 byte
				
				Non-Empty Record : "#1aaaa3"
				Empty Record     : "0_ _ _ _ _ _"
				Deleted Record   : "03cccc5" 

4) Page0 in the Page File contains only information, no records. 
	It contains: Number of tuples in the table, First page with empty slots in the file, the Schema of the Table.
					
5) We used LFU as the Page replacement Algorithm, Since Page0 will be frequently used we felt its good if it remains in memory
//initBufferPool(&tableMgmt->bm, name, size, RS_LFU, NULL);

----------------------------------------------------------------------------------------------------------------------------------------

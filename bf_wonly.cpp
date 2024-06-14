//------------------------------------------------------------------------------------
// File:		bf_wonly.cpp
// Type:		C++ class implementation
// Definition File:	bf_wonly.hpp
// Author(s):		Principle original authors Debbie Kesicki, Jack Doman
//		 	Adapted to class members by Kim Batch and Tim Hoffman
// Synopsis:		Member functions of bf_write_only to open and write to bfiles

// Modified			RJS 1/31/03 - Changed a null-pointer value to string in strcmp  to avoid crash
//
// Modified			09-22-03 R Seres made changes to suppress hp warnings
// Modified			12-02-03 Removed "c_prompt()" and replaced it with "c_prompt_len()"
//							 c_prompt() used gets(), an unsafe function
// Modified:		05-15-05 R Seres added ability to read in an EDF file using the bfile class
// MEMBER TABLE:
//
// BF_WRITE_ONLY() 	Interactive class constructor. Prompts user for bfilnm. If 
//			bfilnm or bfile already exists, user is prompted for another 
//			choice. Initializes bfilnm, and fptr & fpos by calling openbfl().
// BF_WRITE_ONLY(char[]) Class constructor. Gets outfilnm from client. If outfilnm or
//			bfile exists, program aborts. Initializes bfilnm, and fptr & fpos 
//			by calling openbfl().
//			argument(s): 	R->outfilnm	bfile name string
//~BF_WRITE_ONLY()	Class destructor
// OPENBFL()		Creates bfile for write only and sets bhdr creation date and
//			time. Returns true if bfile creation true, else returns false.
// WRITE_BHDR()		Expects bhdr to be initialized. Writes bhdr to beginning of
//			file. Calls init_buf(). Returns true if write successful,
//			else returns false.
// WRITE_EPOCH(unsigned short[]) Retrieves epochful of values from client through
//			parameter variable chanvals and puts in buf. Calls bflush() to
//			write data to disk. Returns true if buf management successful, 
//			else returns false. ASSUMES WRITE_BHDR HAS BEEN CALLED!!!
//			argument(s): 	R->chanvals[]	array of channel values
// CLOSEBFL()		If buf not empty, calls bflush(). Closes bfile, returns true.
// 

// #include <unistd.h>  	// needed for prototypes on unlink call
 
#include "stdafx.h"
 
#include "bf_wonly.hpp"	// bf_write_only class definition file

//--------------------------------------------------------------------------------------
// Function name:	bf_write_only()
// Source file:		bf_wonly.cpp
// Prototype file:	bf_wonly.hpp
// Type:		C++ class constructor
// Author(s):		Debbie Kesicki (originally PROC_BFL)
// Last Modified:	5-APR-95 Kim Batch (as advised by Ken Tew) removed parameter and
//			made function completely 
//			31-Mar-95 Modified as class constructor by Tim Hoffman & K.Batch
//
// Effect on member variable(s):	bfilnm set to prompted bfile name
//					calls openbfl()
//					bhdr's creation date & time <- system date & time
//
bf_write_only::bf_write_only()
{ 
	strcpy(bfilnm,"");  // JPassios - Changed null-pointer value to string
	while ( strcmp(bfilnm, "") == 0 )// RJS - Changed null-pointer value to string 1/31/03
  	{
    		if((!c_prompt_len((char *)"\nEnter output bfilename: ",bfilnm, 80)) )
         	{ 
			sprintf(bfilnm,"%s",'\0');
	   		fprintf(stderr,"%% No Output file specified.\n");
			exit(-1);
	 	}                                                                   
       		else if (file_exists(bfilnm)) 	                             
	 	{ 
			fprintf(stderr,"%% File Already Exists - Quit(^D) or Try new name ");
	   		sprintf(bfilnm,"%c",'\0');
	 	}                                                                   
  	} 

  	if (!openbfl())  // attempt to create a new file called bfilnm[]
   	{
     		fprintf(stderr,"%% Can't create bfile %s  Aborting program\n\n",bfilnm);
     		exit(0);
   	}
   	sysdatetime(&bhdr.bfile_creation_date,&bhdr.bfile_creation_time);
   	buf = NULL;

}//END OF BF_WRITE_ONLY

//--------------------------------------------------------------------------------------
// Function name:	bf_write_only(char outfilnm[])
// Source file:		bf_wonly.cpp
// Prototype file:	bf_wonly.hpp
// Type:		C++ class constructor
// Author(s):		Debbie Kesicki (originally PROC_BFL)
// Last Modified:	5-APR-95 Kim Batch (as advised by Ken Tew) removed interactive
//			loop. Program aborts if outfilnm not valid. Also added the ability
//			to use stdout as an output file
//			31-Mar-95 Modified as class constructor by Tim Hoffman & K.Batch
//
// Effect on member variable(s):	bfilnm = outfilnm (may be stdout)
//					calls openbfl()
//					bhdr's creation date & time <- system date & time
//
// Last Modified:	15-MAY-05 Rob Seres can now export as an EDF file
bf_write_only::bf_write_only( char outfilnm[] )
{ 
 strcpy(bfilnm,outfilnm); 
 if (strcmp(bfilnm,"stdout") == 0) fptr = stdout;
 else
 {
       	if (file_exists(bfilnm)) 	                             
	{ 
		fprintf(stderr,"%% Bfile %s Already Exists - Aborting program\n\n",bfilnm);
		exit(0);
	}
	char *ptr = &outfilnm[strlen(outfilnm)-3];
	if(strcmp_nocase(ptr, "edf") != 0)
	{
		//if not an EDF file, open it like a b file
		if (!openbfl())  // attempt to create a new file called bfilnm[]
   		{
			fprintf(stderr,"%% Can't create bfile %s  Aborting program\n\n",bfilnm);
     			exit(0);
   		}
	}
	else
	{
		OpenAsEDF();
	}
 }
 sysdatetime(&bhdr.bfile_creation_date,&bhdr.bfile_creation_time);
 buf = NULL;

}//END OF BF_WRITE_ONLY

//--------------------------------------------------------------------------------------
// Function name:	~bf_write_only
// Source file:		bf_wonly.cpp
// Prototype file:	bf_wonly.hpp
// Type:		C++ class destructor
// Author(s):		Kim Batch 31-Mar-95
// Last Modified:	14-APR-95 K.Batch added delete buf
//
// Effect on member variable(s):	buf is deleted
//
//
bf_write_only::~bf_write_only()
{
	if (buf!=NULL) delete buf;
}//END OF ~BF_WRITE_ONLY

//--------------------------------------------------------------------------------------
// Function name:	openbfl()
// Source file:		bf_wonly.cpp
// Prototype file:	bf_wonly.hpp
// Type:		C++ class protected short
// Author(s):		Debbie Kesicki (Jack Doman?)(originally OPENOUTBFL)
// Last Modified:	3-APR-95 Added initialization of decomp_buf_ind
// Last Modified:	31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
// Last Modified:	15-May-05 Rob Seres initialized bIsEdfFile to false
//
// Effect on member variable(s):	(When function evaluates to TRUE)
//					Initializes fptr, pos
//
short bf_write_only::openbfl ()
{
	bIsEdfFile = false;
   fptr = fopen( bfilnm, "wb" );
   if (fptr==NULL) return(FALSE);
   // bfile has been validated: Now extract path info from file name. 
   set_bfilpath();
   return(TRUE);			
}//END OF OPENBFL ROUTINE		

// Write this file as and EDF file
short bf_write_only::OpenAsEDF()
{
	bIsEdfFile = true;
   EDFFile.OpenEDFFileForWriting(bfilnm);
   if (fptr==NULL) return(FALSE);
   // bfile has been validated: Now extract path info from file name. 
   set_bfilpath();
   return true;
}
//--------------------------------------------------------------------------------------
// Function name:	write_bhdr()
// Source file:		bf_wonly.cpp
// Prototype file:	bf_wonly.hpp
// Type:		C++ class public short
// Author(s):		Debbie Kesicki (Jack Doman?)(originally WRITEOUTHEAD)
// Last Modified:	31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
// Last Modified:	05-May-05 Rob Seres can now write the header as an EDF header
//
// Effect on member variable(s):  	(When function evaluates to true)
//					calls init_buf()
//
short bf_write_only::write_bhdr()
 {
	if(bIsEdfFile)
	{
		EDFFile.WriteEDFHeader();
	}
	else
	{
		fseek(fptr,0L,SEEK_SET);			// go to beginning of file 

#ifdef HPUX					// write header to file   
		if(write_bhead_hp(fptr,&bhdr)==FALSE)
#else
	//if(fwrite(&bhdr,B_H_SIZE,1,fptr) == FALSE)
		if(write_bhead_pc(fptr,&bhdr)==FALSE) // JPassios 
#endif
			return(FALSE);				// unable to write header

	// if the buffer has not already been initialized, initialize it
		if (buf == NULL)
		{
			buf = new bf_disk_buf(bhdr,1);
			if (buf == NULL) 
			{
				fprintf(stderr,"NO MEMORY FOR BUFFER _ ABORTING PROGRAM\n");
				exit(-1);
			}
		}	 
	}
   return(TRUE);
}//END OF WRITE_BHEADER ROUTINE

//--------------------------------------------------------------------------------------
// Function name:	write_epoch
// Source file:		bf_wonly.cpp
// Prototype file:	bf_wonly.hpp
// Type:		C++ class public short
// Author(s):		Debbie Kesicki (Jack Doman?)(originally PUT_EPOCH)
// Last Modified:	31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
// Last Modified:	05-May-05 Rob Seres can now write an epoch into an EDf File
//
// Effect on member variable(s):	(When function evaluates to true)
//					buf at decomp_buf_ind is assigned chan_cnt values
//					from chanvals
//					calls bflush() when buf is full
// 
short bf_write_only::write_epoch(unsigned short chanvals[])
 { short i; // loop variable
		   
	if(bIsEdfFile)
	{
		EDFFile.WriteEDFEpoch(chanvals);
	}
	else
	{
		if(buf->decomp_buf_ind >= buf->decomp_buf_siz)   // buffer filled		   
			if(!buf->bflush(fptr)) return(FALSE);	    // write buffer to output file 
		for (i=0; i<bhdr.chan_cnt; i++)		    // store epoch in buffer	   
			buf->buf[buf->decomp_buf_ind++] = chanvals[i];
	}
   return(TRUE);
}//END OF WRITE_EPOCH ROUTINE

//--------------------------------------------------------------------------------------
// Function name:	closebfl()
// Source file:		bf_wonly.cpp
// Prototype file:	bf_wonly.hpp
// Type:		C++ class public overloaded virtual short
// Author(s):		Debbie Kesicki (Jack Doman?)(originally BFLUSH_CLOSE)
// Last Modified:	4-APR-95 K.Batch (as advised by Ken Tew) added protection against
//			closing stdout
//			31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
// Last Modified:	05-May-05 Rob Seres if written out as an EDF file, clean up and close it
//
// Effect on member variable(s):	deletes fptr if not equal to stdout
//					calls bflush() is buf not empty
//					
//
short bf_write_only::closebfl()
{
  // If a buffer has been initialized check the buffer for leftover vals
	if(bIsEdfFile)
	{
		EDFFile.CloseEDFFile();
		EDFFile.Clean();
	}
	else
	{
		if( buf!=NULL )
		{ 
  			if( buf->decomp_buf_ind > 0 ) buf->bflush(fptr); 	
  	// calculate sample count & fix count in each channel  
			  //RJS CHANGE BECAUSE HEADER SIZE BAD!!!! 	fix_up_sample_count(calc_sample_count(get_byte_ratio()));
  	// write to disk
  			write_bhdr();
		}

		if (fptr != stdout) fclose(fptr); // close output file but not stdout    
	}
  return(TRUE);
}//END OF CLOSEBFL ROUTINE



















//------------------------------------------------------------------------------------
// File:		bf_ronly.cpp
// Type:		C++ class implementation
// Definition File:	bf_ronly.hpp
// Author(s):		Principle original authors Debbie Kesicki, Jack Doman,
//			Tim Hoffman. Adapted to class members by Kim Batch and
//			Tim Hoffman.
// Synopsis:		Member functions of bf_read_only to open and read bfiles.
//
// Modified: 	//rjs jan 21 2003 changed to use ftell and fseek for compatibility for Linux port
// Modified: 	//rjs dec 2 2003 Removed "c_prompt()" and replaced it with "c_prompt_len()"
//								c_prompt() used gets(), an unsafe function
// Modified:	//rjs may 15 2005 added ability to read in an EDF file using the bfile class

//
// MEMBER TABLE:
//
// BF_READ_ONLY()	Interactive class constructor. Prompts for bfilnm from client. 
//			If bfilnm or bfile is not valid, user will be prompted for another 
//			choice. Initializes bfilnm, and	fptr, fpos, bheader, buf, and buf 
//			indices by calling openbfl(). 
// BF_READ_ONLY(char[])	Overloaded class constructor. Gets infilnm from client through
//			parameter list. If infilnm or bfile is not valid, program will
//			abort.Initializes bfilnm, and fptr, fpos, bheader, buf, and buf 
//			indices by calling openbfl(). 
//			argument(s): 	R->infilnm	bfile name string 
//~BF_READ_ONLY()	Class destructor
// OPENBFL()		Opens bfile for read only, loads bheader into memory, calls 
//			check_bhdr, assign_cal_constants, read_epoch, and skip_epoch, etc.
//			Returns true if all calls to functions return true, else returns
//			false.
// READ_EPOCH(unsigned short[])	Returns to client an epochful of values from buf through 
//			the parameter variable chanvals. Returns true if read is successful,
//			returns false if a disk read error or decompress error. Calls
//			read_buf to get data from disk.
//			argument(s): 	W->chanvals[]	array of channel values
// SKIP_EPOCH(long)	Causes next call to read_epoch to return chanvals ticks away 
//			argument(s): 	R->ticks		# of time ticks
// SKIP_SMP(long,long) 	Calls read_epoch until the beginning of the next 
//			complete epoch.
//			arguments:	R->esize	epoch size
//					R->sampsec	samples per second
// REWIND()             Deletes bfile buffer, closes bfile, reopens bfile via openbfl()
//                      Bfile is thus rewound back to start of file
 
#include "stdafx.h"
 
#include "bf_ronly.hpp"  // bf_read_only class definition file
#include "EDFFile.h"


// ----------------------------------------------------------------------------------------
// Function name:	bf_read_only()
// Source file:		bf_ronly.cpp
// Prototype file:	bf_ronly.hpp
// Type:		C++ class constructor
// Author(s):		Debbie Kesicki (originally PROC_BFL)
// Last modified:	5-APR-95 Kim Batch (as advised by Ken Tew) removed parameter list
//			and made function completely interactive.
//			30-Mar-95 Modified as class constructor by Tim Hoffman & Kim Batch
//			09-22-03 R Seres changes to suppress hp warnings
// Effect on member variable(s):	bfilnm set to value of input bfile name 
//					calls openbfl()
//
bf_read_only::bf_read_only()
{ 
	strcpy(bfilnm,"");  // JPassios - changed NULL-pointer to string
	while (strcmp(bfilnm,"")==0)   // changed NULL-pointer to string
	{
		// prompt user for input bfile name
		if((!c_prompt_len((char*)"\nEnter input bfilename: ",bfilnm, 80))) 
	 	{ 
			strcpy(bfilnm,"");  // changed NULL-pointer to string 
	   	  	fprintf(stderr,"%% No Input file specified.\n");
			exit(-1);
	 	}
		else
			if (!openbfl()) strcpy(bfilnm,""); //changed NULL-ptr to string

	}; // end of while

}// END OF BF_READ_ONLY ROUTINE

// ----------------------------------------------------------------------------------------
// Function name:	bf_read_only(char infilnm[])
// Source file:		bf_ronly.cpp
// Prototype file:	bf_ronly.hpp
// Type:		C++ class constructor
// Author(s):		Debbie Kesicki (originally PROC_BFL)
// Last modified:	14-APR-95 K.Batch added references to a buffer object 
//			5-APR-95 Kim Batch (as advised by Ken Tew) removed interactive loop.
//			Function aborts if infilnm not valid.
//			30-Mar-95 Modified as class constructor by Tim Hoffman & Kim Batch
//
// Effect on member variable(s):	Initializes bfilnm to value of infilnm
//					calls openbfl()
//
bf_read_only::bf_read_only(char infilnm[])
{ 
// printf("\n\n -- BF_RONLY CONSTRUCTOR -- \n\n");
	buf = NULL;
	strcpy(bfilnm,infilnm);
	if (!openbfl()) exit(0); 	// Abort program if infilnm is not viable
					// Error messages in openbfl()
}// END OF BF_READ_ONLY ROUTINE

bf_read_only::bf_read_only(char infilnm[], bool &Success, char Error[])
{ 
// printf("\n\n -- BF_RONLY CONSTRUCTOR -- \n\n");
	buf = NULL;
	Success = false;
	strcpy(bfilnm,infilnm);
	LastError[0] = '\0';
	if (!openbfl())
	{
		strcpy(Error, LastError);
		return;
	}
	Success = true;
					// Error messages in openbfl()
}// END OF BF_READ_ONLY ROUTINE

//---------------------------------------------------------------------------------------
// Function name:	~bf_read_only
// Source file:		bf_ronly.cpp
// Prototype file:	bf_ronly.hpp
// Type:		C++ class destructor
// Author(s):		Kim Batch 30-Mar-95
// Last modified:	14-APR-95 K.Batch added delete buf
//
// Effect on member variable(s):	buf deleted
//
bf_read_only::~bf_read_only()
{
	if(buf)
 		delete buf;
	if(bIsEdfFile)
		EDFFile.Clean();
}// END OF ~BF_READ_ONLY ROUTINE

// ----------------------------------------------------------------------------------------
// Function name:	openbfl()
// Source file:		bf_ronly.cpp
// Prototype file:	bf_ronly.hpp
// Type:		C++ class protected overloaded virtual short	
// Author(s):		Debbie Kesicki
// Last modified:	30-Mar-95 Modified for class by Tim Hoffman & Kim Batch
// Modified:		04-26-00  C Pratt called function assign_ad_constants
//	
// Effect on member variables:	(When function evaluates to true, and compressed)
//				Initializes fptr, fpos, bhdr
// 				sets data_compressed to TRUE
//				max_decomp_buf_siz = (com_buf_siz/3)*4
//				comp_buf_siz = (buf_siz/4)*3 - adjustment for chan_cnt
//				calls read_epoch(), skip_epoch()
//
short bf_read_only::openbfl()
{
	unsigned short temp[MAX_CHANNELS_V2];
	bIsEdfFile = 0; 
	CheckFileType(bfilnm); 
	if(bIsEdfFile)
	{
		if(! OpenEdfAsBFile(bfilnm))
			return 0;
		
	}
	else
	{
		
		if ((fopen_s(&fptr,bfilnm, "rb")) != 0)
		{
			fprintf(stderr,"%% Couldn't open input bfile: %s\n",bfilnm);
			sprintf(LastError,"%% Couldn't open input bfile: %s\n",bfilnm);
			return(FALSE);
		}

#ifdef HPUX
		if(read_bhead_hp(fptr,&bhdr) == FALSE)
#else
	//if(fread(&bhdr,B_H_SIZE,1,fptr) == 0)
	 
		if(read_bhead_pc(fptr,&bhdr) == FALSE) // JPassios
#endif
		{ 
			fprintf(stderr,"I/O ERROR - Unable to read header\n");
			sprintf(LastError,"I/O ERROR - Unable to read header\n");
			closebfl();					
			return(FALSE);
		}
	}

  if(!check_bhdr()) 
  {
	  return(FALSE);
  }
  assign_cal_constants();
  assign_ad_constants();

	if(!bIsEdfFile)
	{
		buf = new bf_disk_buf(bhdr);
		if (buf == NULL) 
		{
			fprintf(stderr,"NO MEMORY FOR BUFFER _ ABORTING PROGRAM\n");
			sprintf(LastError,"NO MEMORY FOR BUFFER _ ABORTING PROGRAM\n");
		 
		}
	}
	else
		buf = NULL;
  if (read_epoch(temp) == FALSE)
	{
	fprintf(stderr,"I/O ERROR - Unable to read data\n");
	sprintf(LastError,"I/O ERROR - Unable to read data\n");
	closebfl();				
	return(FALSE);
	}
 
  if (skip_epoch(-1) == FALSE)
	{
	fprintf(stderr,"I/O ERROR - Unable to get start of data\n");
	sprintf(LastError,"I/O ERROR - Unable to get start of data\n");
	closebfl();				
	return (FALSE);
	}
 
  if(bIsEdfFile)
  {//skip seconds of an EDF so we start at an even minute
	  
	  char *cptr = &EDFFile.EdfHeader.starttime[6];//get the seconds from the start time in an HH.MM.SS string
	  short SkippedSeconds = (60 - atoi(cptr)) % 60;
 
	  if(SkippedSeconds != 0)
	  {
		  unsigned short junk[30];
		  for(int index = 0; index < (SkippedSeconds * EdfRecordSize )/ EDFFile.EdfHeader.duration; index++)
			read_epoch(junk);
				//read_epoch((SkippedSeconds * EdfRecordSize )/ EDFFile.EdfHeader.duration);	
		  increment_date_time_by_sec( &bhdr.date, &bhdr.time, SkippedSeconds );
	  }	  
	  
  }
 
  // bfile has been validated: Now extract path info from file name and initialize 
  // value of bsf_ro_rw::bf_compf_factor member var.
  set_bfilpath();
  if (bhdr.output_compr_alg == 1) // compression of output bfile on disk - NOT input signal rate 
    bf_compr_factor = 4;          // this # is only the numerator of either 4/3        
  else
    bf_compr_factor = 3;          // or 3/3   (see bf_compr_factor in bf_ronly.hpp)
  return(TRUE);
}// END OF OPENBFL ROUTINE 


// ---------------------------------------------------------------------------------------
// Function name:	read_epoch()
// Source file:		bf_ronly.cpp
// Prototype file:	bf_ronly.hpp
// Type:		C++ class public short	
// Author(s):		Debbie Kesicki (originally DISKDAT)
// Last modified:	30-Mar-95 Modified for class Kim Batch and Tim Hoffman
//	
// Effect on member variables:	(When function evaluates to true)
//				assigns chanvals chan_cnt values from buf at decomp_buf_ind
//				calls read_buf() when buf is empty
//
#define MAX_VALUE 4095
short bf_read_only::read_epoch(unsigned short chanvals[])
{


short i;
 
	if(bIsEdfFile)	
	{
//if an EDF, read the info into the EDF structure
		if(IndexIntoRecord >= EdfRecordSize)
		{
			IndexIntoRecord = 0;
			if(!EDFFile.ReadEDFSample())
				return 0;
		}
		if(bhdr.device_id == 0 || bhdr.device_id == 2)
		{
			//note: we use the constants for the ADZERO here, rather than ADZERO itself. 
			//the reason is that bfiles DO NOT rely on ADZERO to read in their data, 
			//but EDF does. Other programs, such as DELREM, change the ADZERO values
			//during processing, and that alters the wav data in a bad way. 
			for(i = 0; i < bhdr.chan_cnt; i++)
			{
				chanvals[i] =  (unsigned short)(  *(DataPtrs[i] + IndexIntoRecord) + ADZERO_12); 			
			}
		}
		else
		{
			for(i = 0; i < bhdr.chan_cnt; i++)
			{
				chanvals[i] =  (unsigned short)(  *(DataPtrs[i] + IndexIntoRecord) + ADZERO_16); 			
			}
		}
		IndexIntoRecord++;
	}
	else
 
	{
		// printf("INSIDE BF_READ_ONLY::READ_EPOCH\n"); // DBG
		if(buf->decomp_buf_ind >= buf->decomp_buf_siz)
			if(buf->read_buf(fptr,&fpos) == FALSE) return (FALSE);
		// printf("ASSIGNING CHANVALS LOOP\n"); // DBG
		for (i = 0; i<bhdr.chan_cnt; i++)
			chanvals[i] = buf->buf[buf->decomp_buf_ind++];
		// printf("LEAVING BF_READ_ONLY::READ_EPOCH. Decompressed Buf Index = %ld\n",buf->decomp_buf_ind); // DBG
	}
	return(TRUE);
}// END OF READ_EPOCH ROUTINE

// -------------------------------------------------------------------------------------
// Function name:	skip_epoch()
// Source file:		bf_ronly.cpp
// Prototype file:	bf_ronly.hpp
// Type:		C++ class public short
// Author(s):		Jack Doman (Originally SKIPDAT)
// Last modified:	30-Mar-95 Modified for class by Kim Batch and Tim Hoffman
//
// Effect on member variables:	(When function evaluates to true)
//				ticks = value of parameter iinput
//				decomp_buf_ind = index ticks away from previous index
//				decomp_buf_siz = siz of buf where index is located
//				if ticks away index not in current buf calls read_buf() 
//	
short bf_read_only::skip_epoch( long ticks )
{
	//rjs jan 21 2003 changed this to use ftell and fseek to avoid using fpos_t structures that arn't defined
	//well in our version of linux
        long    new_tick;       // adjusted input file position in ticsk 
        long    new_buf;        // number of buffers to seek from current
        long    ticksthisbuf;   // number of ticks in current buffer
        unsigned short loc_bx;  // current bx value in case we must reset
        long   loc_fpos;      // current file positn in case we must reset
// ------------------------------------------------------------------------------------- 

        if (ticks == 0) return(TRUE);           // ticks == 0  =>  noop 
 
		if(bIsEdfFile)
		{
			//edf version of skip epoch

			while(ticks)
			{
				if(ticks < EdfRecordSize - IndexIntoRecord)
				{
					//we're skipping less than the record size, so just update our position and return
					IndexIntoRecord += ticks;
					ticks = 0;
					return true;				
				}
				else
				{
					//we need to skip more than our record size, so read in the next record and
					//update our position
					ticks -= EdfRecordSize - IndexIntoRecord;
					IndexIntoRecord = 0;
					if(!EDFFile.ReadEDFSample())
					{
						fprintf(stderr,"\n Unable to read buf at new position \n");
						fprintf(stderr," UNABLE to re-read buffer!\n");
						fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");							  
						return false;
					}
				}
			}//while there are still ticks to skip
		}//if edf
		else
		{
			new_tick = (buf->decomp_buf_ind/bhdr.chan_cnt) + ticks;        // desired change in time 
													//  relative to buffer start 
			ticksthisbuf = buf->decomp_buf_siz/bhdr.chan_cnt;        // # ticks in current buffer 
			if (new_tick >= 0  &&  new_tick <= ticksthisbuf)
					{                               // new time is within buffer 
					  buf->decomp_buf_ind = (unsigned short)(new_tick*bhdr.chan_cnt);
					  return(TRUE); // just adjust the pointer & return  
					}
							// Now, set file position to start of current buffer 
							//      We do this in case the last readbuf returned 
							//      the last buffer in the file.  This last one  
							//      may not have been of the standard size.      
			loc_bx = buf->decomp_buf_ind;            // save the current pointer values   
			loc_fpos = fpos;        // save current file position pointer      
			buf->decomp_buf_siz = buf->max_decomp_buf_siz; 
			if (fseek( fptr, fpos, 0) != 0)
							{ fprintf(stderr,"\n UNABLE TO RESET FILE POSITION!\n");
							  fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
							  exit(0);
							}

			ticksthisbuf = buf->max_decomp_buf_siz/bhdr.chan_cnt;      // #ticks in standard buffer 
			new_buf = new_tick/ticksthisbuf;        // find # buffers to skip    
			if ((new_buf*ticksthisbuf) > new_tick)  // "backup" an extra buffer  
					   new_buf--;                   //  so new position is ahead 
			if (fseek( fptr, new_buf*buf->comp_buf_siz*sizeof(unsigned short), SEEK_CUR) != 0)
					{ if (fseek( fptr, loc_fpos, 0) != 0)
							{ fprintf(stderr,"\n Unable to fseek new position - eof\n");
							  fprintf(stderr,"\n UNABLE TO RESET FILE POSITION!\n");
							  fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
							  exit(0);
							}
					  if (buf->read_buf(fptr,&fpos) == FALSE)
							{ fprintf(stderr,"\n Unable to fseek new position - eof\n");
							  fprintf(stderr," ERROR - UNABLE to re-read buffer!\n");
							  fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
							  exit(0);
							}
					  buf->decomp_buf_ind = loc_bx;                   // reset the bx pointer 
					  return(FALSE);
					}
			new_tick = new_tick - (new_buf*ticksthisbuf);
			if (buf->read_buf(fptr,&fpos) == FALSE  ||  (long)buf->decomp_buf_siz/bhdr.chan_cnt < new_tick)
					{       // there was an error reading buffer or EOF     
					  if (fseek(fptr, loc_fpos, 0) != 0) // start of last buffer 
							{ fprintf(stderr,"\n Unable to read buf at new position \n");
							  fprintf(stderr,"ERROR - UNABLE TO SET FILE POSITION!\n");
							  fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
							  exit(0);
							}
					  if (buf->read_buf(fptr,&fpos) == FALSE)
							{ fprintf(stderr,"\n Unable to read buf at new position \n");
							  fprintf(stderr," UNABLE to re-read buffer!\n");
							  fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
							  exit(0);
							}
					  buf->decomp_buf_ind = loc_bx;          // reset the bx pointer 
					  return(FALSE);
					}
			buf->decomp_buf_ind = (unsigned short)(new_tick * bhdr.chan_cnt);//point to position within buffer  
		}
        return(TRUE);
}// END OF SKIP_EPOCH ROUTINE 

//-----------------------------------------------------------------------------------
// Function name:	skip_smp(long esize, long sampsec)
// Source file:		bf_ronly.cpp
// Prototype file:	bf_ronly.hpp
// Type:		C++ class public short
// Authors:		Jack Doman
// Last modified:	28-APR-95 K.Batch moved from digsub.c to bf_ronly.cpp and 
//			modified as a bf_read_only class member function
//			10/8/1990       TIM HOFFMAN & JACK DOMAN
//			8/13/1990       DEBBIE FRASCA
//                         
// Effect on global (or member) variables: calls read_epoch(char[])
//
short bf_read_only::skip_smp(long esize, long sampsec)
{
	unsigned short sample[MAX_CHANNELS_V2]; // garbage channel values
  long	  i,j;       // loop indexes:  modified TH,JD 10-8-90  now ints    
//------------------------------------------------------------------------- 
  if ((j = esize - ((sampsec*(bhdr.time%100))%esize)) < esize )
     {
       for (i=0; i<j; i++)
	   if ( !read_epoch(sample))
	      { fprintf(stderr,("%%EOF or error processing: %s\n",bfilnm));
		exit(0);
	      }
       increment_date_time_by_sec(&bhdr.date,&bhdr.time,(long)j/sampsec);
     }
  return(TRUE);
} // END OF SKIP_SMP ROUTINE

// ----------------------------------------------------------------------------------------
// Function name:	rewind()
// Source file:		bf_ronly.cpp
// Prototype file:	bf_ronly.hpp
// Type:		C++ class short	
// Author(s):		Kim Batch (September 24, 1997)
// Last modified:	
//	
// Effect on member variables: calls closebfl and openbfl
//
short bf_read_only::rewind()
{
	if(buf)
		delete buf;
	fclose(fptr); // Call the primitive fclose so that derived classes 
  // may override closebfl and still use rewind with no side effects
	return (openbfl());
}

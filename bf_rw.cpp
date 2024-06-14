//-----------------------------------------------------------------------------------
// File:		bf_rw.cpp
// Type:		C++ class implementation
// Definition File:	bf_rw.hpp
// Authors:		Principle original authors Debbie Kesicki, Jack Doman
//			Adapted to class members by Kim Batch, Tim Hoffman
// Last Modified:	
// Modified:	05-22-02  R Seres made fix for pc version so header would write out properly
				//rjs jan 21 2003 changed to use ftell and fseek to avoid using fpos_t structure that
//				is not defined well in our version of linux
// Modified:	09-03-03   R Seres changed NULL-pointers to string to prevent crashing on windows systems
// Modified		09-22-03 R Seres changes to suppress hp warnings
// Modified: 	12-02-03 R Seres Removed "c_prompt()" and replaced it with "c_prompt_len()"
//								c_prompt() used gets(), an unsafe function
// Modified:	05-15-05 R Seres added ability to read in an EDF file using the bfile class
// Synopsis:		Member functions of bf_read_write class
//
// MEMBER TABLE:
//
// BF_READ_WRITE(short mode,size_t siz)Overloaded interactive class constructor.
//			Initializes bfilnm. Calls openbfl(). Initializes read and write
//			buffers. If read_write is true, it is used exclusively for recordpc 
//			to allow reading from and writing to a bfile at the same time. If
//			it is zero, you can read the header and data but, you can only modify
//			the header.
//			arguments:	R->read_write	logical indicating # of bufs 
// BF_READ_WRITE(char[], short read_write)Overloaded class constructor Gets filnm from client
//			through parameter list. If filnm is not valid, program aborts. 
//			initializes bfilnm. Calls openbfl(). Initializes read and write
//			buffers. If read_write is true, it is used exclusively for recordpc 
//			to allow reading from and writing to a bfile at the same time. If
//			it is zero, you can read the header and data but, you can only modify
//			the header. 
//			arguments:	R->char[]		client input filnm	
//						R->read_write	logical indicating # of bufs
// ~BF_READ_WRITE()	Class destructor deletes rbuf and wbuf if used. 
// OPENBFL()	Opens bfile for read/write priviledges. Loads bhdr into memory.
// READ_EPOCH(unsigned short[])	Returns to client an epochful of values from buf through 
//			the parameter variable chanvals. Returns true if read is successful,
//			returns false if a disk read error or decompress error. Calls
//			read_buf to get data from disk.
//			argument(s): 	W->chanvals[]	array of channel values
// SKIP_EPOCH(long)	Causes next call to read_epoch to return chanvals ticks away 
//			argument(s): 	R->ticks	# of time ticks 
// SKIP_TO_HOUR(short) Skips to hour in bfile
//							R->hour		hour 1 - 12 marker
// SKIP_TO_SEC(long) Skips to # of seconds from the beginning of the bfile
//							R->seconds  # of seconds to skip 
// SKIP_TO_END(): 	Assigns readpos the last position in the file. To be called 
//			prior to skip_epoch when reading relative to the end of the file
//			is desired (as in recordpc).
// SKIP_SMP(long*,long*,long,long) Calls read_epoch until the beginning of the next 
//			complete epoch.
//			arguments:		R->date         start date of study   
//	 						R->time         start time of study  
//							R->esize        epoch size          
//							R->sampsec      samples per second 
// WRITE_BHDR()		Expects bhdr to be initialized. Writes bhdr to beginning of
//			file. Calls init_buf(). Returns true if write successful,
//			else returns false.
// WRITE_EPOCH(unsigned short[]) Retrieves epochful of values from client through
//			parameter variable chanvals and puts in wbuf. Calls bflush() to
//			write data to disk. Primes rbuf on first call. Returns true if buf management successful, 
//			else returns false.
//			argument(s): 	R->chanvals[]	array of channel values
// CLOSEBFL()		If buf not empty, calls bflush(). Closes bfile, returns true. 
//------------------------------------------------------------------------------------
 
#include "stdafx.h"
 

#include "bf_rw.hpp" // class definitions
	
//--------------------------------------------------------------------------------------
// Function name:	bf_read_write(short read_write)
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:		C++ overloaded class constructor
// Author(s):		Debbie Kesicki (originally PROC_BFL)
// Last Modified:	5-APR-95 Kim Batch (as advised by Ken Tew) removed parameter and
//			made function completely interactive 
//			31-Mar-95 Modified as class constructor by K.Batch
//
// Effect on member variable(s):	bfilnm set to input bfile name
//									calls openbfl()
//

 
bf_read_write::bf_read_write(short read_write_mode, size_t siz)
{ 
   strcpy(bfilnm, "");// changed NULL-pointer to string
   while (strcmp(bfilnm, "")==0) // changed NULL-pointer to string
	{
	// prompt user for input bfile name
	if((!c_prompt_len((char*)"\nEnter input bfilename: ",bfilnm, 80))) 
		{ 
		strcpy(bfilnm, "");   // changed NULL-pointer to string
	 	fprintf(stderr,"%% No Input file specified.\n");
		exit(-1);
	 	}
	else
		if (!openbfl(read_write_mode)) strcpy(bfilnm, ""); // open file

	}; // end of while
   initialize_buffers(read_write_mode, siz);

} // END OF BF_READ_WRITE

//------------------------------------------------------------------------------------ 
// Function name:	bf_read_write(char filnm[], short read_write_mode, size_t siz)
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:		C++ overloaded class constructor
// Authors:		Debbie Kesicki (originally PROC_BFL)
// Last modified:	6-APR-95 Modified as class constructor by Kim Batch
//
// Effect on member variables:	bfilnm set to value of filnm
//								calls openbfl()
//
bf_read_write::bf_read_write(char filnm[], short read_write_mode, size_t siz)
{ 
  strcpy(bfilnm,filnm);
  if (!openbfl(read_write_mode)) exit(-1); // ABORT if openbfl fails
  // Initialize buffers needed for data writes and/or reads
  initialize_buffers(read_write_mode, siz);

} // END OF BF_READ_WRITE

//------------------------------------------------------------------------------------ 
// Function name:	~bf_read_write()
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class destructor
// Authors:			Kim Batch
// Last modified:	
//
// Effect on member variables:	deletes rbuf & wbuf if used
//
bf_read_write::~bf_read_write()
{
	delete rbuf;
	if (wbuf!=NULL) delete wbuf;
} // END OF ~BF_READ_WRITE

//------------------------------------------------------------------------------------
// Function name:	openbfl(short read_write)
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class protected overloaded virtual short
// Authors:			Debbie Kesicki (originally OPENRWBFL)
// Last modified:	8-JUN-95 K.Batch - excludes	a bhdr check from 
//					recordpc's bhdr.
//					6-APR-95 Modified as class member by Kim Batch
//
// Effect on member variables:	Initializes fptr, bhdr,
//								readpos, currentpos
//
short bf_read_write::openbfl(short read_write)
{
  // Verify existence, don't open a non-existent file
  if (!file_exists(bfilnm)) 
     { fprintf(stderr,"ERROR - FILE DOESN'T EXIST\n");
       return(FALSE);
     }

  
  	bIsEdfFile = 0; 
	CheckFileType(bfilnm); 
	if(bIsEdfFile)
	{
		if(!OpenEdfAsBFile(bfilnm))
			return 0;
  	  
		char *cptr = &EDFFile.EdfHeader.starttime[6];//get the seconds from the start time in an HH.MM.SS string
		//we want to start on an even minute, so skip the first partial minute
		short SkippedSeconds = (60 - atoi(cptr)) % 60;

		if(SkippedSeconds != 0)
		{
			unsigned short junk[30];
			for(int index = 0; index < (SkippedSeconds * EdfRecordSize )/ EDFFile.EdfHeader.duration; index++)
				read_epoch(junk);//read in and do nothing with				
			increment_date_time_by_sec( &bhdr.date, &bhdr.time, SkippedSeconds );//adjust are date time to reflect the skip		  
		}	  

	}
	else
	{
  // Attempt to open file

  if((fptr=fopen(bfilnm,"r+b"))==NULL)	  
     { fprintf(stderr,"ERROR - UNABLE TO OPEN FILE\n");
       return(FALSE);
     }

  // Read bhdr into memory
#ifdef HPUX
  if(read_bhead_hp(fptr,&bhdr)==FALSE)
#else
  // 11-Nov-2001 J.Passios - Enables PC to read binary file header
  if(read_bhead_pc(fptr,&bhdr) == FALSE)
  //if(fread(&bhdr,B_H_SIZE,1,fptr)==0)	// read header structure
#endif
     { 	fprintf(stderr,"ERROR - UNABLE TO READ HEADER\n");
       	return(FALSE);
     }
	}
  // Check bhdr validity except recordpc's bhdr. It's not valid yet.
	if (!read_write)
  		if(!check_bhdr()) 
	  	{ 	fprintf(stderr,"ERROR - BFILE HEADER NOT VALID\n");
        	return(FALSE);
	  	}

  // Retrieve file position for read pointers
	currentpos = ftell(fptr);
	if(currentpos == -1)
     {
		fprintf(stderr,"\n UNABLE TO GET CURRENT FILE POSITION!!!\n");
		exit(-1);
     }
  readpos = currentpos;
  // bfile has been validated: Now extract path info from file name. 
  set_bfilpath();
  assign_ad_constants();	//-JD to set ADZERO and ADMAX_Float
  return(TRUE);
} //  END OF OPENBFL ROUTINE

//------------------------------------------------------------------------------------
// Function name:	initialize_buffers(short read_write, size_t siz)
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class private void
// Authors:			Kim Batch
// Last modified:	
//
// Effect on member variables:	if read_write = 1, wbuf and rbuf are
//								initialized - rbuf_primed is false
//								if read_write = 0, wbuf = NULL, rbuf
//								is initialized, calls prime_read_buf()
//								
//
void bf_read_write::initialize_buffers(short read_write, size_t siz)
{
  if ( read_write ) // both buffers used with this instantiation
  {
	wbuf = new bf_disk_buf(bhdr,1,siz/4);  // divide between 2 buffers
	rbuf = new bf_disk_buf(bhdr,0,siz/4);
	if (rbuf == NULL || wbuf == NULL)
	{
		fprintf(stderr,"NO MEMORY FOR BUFFERS _ ABORTING PROGRAM\n");
		exit(-1);
	}	
	rbuf_primed = FALSE; // but read buf not primed yet
  }
  else // only read buffer used 
  { 
  	wbuf = NULL;
	rbuf = new bf_disk_buf(bhdr,0,siz);
	if (rbuf == NULL) 
	{
		fprintf(stderr,"NO MEMORY FOR BUFFER _ ABORTING PROGRAM\n");
		exit(-1);
	} 
	if (!prime_read_buf()) 
	{
		fprintf(stderr,"CANNOT PRIME READ BUFFER - ABORTING PROGRAM\n");
		exit(-1);
	}
  }
  check_current = TRUE;
} // END OF INITIALIZE_BUFFERS ROUTINE

//------------------------------------------------------------------------------------
// Function name:	prime_read_buf()
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class private short
// Authors:			Debbie Kesicki (originally part of OPENBFL)
// Last modified:	6-APR-95 Modified as class member by Kim Batch
//
// Effect on member variables:	rbuf_primed = TRUE
//								calls read_epoch but doesn't use temp vals
//								calls skip_epoch with -1,setting index
//								back to beginning of rbuf->buf
// *** IMPORTANT:	This function must be called before read buffer can 
//					be used by read_epoch, skip_epoch, skip_to_hour, or
//					skip_to_end
//
short bf_read_write::prime_read_buf()
{
	unsigned short temp[MAX_CHANNELS_V2];

   	rbuf_primed = TRUE; // read buf primed with initial calls 
   	
	if (read_epoch(temp) == FALSE)
	{
		fprintf(stderr,"I/O ERROR - Unable to read data\n");
		closebfl();				
		return(FALSE);
	}
   	if (skip_epoch(-1) == FALSE)
	{
		fprintf(stderr,"I/O ERROR - Unable to get start of data\n");
		closebfl();				
		return(FALSE);
	}
   	return(TRUE);  
}

// ---------------------------------------------------------------------------------------
// Function name:	read_epoch()
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class public short	
// Author(s):		Debbie Kesicki (originally DISKDAT)
// Last modified:	30-Mar-95 Modified for class Kim Batch and Tim Hoffman
//	
// Effect on member variables:	(When function evaluates to true)
//				assigns chanvals chan_cnt values from buf at decomp_buf_ind
//				calls read_buf() when buf is empty
//
short bf_read_write::read_epoch(unsigned short chanvals[])
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
	// Don't use this function until the read buffer has been primed!
		if(!rbuf_primed) 
			return (FALSE);  // i.e. no data written to disk yet
		
		// If the buffer is empty then get another buffer full of data
		if(rbuf->decomp_buf_ind >= rbuf->decomp_buf_siz)
		{	
			if(fseek(fptr, readpos, 0) != 0)
			{
	    		fprintf(stderr,"\nUNABLE TO RESET FILE POSITION!!!\n");
				exit(-1);
    		}
			if(rbuf->read_buf(fptr,&readpos) == FALSE) 
				return (FALSE);

			readpos = ftell(fptr);
  		if(readpos == -1)
    		{
				fprintf(stderr,"\nUNABLE TO GET CURRENT FILE POSITION!!!\n");
				exit(-1);
     		}
		}
		// Place buffer data into chanvals for return to client
		for (i = 0; i<bhdr.chan_cnt; i++)
			chanvals[i] = rbuf->buf[rbuf->decomp_buf_ind++];
	}
	return(TRUE);
}// END OF READ_EPOCH ROUTINE

// -------------------------------------------------------------------------------------
// Function name:	skip_epoch(long ticks)
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class public short
// Author(s):		Jack Doman (Originally SKIPDAT)
// Last modified:	30-Mar-95 Modified for class by Kim Batch and Tim Hoffman
//
// Effect on member variables:	(When function evaluates to true)
//				ticks = value of parameter input
//				decomp_buf_ind = index ticks away from previous index
//				decomp_buf_siz = siz of buf where index is located
//				if ticks away index not in current buf, currentpos & 
//				readpos are modified
//				calls read_buf()
//				check_current = TRUE 
//	
short bf_read_write::skip_epoch( long ticks )
{
        long    new_tick;       // adjusted input file position in ticsk 
        long    new_buf;        // number of buffers to seek from current
        long    ticksthisbuf;   // number of ticks in current buffer
        unsigned short loc_bx;  // current bx value in case we must reset
        long   loc_fpos;      // current file positn in case we must reset
// ------------------------------------------------------------------------------------- 

	// Don't use this function if the read buffer has not been primed
	if (!rbuf_primed) return (FALSE);

	// If ticks == 0 don't move index (noop)
    if (ticks == 0) return (TRUE);
    
    // Desired change in time relative to buffer start     		 
    new_tick = rbuf->decomp_buf_ind/bhdr.chan_cnt + ticks;  
    
                                                		 
    if (check_current)
    {                                           		
	// If tick in current buffer, adjust index and return
	    ticksthisbuf = rbuf->decomp_buf_siz/bhdr.chan_cnt;     
        if (new_tick >= 0  &&  new_tick <= ticksthisbuf)
       	{ // new time is within current buffer 
        	rbuf->decomp_buf_ind = (unsigned short)(new_tick*bhdr.chan_cnt);
        	return(TRUE); 
        }
   	} // if only a read buffer is used
    
	// Else get new buffer (always if wbuf used) & 
    // calculate the currentpos relative to readpos
	 
    currentpos = readpos - (rbuf->comp_buf_ind * 2); // -debug jp 11/06/2001 
                                     // use actual index of compressed data retrieved
    loc_bx = rbuf->decomp_buf_ind;      // save the current pointer values   
    loc_fpos = currentpos;        		// save current file position pointer      

    // Set file position to beginning of last current read buffer 
    rbuf->decomp_buf_siz = rbuf->max_decomp_buf_siz; 
    if (fseek( fptr, currentpos, 0) != 0)
    { 	fprintf(stderr,"\n UNABLE TO RESET FILE POSITION!\n");
        fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
        exit(-1);
    }
    ticksthisbuf = rbuf->max_decomp_buf_siz/bhdr.chan_cnt; // #ticks in standard buffer 
    new_buf = new_tick/ticksthisbuf;        // find # buffers to skip    
    if ((new_buf*ticksthisbuf) > new_tick)  // "backup" an extra buffer  
               new_buf--;                   //  so new position is ahead 
               
    // Fseek from the new currentpos calculated above or in write_epoch 
    if (fseek( fptr, new_buf*rbuf->comp_buf_siz*sizeof(unsigned short), SEEK_CUR) != 0)
    { 	if (fseek( fptr, loc_fpos, 0) != 0)
        { 	fprintf(stderr,"\n Unable to fseek new position - eof\n");
            fprintf(stderr,"\n UNABLE TO RESET FILE POSITION!\n");
            fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
            exit(-1);
        }
        // set to full buffer size to re-read last buffer
    	rbuf->decomp_buf_siz = rbuf->max_decomp_buf_siz;
        if (rbuf->read_buf(fptr,&currentpos) == FALSE)
        { 	fprintf(stderr,"\n Unable to fseek new position - eof\n");
            fprintf(stderr," ERROR - UNABLE to re-read buffer!\n");
            fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
            exit(-1);
        }
        rbuf->decomp_buf_ind = loc_bx; // reset the original pointer
        return(FALSE);
   	}
   	
    new_tick = new_tick - (new_buf*ticksthisbuf);
    if (rbuf->read_buf(fptr,&currentpos) == FALSE ||(long)rbuf->decomp_buf_siz/bhdr.chan_cnt<new_tick)
    { // there was an error reading buffer or EOF     
    	if (fseek(fptr, loc_fpos, 0) != 0) // start of last buffer 
        { 	fprintf(stderr,"\n Unable to read buf at new position \n");
            fprintf(stderr,"ERROR - UNABLE TO SET FILE POSITION!\n");
            fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
            exit(0);
        }
        // set to full buffer size to re-read last buffer 
    	rbuf->decomp_buf_siz = rbuf->max_decomp_buf_siz; 
		if (rbuf->read_buf(fptr,&currentpos) == FALSE)
        { 	fprintf(stderr,"\n Unable to read buf at new position \n");
            fprintf(stderr," UNABLE to re-read buffer!\n");
            fprintf(stderr," CONTACT PROGRAMMER IMMEDIATELY!\n");
            exit(0);
        }
        rbuf->decomp_buf_ind = loc_bx;          // reset the bx pointer
        return(FALSE);
     }
     // point to position within buffer 
     rbuf->decomp_buf_ind = (unsigned short)(new_tick * bhdr.chan_cnt);
     // -debug jp 11/06/2001   use actual index of compressed data retrieved
	 readpos = currentpos + (rbuf->comp_buf_ind * 2); // set currentpos
     check_current = TRUE;
     return(TRUE);
}// END OF SKIP_EPOCH ROUTINE 

//--------------------------------------------------------------------------------------
// Function name:		skip_to_hour( short hour)
// Source file:         bf_rw.cpp
// Prototype file:      bf_rw.hpp
// Type:				C++ class public non-overloaded, non-virtual
// Author(s):			Timothy L. Hoffman with design help from Kim Batch
// Last Modified:		19-MAR-95 Tim Hoffman
// Effect on member variable(s):  modifies bf_rw   currentpos variable
//						readpos    variable
//				  		modifies bf_dbuf by calling read_buf()
//					   	see read_buf() for details on
//					   	which class vars it modifies
// Synopsis: takes a parameter from 1 thru 12 representing a specific hour
// within the bfile (which may still be open and growing). Calculates the
// byte offset that represents the start of the hour value passed in. For
// instance a value of 1 results in a byte offset of 0 - i.e. start of the
// first hour etc.  Once that value is calculated, read_buf() is called to
// go to that pos in the file and fill a read buffer.
// *** IMPORTANT: This function causes the bf_buf class to be modified. The
//		  The caller must take responsibility to syncronize any
//		  screen display info or any other values that should reflect
//		  this modification.
short bf_read_write::skip_to_hour( short hour )
{
   long   	samps_per_hr,   // samp per minute (lcm) * 60;
	  		bytes_per_hr;   // how many bytes in 1hr of 1 channel's data
   	long orig_pos;

   if (!rbuf_primed)
     return( FALSE );  //buffer must be primed before use.

   orig_pos = currentpos; // save original position before jump attempted
// based on sampling rate and # chans in bfile. calculate fileposition
// for start of hour passed in. if hour is 1, offset is 0, etc.

   samps_per_hr	 = bhdr.lcm_spm*60;	    	// all chans at same lcm_spm
   bytes_per_hr  = samps_per_hr/2*3;	    // compression ratio is 3/4
   bytes_per_hr *= bhdr.chan_cnt;	    	// counting all channels in bfile
   currentpos	 = 1024 + bytes_per_hr*(hour-1); // beginning of n'th hr
   if ( fseek( fptr, currentpos, 0 ) != 0)	// physically place read head
     {	currentpos = orig_pos;              // Watch out! it will set beyond EOF
		fseek( fptr, currentpos, 0 );		
		return( FALSE );
     }
// call readbuf() passing in this fileposition. readbuf() will use the
// filepoistion to set the *FILE pointer and return it modified to it's
// new value to reflect teh read just done.

   if ( !rbuf->read_buf( fptr, &currentpos )) // read class version of read_buf
     {	currentpos = orig_pos;
		fseek( fptr, currentpos, 0 );
		return( FALSE );
     } 
   readpos = currentpos + (rbuf->comp_buf_siz * 2); // set readpos
   return(TRUE);
}

//--------------------------------------------------------------------------------------
// Function name:		skip_to_sec( long seconds )
// Source file:         bf_rw.cpp
// Prototype file:      bf_rw.hpp
// Type:				C++ class public non-overloaded, non-virtual
// Author(s):			Kim Batch modified Tim's skip_to_hour
// Last Modified:		
// Effect on member variable(s):  modifies bf_rw   currentpos variable
//						readpos    variable
//				  		modifies bf_dbuf by calling read_buf()
//					   	see read_buf() for details on
//					   	which class vars it modifies
// IMPORTANT:
//		  The caller must take responsibility to syncronize any
//		  screen display info or any other values that should reflect
//		  this modification.
//
short bf_read_write::skip_to_sec( long seconds )
{
   long   	bytes_per_sec;   // how many bytes in 1 sec of data
	long	orig_pos;

   if (!rbuf_primed)
     return( FALSE );  //buffer must be primed before use.
  
   orig_pos = currentpos; // save original position before jump attempted
// based on sampling rate and # chans in bfile. calculate fileposition
// for # seconds from the beginning of the file

   bytes_per_sec  = (bhdr.lcm_spm/40);	    // 60 sec per min * compression ratio is 2/3
   bytes_per_sec *= bhdr.chan_cnt;	    	// counting all channels in bfile
   currentpos	  = 1024 + bytes_per_sec*(seconds); // # of seconds from start

   if ( fseek( fptr, currentpos, 0 ) != 0)	// physically place read head
     {	currentpos = orig_pos;              // Watch out! it will set beyond EOF
		fseek( fptr, currentpos, 0 );		
		return( FALSE );
     }
// call readbuf() passing in this fileposition. readbuf() will use the
// filepoistion to set the *FILE pointer and return it modified to it's
// new value to reflect teh read just done.

   if ( !rbuf->read_buf( fptr, &currentpos )) // read class version of read_buf
     {	
	   currentpos = orig_pos;
		fseek( fptr, currentpos, 0 );
		return( FALSE );
     } 
   readpos = currentpos + (rbuf->comp_buf_siz * 2); // set readpos
   return(TRUE);
}

// -------------------------------------------------------------------------------------
// Function name:	skip_to_end()
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class public void
// Author(s):		Kim Batch
// Last modified:	
//
// Effect on member variables:	readpos = lastpos
//								check_current = FALSE
// *** IMPORTANT: 	This function call must be directly followed by a 
//					call to skip_epoch.
//
void bf_read_write::skip_to_end()
{
	if(!rbuf_primed) return;
	readpos = lastpos;
	check_current = FALSE;
	return;
}	

//-----------------------------------------------------------------------------------
// Function name:	skip_smp(long *date, long *time, long esize, long sampsec)
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class public short
// Authors:			Jack Doman
// Last modified:	28-APR-95 K.Batch moved from digsub.c to bf_rw.cpp and 
//			modified as a bf_read_write class member function
//			10/8/1990       TIM HOFFMAN & JACK DOMAN
//			8/13/1990       DEBBIE FRASCA
//                         
// Effect on global (or member) variables: calls read_epoch(char[])
//
short bf_read_write::skip_smp(long *date, long *time, long esize, long sampsec)
{
	unsigned short sample[MAX_CHANNELS_V2]; // garbage channel values
  long	  i,j;       // loop indexes:  modified TH,JD 10-8-90  now ints    
//------------------------------------------------------------------------- 
  if ((j = esize - ((sampsec*(*time%100))%esize)) < esize)
     {
       for (i=0; i<j; i++)
	   if ( !read_epoch(sample))
	      { fprintf(stderr,("%%EOF or error processing: %s\n",bfilnm));
			exit(-1);
	      }
       increment_date_time_by_sec(date,time,(long)j/sampsec);
     }
  return(TRUE);
} // END OF SKIP_SMP ROUTINE

//------------------------------------------------------------------------------------
// Function name:	write_bhdr()
// Source file:		bf_wonly.cpp
// Prototype file:	bf_wonly.hpp
// Type:			C++ class public short
// Author(s):		Debbie Kesicki (Jack Doman?)(originally WRITEOUTHEAD)
// Last Modified:	31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
//
// Effect on member variable(s):  	(When function evaluates to true)
//									modifies file position - not assigned
//
short bf_read_write::write_bhdr()
 {
   fseek(fptr,0L,SEEK_SET);			// go to beginning of file 
#ifdef HPUX					// write header to file   
   if(write_bhead_hp(fptr,&bhdr)==FALSE)
#else
   if(write_bhead_pc(fptr,&bhdr)==FALSE)// 05-22-02  RJS fix for pc
#endif
       return(FALSE);				// unable to write header
   return(TRUE);
}//END OF WRITE_BHEADER ROUTINE

//--------------------------------------------------------------------------------------
// Function name:	write_epoch
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class public short
// Author(s):		Debbie Kesicki (Jack Doman?)(originally PUT_EPOCH)
// Last Modified:	28-Apr-95 K.Batch added read_write functionality
//					31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
//
// Effect on member variable(s):	(When function evaluates to true)
//					wbuf at decomp_buf_ind is assigned chan_cnt values
//					from chanvals
//					when wbuf is full, calls bflush(), on first fill,
//					calls prime_read_buf, sets lastpos to last position in
//					the file.
// 
short bf_read_write::write_epoch(unsigned short chanvals[])
 { 
    short i; // loop variable
	
    if (wbuf == NULL) return (FALSE);   
    if(wbuf->decomp_buf_ind >= wbuf->decomp_buf_siz) // if buf filled	   
    {
      	if(!wbuf->bflush(fptr)) return(FALSE); // write to output file

	    // If the read buffer has not been primed,
	    // prime and set to TRUE 
		if(!rbuf_primed)
		{
			if(!prime_read_buf())
			{
				fprintf(stderr,"CANNOT PRIME READ BUFFER_ABORTING PROGRAM");
				exit(-1);
			}
		}
		// Set lastpos to the end of the file
		lastpos = ftell(fptr);
		if(lastpos == -1)
		{
			fprintf(stderr,"\n UNABLE TO GET LAST FILE POSITION!!!\n");
			exit(-1);
		}
  	}
   
   	for (i=0; i<bhdr.chan_cnt; i++) // store epoch in buffer	   
  	    wbuf->buf[wbuf->decomp_buf_ind++] = chanvals[i];
  	return(TRUE);
}//END OF WRITE_EPOCH ROUTINE

//--------------------------------------------------------------------------------------
// Function name:	closebfl()
// Source file:		bf_rw.cpp
// Prototype file:	bf_rw.hpp
// Type:			C++ class public overloaded virtual short
// Author(s):		Debbie Kesicki (Jack Doman?)(originally BFLUSH_CLOSE)
// Last Modified:	19-May-95 K.Batch added sample count calculation
//					31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
// Last Modified:	05-May-05 Rob Seres if written out as an EDF file, clean up and close it
//
// Effect on member variable(s):	deletes fptr if not equal to stdout
//									calls bflush() is buf not empty
//					
// IMPORTANT!!! Don't call more than once
//
short bf_read_write::closebfl()
{

	if(bIsEdfFile)
	{
		EDFFile.CloseEDFFile();
		EDFFile.Clean();
	}
	else
	{
  // if write buf used, check buf & flush remaining vals 
		if( wbuf!=NULL )
		{ 
			if( wbuf->decomp_buf_ind > 0 ) wbuf->bflush(fptr); 	
			// calculate sample count & fix count in each channel  
			fix_up_sample_count(calc_sample_count(get_byte_ratio()));
			// write to disk
			write_bhdr();
		}
	}
  
  fclose(fptr); // close output file    
  return(TRUE);
}//END OF CLOSEBFL ROUTINE
 
//------------------------------------------------------------------------------------
// File:		bsf_ronly.cpp
// Type:		C++ class implementation
// Definition File:	bfA_ronly.hpp
// Author(s):		Principle original authors Debbie Kesicki, Jack Doman,
//			Tim Hoffman. Adapted to class members by Kim Batch and
//			Tim Hoffman. 
// Synopsis:		Member functions of bsf_read_only manipulate the Artifact data in the
//                      S Files. ALthouth the name of the class if bsf_read_only, these members
//                      read, modify and write to S Files artifact arrays. They do NOT modify
//                      any other header or data information in the Sfiles, nor do they modify
//                      the bfile in any way.  Modifications to the ABuffer do not modify the
//                      sixe of the buffer, only the 1/0 flag values in ABuffer.
// MEMBER TABLE:
//
// BSF_READ_ONLY()	Interactive class constructor. Prompts for bfilnm from client. 
//			If bfilnm or bfile is not valid, user will be prompted for another 
//			choice. Initializes bfilnm, and	fptr, fpos, bheader, buf, and buf 
//			indices by calling openbfl(). 
//
// BSF_READ_ONLY(char[])	Overloaded class constructor. Gets infilnm from client through
//			parameter list. If infilnm or bfile is not valid, program will
//			abort.Initializes bfilnm, and fptr, fpos, bheader, buf, and buf 
//			indices by calling openbfl(). 
//			argument(s): 	R->infilnm	bfile name string 
//
//~BSF_READ_ONLY()	Class destructor
//
// BSF_BTICKS_PER_RAW() Function returns the number of bfile ticks per raw spectral epoch for the
//                      particular bfile channel passed in.  Looks at that channel's associated SFile
//                      to compute this number. This value is used by the read_epoch() function.
//
// DATE_TIME_TO_BTICK( long, long, *long ) Converts an incoming date/time to the index of the 
//                      btick (epoch) at that date/time in the bfile
//
// TOGGLE_ARTIFACT_AT_DATE_TIME( long, long, short, short )  
//                      Toggles the artifact value for the selected file(s) at the specified 
//                      time, 
//
// TOGGLE_ARTIFACT_AT_BTICK( long, short, short )
//                      Same as above except instead of using time to find the desired Artifact 
//                      element, the btick index is used, by converting it to an Artifact index.
//
// INIT_SPTRS()         Looks at each channel in the bfile header, then goes out and looks for a
//                      corresponding S File for that channel. If one exists, an S Object is
//                      instantiated for it. Function returns the number of S Files found. 
// SAVE_ARTIFACT( short ) Copies the Artifact buffer values from the S Object in memory, out to
//                      the artifact area of it's corresponding  S File on disk.
//
// READ_EPOCH(unsigned short[])	Returns to client an epochful of values from buf through 
//			the parameter variable chanvals. Returns true if read is successful,
//			returns false if a disk read error or decompress error. Calls
//			read_buf to get data from disk. uses SFile artifact data to clean the
//                      chan_vals[] array before passing back to caller.
//			argument(s): 	W->chanvals[]	array of channel values
 
#include "stdafx.h"
 
#include "bsf_ronly.hpp"  // bf_read_only class definition file

// ----------------------------------------------------------------------------------------
// Function name:	bsf_read_only()
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class constructor
/// Author(s):		Debbie Kesicki (originally PROC_BFL)
// Last modified:	19-SEP-97 T.Hoffman. Modified name to reflect new derived class
//                      5-APR-95 Kim Batch (as advised by Ken Tew) removed parameter list
//			and made function completely interactive.
//			30-Mar-95 Modified as class constructor by Tim Hoffman & Kim Batch
//
// Effect on member variable(s):	bfilnm set to value of input bfile name 
//					calls openbfl() (the new SFile version of openbfl)
//
bsf_read_only::bsf_read_only()
{
  num_Sfiles_found = init_Sptrs(); // search default directory for Sfile matching our bfile channels
  bytes_per_B_epoch = bhdr.chan_cnt*2; // # of uncompressed bytes per epoch (tick)
  return;
}// END OF BF_READ_ONLY ROUTINE


// ----------------------------------------------------------------------------------------
// Function name:	bsf_read_only(char infilnm[])
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class constructor
// Author(s):		Debbie Kesicki (originally PROC_BFL)
// Last modified:	19-SEP-97: T.Hoffman. Changed name to reflect new derived class
//                      14-APR-95 K.Batch added references to a buffer object 
//			5-APR-95 Kim Batch (as advised by Ken Tew) removed interactive loop.
//			Function aborts if infilnm not valid.
//			30-Mar-95 Modified as class constructor by Tim Hoffman & Kim Batch
//
// Effect on member variable(s):	Initializes bfilnm to value of infilnm
//					calls openbfl()
//
bsf_read_only::bsf_read_only(char infilnm[]): bf_read_only( infilnm )
{ short num_Sfiles_found;
  num_Sfiles_found = init_Sptrs(); // search default directory for Sfile matching our bfile channels
                                   // instantiate S-objects for each one found
  bytes_per_B_epoch = bhdr.chan_cnt*2; // # of uncompressed bytes per epoch (tick)
  return;
}// END OF BF_READ_ONLY ROUTINE

bsf_read_only::~bsf_read_only()
{ 
 short i;
 for (i=0;i<bhdr.chan_cnt;i++)
   if (Sptrs[i] != NULL)
       delete Sptrs[i];
 return;
}// END OF DESTRUCTOR

//-------------------------------------------------------------------------------------
// Function Name:	closebfl() 
// Source File:		bsf_ronly.cpp
// Prototype File:	bsf_ronly.hpp
// Type:		C++ class public virtual short
// Author(s):		Debbie Kesicki		
// Last Modified:	30-Mar-95
//
// Effect on member variable(s):  fptr closed, Sobjects closed 
//
short bsf_read_only::closebfl()
{ short i;
        fclose(fptr);   // bfile fptr
        for (i=0;i<bhdr.chan_cnt;i++)
           if (Sptrs[i] != NULL)
               Sptrs[i]->Close();   
	return(TRUE);
        
}// END OF CLOSEBFL ROUTINE 

// ---------------------------------------------------------------------------------------
// Function name:	bticks_per_raw()
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class public short	
// Author(s):		Debbie Kesicki
// Last modified:	23-SEP-97 T.Hoffman.
//	                returns the number of bfile ticks (epochs) per raw spectral epoch.
//                      Example: Spectral epochs may be 4 seconds in width, while a bfile 
//                      tick (epoch) may only be 1/128'th or 1/256'th of a second in width.
//
long bsf_read_only::bticks_per_raw( short chan_ind )
{
  return( ((float)bhdr.chans[chan_ind].spm/60.0) * Sptrs[chan_ind]->raw_epoch_len );
}


// ----------------------------------------------------------------------------------------
// Function name:	date_time_2_btick( long, long *long )
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class constructor
// Author(s):		T.Hoffman
// Last modified:	24-OCT-97: T.Hoffman. Written for new POLY program which needs
//                      to know the current virtual bfile tick number at this date & time
// Retuns:              TRUE unless this date & time are before start of Bfile
//
short bsf_read_only::date_time_2_btick( long thisdate, long thistime, long *btick )
{
  long secs, bstartdate, bstartime, bticks_per_sec;
  bstartdate = bhdr.date;
  bstartime  = bhdr.time;
  secs = time_dif( thisdate, thistime, bstartdate, bstartime );
  if (secs < 0) return( 1 ); // Return 1 = error: invalid date/time passed in
  bticks_per_sec =  bhdr.chans[0].spm/60;
 *btick = secs * bticks_per_sec;
 return( 0 );   // Return 0 = success
}


// ----------------------------------------------------------------------------------------
// Function name:	toggle_artifact_at_date_time( long, long, short, short )
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class constructor
// Author(s):		T.Hoffman
// Last modified:	24-OCT-97: T.Hoffman. Written for new POLY program 
// Retuns:              0 for Success  if artifact is sucessfully toiggled. 
//
short bsf_read_only::toggle_artifact_at_date_time( long thisdate, long thistime, short primary_chan_ind,
                                                   short all_chans_flag  )
{ 
  short result;
  long  btick;

  result = date_time_2_btick( thisdate, thistime, &btick );
  if (!result) 
     return( result );
  return (toggle_artifact_at_btick(btick, primary_chan_ind, all_chans_flag) );
}


// ----------------------------------------------------------------------------------------
// Function name:	toggle_artifact_at_btick( long , short, short )
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class constructor
// Author(s):		T.Hoffman
// Last modified:	24-OCT-97: T.Hoffman. Written for new POLY program 
// Retuns:              TRUE unless this date & time are before start of Bfile
//
short bsf_read_only::toggle_artifact_at_btick( long btick, short primary_chan_ind, short all_chans_flag )
{
  long  Aind;
  short i,new_artval;

  i = primary_chan_ind;
  if ((i<0) || (i>=bhdr.chan_cnt))
     return( 1 );                 // return 1 = primary channel index not in bhdr 

  if (Sptrs[i] == NULL)           // return 2 = No Sfile found for this channel 
     return( 2 );

  if (Sptrs[i]->ABuffer == NULL)  // return 3 = SFile has no artifact info in it
     return( 3 );

  Aind = btick/B_epochs_per_A_epoch[i];
  if ((Aind < 0)  || (Aind >= Sptrs[i]->raw_epoch_cnt) )
     return( 4 );                 // return 4 = Btick out of Sfile's range 
  
  new_artval = Sptrs[i]->ABuffer[Aind] = 1 - Sptrs[i]->ABuffer[Aind];

  if (all_chans_flag)   // errors in non-primary channels return negative error codes
    { 
      for (i = 0; i<bhdr.chan_cnt; i++)
         if ((Sptrs[i] != NULL) && (i != primary_chan_ind))
           { 
             if (Sptrs[i]->ABuffer == NULL)  // return -3 = SFile has no artifact info in it
                 return( -3 );
	     Aind = btick/B_epochs_per_A_epoch[i];
             if ((Aind < 0)  || (Aind >= Sptrs[i]->raw_epoch_cnt) )
                return( 4 );                 // return -4 = Btick out of Sfile's range 
	     Sptrs[i]->ABuffer[Aind] = new_artval;;
           }
    }

  return( 0 );                               // return 0 = Success: No errors 

}


// ---------------------------------------------------------------------------------------
// Function name:	init_Sptrs()
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class public short	
// Author(s):		Debbie Kesicki (originally DISKDAT)
// Last modified:	23-SEP-97 T.Hoffman. Instantiates SFile object for each SFile that
//	                Spectral created from this bfile. 
// Effect on member variables:	Intializes Sptrs[] array. 
//
short bsf_read_only::init_Sptrs()
{
  short i;
  short result;
  char ret_str[80];  // returned from call to GetPrivateProfileString()
  char sfilename[80];
  char pathname[80];
  char chan_label[5];  // "EEG", "EOG1" etc.
  long  id;
  short study; 
  short with_bfile, in_SFILE_path;

  printf("\n\n\n *************** In INIT SPTRS ******************\n\n\n");
  num_Sfiles_found = 0;
  for (i=0;i<bhdr.chan_cnt;i++)
    {  
       id = bhdr.id;
       study = bhdr.study;
       strcpy(chan_label,bhdr.chans[i].label);
       with_bfile = in_SFILE_path = FALSE;     // start assuming no SFile found yet

   //  first look in same directory ad the bfile
 
       get_bfilpath( pathname );
       Sptrs[i] = new SFile(id,chan_label,study,pathname);
       printf("[%d] Looking for Sfile %s in same dir as bfile\n",i,Sptrs[i]-> GetSFilename() );

   // if it is not there, try the $SFILE directory

       if (!Sptrs[i]->Exists())  
	  { with_bfile = FALSE;
            printf("...SFile Not found with Bfile.. Deleting S object[%d]\n",i);
            delete Sptrs[i];
            if (strcpy( pathname, getenv("SFILE" )) == NULL)
	      {
	         in_SFILE_path = FALSE;
		 printf("...logical is unbound.. Aborting search for Sfile[%d]\n",i);
	      }
            else // $SFILE is bound
	      {
#ifdef HPUX
		strcat( pathname, "/" );
#else
		strcat( pathname, "\\" );
#endif
		Sptrs[i] = new SFile(id,chan_label,study,pathname);
	        printf("...Logical $SFILE is bound to %s.. looking for Sfile %s\n",
                           pathname, Sptrs[i]->GetSFilename() );
                if ( !Sptrs[i]->Exists() )
		  {
		    printf("...SFile %s NOT FOUND in $SFILE directory..  Deleting Sobject[%d]\n",
                               Sptrs[i]->GetSFilename(), i );
		    delete Sptrs[i];
		    in_SFILE_path = FALSE;
                  }
		else
		  {
		    in_SFILE_path = TRUE;
		    printf("...Sfile %s FOUND in $SFILE directory.. Now verifying open RW ability\n",
                               Sptrs[i]->GetSFilename(), i);
		  }
	      } // END Else $SFILE is bound
	  } // END if Sptrs[]i !Exist 
       else
	 {
	   with_bfile = TRUE;
	   printf("...Sfile %s FOUND in bfile's directory.. Now verifying open RW ability\n",
                      Sptrs[i]->GetSFilename(), i);
	 }

    // If an Sfile was found in either place for this bfile channel, verify read write ability  
 
       if (with_bfile || in_SFILE_path)
	 {
	   result = Sptrs[i]->Open(SF_RW);
	   if (result != 1)  // could not open SFile for read_write: delete this object - try next
	     {
	       fprintf(stderr,"...SFile::Open(SF_RW) failed with return code of %d\n", result);
	       fprintf(stderr,"...Sfile %s CANNOT BE OPENED READ/WRITE!  Going on to next Sfile\n",
                                  Sptrs[i]->GetSFilename() );
	       with_bfile = in_SFILE_path = FALSE;
	     }
	   else  // compute & store B to A epoch ratio, increment # Sfiles found
	     {  
	       B_epochs_per_A_epoch[i] = (long)Sptrs[i]->raw_epoch_len * (bhdr.chans[i].spm/60);
               num_Sfiles_found++;
	       printf("...Sfile %s exists and is read/write able. # of SFiles found now = %d\n",
                          Sptrs[i]->GetSFilename(), num_Sfiles_found );
	     }
         } // END: If  an SFile was found for this bfile channel
       else
	 {
	   printf("...SFile %s NOT FOUND ANYWHERE!  Going on to next Sfile\n",
                       Sptrs[i]->GetSFilename(),  i );
	 }
       printf("\n\n\n");
    } // END FOR i

  return(num_Sfiles_found );  
}



// ---------------------------------------------------------------------------------------
// Function name:	save_artifact()
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class public short	
// Author(s):		T.Hoffman
// Last modified:	07-NOV-97 T.Hoffman. Writes S object's artifact data out to S file on disk
// Effect on member variables:	The call to Save() changes the artifact flag value to 2
// which means "modified by poly" then writes the object header over the disk's header 
//
short bsf_read_only::save_artifact( short chan_ind )
{
  short i;
  short result;
  
  i = chan_ind;
  if ((i<0) || (i>=bhdr.chan_cnt))          
   return( 1 );                            // return 1 = no such chan index in this bfile 
  if (Sptrs[i] == NULL)    return( 2 );    // return 2 = No Sfile object for this channel
  if (!Sptrs[i]->Exists()) return( 3 );    // return 3 = Sfile object exists but no associated disk file
  result = Sptrs[i]->Save();
  if (result != TRUE)	  
      return( 4 );                         // return 4 = Failure attempting to write hdr &or artifact
  else
      return( 0 );                         // return 0 = SUCCESS with no errors
} 


// ---------------------------------------------------------------------------------------
// Function name:	read_epoch()
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class public short	
// Author(s):		Debbie Kesicki (originally DISKDAT)
// Last modified:	23-SEP-97 T.Hoffman. Added code to look at artifact value for
//                      each channel in chan_vals[] and do artifact cleaning on chan_vals[]
//                      30-Mar-95 Modified for classes by Kim Batch and Tim Hoffman
//	
// Effect on member variables:	(When function evaluates to true)
//				assigns chanvals chan_cnt values from buf at decomp_buf_ind
//				calls read_buf() when buf is empty
//
short bsf_read_only::read_epoch(unsigned short chanvals[])
{
short i;
long  Aind;              // index into Sfile's ABuffer array corresponding to i'th element in chan_vals[]

if(buf->decomp_buf_ind >= buf->decomp_buf_siz)
  if(buf->read_buf(fptr,&fpos) == FALSE) return (FALSE);
  
// now that we know which (virtual) bfile epoch (tick) we are cleaning, we can convert 
// that index into each corresponding S-File artifact array index. The bfile index is virtual, 
// i.e. no singe array is holding all the bfile data.  The S-File's artifact array however is 
// stored in it's entirety in the ABuffer array. We can convert curr_B_epoch_num into each chan's 
// corresponding curr_A_epoch_ind_[i], and use that index to get the artifact data for this epoch.
// **Note. Although The bfile index is the same for all channels,  the artifact index can
//   change across channels because each Sfile may have been run with differing parameters. 
// Specifically, differing artifact epoch widths cause a different artifact index being derived 
// from the same bfile epoch index. For this reason we use the stored B_epochs_per_A_epoch[] array 
// to compute the corresponding artifact indices in the chan_vals loop.

for (i = 0; i<bhdr.chan_cnt; i++)
  {
    if (Sptrs[i] != NULL)
       { 
	 // Calculate "virtual" index where fpos currently points into bfile. 
	 Aind = compute_B_epoch_ind() / B_epochs_per_A_epoch[i]; // KIM
	 // artvals[i]  = Sptrs[i]->ABuffer[Aind] DBG: see .hpp file for global variable decl.
         chanvals[i] = buf->buf[buf->decomp_buf_ind++] * Sptrs[i]->ABuffer[Aind];
       }
    else 
	 chanvals[i] =  buf->buf[buf->decomp_buf_ind++]; 
     //  printf("%4u  " ,chanvals[i] );  // DBG:
  }    
// printf("\n");  // DBG:

return(TRUE);

}// END OF READ_EPOCH ROUTINE

// -------------------------------------------------------------------------------------
// Function name:	compute_B_epoch_ind() [ virtual overwrite of bf_read_only]
// Source file:		bsf_ronly.cpp
// Prototype file:	bsf_ronly.hpp
// Type:		C++ class public short
// Author(s):		Tim Hoffman
// Last modified:	
//
// Effect on member variables: 
//
//	
//long bsf_read_only::compute_B_epoch_ind()
//{
  // long  curr_B_byte_ind;   // # bytes [0..bfile_size] that fpos is currently offset from bfile start.  

  // curr_B_byte_ind = (fpos-B_H_SIZE)*4/3;    // # of uncompressed bytes fpos offest from hdr 
  // curr_B_epoch_ind = ((fpos-B_H_SIZE)*4/3)/bytes_per_B_epoch;  // curr bfile epoch index (well..almost)
   
// We need to make one more adjustment to curr_Bfile_epoch_num. fpos did not really point to the 
// current epoch we are about to pass to the caller. It really points to the first byte of the physical

   // curr_B_epoch_ind += buf->decomp_buf_ind/bhdr.chan_cnt;  //  # ticks from start of bfile
   // fprintf(stderr, "Curr Btick#: %ld\n",curr_B_epoch_ind); // KIM
  //return (curr_B_epoch_ind);
//}


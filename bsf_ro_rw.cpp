//------------------------------------------------------------------------------------
// File:		bsf_ro_rw.cpp
// Type:		C++ class implementation
// Definition File:	bsf_ro_rw.hpp
// Author(s):		Principle original authors Debbie Kesicki, Jack Doman,
//			Tim Hoffman. Adapted from bsf_ronly class by Kim Batch & Tim Hoffman. 
// Synopsis:		Member functions of bsf_read_only manipulate the Artifact data in the
//                      S Files. The bfile is opened read only but the Sfile may be opeoed either
//                      read only or read write depending on the FLAG passed into the constructor.

//                      This class may modify the Artifact data array, but it does NOT modify
//                      any other header or data information in the Sfiles, nor do they modify
//                      the bfile in any way.  Modifications to the ABuffer do not modify the
//                      size of the buffer, only the 1/0 flag values in ABuffer.
// Modified 7/23/03 Rob Seres for Linux/Pc port

// Modified Sept 1st 2010 Fixed a bug with setting artifacts through remreject
// MEMBER TABLE:
//
// BSF_RO_RW( FLAG )    Interactive class constructor. Prompts for bfilnm from client. 
//			If bfilnm or bfile is not valid, user will be prompted for another 
//			choice. Initializes bfilnm, and	fptr, fpos, bheader, buf, and buf 
//			indices by calling openbfl(). 
//
// BSF_RO_RW( char[], FLAG ) Overloaded class constructor. Gets infilnm from client through
//			parameter list. If infilnm or bfile is not valid, program will
//			abort.Initializes bfilnm, and fptr, fpos, bheader, buf, and buf 
//			indices by calling openbfl(). 
//			argument(s): 	R->infilnm	bfile name string 
//
//~BSF_RO_RW()	Class destructor
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
// SET_ARTIFACT_AT_BTICK( long, short )
//						Same as above except sets the point to artifact, including the second before and after.
//						If all_chans_flag is false, only EEG channel will be set.
//
// INIT_SPTRS( FLAG )   Looks at each channel in the bfile header, then goes out and looks for a
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

//Modified 4/7/2010 RJS : Adding the ability to set artificats on C4, F3, and F4 channels.
//specrem needed tis ability
 
#include "stdafx.h"
 
#include "bsf_ro_rw.hpp"  // bsf_ro_rw class definition

// ----------------------------------------------------------------------------------------
// Function name:	bsf_ro_rw( const char *sf_iomode_str )
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class constructor
/// Author(s):		Debbie Kesicki (originally PROC_BFL)
// Last modified:	19-SEP-97 T.Hoffman. Modified name to reflect new derived class
//                      5-APR-95 Kim Batch (as advised by Ken Tew) removed parameter list
//			and made function completely interactive.
//			30-Mar-95 Modified as class constructor by Tim Hoffman & Kim Batch
//			25-Nov-2003 Rob Seres added some pointer checking in case SFILE creation fails
//
// Effect on member variable(s):	bfilnm set to value of input bfile name 
//					calls openbfl() (the new SFile version of openbfl)
//
bsf_ro_rw::bsf_ro_rw( const char *sf_iomode_str )  // arg specifies Sfile as RO or RW
{
  set_sf_iomode( sf_iomode_str );  // sets value of member var sf_iomode_string (see this.hpp)
                                   // and member var sf_iomode (see sfile.h)
  num_Sfiles_opened = init_Sptrs(); // search bfile dir first then $SFILE dir next 
  bytes_per_B_epoch = bhdr.chan_cnt*2; // # of uncompressed bytes per epoch (tick)
  return;
}// END OF BF_READ_ONLY ROUTINE


// ----------------------------------------------------------------------------------------
// Function name:	bsf_ro_rw(char infilnm[], const char *sf_iomode_str)
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
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
bsf_ro_rw::bsf_ro_rw( char infilnm[], const char *sf_iomode_str ): bf_read_only( infilnm )
{ 
// printf("\n\nInside bsf_ro_rw CONSTRUCTOR\n\n");  // DBG
  set_sf_iomode( sf_iomode_str );  // sets value of member var sf_iomode_string (see this.hpp)
                                   // and member var sf_iomode (see sfile.h) 
  num_Sfiles_opened = init_Sptrs(); // search bfile dir first then $SFILE dir next 
  bytes_per_B_epoch = bhdr.chan_cnt*2; // # of uncompressed bytes per epoch (tick)
  return;
}// END OF BF_READ_ONLY ROUTINE

bsf_ro_rw::~bsf_ro_rw()
{ 
 short i;
 for (i=0;i<bhdr.chan_cnt;i++)
   if (Sptrs[i] != NULL)
       delete Sptrs[i];
 return;
}// END OF DESTRUCTOR

void bsf_ro_rw::set_sf_iomode( const char *sf_iomode_str )
{
  strcpy( sf_iomode_string, sf_iomode_str );
  if (strcmp(sf_iomode_string,BSF_READ)==0)
    sf_iomode = SF_READ;
  else if (strcmp(sf_iomode_string,BSF_WRITE)==0)
    sf_iomode = SF_WRITE;
  else if (strcmp(sf_iomode_string,BSF_RW)==0)
    sf_iomode = SF_RW; 
  else
    {
       strcpy( sf_iomode_string, BSF_RW );
       sf_iomode = SF_RW;
    }
  return;
}


//-------------------------------------------------------------------------------------
// Function Name:	closebfl() 
// Source File:		bsf_ro_rw.cpp
// Prototype File:	bsf_ro_rw.hpp
// Type:		C++ class public virtual short
// Author(s):		Debbie Kesicki		
// Last Modified:	30-Mar-95
//
// Effect on member variable(s):  fptr closed, Sobjects closed 
//
short bsf_ro_rw::closebfl()
{ short i;
        fclose(fptr);   // bfile fptr
        for (i=0;i<bhdr.chan_cnt;i++)
           if (Sptrs[i] != NULL)
               Sptrs[i]->Close();   
	return(TRUE);
        
}// END OF CLOSEBFL ROUTINE 

// ---------------------------------------------------------------------------------------
// Function name:	bticks_per_raw()
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class public short	
// Author(s):		Debbie Kesicki
// Last modified:	23-SEP-97 T.Hoffman.
//	                returns the number of bfile ticks (epochs) per raw spectral epoch.
//                      Example: Spectral epochs may be 4 seconds in width, while a bfile 
//                      tick (epoch) may only be 1/128'th or 1/256'th of a second in width.
//
long bsf_ro_rw::bticks_per_raw( short chan_ind )
{
  return(long)( ((float)bhdr.chans[chan_ind].spm/60.0) * Sptrs[chan_ind]->raw_epoch_len );
}


// ----------------------------------------------------------------------------------------
// Function name:	date_time_2_btick( long, long *long )
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class constructor
// Author(s):		T.Hoffman
// Last modified:	24-OCT-97: T.Hoffman. Written for new POLY program which needs
//                      to know the current virtual bfile tick number at this date & time
// Retuns:              TRUE unless this date & time are before start of Bfile
//
short bsf_ro_rw::date_time_2_btick( long thisdate, long thistime, long *btick )
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
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class constructor
// Author(s):		T.Hoffman
// Last modified:	24-OCT-97: T.Hoffman. Written for new POLY program 
// Retuns:              0 for Success  if artifact is sucessfully toiggled. 
//
short bsf_ro_rw::toggle_artifact_at_date_time( long thisdate, long thistime, short primary_chan_ind,
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
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class constructor
// Author(s):		T.Hoffman
// Last modified:	24-OCT-97: T.Hoffman. Written for new POLY program 
// Retuns:              TRUE unless this date & time are before start of Bfile
//
short bsf_ro_rw::toggle_artifact_at_btick( long btick, short primary_chan_ind, short all_chans_flag )
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
//This forces delrem assigned artifacts to be marked with something specedit can recognized, as opposed to plain zero.
short SetArtValue(char v)
{
	//If it's 
	if(v == 0 || v == -3)
		return -3;
	return -2;
}
// ----------------------------------------------------------------------------------------
// Function name:	set_artifact_at_btick( long , short )
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class constructor
// Author(s):		C. Conrad
// Last modified:	05-09-2002: C Conrad. Written for new specrem program 
// Retuns:              TRUE unless this date & time are before start of Bfile
//

short bsf_ro_rw::set_artifact_at_btick( long btick, short all_chans_flag )
{
	long  Aind;
	short i;

	for (i = 0; i<bhdr.chan_cnt; i++)
	{
		if ( Sptrs[i] != NULL )
		{ 
			//Added C3, F3, and f4 channels. 
			if ( all_chans_flag || strcmp(Sptrs[i]->bf_label,"C3")==0 || strcmp(Sptrs[i]->bf_label,"C4")==0 ||strcmp(Sptrs[i]->bf_label,"F4")==0 ||strcmp(Sptrs[i]->bf_label,"F3")==0 ||
				strcmp(Sptrs[i]->bf_label,"EEG")==0 ||
					strcmp(Sptrs[i]->bf_label,"EEG1")==0 )	// if not all channels, only change EEG
			{
				if (Sptrs[i]->ABuffer == NULL)  // return -3 = SFile has no artifact info in it
					return( 3 );
				Sptrs[i]->artifacts_uncompressed = 1;
				Aind = btick/B_epochs_per_A_epoch[i];
				if ((Aind < 0)  || (Aind >= Sptrs[i]->raw_epoch_cnt) )
					return( 4 );                 // return -4 = Btick out of Sfile's range 
				Sptrs[i]->ABuffer[Aind] = SetArtValue(Sptrs[i]->ABuffer[Aind]);		// set artifact
				if(btick % B_epochs_per_A_epoch[i] < B_epochs_per_A_epoch[i]/4)
					Sptrs[i]->ABuffer[Aind - 1] = SetArtValue(Sptrs[i]->ABuffer[Aind - 1]);	// set previous time point
				if ( btick % B_epochs_per_A_epoch[i] != 0 && Aind < (Sptrs[i]->raw_epoch_cnt - 1) && btick % B_epochs_per_A_epoch[i] > 3 *  B_epochs_per_A_epoch[i]/4)
					Sptrs[i]->ABuffer[Aind + 1] = SetArtValue(Sptrs[i]->ABuffer[Aind + 1]);	// if not on a tick, also set time point after
			}
		}
	}

  return( 0 );                               // return 0 = Success: No errors 

}

// ---------------------------------------------------------------------------------------
// Function name:	init_Sptrs()
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class public short	
// Author(s):		Debbie Kesicki (originally DISKDAT)
// Last modified:	23-SEP-97 T.Hoffman. Instantiates SFile object for each SFile that
//	                Spectral created from this bfile. 
// Effect on member variables:	Intializes Sptrs[] array. 
//
short bsf_ro_rw::init_Sptrs( )
{
  short i;
  
  char pathname[80];

  num_Sfiles_opened = 0;
  set_sfstatus( BSF_NONE_FOUND );
  get_bfilpath( pathname ); // dirpath = bfile_path
  for ( i=0;i<bhdr.chan_cnt;i++ ) // for each channel[i] in bfile header
  { 
     Sptrs[i] = new SFile(bhdr.id,bhdr.chans[i].label,bhdr.study,pathname);
	if(!Sptrs[i])
	{
		fprintf(stderr,"Could not create SFILE\n");
	    set_sfstatus( BSF_UNOPENABLE );
		return FALSE;
	}
    // printf("[%d] Looking for Sfile %s\n",i,Sptrs[i]->GetSFilename() );
    if ( Sptrs[i]->Exists() )
      {
	//  printf("....Sfile %s FOUND. Now trying to open as %s\n",
	//      Sptrs[i]->GetSFilename(),sf_iomode_string );
	 if ( Sptrs[i]->Open( get_sf_iomode() ) == 1 )
	   {
	     B_epochs_per_A_epoch[i] = (long)Sptrs[i]->raw_epoch_len * (bhdr.chans[i].spm/60);
	     num_Sfiles_opened++;
	     // printf("....Sfile %s OPENED as %s. # of Sfiles Open = %d\n",
	     //         Sptrs[i]->GetSFilename(), sf_iomode_string, num_Sfiles_opened );
	   }
	 else
	   {
	     // printf("....SFile %s UNOPENABLE as %s. Deleting S object[%d]\n",
	     //     Sptrs[i]->GetSFilename(), sf_iomode_string,i );
	     delete Sptrs[i];
	     Sptrs[i] = NULL;
	     set_sfstatus( BSF_UNOPENABLE );
	     return FALSE;
	   }
      }
    else 
      {
	delete Sptrs[i];
	Sptrs[i] = NULL;
      }
  }
  if ( num_Sfiles_opened > 0 )
    {
      set_sfstatus( BSF_SUCCESS );
      return TRUE;
    }
  else // then look in $SFILE directory
    {
#ifdef HPUX
      strcpy( pathname,getenv("SFILE"));
#else
#ifdef LINUX
	strcpy( pathname,getenv("SFILE"));
#else
	  set_sfstatus(BSF_NONE_FOUND);
	  return FALSE;	//windows will not have environment variables, so we won't know where any SFILE dir could be
#endif
#endif
      if (pathname == NULL)
	{
	  set_sfstatus(BSF_NONE_FOUND);
	  return FALSE;
	}
      else  // $SFILE is bound. Look there for the Sfiles
	{
#ifdef HPUX
	  strcat( pathname, "/" );
#else
	  strcat( pathname, "\\" );
#endif
	  //  printf("\n..Now Searching $SFILE directory\n\n");
	  for ( i=0;i<bhdr.chan_cnt;i++ ) // for each channel[i] in bfile header
	    { 
	      Sptrs[i] = new SFile(bhdr.id,bhdr.chans[i].label,bhdr.study,pathname);
 		  if(!Sptrs[i])
		  {
			 fprintf(stderr,"Could not create SFILE\n");
			 set_sfstatus( BSF_UNOPENABLE );
			 return FALSE;
		  }
	      // printf("[%d] Looking for Sfile %s\n",i,Sptrs[i]->GetSFilename() );
	      if ( Sptrs[i]->Exists() )
		{
		  // printf("....Sfile %s FOUND. Now trying to open as %s\n",
		  //      Sptrs[i]->GetSFilename(),sf_iomode_string );
		  if ( Sptrs[i]->Open( get_sf_iomode() ) == 1 ) 
		    {
		      B_epochs_per_A_epoch[i] = (long)Sptrs[i]->raw_epoch_len * (bhdr.chans[i].spm/60);
		      num_Sfiles_opened++;
		      // printf("....Sfile %s OPENED as %s. # of Sfiles Open = %d\n",
		      // Sptrs[i]->GetSFilename(), sf_iomode_string, num_Sfiles_opened );
		    }  
		  else
		    {
		      // printf("....SFile %s UNOPENABLE as %s.  Deleting S object[%d]",
		      //       Sptrs[i]->GetSFilename(), sf_iomode_string,i );
		      delete Sptrs[i];
		      Sptrs[i] = NULL;
		      set_sfstatus( BSF_UNOPENABLE );
		      return FALSE;
		    }
		}
	      else
		{
		  delete Sptrs[i];
		  Sptrs[i] = NULL;
		}
	    }
	}
      if ( num_Sfiles_opened > 0 )
        {
           set_sfstatus( BSF_SUCCESS );
	   return TRUE;
	}
      else
	{
	  set_sfstatus( BSF_NONE_FOUND );
	  return FALSE;
	}
    } // END ELSE look in $SFILE directory
    
} // END function  bsf_ro_rw::init_Sptrs()

// ---------------------------------------------------------------------------------------
// Function name:	save_artifact()
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class public short	
// Author(s):		T.Hoffman
// Last modified:	28-Feb-98 T.Hoffman. Writes S object's artifact data out to S file on disk
// Effect on member variables:	The call to Save() changes the artifact flag value to 2
// which means "modified by poly" then writes the object header over the disk's header 
//					05-10-2002 C. Conrad. Incorporated funtion check_sfile_for_save to reuse code
// *** As of Feb-98 The only client of this function is POLY and POLY_20. 
//
short bsf_ro_rw::save_artifact( short chan_ind )
{
	short result;

// Preliminary errors tested for: Now try to write update the artifact segemnt of the S File.
	if ( (result = check_sfile_for_save(chan_ind)) == TRUE )
	{
		if ( Sptrs[chan_ind]->Save() )
			return BSF_SUCCESS;
		else
			return IO_FAILURE_WRITING_TO_SFILE;
	}
	return result;
} 

// ---------------------------------------------------------------------------------------
// Function name:	save_artifact_specrem()
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class public short	
// Author(s):		C. Conrad
// Last modified:	05-10-2002 C. Conrad. Writes S object's artifact data out to S file on disk
// Effect on member variables:	The call to Save() changes the artifact flag value to 3
// which means "modified by specrem" then writes the object header over the disk's header 
//
short bsf_ro_rw::save_artifact_specrem( short chan_ind )
{
	short result;
  
	if ( (result = check_sfile_for_save(chan_ind)) == TRUE )
	{
		Sptrs[chan_ind]->art_rej_flag = 3;  // artifact rejection flag set for alteration by specrem
	 
		if ( Sptrs[chan_ind]->WriteHead() )	  
			return BSF_SUCCESS;                    
		else
			return IO_FAILURE_WRITING_TO_SFILE;
	}
	return result;
}

// ---------------------------------------------------------------------------------------
// Function name:	check_sfile_for_save()
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
// Type:		C++ class public short	
// Author(s):		T.Hoffman
// Last modified:	05-10-2002 C. Conrad. Checks for problems anticipated in writing to an sfile
//					Separated from function save_artifact for reusable code
short bsf_ro_rw::check_sfile_for_save(short chan_ind )
{
  
#define NO_SUCH_CHAN_IN_BFILE          1
#define NO_SFILE_FOR_THIS_CHAN         2
#define NO_DISK_FILE_FOR_THIS_SOBJECT  3
#define IO_FAILURE_WRITING_TO_SFILE    4
#define SOBJECT_IS_READ_ONLY           5

// chan_ind = index of bfile channel whose associated S File will have it's 
// artifact data modified to reflect changes the user just made
// on screen via POLY or through automatic spectral rejection of specrem 
// When a POLY user saves artifact changes, The POLY prgram blindly loops through all the 
// channels in the bfile, asking this funct to save artifact to an S file for every channel.
// In most cases, there wil not be ab S file for every chan in the bfile. Therefore we must
// look for this case first of all error cases and return a value code signifying a benign 
// attampt to save on a non artifact analysed channel was recognized and ignored. 
 
 if (Sptrs[chan_ind] == NULL)    
    return( NO_SFILE_FOR_THIS_CHAN );  // Most common (and benign) reason that SAVE will fail

// The next most common reason for SAVE to fail would be that the user tried to save
// artifact changes, not realizing that the S files were opened as read/only.
// Althoug POLY notifies the user at S File open-time, POLY does not check for this at
// user save time, instead POLY again blindly loops through all it's channels,
// sending them here for a save, expecting this routine to detect incruencies between
// requests and status/availablilty of the S-files. This error should also be treated 
// as benign. We therefore return a code value telling POLY to notify user that the 
// S files are read/only.

  if (strcmp(sf_iomode_string,BSF_READ) == 0) 
    return( SOBJECT_IS_READ_ONLY );        // Don't try writing artifact to a READ/ONLY Sfile
                                           // Such a request is however common and benign 
                      
// The next errors are serious and should be investigated by the programming staff
// They reflect the following unexpected states:
//  - a non-existent channel index being passed in  
//         [ programming logic error ]
//  - an S File disspearing after it was opened    i.e. S object exists but file is gone
//         [ OS anomaly. File locking should  have prevented it ]
//  - an I/O error when attempting to write to an other wise valid and writable S File
//         [ OS anomaly or programming logic ]

  if ((chan_ind<0) || (chan_ind>=bhdr.chan_cnt))          
   return( NO_SUCH_CHAN_IN_BFILE );
 
  if (!Sptrs[chan_ind]->Exists()) return(NO_SFILE_FOR_THIS_CHAN);
  return TRUE;
}

// ---------------------------------------------------------------------------------------
// Function name:	read_epoch()
// Source file:		bsf_ro_rw.cpp
// Prototype file:	bsf_ro_rw.hpp
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
short bsf_ro_rw::read_epoch(unsigned short chanvals[])
{
short i;
long  Aind;              // index into Sfile's ABuffer array corresponding to i'th element in chan_vals[]
//unsigned short artvals[16]; // DBG:

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


// fprintf(stderr,"Bepoch[%06ld] ", compute_B_epoch_ind() );  // DBG:
for (i = 0; i<bhdr.chan_cnt; i++)
  {
    if (Sptrs[i] != NULL)
       { 
	 // Calculate "virtual" index where fpos currently points into bfile. 
	 Aind = compute_B_epoch_ind() / B_epochs_per_A_epoch[i]; // KIM
         // artvals[i]  = Sptrs[i]->ABuffer[Aind]; // DBG: see .hpp file for global var //DBG:
	 chanvals[i] = buf->buf[buf->decomp_buf_ind++] * Sptrs[i]->ABuffer[Aind];
	 // chanvals[i] = buf->buf[buf->decomp_buf_ind++];             // DBG:  
	 // fprintf(stderr,"i=%ld Cv:%4u Ai:%ld AV():%d Av:%d Cln:%4u  ",  // DBG
	 // i,chanvals[i], Aind, Sptrs[i]->GetArtifact(Aind), artvals[i],  // DBG
	 // chanvals[i]*artvals[i]); // DBG
       }
    else 
       {
	 chanvals[i] = buf->buf[buf->decomp_buf_ind++];
       }	
  }

return(TRUE);

}// END OF READ_EPOCH ROUTINE








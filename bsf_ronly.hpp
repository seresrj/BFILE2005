//---------------------------------------------------------------------------------------
// File:		bsf_ronly.hpp
// Type:		C++ class definition
// Implementation File:	bsf_ronly.cpp
// Author(s):		Kimberly R. Batch, Timothy L. Hoffman
// Last Modified:	Sep 97: T Hoffman, K Batch
//
// Synopsis:		Class derived from bf_ronly. This derivative differs from it's parent in
//			that it reads SFiles to get artifact info on the channels in the bfile
//			so that it can artifact clean all the epochs returned by read_epoch()
//		MAJOR CHANGES ARE AS FOLLOWS:
//		1) addition of private  SFile *Sptrs[MAX_CHANNELS] array. This array provides a 
//		   pointer for every possibe channel in the bfile. Usually however, only the EEG 
//		   or EOG channels will ever have an associated Sfile out on disk, but it is 
//		   *possible* that SPECTRAL could have been run on every channel in the bfile.
//		2) new private function init_Sptrs() which does the search for Sfiles and 
//		   instantiation of a pointer when a bhdr cannel has a corresponding Sfile present
//		   on disk in the default (or otherwise specified) directory.
//		3) ::read_epoch(), which now uses that above mentioned SFile (artifact) data to do 
//		   artifact cleaning on the chan_vals[] array before returning that array to
//		   it's callers
//		4) private array variable B_epochs_per_A_epoch, which stores some info used by
//		   get_epoch to index into each Sfile's artifact array. This variable is just a 
//		   computational convenience to avoid repetative calculation every get_epoch() call
//----------------------------------------------------------------------------------------

#ifndef BSF_READ_ONLY
#define BSF_READ_ONLY 1

#include "bf_base.hpp"	// base class definitions
#include "bf_dbuf.hpp"	// buffer class definitions
#include "sfile.h"      // S Class definitions


class bsf_read_only : public bf_read_only
{
public:	// PUBLIC MEMBERS: may be referenced by any client of this class

  bsf_read_only(void);				// Class constructor (interactive)
  bsf_read_only( char infilnm[] );		// Class constructor
 ~bsf_read_only();				// Class destructor
  short closebfl();		                // closes bfile then saves and closes Sfiles
  	
 				
  long bticks_per_raw( short );			// returns # of bfiles ticks per artifact value

  short date_time_2_btick( long, long, long *); // converts date/time into a bfile tick (virtual) index

  short toggle_artifact_at_date_time( long thisdate, long thistime, short primary_chan_ind, 
                                      short all_chans_flag); 

  short toggle_artifact_at_btick( long btick,  short primary_chan_ind, short all_chans_flag); 

  short save_artifact( short chan_ind );        // writes S object's artifact data to disk 
  short read_epoch(unsigned short chanvals[]);	// Reads into chanvals an epochful of uns shorts

  short num_Sfiles_found;       // # of Sfile found that correspond to the bfile channels. If none
                                // are found, caller must decide to abort or continue. This variable
				// is public so that the calling program can look at it immediately
				// after construction to make the abort/continue decision.

protected: // PROTECTED MEMBERS: may be referenced by members of this class or derived classes

// The new read_epoch() function will use the artifact data in the SFile classes to
// clean the chan_vals[] array before returning to various callers. These pointers are to be referenced
// only by the constructor and member function ::bsf_read_chans()

	SFile *Sptrs[MAX_CHANNELS_V2];   // for each possible channel in a bfile, we provide pointer for
			     	// that channel's corresponding SFile. Usually only EEG or EOG
				// channels have an SFile but it is possible for Spectral to have
				// been run on all channels in this bfile. It is also possible that
				// no Sfiles exist for any of the channels.

// new function init_Sptrs() looks for a corresponding Sfile for each channel in the bhdr. When
// an Sfile is found, Sptr[i] is instantited and the S-disk file is associated with it.

  short init_Sptrs();  // initializes Sptrs[] array and returns number of pointers instantiated
		       // This function is only called by the constructors - thus a protected function

  long compute_B_epoch_ind()  	// calculates virtual index into bfile
  {  
   return ((((fpos-B_H_SIZE)*4/3)/bytes_per_B_epoch) + buf->decomp_buf_ind/bhdr.chan_cnt);
  }  
  long B_epochs_per_A_epoch[MAX_CHANNELS_V2];      // used by read_epoch[] to index artifact data. This
						// array variable is accessed only by the read_epoch()
						// function and serves to save read_epoch() from having
						// to compute this info again with each get_epoch call
  long  bytes_per_B_epoch;			// used repeatedly in ::vitual_btick_index()
						// and gets initialized in constructor
  // long  curr_B_epoch_ind;  // which epoch [0..sampl_cnt] are we processing right now in read_epoch()
 //  short artvals[MAX_CHANNELS]; // DBG ONLY:  see ::read_epoch()
};

#endif



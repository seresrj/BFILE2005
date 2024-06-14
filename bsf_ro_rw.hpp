//---------------------------------------------------------------------------------------
// File:		bsf_ro_rw.hpp
// Type:		C++ class definition
// Implementation File:	bsf_ro_rw.cpp
// Author(s):		Kimberly R. Batch, Timothy L. Hoffman
// Last Modified:	Sep 97: T Hoffman, K Batch
// Modified			09-22-03 R Seres made changes to suppress hp warnings
//
// Synopsis:		Class derived from bf_ronly. This derivative differs from it's parent in
//			that it reads SFiles to get artifact info on the channels in the bfile
//			so that it can artifact clean all the epochs returned by read_epoch()
//		MAJOR CHANGES "INHERITED" FROM BSF_RONLY ARE AS FOLLOWS:
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

#ifndef BSF_RO_RW
#define BSF_RO_RW 1

#include "bf_base.hpp"	// base class definitions
#include "bf_dbuf.hpp"	// buffer class definitions
#include "bf_ronly.hpp" // bf_ronly class definition
#include "sfile.h"      // S Class definitions

#define BSF_SUCCESS    0
#define BSF_NONE_FOUND 1
#define BSF_UNOPENABLE 2

#define BSF_READ  "SF_READ"    // string version of constant version of I/O mode (see sfile.h)
#define BSF_RW    "SF_RW"      // string version of constant version of I/O mode (see sfile.h)
#define BSF_WRITE "SF_WRITE"   // currently unused since SPECTRAL is only user, SPECTRAL has
   
// next 5 defines are error return codes used by member ::save_artifact()
// All Clients (i.e. POLY and POLY_20) who call ::save_artifact() should look for these 
// codes upon return

#define NO_SUCH_CHAN_IN_BFILE          1
#define NO_SFILE_FOR_THIS_CHAN         2
#define NO_DISK_FILE_FOR_THIS_SOBJECT  3
#define IO_FAILURE_WRITING_TO_SFILE    4
#define SOBJECT_IS_READ_ONLY           5

class bsf_ro_rw : public bf_read_only
{
public:	// PUBLIC MEMBERS: may be referenced by any client of this class

  bsf_ro_rw(  const char *sf_iomode_str );	// Class constructor (interactive)
  bsf_ro_rw( char *infilnm, const char *sf_iomode_str ); // Class constructor
 ~bsf_ro_rw(void);				// Class destructor
  short closebfl();		                // closes bfile then saves and closes Sfiles
  short get_sfstatus() {return SFileStatus;};	// return sfile status to calling program
  short get_sf_iomode() { return sf_iomode; };  // tells what I/O mode this class tried to open
						// the Sfiles, by returning one of: SF_READ,
						// SF_WRITE, or SF_RW. 
  long bticks_per_raw( short );			// returns # of bfiles ticks per artifact value

  short date_time_2_btick( long, long, long *); // converts date/time into a bfile tick (virtual) index

  short toggle_artifact_at_date_time( long thisdate, long thistime, short primary_chan_ind, 
                                      short all_chans_flag); 

  short toggle_artifact_at_btick( long btick,  short primary_chan_ind, short all_chans_flag); 
  short set_artifact_at_btick( long btick, short all_chans_flag);	// sets artifact
  short save_artifact( short chan_ind );        // writes S object's artifact data to disk 
  short save_artifact_specrem( short chan_ind );// saves artifact and sets artifact rejection flag to 3 for specrem
  short check_sfile_for_save(short chan_ind );	// checks for valid sfile
  short read_epoch(unsigned short chanvals[]);	// Reads into chanvals an epochful of uns shorts

  short num_Sfiles_opened;      // # of Sfile found that correspond to the bfile channels. If none
                                // are found, caller must decide to abort or continue. This variable
				// is public so that the calling program can look at it immediately
				// after construction to make the abort/continue decision.

protected: // PROTECTED MEMBERS: may be referenced by members of this class or derived classes

// The new read_epoch() function will use the artifact data in the SFile classes to
// clean the chan_vals[] array before returning to various callers. These pointers are to be referenced
// only by the constructor and member function ::bsf_read_chans()

	SFile *Sptrs[MAX_CHANNELS_V2];   // for each channel in a bfile, this class provides a pointer for
			     	// that channel's corresponding SFile. Usually only EEG or EOG
				// channels have an SFile but it is possible for Spectral to have
				// been run on all channels in this bfile. It is also possible that
				// no Sfiles exist for any channels.
  short SFileStatus;		// the status of the embedded sfile classes
  char sf_iomode_string[10];    // "SF_READ", "SF_WRITE" or  "SF_RW"  (string constants )
  short sf_iomode;              //  SF_READ, SF_WRITE, or SF_RW  (defined constants see sfile.h)
  void set_sfstatus( short sfstatus ) { SFileStatus = sfstatus; };
  void set_sf_iomode( const char *sf_iomode_str ); 

// new function init_Sptrs() looks for a corresponding Sfile for each channel in the bhdr. When
// an Sfile is found, Sptr[i] is instantited and the S-disk file is associated with it.

  short init_Sptrs( ); // initializes Sptrs[] array and returns number of pointers instantiated
		       // This function is only called by the constructors - thus a protected function

  long compute_B_epoch_ind()  	// calculates virtual index into bfile
  { 
    return(long)( ((fpos-B_H_SIZE)/bytes_per_B_epoch* bf_compr_factor/3) + buf->decomp_buf_ind/bhdr.chan_cnt );
  } 
 
  long B_epochs_per_A_epoch[MAX_CHANNELS_V2];      // used by read_epoch[] to index artifact data. This
						// array variable is accessed only by the read_epoch()
						// function and serves to save read_epoch() from having
						// to compute this info again with each get_epoch call
  long  bytes_per_B_epoch;			// used repeatedly in ::virtual_btick_index()
						// and gets initialized in constructor
  // long  curr_B_epoch_ind;  // which epoch [0..sampl_cnt] are we processing right now in read_epoch()
 //  short artvals[MAX_CHANNELS]; // DBG ONLY:  see ::read_epoch()
};

#endif

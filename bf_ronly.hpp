//---------------------------------------------------------------------------------------
// File:		bf_ronly.hpp
// Type:		C++ class definition
// Implementation File:	bf_ronly.cpp
// Author(s):		Kimberly R. Batch, Timothy L. Hoffman
// Last Modifed 20-AUG-2012 R. Seres needed error string in constructor
//  Modified:	28-APR-95 K.Batch added skip_smp
//			14-APR-95 K.Batch added instance of disk_buf class
// Synopsis:		Class derived from bf_base class designed to open and read bfiles.
//----------------------------------------------------------------------------------------

#ifndef BF_READ_ONLY
#define BF_READ_ONLY 1

#include "bf_base.hpp"	// base class definitions
#include "bf_dbuf.hpp"	// buffer class definitions

class bf_read_only : public bf_base
{

public:	// PUBLIC MEMBERS: may be referenced by any client of this class

  bf_read_only();				// Class constructor (interactive)
  bf_read_only( char infilnm[] );		// Class constructor
  bf_read_only( char infilnm[], bool &Success, char Error[]);		// Class constructo
 ~bf_read_only();				// Class destructor


  virtual short read_epoch(unsigned short chanvals[]); // Reads into chanvals an epochful of uns shorts
  virtual short skip_epoch(long ticks);		// Moves the buf index to ticks away from 
						// current index
  virtual short skip_smp(long esize, long sampsec); // Determines #samples to skip so analysis start
						// at epoch start
  virtual short rewind();			// rewinds bfile to beginning of data
 
protected: // PROTECTED MEMBERS: may be referenced by members of this class or derived classes

 
  bf_disk_buf *buf;				// Buffer needed for disk reads
  short bf_compr_factor;        		// initilized as soon as bfile is opened in openbfl
  short openbfl();				// Opens bfile as read only
};


#endif



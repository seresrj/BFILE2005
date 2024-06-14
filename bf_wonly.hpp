//------------------------------------------------------------------------------------
// File:		bf_wonly.hpp
// Type:		C++ class definition
// Implementation File: bf_wonly.cpp
// Author(s):		Kimberly R. Batch, Timothy L. Hoffman
// Last Modified:	14-APR-95 K.Batch added instance of disk_buf class
// Last Modified:	15-MAY-05 R. Seres added ability to read in EDf files
// Synopsis:		Class derived from bf_base class designed to open and write
//			bfiles
//-------------------------------------------------------------------------------------

#ifndef BF_WRITE_ONLY
#define BF_WRITE_ONLY

#include "bf_base.hpp"   // base class definitions
#include "bf_dbuf.hpp"   // buffer class definitions

class bf_write_only : public bf_base
{
public: // PUBLIC MEMBERS: referenced by members or clients of this class

  bf_write_only();				// Class constructor (interactive)
  bf_write_only( char outfilnm[] );		// Class constructor
 ~bf_write_only();				// Class destructor
  short write_bhdr();			// Writes bfile bhdr to disk
  short write_epoch(unsigned short chanvals[]); // Writes bfile data to disk
  short closebfl();				// Overloaded closebfl flushes & closes
  short OpenAsEDF();
private: // PRIVATE MEMBERS: referenced by members of this class only

  bf_disk_buf *buf;				// Buffer needed for disk writes
  short openbfl();				// Opens bfile as write only
  

};

#endif
     

//-------------------------------------------------------------------------------
// File:			bf_rw.hpp
// Type:			C++ class definition
// Implementation File:		bf_rw.cpp
// Authors:			Kim Batch
// Last modified:	
//					rjs jan 21 2003 changed readpos from a long int to a fpos_t for linux port
// Synopsis:			Class derived from bf_base class designed to open,
//				read to and write from a bfile
//-------------------------------------------------------------------------------

#ifndef BF_READ_WRITE_CLASS
#define BF_READ_WRITE_CLASS

#include "bf_base.hpp"
#include "bf_dbuf.hpp"

class bf_read_write : public bf_base
{
public:

  bf_read_write( short read_write_mode = 0, size_t siz = MAX_BUF ); // Class interactive constructor
  bf_read_write( char filnm[], short read_write_mode = 0, size_t siz = MAX_BUF ); // Class constructor
 ~bf_read_write();					// Class destructor
  short read_epoch( unsigned short chanvals[] );// Reads into chanvals an epochful of values
  short skip_epoch( long ticks );	// Moves rbuf index to ticks away from
					// current index 
  short skip_to_hour(short hour);       // Skips to hour markers in file
  short skip_to_sec(long seconds);	// Skips to #seconds from start of data
  void  skip_to_end();			// Sets readpos to EOF prior to a call to
  					// skip_epoch
  short skip_smp(long *date, long *time, long esize, long sampsec); // Determines #samples to
					// skip so analysis starts at epoch start  									
  short write_bhdr();			// Writes bfile header to disk
  short write_epoch(unsigned short chanvals[] );// Writes bfile data to disk
  short closebfl();			// Overloaded closebfl flushes & closes

private:

  long	readpos, currentpos,		// file position pointers to assist with 
  		  lastpos;		// read/write manipulations
					// rjs jan 21 2003 changed this from a long int to a fpos_t for lunix port
  bf_disk_buf *rbuf;			// Buffer needed for disk reads
  bf_disk_buf *wbuf;			// Buffer needed for disk writes
  short rbuf_primed,check_current;	// logicals indicating status of rbuf  
  short openbfl(){return (TRUE);};	// redefined virtual - not used
  short openbfl(short read_write);	// overloaded,opens bfile as read/write
  void initialize_buffers(short read_write, size_t siz); // calls appropriate buf init functions
  short prime_read_buf();		// makes initial calls to read_epoch & 
					// skip_epoch to prime the rbuf
};

#endif

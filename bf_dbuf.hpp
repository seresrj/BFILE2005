//-------------------------------------------------------------------------------
// File:			bf_dbuf.hpp
// Type:			C++ class definition
// Implementation File:		bf_dbuf.cpp
// Authors:			Kim Batch
// Last modified:		T.Hoffman 6-19-95  buffers are malloc'd
//				instead of declared as arrays of const size
//						rjs jan 21 2003 changed file position structure for Linux port
// Synopsis:			Definitions of bf_disk_buf class, a buffer system
//				used by the bfile classes.
//-------------------------------------------------------------------------------

#ifndef  BF_DISK_BUF
#define BF_DISK_BUF 1

#include "bf_base.hpp"	// for public definitions

class bf_disk_buf
{
public:

  bf_disk_buf(struct B_HEADER bhdr, short mode = 0, size_t buf_siz = MAX_BUF); // class constructor
 ~bf_disk_buf(); // destructor..deallocates malloc'd buffer
									// 16 bit words in a single fread/fwrite call.
  unsigned short *buf;		 // all bfile I/O is buffered to/from buf[]
  unsigned short buf_siz;	 // # of unsighed shorts succesfully malloc'd
  unsigned short comp_buf_siz;	 // # of uns shorts that read_buf() will try to read into buf[]
				 // from a compressed bfile
  unsigned short comp_buf_ind;	 // the actual number of uns shorts that read_buf() actually
				 // read in from compressed bfile
  unsigned short decomp_buf_siz; // # of uns shorts in buf[] after inline decompression. This
				 // decompressed # should be 4/3 of the compressed #
  unsigned short decomp_buf_ind; // init to 0 then advances to end of buffer (decomp_buf_siz)
				 // as read_epoch or write_epoch is succesively called. When
				 // decomp_buf_ind reaches decomp_buf_siz, buf[] must be
				 // refilled or flushed depending on which class (r_only/_wonly)
  unsigned short max_decomp_buf_siz;// Maximum size of decompressed buf for #chans
  short data_compressed;		// Flag indicating status of bfile data on disk
  short compress_data;			// Flag indicating compression needed
  short read_buf(FILE *fptr, long *fpos); // Reads a buffer full of data from bfile on disk
										// rjs jan 21 2003 changed this from a long int to a fpos_t for lunix port
  short bflush(FILE *fptr); 	// Flushes data in buf to disk
  short decompress_buf();		// Decompresses data in buf
  short compress_buf();			// Compresses data in buf
  short allocate_buf( struct B_HEADER bhdr, short mode, size_t buf_siz );
};
#endif

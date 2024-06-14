//--------------------------------------------------------------------------------------
// File:		bf_dbuf.cpp
// Type:		C++ Class implementation
// Definition file:	bf_dbuf.hpp
// Authors:		Principle original authors Debbie Kesicki, Jack Doman
//			Modified to class by Kim Batch
// Last modified:
//				rjs jan 21 2003 for Linux port
//				rjs Nov 6  2003 added change to read_epoch to prevent an over-read
//				rjs May 15  2005 added check to see if buf has been allocated before deletion
// Synopsis:		Member functions for bf_disk_buf class
//
// MEMBER TABLE:
//
// BF_DISK_BUF(struct B_HEADER, short)Class constructor initializes buf and buf indices.
//			arguments:
//			R->struct B_HEADER bhdr	bfile header structure
//			R->short write		create a write buf if true else a read buf
// READ_BUF(FILE*, fpos_t*)Reads next input buffer from disk. Returns true if buf is retrieved
//			and decompressed, else returns false.
//			arguments:
//			R->FILE *fptr		pointer to bfile
//			W->fpos_t *fpos		pointer to position in the bfile
// DECOMPRESS_BUF()	Decompresses 12 bit numbers in buf to 16 bit unsigned shorts
// BFLUSH(FILE*)	Writes buf to the end of the file. Returns true if successful, 
//			else returns false.
//			arguments:
//			R->FILE *fptr		pointer to bfile
// COMPRESS_BUF()	Compresses 16 bit unsigned shorts into 12 bits. Returns false if
//			buf empty, else returns true.
// ALLOCATE_BUF(buff_siz)  Allocates buffer of buff_size unsigned shorts
 
#include "stdafx.h"
 
#include "bf_dbuf.hpp"  // definition file
//--------------------------------------------------------------------------------------
// Function name:	bf_disk_buf(struct B_HEADER, short mode, size_t buf_siz)
// Source file:		bf_dbuf.cpp
// Prototype file:	bf_dbuf.hpp
// Type:		C++ class constructor
// Author(s):		Debbie Kesicki (Jack Doman?)(originally OPENOUTBFL)
// Last Modified:	14-APR-95 Modified as class constructor by Kim Batch
//
// Effect on member variable(s):	sets compress_data and data_compressed to TRUE
//  (for true and compressed)		comp_buf_siz = (buf_siz/4) *3 - adj. for chan_cnt
//					decomp_buf_siz = (comp_buf_siz/3) *4
//					decomp_buf_ind = 0 for a write buffer
//					decomp_buf_ind = decomp_buf_siz for a read buffer
//
bf_disk_buf::bf_disk_buf( struct B_HEADER bhdr, short mode, size_t buf_siz )
{
   buf = NULL;
   if (buf_siz > MAX_BUF) buf_siz = MAX_BUF;

   buf = (unsigned short *) calloc( buf_siz, sizeof( unsigned short ) );
   if (buf==NULL)
    {
     fprintf(stderr,"Unable to calloc %u unsigned shorts. Aborting RECORDPC\n\n",buf_siz);
     exit(0);
    }

   if (bhdr.output_compr_alg)
	{
	data_compressed = TRUE; compress_data = TRUE;
	comp_buf_siz = (buf_siz/4)*3;
	decomp_buf_siz = (comp_buf_siz/3)*4;
	while(decomp_buf_siz%bhdr.chan_cnt != 0)	
		{
		comp_buf_siz -=3;
		decomp_buf_siz = (comp_buf_siz/3)*4;
		}
	}
  else
	{
	data_compressed = FALSE; compress_data = FALSE;
	comp_buf_siz = buf_siz	- (buf_siz%bhdr.chan_cnt);
	decomp_buf_siz = comp_buf_siz;
	}
  if (mode == 1)
	decomp_buf_ind = 0;	// for WRITE
  else
  {
	decomp_buf_ind = decomp_buf_siz; // for READ 
  	max_decomp_buf_siz = decomp_buf_siz;
  }

}

// DESTRUCTOR ----

bf_disk_buf::~bf_disk_buf()
{
	if(buf)
		free( buf );	//release buf memory back to DOS
	buf = NULL;
}

//---------------------------------------------------------------------------------------
// Function name:	read_buf(FILE *fptr, fpos_t *fpos)
// Source file:		bf_dbuf.cpp
// Prototype file:	bf_dbuf.hpp
// Type:		C++ class private short
// Author(s):		Jack Doman 
// Last modified:	21-APR-95 K.Batch Returns false only when there is no more data.
//			All critical errors cause aborts.
//			14-APR-95 K.Batch moved to class bf_disk_buf
//			30-Mar-95 Modified for class by Kim Batch and Tim Hoffman
//
// Effect on member variables:	(When function evaluates to true)
//				decomp_buf_siz = actual # of decompressed unsigned shorts
//				decomp_buf_ind = 0
//				comp_buf_ind = actual index of compressed data retrieved 
//				fpos is located at end front of current buffer read
//				calls decompress_buf()
//
short bf_disk_buf::read_buf(FILE *fptr, long *fpos)
{
	//rjs jan 21 2003 changed this to use a long int rather than a fpos_t struct for linux port 
#ifdef HPUX
	long iii;
#endif
	
	// If the previous buffer read was the last buffer, return false - no data
	if(decomp_buf_siz < max_decomp_buf_siz) return (FALSE);
	*fpos = ftell(fptr);
	if(*fpos == -1)
		{
		fprintf(stderr,"\nUNABLE TO GET CURRENT FILE POSTION!!!\n");
		fprintf(stderr,"CONTACT PROGRAMMER IMMEDIATELY!\n");
		exit(-1);
		}

	// If fread returns zero, return false - no data
	if((comp_buf_ind=fread(buf,sizeof(unsigned short),comp_buf_siz,fptr)) == WNULL) return (FALSE);
 
	if(comp_buf_ind != comp_buf_siz)//in case we read less bytes than we expected, adjust our
		//decom_buf_siz to reflect how many bytes we actually read in
		decomp_buf_siz = comp_buf_ind;
#ifdef HPUX
	for(iii = 0; iii<decomp_buf_siz; iii++)
		switch_bytes_hp((unsigned char *)&buf[iii], sizeof(unsigned short));
#endif

if(data_compressed)
	{
	if (decompress_buf() == FALSE)
		{
		fprintf(stderr,"\nERROR-DECOMPRESSION contact programmer.\n");
		exit(-1);
		}
    }
decomp_buf_ind = 0;  
return(TRUE);
}// END OF READBUF ROUTINE 

// --------------------------------------------------------------------------------------
// Function name:	decompress_buf()
// Source file:		bf_dbuf.cpp
// Prototype file:	bf_dbuf.hpp
// Type:		C++ class private short
// Author(s):		Jack Doman (Originally DECOMPRESS_DATA_1)
// Last modified:	14-APR-95 K.Batch moved to bf_disk_buf class
//			30-Mar-95 Modified for class by Kim Batch and Tim Hoffman
//
// Effect on member variables:	(When evaluates to true)
//				decomp_buf_siz = actual siz of decompressed unsigned
//				shorts in buf
//
//---------------------------------------------------------------------------------------
#define MASK12  (unsigned short) 0x0FFF // mask for right 12 bits       
#define MASK08  (unsigned short) 0x00FF // mask for right 08 bits       
#define MASK04  (unsigned short) 0x000F // mask for right 04 bits       
//---------------------------------------------------------------------------------------
short bf_disk_buf::decompress_buf()
{
  long           i,j;                   // for loop indecies    
//---------------------------------------------------------------------------------------
  if (comp_buf_ind < 3)  return (FALSE);
  decomp_buf_siz = (comp_buf_ind/3)*4;       // determine the decompressed length    
  for (j= decomp_buf_siz -1, i= comp_buf_ind -1;  i > 0;  )
		{ // buffer[0]=AAAB, buffer[1]=BBCC, buffer[2]=CDDD     
		buf[j--] = buf[i]&MASK12; 	// buffer[3] = 0DDD     
		buf[j]  =  buf[i--]>>12;  	// buffer[2] = 000C     
		buf[j--] |= (buf[i]&MASK08)<<4;   // b[2] = 0CCC  
		buf[j] = buf[i--]>>8;     	// buffer[1] = 00BB     
		buf[j--] |= (buf[i]&MASK04)<<8;   // b[1] = 0BBB  
		buf[j--] = buf[i--]>>4;   	// buffer[0] = 0AAA     
		}
  return (TRUE);
} // END OF DECOMPRESS_DATA ROUTINE

//-------------------------------------------------------------------------------------
// Function name:	bflush(FILE *fptr)
// Source file:		bf_dbuf.cpp
// Prototype file:	bf_dbuf.hpp
// Type:		C++ class private short
// Author(s):		Debbie Kesicki (Jack Doman?)
// Last Modified:	14-APR-95 K.Batch moved to bf_disk_buf class
//			31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
//
// Effect on member variable(s):	(When function evaluates to true)
//					decomp_buf_ind is reset to 0
//
short bf_disk_buf::bflush(FILE *fptr)
{
#ifdef HPUX
  long iii;
#endif

 if(compress_data)
 {		// compress data in buf
     if (compress_buf() == FALSE)
	{ fprintf(stderr," ERROR - Unable to compress data \n");
	  return(FALSE);
	}
 }
 else
     comp_buf_siz = decomp_buf_ind;
 

#ifdef HPUX
  for(iii=0; iii<comp_buf_siz; iii++)
	 switch_bytes_hp( (unsigned char *)&buf[iii],sizeof(unsigned short));
#endif

  fseek(fptr,0L,SEEK_END);	// go to end of file	  
  if((fwrite(buf, sizeof(unsigned short), comp_buf_siz, fptr)) != comp_buf_siz)
    { fprintf(stderr," I/O ERROR writing buffer \n");
      return(FALSE);
    }

  decomp_buf_ind = 0;		//reset index into decompressed buffer 
  return(TRUE);
} // END OF BFLUSH ROUTINE

//--------------------------------------------------------------------------------------
// Function name:	compress_buf()
// Source file:		bf_dbuf.cpp
// Prototype file:	bf_dbuf.hpp
// Type:		C++ class private short
// Author(s):		Jack Doman (originally COMPRESS_DATA_1)
// Last Modified:	14-APR-95 K.Batch moved to bf_disk_buf class
//			31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
//
// Effect on member variable(s):	(When function evaluates to true)
//					comp_buf_siz = (decomp_buf_ind/4) *3
//
short bf_disk_buf::compress_buf()
{
  unsigned short i, k, outcount;        // for loop indecies    
  unsigned short *bp_in,*bp_out;        // pointers into buffer array   
//-------------- -------------------------------------------------------------------- 
  if (decomp_buf_ind == 0) return(FALSE);
  if ((i=4-(decomp_buf_ind%4)) != 4)     // if not a multiple of 4, pad with 0's 
     for ( k=0; k < i; k++)
	buf[decomp_buf_ind++] = 0;
  outcount = (decomp_buf_ind/4);
  for ( bp_in = bp_out = buf, i=0; i<outcount; i++ )
     { // buffer[0]=0AAA, buffer[1]=0BBB, buffer[2]=0CCC    
       *bp_out      = *(bp_in++) << 4;   // buffer[0] = AAA0  
       *(bp_out++) |= *bp_in >> 8;       // buffer[0] = AAAB  
       *bp_out      = *(bp_in++) << 8;   // buffer[1] = BB00  
       *(bp_out++) |= *bp_in   >> 4;     // buffer[1] = BBCC  
       *bp_out      = *(bp_in++) << 12;  // buffer[2] = C000  
       *(bp_out++) |= *bp_in++;          // buffer[2] = CDDD  
     }
  comp_buf_siz = outcount * 3;   // determine the compressed length  
  return(TRUE);
} // END OF COMPRESS_BUF ROUTINE

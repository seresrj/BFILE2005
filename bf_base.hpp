//-----------------------------------------------------------------------------------------------------
// File:			bf_base.hpp
// Type:			C++ Class definition
// Implementation File:		bf_base.cpp
// Author(s):			Kimberley Batch, Timothy L. Hoffman
// Last Modified:		28-APR-95 K.Batch added assign_cal_constants, check_sample_count,
//				fix_up_sample_count, chan_nam, and write_comments as a regular function
//				14-APR-95 K.Batch moved buffer variables and functions to a buffer
//				class "bf_disk_buf", and added rename_bfilnm()
// Modified:	23-MAR-97 K.Batch added constructor and destructor definitions.
// Modified:	04-26-00  C Pratt added assign_ad_constants 
// Modified:	05-15-05  R Seres added ability to read in an EDF file using the bfile class
// Synopsis:	Base class for derived classes bf_read_only,  bf_write_only and 
//				bf_read_write.  These derived classes are used by the digitization 
//				program family
//-----------------------------------------------------------------------------------------------------
 

#ifndef BF_BASE_CLASS
#define BF_BASE_CLASS 1

#define MAX_BUF (size_t) 16000	// for all buffer classes
#define PROG_VERSION 12.0f		//New version is to handle different header sizes

//#define PROG_VERSION 11.1f		//for bfile program version. 

#include <stdio.h>		// disk & kbd I/O
#include <stdlib.h>		// disk & kbd I/O in derived classes
#include <string.h>		// string manipulation in derived classes
#include <time.h>		// system time calls in derived classes

#include "local.h"
#ifdef LOCAL
  #include "wpicc.h"		// #define's for TRUE FALSE etc.
  #include "bfile.h"		// #define's of bfile header struct and CAL CONSTANTS
 
  #include "generic.h"		// protos for generics.c timesub.c extract.cpp bsmooth.cpp bplot.cpp
  #include "wpiclib.h"		// protos for remainder of digitizing family code
 
 
#endif
#include "EDFFile.h"

class bf_base			// BASE CLASS FROM WHICH ALL BFILE CLASSES ARE DERIVED
{
public:		// PUBLIC MEMBERS:  can be referenced by any client of this class

  bf_base();				// constructor
  ~bf_base();				// destructor
  struct B_HEADER bhdr; 	// may be defined as a class in future 
  short check_bhdr();		// validates most scalar fields of header
  short disp_bhdr(short HHMM = 0); // prints all scalar header fields in a paragraph to screen
  short build_bhdr(struct B_HEADER oldhdr, short new_chans[], short new_chan_cnt); // builds a
				// new header from an old header and nulls chan values
  unsigned short chanvals[MAX_CHANNELS_V2];// holds one epoch to or from client

  void get_bfilnm(char bfilename[]);	// Returns bfilnm to client
  void get_bfilpath(char bfilepathname[]); // Returns bfilpath to client
  void set_bfilpath();			// Extracts bfilpath from bfilnm
  short rename_bfilnm(char newbfilnm[]);// Allows user to rename the file to newbfilnm
  virtual short closebfl();		// closes bfile (input or output same)

  long calc_sample_count(float byte_ratio); // calculates sample count from bfile size
  float get_byte_ratio();		// calculates byte ratio
  short check_sample_count(long sample_count);// checks if channel counts = sample count
  short fix_up_sample_count(long sample_count);// sets sampl_count->sample_count in channels

  short chan_nam(char *channels[], short min_chan, short report); // validates channel names 
  //edf conversion and reading functions
  void CheckFileType(char FileName[]); //Sets bIsEdfFile if the file is really an EDF
  void MakeBFileHeaderFromEDF(EDF_FILE *EdfFile, long ID, short Study, short Frequency);
  void MakeEDFHeaderFromBfile(bf_base *bfile);
  bool IsEDFFile(){ return bIsEdfFile;};
  void ImportEDFInfo(EDF_FILE *Source);
  EDF_FILE	*GetEDFFile(){ return &EDFFile;};
  void DecimateSampleCount(short Decimate);//decimates the sample counts if this is an edf file
  short OpenEdfAsBFile(char bfilnm[]);
  char* GetFileName(){ return &bfilnm[0];};
// PROTECTED MEMBERS: can be referenced only by derived classes of this class

  char   bfilnm[80];			// full path and filename of bfile being read/written
  char	 bfilpath[80];  
  FILE   *fptr;			// file handle of file being read/written
  long	 fpos;			// copy of position within file for virtual buffering
						//-rjs jan 2003 changed this for linux port, no longer a fpos_t struct
  EDF_FILE EDFFile;
protected:
  bool   bIsEdfFile;	//BFiles are not going to be used anymore, but we are still supporting
						//them. This variable is true if the information was obtained from and EDF
						//file instead. 
  short	 EdfRecordSize; //we can only have one frequency in a bfile, so this is the length of
						//the edf record with the frequency we are interested in
  long	 IndexIntoRecord;
  short  StartSeconds;//an edf file will almost certainly not start on an even minute. This is how
						//many seconds we must skip to get to an even minute. 
  short *DataPtrs[MAX_CHANNELS_V2];

  char LastError[256]; //last error we had
  virtual short openbfl(){ return 0;};		// overloaded funct defined only in derived classes
 
  void assign_cal_constants();		// If bfile date is after ONE_VOLTCAL_DATE, assigns
									// new cal constants, else asssigns old ones
  void assign_ad_constants();		// device_id determines ad constants for 12 or 16 bit data


};

#endif

// Definition of the SFILE classes 

//	Modified RJS July 21st for more Linux compilation  
//  Modified 09-22-03 R Seres made changes to suppress hp warnings
//	Modified 12-12-05 R Seres  Added support for additional artifact bands
//  Modified 01092007 R Seres Added support for uncompressed artifacts. 
//									if 0, artifact was Brunner algorithm reject
//									if -1, artifact was marked by hand. 
//									if -2, artifact was Brunner algorithm with high frequency rejection. 
//		R Seres March 20 2007	Added reopen function for SpecEdit
//	R Seres April 19 2009		Added 2d artifcats, an artifact that can span frequency ranges as well as time.
//					

#include "local.h"
#include "wpicc.h"		// defines MAX_CHANNELS

#ifndef SFILE_H			// test for previously included sfile.h
#define SFILE_H
#define SFILE_HEADER_VERSION 2.0
#define MAX_BIN 256		// if you want more frequency bins than this,
				// then the sfile header will change
#define MAX_ART_BANDS 10
#define MAX_2D_ARTIFACTS 16
#define S_H_SIZE 4096	        // size of S_HEADER in bytes (including extra space)
#define FILLER_SIZE 1958 - (16 * MAX_ART_BANDS +2) - (2 + 12 * MAX_2D_ARTIFACTS) ////rjs modified July 27 2005 to add additional rejection info
											//rjs modified Jan 9th 2007 to add artifacts_uncompressed

//1959
#define EQUIPMENT_UNDEFINED 0
#define RECORD_PC 1
#define HARMONIE 2

#define MAX_BANDS 10

#ifdef SPLUS
#include "bfile.h"		// defines LABSIZ

/* ---- DEFINE BINARY SFILE HEADER STRUCTURE FOR USE IN SPLUS ----*/
struct S_HEADER {
// version number of this sfile header class/structure
 
  float   sf_hdr_version; 

// sfile header values that come straight from the bfile header or describe

  char    bf_filename[16];	// bfile name
  char    bf_label[LABSIZ];	// "EEG","EOG1","NPT" etc.
  short   bf_study;		// study (from bfile)
  short   bf_channel;	        // channel of bfile which was analyzed
  short   bf_samples_per_sec;   // sampling rate of bfile
  long    bf_id;		// id for subject (from bfile)
  long    bf_date;		// date of study (from bfile header)
  long    bf_time;		// time of study (from bfile header)
  float   chan_calampuv; // traditionally 50 uv (unless some change in cal occurs)

// sfile header values pertaining to artifact rejection data in this sfile

  short   art_rej_flag;	    // was artifact rejection performed ?
  float   raw_epoch_len;    // artifact epoch length (i.e. 4 seconds)
  long    raw_epoch_cnt;    // number of raw (same as # artifact) epochs in this file
			    // This is NOT the number of spectral values
  float   art_med_thresh;	// artifact threshold
  short   art_med_pre_len;	// number of epochs of median preceeding epoch
  short   art_med_post_len;	// number of epochs of median following epoch
  float   art_bin_low;		// low cutoff frequency of artifact band
  float   art_bin_high;	        // high cutoff

// sfile header values specified by spectral analysis data in this sfile

  short   raws_per_spec;        // each spec epoch is many raws averaged together 
  short   spec_bin_count;	// frequency bins per epoch
  short   spec_windowing_flag;	// windowing?
  short   spec_unit_flag;	// flag: v^2/Hz, v^2/octave or v^2
  short   spec_one_sided_flag;	// flag: one-sided or two-sided spectrum
  float   spec_bin_low_freq[MAX_BIN]; // low freq cut off for bin, freq included
  float   spec_bin_high_freq[MAX_BIN]; // high freq cut off for bin, freq excluded

  unsigned short num_fft_pts;   // number of points in FFT analysis
  unsigned short num_samp_overlap; // number of samples overlapped
  unsigned char    data_acquisition_equipment; //the device used to acquire the bfile data the code is 1 + the normal device id number, 0 is unkown//rjs added sept 16 202
  unsigned char	   device_scaled_to;//the device the data has been scaled to, the code is 1 + the normal device id number, 0 is unscaled	//rjs added sept 16 202
 
  short	  number_of_additional_artifactbands;
  float   additional_art_bin_low[MAX_ART_BANDS];		// low cutoff frequency of artifact band
  float   additional_art_bin_high[MAX_ART_BANDS];	        // high cutoff
  float	  additional_art_medthresh[MAX_ART_BANDS];
  short	  additional_art_pre_len[MAX_ART_BANDS];
  short	  additional_art_post_len[MAX_ART_BANDS];
  char    artifacts_uncompressed;
  char    junk[FILLER_SIZE];	// extra space to grow
};

#else // if not defined SPLUS
#include "bf_ronly.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#ifdef HPUX		               // ios stream class definitions
//ios::binary not recognized by hp
#define SF_READ ios::in|ios::nocreate		  // sfile read only 
#define SF_RW ios::in|ios::out|ios::nocreate  // sfile read write
#define SF_WRITE ios::out                    // sfile write only
#elif LINUX						// nocreate not recognized by linux
#define SF_READ ios::in		  // sfile read only 
#define SF_RW ios::in|ios::out  // sfile read write
#define SF_WRITE ios::out                    // sfile write only
#else
#define SF_READ ios::in|ios::binary		  // sfile read only 
#define SF_RW ios::in|ios::out|ios::binary  // sfile read write
#define SF_WRITE ios::out|ios::binary                    // sfile write only
#endif

#ifdef HPUX
#define DEFAULT_SFILENAME (char*)"/usr/local/spec/bin/SFILE.DEF"
#elif LINUX	
#define DEFAULT_SFILENAME (char*)"/usr/local/spec/bin/SFILE.DEF"
#else 
#define DEFAULT_SFILENAME (char*)"c:\\temp\\SFILE.DEF"
#endif
//#define DEFAULT_SFILENAME "/db1/development/SFILE.DEF"

#define LOCAL_SFILENAME   (char*)"SFILE.DEF"


#define DEFAULT_PARAM (char*)"/usr/local/spec/bin/spec.param"
#define LOCAL_PARAM "spec.param"

//Contains start and stoppign epochs of teh artifact, as well as teh high and low frequency band it contains. 
struct ARTIFACT_2D
{
	float StartBand;
	float EndBand;
	short StartSegment;
	short EndSegment;
};
 

class SFileHeader
{
public:

  float   sf_hdr_version; // version number of this sfile header class/structure

// sfile header values that come straight from the bfile header or describe

  char    bf_filename[16];	// bfile name
  char    bf_label[LABSIZ];	// "EEG","EOG1","NPT" etc.
  short   bf_study;		 // study (from bfile)
  short   bf_channel;	 // channel of bfile which was analyzed
  short   bf_samples_per_sec; // sampling rate of bfile
  long    bf_id;		 // id for subject (from bfile)
  long    bf_date;		 // date of study (from bfile header)
  long    bf_time;		 // time of study (from bfile header)
  float   chan_calampuv; // traditionally 50 uv (unless some change in cal occurs)

// sfile header values pertaining to artifact rejection data in this sfile

  short   art_rej_flag;	    // was artifact rejection performed ?
  float   raw_epoch_len;    // artifact epoch length (i.e. 4 seconds)
  long    raw_epoch_cnt;    // number of raw (same as # artifact) epochs in this file
			    // This is NOT the number of spectral values
  float   art_med_thresh;	// artifact threshold
  short   art_med_pre_len;	// length of median preceeding epoch in seconds
  short   art_med_post_len;	// length of median following epoch in seconds
  float   art_bin_low;		// low  cutoff frequency of artifact band
  float   art_bin_high;	        // high cutoff

// sfile header values specified by spectral analysis data in this sfile

  short   raws_per_spec;        // each spec epoch consists of this many raws averaged together 
  short   spec_bin_count;	// frequency bins per epoch
  short   spec_windowing_flag;	// windowing?
  short   spec_unit_flag;	// flag: v^2/Hz, v^2/octave or v^2
  short   spec_one_sided_flag;	// flag: one-sided or two-sided spectrum
  float   spec_bin_low_freq[MAX_BIN]; // low freq cut off for bin, freq included
  float   spec_bin_high_freq[MAX_BIN]; // high freq cut off for bin, freq excluded

  unsigned short   num_fft_pts;          // number of points in FFT analysis
  unsigned short   num_samp_overlap;     // number of samples overlapped
  
  unsigned char    data_acquisition_equipment; //the device used to acquire the bfile data the code is 1 + the normal device id number, 0 is unkown//rjs added sept 16 202
  unsigned char	   device_scaled_to;//the device the data has been scaled to, the code is 1 + the normal device id number, 0 is unscaled	//rjs added sept 16 202
  short	  number_of_additional_artifactbands;
  float   additional_art_bin_low[MAX_ART_BANDS];		// low cutoff frequency of artifact band
  float   additional_art_bin_high[MAX_ART_BANDS];	        // high cutoff
  float	  additional_art_medthresh[MAX_ART_BANDS];
  short	  additional_art_pre_len[MAX_ART_BANDS];
  short	  additional_art_post_len[MAX_ART_BANDS];
  char    artifacts_uncompressed;	//RJS true if the artifacts are uncompressed. When they are compressed, information on how they were determined is lost
									//specedit does not compress them. 
  char    junk[FILLER_SIZE];	// extra space to grow
  short	  NumberOf2DArtifacts;
  ARTIFACT_2D	Artifacts2D[MAX_2D_ARTIFACTS];

  SFileHeader();		// constructor 
  ~SFileHeader();		// destructor does nothing
  void dump();                  // debug assistant dumps header values to stderr

protected:

  void init_header();		// initializes above to zero or NULL

};

class SFile: public SFileHeader
{
public:
	SFile();
	SFile(char* sfilename);
	SFile(long ID, char label[], short study, char path[]);
	SFile(bf_read_only* bfile, char* label = "C4");
	~SFile();

	short Exists(char* path = NULL);
	short Open(short FLAG /*SF_READ,SF_RW,SF_WRITE*/);
	short OpenSP(short FLAG /*SF_READ,SF_RW*/);
	short Save() { art_rej_flag = 2; return WriteHead(); }; // POLY writes out header and artifact 
	short ReOpen(short Flag);
	void  Close();
	short TempClose(short Flag);
	void  Flush();
	char* GetSFilename() { return sfilnm; }; // return the sfile name
	short GetSpecParams(); // uses sfile object for parameter storage
	short GetSpecParams(char* pfilename, unsigned short USE_DEFAULT = TRUE); // uses spec.param
	void  DisplaySpecParams();   // displays this hheaders spectral analysis related fields
	void  DisplaySpecParams( SFile *); // same as above plus the passed in class in parenthes
	void  ModifySpecParams(short *modified);  // let user change fields one by one. (like bf_edit)
        short SetHeaderFields(bf_read_only* bfile, char* label);
	short WriteHead(short head = TRUE, short artifact = TRUE); 

	short* ABuffer=NULL; // artifact buffer of zeros and ones, maybe compressed or uncompressed
	short GetArtifact(long index);
	short In2dArtifact(short segment, short bin);

	short ReadSpec(float* data);
	short WriteSpec(float* data);

	void dump_all();
	void dump_artifact();
	void dump_artifact(char artfile[]);
	void dump_spectral();

	void SetFilename(char *nf){ strcpy(sfilnm, nf); }; 
 
protected:
	fstream sf;     // the sfile handle
	streampos dpos; // the start position of the data in the sfile
	char sfilnm[80];
	void create_SFilename(long ID, char* label, short study, char* path);
	short init_artifact();
	short write_header_hp();
	short write_header_pc();
	short write_header();
	short write_artifact();
	short read_header_hp();
	short read_header_pc();
	short read_header();
	short read_artifact();
};
void _create_SFilename(long ID,char* label,short study,char* path, char sfilnm[]);
#endif                  // end of #ifndef SPLUS
#endif			// end of #ifndef SFILE_H

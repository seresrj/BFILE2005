//-----------------------------------------------------------------------------
// File:  sfile.cpp
// Type:  C++ class implementation file
// Definition File: sfile.h
// Authors:  Kim Batch, Tim Hoffman and Ray Vasko
// Last Modified:
//		R Seres for hp3 compile
//		R Seres sept 16 2002 to added device id fields
//		R Seres march 3 2003 made changes to Open functions for Linux compile
//		R Seres march 14 2003 made changes to create_SFilename to fix a bug with filename generation
//		R Seres july 23 2003 made fixes where it compares IO flags 
//		R Seres sept 3 2003 changed NULL-pointers to string to prevent crashing on windows systems
//		R Seres sept 23 2003  made changes to suppress hp warnings
//		R Seres dec 2 2003 Removed "c_prompt()" and replaced it with "c_prompt_len()."
//				c_prompt() used gets(), an unsafe function	
//		R Seres fixed some #ifdef defines to work on Linux
//		R Seres dec 12 2005 Added support for additional artifact bands
//	 
//		R Seres Feb 2007 Changed artifact compression so the artifact data can store how the artifact was marked
//										1 = No Artifact
//										0 = Brunner high freq rejection
//										-1 = By hand
//										-2 = Brunner low freq rejection
//		R Seres March 20 2007	Added reopen function for SpecEdit
//		R Seres April 4 2010 Added ability to store 2D artifacts, which have a time and band range which they affect. These are marked in SpecEdit
//      R Seres August 10 2021 Will process HD files, denoted by a numberic label
// Synopsis: Implementation of the SFileHeader and SFile classes
//
// MEMBER TABLE:
//
//      SFileHeader()
//      ~SFileHeader()
//      SFileHeader::init_header()
//      SFileHeader::dump()
//
//      SFile()
//      SFile(char* sfilename)
//      SFile(id,label,study)
//      SFile(bfile*,label)
//      ~SFile()dump
//      SFile::Exists(path)
//      SFile::Open(flag,data)
//      SFile::GetArtifact(index)
//      SFile::WriteHead(head,data)
//      SFile::ReadSpec(*data)
//      SFile::display_spec_params()
//      SFile::display_spec_params( SFile *)
//      SFile::WriteSpec(*data)
//      SFile::dump()
//      SFile::create_SFilename(id,label,study)
//      SFile::init_artifact()
//      SFile::write_header()
//      SFile::write_artifact()
//      SFile::read_header()
//      SFile::read_artifact()
//
//-----------------------------------------------------------------------------
 
#include "stdafx.h"
 


#include "sfile.h"
#include "local.h"
#include <memory.h>

#ifndef HPUX
#ifndef LINUX
#define WINDOWS_CP 1
#include <windows.h>
#include "TextCompatibility.h"
#endif
#endif

//-----------------------------------------------------------------------------------
// Function name:       SFileHeader
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class constructor
// Authors:             K.Batch
// Last modified:
//
// Effect on global (or member) variables:      Calls init_header.
//
SFileHeader::SFileHeader()
{
        init_header();
}

//-----------------------------------------------------------------------------------
// Function name:       ~SFileHeader
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class destructor
// Authors:             K.Batch
// Last modified:
//
// Effect on global (or member) variables:      Does nothing
//
SFileHeader::~SFileHeader()
{
}

//-----------------------------------------------------------------------------------
// Function name:       init_header
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function
// Authors:             K.Batch
// Last modified:
//
// Effect on global (or member) variables:      All header fields are set to zero or NULL
//
void SFileHeader::init_header()
{
        int i;

        sf_hdr_version = SFILE_HEADER_VERSION;
		memset(bf_filename, 0, sizeof(bf_filename));//this is just to make sure that the enitre
		//string is set to zero, even though we don't care what's after the string delimter
		//this is so that the header files will appear binarily identical on all systems
		//RJS july9th 2003
		memset(bf_label, 0, sizeof(bf_label));

        bf_study = 0;
        bf_channel = 0;
        bf_samples_per_sec = 0;
        bf_id = 0;
        bf_date = 0;
        bf_time = 0;
        chan_calampuv = 0.0f;

        art_rej_flag = 0;
        raw_epoch_len = 0;
        raw_epoch_cnt = 0;
        art_med_thresh = 0.0;
        art_med_pre_len = 0;
        art_med_post_len = 0;
        art_bin_low = 0.0f;
        art_bin_high = 0.0f;

        raws_per_spec = 0;
        spec_bin_count = 0;
        spec_windowing_flag = 0;
        spec_unit_flag = 0;
        spec_one_sided_flag = 0;

		number_of_additional_artifactbands = 0;

        for (i=0; i<MAX_BIN; i++)
        {
                spec_bin_low_freq[i] = 0.0f;
                spec_bin_high_freq[i] = 0.0f;
        }
        // New
        num_fft_pts = 0;
        num_samp_overlap = 0;
		data_acquisition_equipment = 0; //rjs added sept 16 2002
		device_scaled_to = 0;
		memset(junk, 0, sizeof(char) * FILLER_SIZE); //setting this to zero just like the
		//filenames above
		artifacts_uncompressed = 1;//Changed this to default uncompressed, which allows more complicated artifact marking. 
 
}

//-----------------------------------------------------------------------------------
// Function name:       dump
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class DEBUG member function
// Authors:             K.Batch
// Last modified:
//
// Effect on global (or member) variables: Dumps header fields to std err
//
void SFileHeader::dump()
{
        cerr << "\nVERSION " << sf_hdr_version;
        cerr << "\n\nBFILENAME " << bf_filename;
        cerr << "\nID " << bf_id << "\nSTUDY " << bf_study;
        cerr << "\nDATE " << bf_date << "\nTIME " << bf_time;
        cerr << "\nCHANNEL " << bf_channel << "\nLABEL " << bf_label;
        cerr << "\nCAL AMP " << chan_calampuv;
        cerr << "\nSAMPS/SEC " << bf_samples_per_sec;

        cerr << "\n\nSECS/EPOCH " << raw_epoch_len;
        cerr << "\nOVERLAPPED SAMPLES/EPOCH " << num_samp_overlap;
        cerr << "\nRAW EPOCH COUNT " << raw_epoch_cnt;
        
        cerr << "\n\nRAWS/SPEC " << raws_per_spec;
        cerr << "\nNUM FFT POINTS " << num_fft_pts; 
        cerr << "\nWINDOW " << spec_windowing_flag;
        cerr << "\nUNITS " <<  spec_unit_flag; 
        cerr << "\nSIDES " << spec_one_sided_flag;

        cerr << "\n\nART FLAG " << art_rej_flag;
        cerr << "\nART MED THRESH " << art_med_thresh; 
        cerr << "\nART PRE LENGTH " << art_med_pre_len;  
        cerr << "\nART POST LENGTH " << art_med_post_len;
        cerr << "\nART BIN LOW " << art_bin_low; 
        cerr << "\nART BIN HIGH " << art_bin_high;

        cerr << "\n\nBIN COUNT " << spec_bin_count;

		cerr << "\nDAQ " << (int)data_acquisition_equipment; //rjs added sept 17 2002
		cerr << "\nDEVICE SCALED TO " << (int)device_scaled_to;
        for (int i = 0; i < spec_bin_count; i++)
        {
                cerr << "\n" << i << "  LOW FREQ " << spec_bin_low_freq[i];
                cerr << "  HIGH FREQ " << spec_bin_high_freq[i];
        }
		cerr << "\nART COMPRESSED " << (artifacts_uncompressed != 1);
        cerr << endl << endl;
}

//-----------------------------------------------------------------------------------
// Function name:       SFile
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ overloaded class constructor
// Authors:             Batch,Hoffman,Vasko
// Last modified:
//
// Effect on global (or member) variables: Sets sfilnm to user specified name or name
// piped in through standard in. The artifact buffer is set to NULL. Use this constructor to
// read existent SFiles either interactively or in a pipe (Use for TSD file creation)
//
//
// TRANSITION: Sets the SFile object to its Idle State
//
SFile::SFile()
{
        strcpy(sfilnm, "");
        while (strcmp(sfilnm, "")==0)
        {
                if ((!c_prompt_len((char*)"\nEnter input sfilename: ",sfilnm, 80)))
                {
                        strcpy(sfilnm, "");
                        cerr << "%% No Input file specified.\n";
                        exit(-1);
                }
        }
        ABuffer = NULL;
}

//-----------------------------------------------------------------------------------
// Function name:       SFile
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ overloaded class constructor
// Authors:             Batch,Hoffman
// Last modified:
//
// Effect on global (or member) variables: Sets sfilnm to sfilename. The artifact buffer
// is set to NULL. Use this constructor to read existant sfiles.
//
//
// TRANSITION: Sets the SFile object to its Idle State
//
SFile::SFile(char* sfilename)
{
        strcpy(sfilnm,sfilename);
        ABuffer = NULL;
}

//-----------------------------------------------------------------------------------
// Function name:       SFile
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                Overloaded C++ class constructor
// Authors:             Batch
// Last modified:
//
// Effect on global (or member) variables: Sfilnm is constructed from parameter list.
// The artifact buffer is set to NULL.
//
//
// TRANSITION: Sets the SFile object to its Idle State
//
SFile::SFile(long ID, char label[], short study, char path[])
{
  create_SFilename( ID,label,study,path );
  ABuffer = NULL;
}

//-----------------------------------------------------------------------------------
// Function name:       SFile
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                Overloaded C++ class constructor
// Authors:             Batch
// Last modified:       Apr-08-1998 by Tim Hoffman (as suggested by JD). Modified 
//      This constructor will no longer exit if it finds an existing SFile. This 
//      change was driven by a scenario where Spectral is being run on CD-ROM which 
//      has Bfiles and their associated Sfiles present. The user may wish to create 
//      new Sfiles on the specified $SFILE directory which is other than the CD-ROM. 
//      Now, if the exisiting files are found in the local directory, they are just 
//      ignored, and new ones may be created on $SFILE.

// Effect on global (or member) variables: SFile header values specific to a bfile are
// set to the corresponding bfile header fields. sfilnm is built from the bfile header
// id,study and the parameter list label. The artifact buffer is set to NULL. The spectral
// parameter fileds are set. A new sfile for writing only is created.Use this constructor 
// to create SFiles from known BFiles. This function will always place the S file in the 
// same directory as the B file if it can. If it can't, it places it in the SFILE directory.  
// (Use only for Spectral)
//
// TRANSITION: Sets the SFile object directly to its Active State
//
SFile::SFile(bf_read_only* bfile, char* label)
{
  char pathname[80];
  short open = FALSE;

  bfile->get_bfilpath( pathname );
  if ( SetHeaderFields( bfile,label ) ){
      create_SFilename( bfile->bhdr.id,label,bfile->bhdr.study,pathname );
      if ( Exists() ){
	  // cerr << "%% SFILE " << sfilnm << " ALREADY EXISTS.\n";  commented out by Tim H. 8-Apr-98
	  // exit(-1); commented out by Tim H. 8-Apr-98
	} 
      else if ( Open(SF_WRITE) == 1 ) open = TRUE;
    }
  if (!open){
#ifdef HP
      strcpy( pathname,getenv("SFILE"));
      if ( pathname[0] != NULL ){
	  strcat( pathname, "/" );

#else
	  pathname[0] = '\0';
	  {
#endif

	  create_SFilename( bfile->bhdr.id,label,bfile->bhdr.study,pathname );
	  if ( Exists() ){
	      cerr << "%% $SFILE " << sfilnm << " ALREADY EXISTS.\n";
	   //   exit(-1);
	    }
	  else if ( Open(SF_WRITE) == 1 ) open = TRUE;
	}
    }
  if (!open){
      cerr << "%% UNABLE TO OPEN " << sfilnm << " FOR WRITING.\n";
   //   exit(-1);
    }
}

//-----------------------------------------------------------------------------------
// Function name:       ~SFile
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class destructor
// Authors:             Batch
// Last modified:
//
// Effect on global (or member) variables:      The artifact buffer memory is freed
//
SFile::~SFile()
{
  Close();
}

//-----------------------------------------------------------------------------------
// Function name:       Exists
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function
// Authors:             Batch, Hoffman, et al
// Last modified:
//
// Effect on global (or member) variables: Checks for existance of the sfilnm
// with optional path in the parameter list. Returns TRUE if found, otherwise 
// returns FALSE
//
short SFile::Exists(char* path)
{
        if (path != NULL)
        {
                char temp[80];
                sprintf(temp,"%s/%s",path,sfilnm);
                return file_exists(temp);
        }
        else return file_exists(sfilnm);
}

//-----------------------------------------------------------------------------------
// Function name:       Open
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function
// Authors:             Batch
// Last modified:
//
// Effect on global (or member) variables: After construction of an sfile object
// open the sfile with an appropriate flag, SF_WRITE for writing (Spectral processing),
// SF_READ for reading (TSDR or DELREM processing), or SF_RW for reading and writing
// (POLY processing). The fstream object will not allow reading of a write only stream
// and will not allow writing to a read only stream (the fail bit or bad bit will be set
// - the stream will not be good.) Both reading and writing may be performed on a read
// write stream.
//
// TRANSITION: Moves the SFile object from the Idle State to the Active State when it
// returns TRUE
//
short SFile::Open(short FLAG)
{
   
#ifdef LINUX
			sf.open(sfilnm, (std::_Ios_Openmode)FLAG);//linux does not allow a short here, 
			//has to be converted to a (std::_Ios_Openmode), a type that windows doesn't like
#else
		sf.open(sfilnm, FLAG);
#endif
 
		 
        if (!sf.good()) 
			return(-1);
		//the case statements have been removed since Linux won't allow the constants to be cast as integer types
        if(FLAG == (short)(SF_WRITE))
        {
			if (!init_artifact()) 
				return -2;  // SF_WRITE destroys any exisiting
	                              // then opens new Sfile for Write Only
		}
		else if(FLAG == (short)(SF_READ) || FLAG == (short)(SF_RW))
		{
                                             // SF_READ open Sfile Read Only
			if (!read_header()) 
				return -3;    // SF_RW opens Sfile Read/Write
			if (!read_artifact()) 
				return -4; 
		}
		else
			return 0;
        return 1;
}

short SFile::ReOpen(short FLAG)
{
   
#ifdef LINUX
			sf.open(sfilnm, (std::_Ios_Openmode)FLAG);//linux does not allow a short here, 
			//has to be converted to a (std::_Ios_Openmode), a type that windows doesn't like
#else
		sf.open(sfilnm, FLAG);
#endif
 
		 
        if (!sf.good()) 
			return(-1);
		//the case statements have been removed since Linux won't allow the constants to be cast as integer types
        		
        return 1;
}

//-----------------------------------------------------------------------------------
// Function name:       OpenSP
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function
// Authors:             Batch
// Last modified:
//
// Effect on global (or member) variables:
//
// TRANSITION: Moves the SFile object from the Idle State to an undefined state if
// it returns TRUE. It is used only to open the spectral parameter sfiles!!!!!
//
short SFile::OpenSP(short FLAG)
{
		if (FLAG == (short)(SF_READ))
			sf.open(sfilnm, SF_READ);
		else if (FLAG ==(short) (SF_RW))
			sf.open(sfilnm, SF_RW);
		else if (FLAG == (short)(SF_WRITE))
			sf.open(sfilnm, SF_WRITE);
        if (!sf.good()){
          cerr << "OPEN SFILE IS NOT GOOD.\n";
          return(FALSE);
        }
        if (FLAG != (short)(SF_WRITE))
          {
            if (!read_header())
              {
                cerr << "UNABLE TO READ HEADER.\n";
                return(FALSE);
              }
          }
        return(TRUE);
}

//-----------------------------------------------------------------------------------
// Function name:       Close
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function
// Authors:             Batch
// Last modified:
//
// Effect on global (or member) variables: Sets the ABuffer to NULL and flushes and closes
// the fstream object
//
// TRANSITION: Moves the SFile object from the Active State to the Idle State
//
void SFile::Close()
{
  if (ABuffer != NULL)
    { 
      sf.flush();                                            
      sf.close(); // flush and close the fstream object
      delete ABuffer; ABuffer = NULL; // remove ABuffer
      init_header(); // clear header
    };
}

short SFile::TempClose(short flag)
{ 

      sf.flush();                                            
      sf.close(); // flush and close the fstream object
	  return 1;
}

//-----------------------------------------------------------------------------------
// Function name:  GetArtifact
// Source file:    sfile.cpp
// Prototype file: sfile.h
// Type:           C++ overloaded class member function
// Authors:        Batch
// Last modified:
//
// Effect on global (or member) variables: Returns the short at index. If index is not
// in ABuffer, returns -1.
//
short SFile::GetArtifact(long index)
{
  if ( ABuffer == NULL || index >= raw_epoch_cnt || index < 0 ) return (-1);
  return ABuffer[index];
}

//-----------------------------------------------------------------------------------
// Function name:       WriteHead
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ member function
// Authors:             Batch
// Last modified:
//
// Effect on global (or member) variables: The default parameters are both true.
// Therefore using WriteHead() from the calling program will write both the header and
// the artifact buffer out to disk. For more control, the programmer may write out only
// the header or only the artifact buffer if they so desire. Returns TRUE if the write
// request was successfull, otherwise returns FALSE.
//
short SFile::WriteHead(short head,short artifact)
{
        short success = TRUE;

        if (head)
                success = write_header();
        if (artifact && success)
                success = write_artifact();
        return success;
}

//-----------------------------------------------------------------------------------
// Function name:       SetHeaderFields
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class overloaded member function
// Authors:             K.Batch
// Last modified:
//
// Effect on global (or member) variables: Sets spectral header fields
//
short SFile::SetHeaderFields(bf_read_only* bfile, char* label)
{
   int samples_per_epoch;
   int new_samples_per_epoch;

   strcpy(bf_label,label);
   for (int i = 0; i < bfile->bhdr.chan_cnt; i++) {
     if (strcmp(bf_label,bfile->bhdr.chans[i].label) == 0) bf_channel = i;
   }
   strcpy(bf_filename,bfile->bhdr.bfilename);
   bf_id = bfile->bhdr.id;
   bf_study = bfile->bhdr.study;
   bf_samples_per_sec = (int)(bfile->bhdr.chans[bf_channel].spm/60);
   bf_date = bfile->bhdr.date;
   bf_time = bfile->bhdr.time;
   // open and read sfile parameter file
   if (!GetSpecParams()) return(FALSE);
   
   samples_per_epoch = (int)((float)(bfile->bhdr.chans[bf_channel].spm/60)*raw_epoch_len);
   new_samples_per_epoch = samples_per_epoch-num_samp_overlap;

   if (bfile->bhdr.chans[bf_channel].sampl_count<samples_per_epoch)
     raw_epoch_cnt = 0;
   else
     raw_epoch_cnt = (long)(((bfile->bhdr.chans[bf_channel].sampl_count-
		     samples_per_epoch)/new_samples_per_epoch)+1);
   //rjs added sept 16 2002
   data_acquisition_equipment = bfile->bhdr.device_id + 1;
   
   return(TRUE);
}

//-----------------------------------------------------------------------------------
// Function name:       GetSpecParams
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class overloaded member function
// Authors:             K.Batch
// Last modified:
//
// Effect on global (or member) variables: Sets spectral header fields
//
short SFile::GetSpecParams(char* pfilename, unsigned short USE_DEFAULT)
{
  int i;
  char dummy[80];

  // If the local spec.param file doesn't exist use the default spec.param file if specified
  if (!file_exists(pfilename) && USE_DEFAULT) strcpy(pfilename,DEFAULT_PARAM);
#ifdef LINUX
	ifstream pfile(pfilename,ios::in);
#else
	ifstream pfile(pfilename,ios::in);
#endif
  if (!pfile.good())
  {
    cerr << "UNABLE TO OPEN SPECTRAL PARAMETER FILE - %s!\n" << pfilename;
    return(FALSE);
  }
  // read value then chew up the rest of the line
  pfile >> raw_epoch_len; pfile.get(dummy,80);
  pfile >> chan_calampuv; pfile.get(dummy,80);
  pfile >> spec_windowing_flag; pfile.get(dummy,80);
  pfile >> spec_bin_count; pfile.get(dummy,80);
  pfile >> spec_unit_flag; pfile.get(dummy,80);
  pfile >> spec_one_sided_flag; pfile.get(dummy,80);
  pfile >> raws_per_spec; pfile.get(dummy,80);
  pfile >> art_rej_flag; pfile.get(dummy,80);
  pfile >> art_med_pre_len; pfile.get(dummy,80);
  pfile >> art_med_post_len; pfile.get(dummy,80);
  pfile >> art_med_thresh; pfile.get(dummy,80);
  pfile >> art_bin_low >> art_bin_high; pfile.get(dummy,80);
  for (i=0; i < spec_bin_count; i++)
  {
    pfile >> spec_bin_low_freq[i] >> spec_bin_high_freq[i];
    pfile.get(dummy,80);
  }
  pfile >> num_fft_pts; pfile.get(dummy,80);
  pfile >> num_samp_overlap; pfile.get(dummy,80);
  return TRUE;
}

//-----------------------------------------------------------------------------------
// Function name:       GetSpecParams
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class overloaded member function
// Authors:             K.Batch
// Last modified:
//
// Effect on global (or member) variables: Sets spectral header fields
//
short SFile::GetSpecParams()
{
  int i;

  SFile *pfile = new SFile(LOCAL_SFILENAME);
  if (pfile == NULL){
        cerr << "NO MEMORY FOR LOCAL SFILE OBJECT.\n";
        return FALSE;
  }
  if (!pfile->Exists())
  {
        cerr << "NO LOCAL SPECTRAL PARAMETER FILE EXISTS.\n";
        delete pfile;
        SFile *pfile = new SFile(DEFAULT_SFILENAME);
        if (pfile == NULL){
          cerr << "NO MEMORY FOR DEFAULT SFILE OBJECT.\n";
          return FALSE;
        }
        if (!pfile->Exists()){
          cerr << "NO SPECTRAL PARAMETER FILE EXISTS!\n";
          delete pfile;
          return FALSE;
        }
  }
  if (!pfile->OpenSP(SF_READ))
  {
        cerr << "UNABLE TO OPEN " << pfile->sfilnm << " KIM!\n";
        delete pfile;
        return FALSE;
  }
  chan_calampuv = pfile->chan_calampuv;
  art_rej_flag = pfile->art_rej_flag;
  raw_epoch_len = pfile->raw_epoch_len;
  art_med_thresh = pfile->art_med_thresh;
  art_med_pre_len = pfile->art_med_pre_len;
  art_med_post_len = pfile->art_med_post_len;
  art_bin_low = pfile->art_bin_low;
  art_bin_high = pfile->art_bin_high;
  spec_windowing_flag = pfile->spec_windowing_flag;
  spec_unit_flag = pfile->spec_unit_flag;
  spec_one_sided_flag = pfile->spec_one_sided_flag;
  raws_per_spec = pfile->raws_per_spec;
  spec_bin_count = pfile->spec_bin_count;
  for (i=0; i < spec_bin_count; i++)
  {
       spec_bin_low_freq[i] = pfile->spec_bin_low_freq[i];
       spec_bin_high_freq[i] = pfile->spec_bin_high_freq[i];
  }
  num_fft_pts = pfile->num_fft_pts;
  num_samp_overlap = pfile->num_samp_overlap;


  device_scaled_to = pfile->device_scaled_to;
  number_of_additional_artifactbands = pfile->number_of_additional_artifactbands;
  if(number_of_additional_artifactbands > MAX_ART_BANDS)
	  number_of_additional_artifactbands = MAX_ART_BANDS;
  for(i = 0; i < number_of_additional_artifactbands; i++)
  {
	  additional_art_bin_low[i] = pfile->additional_art_bin_low[i];	 
	  additional_art_bin_high[i] = pfile->additional_art_bin_high[i];	        
	  additional_art_medthresh[i] = pfile->additional_art_medthresh[i];
	  additional_art_pre_len[i] = pfile->additional_art_pre_len[i];
	  additional_art_post_len[i] = pfile->additional_art_post_len[i];
  }
  delete pfile;
  return TRUE;
}

//---------------------------------------------------------------------------------
// Function name:  ReadSpec
// Source file:    sfile.cpp
// Prototype file: sfile.h
// Type:           C++ class member function
// Authors:        Batch, Vasko
// Last modified:
//
// Effect on global (or member) variables: Reads a single epoch of spectral data
// from the fstream buffer. Returns TRUE if read was successful, else returns FALSE.
// The caller is responsible for correct fstream position placement. Call after
// opening an SFile as read or read/write; the position will be at the byte following
// the header structures, or call after a call to ReadSpec(*data,date,time).
//
short SFile::ReadSpec(float* data)
{
  if(sf.fail())
	  return 0;
  sf.read((char*)data,(sizeof(float)*spec_bin_count));
#ifndef HPUX
  for(int index = 0; index < spec_bin_count; index++)
	switch_bytes_hp((unsigned char*)(&data[index]), sizeof(float));
#endif
  return sf.good();
}

//---------------------------------------------------------------------------------
// Function name:  WriteSpec
// Source file:    sfile.cpp
// Prototype file: sfile.h
// Type:           C++ class member function
// Authors:        Batch, Vasko
// Last modified:
//
// Effect on global (or member) variables: Writes a single epoch of data to the
// fstream buffer. It will flush to disk when it becomes full or when the sfile is
// closed. Returns TRUE is write was successful, else returns FALSE. The caller is
// responsible for correct fstream position placement. Call after writing an SFile
// header and ABuffer; the position will be at the byte following these header
// structures.
//
short SFile::WriteSpec(float* data)
{
#ifndef HPUX
	//need to switch the bytes around when writing on a windows system,
	//since hp writes out with a differeny lo/hi order
	float temp;
	for(int index = 0; index < spec_bin_count; index++)
	{
		temp = data[index];
		switch_bytes_hp((unsigned char *)&temp, sizeof(float) );
		sf.write((const char*)&temp, sizeof(float));
	}
#else
  sf.write((const char*)data,(sizeof(float)*spec_bin_count));
#endif
  return sf.good();
}

//---------------------------------------------------------------------------------
// Function name:       dump_all
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function
// Authors:             Batch,Vasko
// Last modified:
//
// Effect on global (or member) variables: Dumps out header, artifact, and spectral
// data to standard err. This function should only be used on read or read write
// sfiles. Use dump to display the header values of a write only sfile. Artifact
// and spectral data can become quite extensive - use dump_all in debug only.
//
void SFile::dump_all()
{
  SFileHeader::dump(); // Dump the header
  dump_artifact(); // Dump the artifact buffer
  dump_spectral(); // Dump the spectral data from disk
}

//---------------------------------------------------------------------------------
// Function name:       dump_artifact
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function
// Authors:             Batch,Vasko
// Last modified:
//
// Effect on global (or member) variables: Dumps out artifact to standard err.
//
void SFile::dump_artifact()
{
  for (int i = 0; i < raw_epoch_cnt; )
  {
    for (int k = 0; k < 3 /*three columns*/; k++)
      {
        for ( int j = 0; j < raws_per_spec && i < raw_epoch_cnt; j++)
             cerr << ABuffer[i++];
        cerr << "  ";
      }
    cerr << endl;
  }
}
//---------------------------------------------------------------------------------
// Function name:       dump_artifact
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function
// Authors:             seres
// Last modified:
//
// Effect on global (or member) variables: Dumps out artifact to file
//
void SFile::dump_artifact(char artfile[])
{
	FILE *fptr = fopen(artfile, "w");
	fprintf(fptr, "%d %d ", bf_id, bf_study);
	fprintf(fptr, "%s ", bf_label);
	fprintf(fptr, "%d ", raw_epoch_cnt);
	for (int i = 0; i < raw_epoch_cnt; i++ )
	{
		fprintf(fptr, "%d ", ABuffer[i]);
	}
	fclose(fptr);
}

//---------------------------------------------------------------------------------
// Function name:       dump_all
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function
// Authors:             Batch,Vasko
// Last modified:
//
// Effect on global (or member) variables: Dumps out header, artifact, and spectral
// data to standard err. This function should only be used on read or read write
// sfiles. Use dump to display the header values of a write only sfile. Artifact
// and spectral data can become quite extensive - use dump_all in debug only.
//
void SFile::dump_spectral()
{
  float *sdata;
  streampos oldpos;
  int i, j = 0;

  cerr << endl << setiosflags( ios::fixed ) << setprecision(4); // prepare stream
  sdata = (float*)malloc(sizeof(float)*(int)spec_bin_count);
  oldpos = sf.tellg();
 // sf.seekg(dpos); // seek to the data position in the sfile - just in case
  while (ReadSpec(sdata))
    {
      for ( i = 0; i < spec_bin_count; i++, j++)
		  cerr << sdata[i] << endl;
      cerr << endl;
    }
  // cerr << "# SPECTRA READ = " << j << endl;
  free(sdata);
  sf.seekg(oldpos);
}

//-----------------------------------------------------------------------------------
// Function name:       SFile::DisplaySpecParams
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function: OVERLOADED
// Authors:             Batch,Vasko,converted to class funct by Tim H
// Last modified:       May 7, 1997
//
// Effect on global (or member) variables: Displays only those sfile header vars that
// pertain to the spectral analysis.

void SFile::DisplaySpecParams(SFile *def)
{
	printf("Scale Spectra as if Acquired On ");
	switch(device_scaled_to)
	{
	case 0:
		printf("UNKNOWN\n");
		break;
	case 1:
		printf("RECORD_PC \n");
		break;
	case 2:
		printf("HARMONIE \n");
		break;
	}


  printf("Number of epochs per average {1=no averaging}: %1d", raws_per_spec);
  printf(" (%1d)\n",def->raws_per_spec);

  printf("Artifact rejection {0=none, 1=segmented median}: %1d", art_rej_flag);
  printf(" (%1d)\n",def->art_rej_flag);

  printf("Epoch length [in seconds]: %3.1f",raw_epoch_len);
  printf(" (%3.1f)\n",def->raw_epoch_len);

  printf("Number of FFT points: %1u",num_fft_pts);
  printf(" (%1u)\n",def->num_fft_pts);

  printf("Number of overlapped samples per epoch: %1u",num_samp_overlap);
  printf(" (%1u)\n",def->num_samp_overlap);

  printf("Spectral window {0=rectangular, 1=Hamming}: %1d",spec_windowing_flag);
  printf(" (%1d)\n",def->spec_windowing_flag);

  printf("Spectral units {0=(v^2)/Hz, 1=(v^2)/octave, 2=(v^2)}: %1d", spec_unit_flag);
  printf(" (%1d)\n",def->spec_unit_flag);

  printf("Type of spectrum {1=one-sided, 2=two-sided}: %1d",spec_one_sided_flag);
  printf(" (%1d)\n",def->spec_one_sided_flag);

  printf("Number of frequency bins: %1d",spec_bin_count);
  printf(" (%3d)\n",def->spec_bin_count);

  printf("Frequency bins: %4.2fHz ",1/def->raw_epoch_len);
  printf("(%4.2fHz) ",1/def->raw_epoch_len);

  printf("wide from %4.2fHz ",def->spec_bin_low_freq[0]);
  printf("(%4.2fHz) ",def->spec_bin_low_freq[0]);

  printf("to %5.2fHz ",spec_bin_low_freq[def->spec_bin_count-1]);
  printf("(%5.2fHz)\n",def->spec_bin_low_freq[def->spec_bin_count-1]);

  if (art_rej_flag==1) {
    printf("Artifact rejection bin: %5.2fHz - %5.2fHz", art_bin_low,def->art_bin_high);
    printf(" (%5.2fHz - %5.2fHz)\n", def->art_bin_low,def->art_bin_high);

    printf("/Art rej/ local window preceding epoch [in epochs]: %2d", art_med_pre_len);
    printf(" (%2d)\n",def->art_med_pre_len);

    printf("/Art rej/ local window following epoch [in epochs]: %2d", art_med_post_len);
    printf(" (%2d)\n",def->art_med_post_len);

    printf("/Art rej/ median threshold: %3.1f",art_med_thresh);
    printf(" (%3.1f)\n",def->art_med_thresh);
   }

  if(def->number_of_additional_artifactbands)
  {
	  printf("Additional artifact rejection bands: %d\n", def->number_of_additional_artifactbands);
	int index;
	for(index = 0; index < def->number_of_additional_artifactbands; index++)
	{
		printf("Band %d from %4.2fHz to %4.2fHz\n", index, def->additional_art_bin_low[index], def->additional_art_bin_high[index]);
		printf("   median threshold %4.2fHz \n", def->additional_art_medthresh[index]);
		printf("   local window preceding epoch [in epochs]: %2d\n", def->additional_art_pre_len[index]);
		printf("   local window following epoch [in epochs]: %2d\n", def->additional_art_post_len[index]);
	}
  }
  else
		printf("No additional artifact rejection.\n");
  printf("Amplitude of calibration signal [in microvolts]: %4.1f",
          def->chan_calampuv);
  printf(" (%4.1f)\n",def->chan_calampuv);


 
}

//-----------------------------------------------------------------------------------
// Function name:       SFile::DisplaySpecParams
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function: OVERLOADED
// Authors:             Batch,Vasko,converted to class funct by Tim H
// Last modified:       May 7, 1997
//
// Effect on global (or member) variables: Displays only those sfile header vars that
// pertain to the spectral analysis.

void SFile::DisplaySpecParams()
{
	printf("Scale Spectra as if Acquired On ");
	switch(device_scaled_to)
	{
	case 0:
		printf("UNKNOWN\n");
		break;
	case 1:
		printf("RECORD_PC \n");
		break;
	case 2:
		printf("HARMONIE \n");
		break;
	}

  printf("Number of epochs per average {1=no averaging}: %1d\n", raws_per_spec);
  printf("Artifact rejection {0=none, 1=segmented median}: %1d\n", art_rej_flag);
  printf("Epoch length [in seconds]: %3.1f\n",raw_epoch_len);
  printf("Number of FFT points: %1u\n",num_fft_pts);
  printf("Number of overlapped samples per epoch: %1u\n",num_samp_overlap);
  printf("Spectral window {0=rectangular, 1=Hamming}: %1d\n",spec_windowing_flag);
  printf("Spectral units {0=(v^2)/Hz, 1=(v^2)/octave, 2=(v^2)}: %1d\n", spec_unit_flag);
  printf("Type of spectrum {1=one-sided, 2=two-sided}: %1d\n",spec_one_sided_flag);
  printf("Number of frequency bins: %1d\n",spec_bin_count);
  printf("Frequency bins: %4.2fHz\n",1/raw_epoch_len);
  printf("wide from %4.2fHz\n",spec_bin_low_freq[0]);
  printf("to %5.2fHz\n",spec_bin_low_freq[spec_bin_count-1]);
 
  if (art_rej_flag==1) {
    printf("Artifact rejection bin: %5.2fHz - %5.2fHz\n", art_bin_low,art_bin_high);
    printf("/Art rej/ local window preceding epoch [in epochs]: %2d\n", art_med_pre_len);
    printf("/Art rej/ local window following epoch [in epochs]: %2d\n", art_med_post_len);
    printf("/Art rej/ median threshold: %3.1f\n",art_med_thresh);
    }

    if(number_of_additional_artifactbands)
  {
	  printf("Additional artifact rejection bands:\n");
	int index;
	for(index = 0; index < number_of_additional_artifactbands; index++)
	{
		printf("Band %d from %4.2f Hz to %4.2f Hz\n", index, additional_art_bin_low[index], additional_art_bin_high[index]);
		printf("   median threshold %4.2fHz \n",additional_art_medthresh[index]);
		printf("   local window preceding epoch [in epochs]: %2d\n", additional_art_pre_len[index]);
		printf("   local window following epoch [in epochs]: %2d\n", additional_art_post_len[index]);
	}
  }
	else
		printf("No additional artifact rejection.\n");
  printf("Amplitude of calibration signal [in microvolts]: %4.1f\n",chan_calampuv);
 
}

#include <math.h>
//-----------------------------------------------------------------------------------
// Function name:       SFile::ModifySpecParams
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class member function: OVERLOADED
// Authors:             Batch,Vasko,converted to class funct by Tim H
// Last modified:       May 7, 1997
//
// Effect on global (or member) variables: Displays only those sfile header vars that
// pertain to the spectral analysis.
// MODIFY_SPEC_PARAMS

void SFile::ModifySpecParams(short *modified)
{
  float bin_width;
  int i,ok;
  char response[80];

  // double float temporary vars for corresponding single float header fields
  // needed because you can't pass floats into a C routine that is auto promoting floats to doubles

  double dbl_raw_epoch_len,
    dbl_spec_bin_low_freq,
    dbl_spec_bin_high_freq,
    dbl_art_bin_low,
    dbl_art_bin_high,
    dbl_art_med_thresh,
    dbl_chan_calampuv;

  short temp_scale;

  if (get_short((char*)"Scale spectra as if acquired on { 1=RecordPc, 2=Harmonie} > ",1,2,&temp_scale))
  {
	device_scaled_to = (char)temp_scale;
    *modified=TRUE;
  }

  if (get_short((char*)"New number of epochs per average {1=no averaging} > ",1,100,&raws_per_spec))
    *modified=TRUE;

  if (get_short((char*)"New artifact rejection {0=none, 1=segmented median, 2=visual} > ", 0,2,&art_rej_flag))
    *modified=TRUE;

  if (get_double((char*)"New epoch length [in seconds]  > ",1.0,500.0, &dbl_raw_epoch_len))
    {
     *modified=TRUE;
      raw_epoch_len = (float)dbl_raw_epoch_len;
    }

  printf("Assuming a 128Hz sampling rate, the raw epoch contains %.0f samples.\n", raw_epoch_len*128.0);
  ok=0;
  while (!ok)
    {
    if (get_unsigned_short((char*)"New number of FFT points (must be power of 2\n\tand at least as many samples as raw epoch) > ",
	(unsigned short)1,(unsigned short)65535,&num_fft_pts))
      {
      /*if ((float)num_fft_pts<raw_epoch_len*128.0)//should probably get rid of this
	{
	printf("Error: must have at least as many FFT points\n");
	printf("       as there are samples in the raw epoch.\n");
	}
      else */
	   if ((log((float)num_fft_pts)/log(2.0))>floor(log((float)num_fft_pts)/log(2.0)))//rjs may 22 2002 had to type cast for hp3 compile
	     printf("Error: must be a power of 2\n");
	   else
	        {
		ok=1; 
		*modified=TRUE;
	        }
      }
    else ok=1;
    }

  if (get_unsigned_short((char*)"New number of overlapped samples per epoch > ",(unsigned short)0,(unsigned short)(num_fft_pts-1),&num_samp_overlap))
    *modified=TRUE;

  if (get_short((char*)"New spectral window {0=rectangular, 1=Hamming} > ",0,1,&spec_windowing_flag))
    *modified=TRUE;

  if (get_short((char*)"New spectral units {0=(v^2)/Hz, 1=(v^2)/octave, 2=(v^2)} > ",0,2,&spec_unit_flag))
    *modified=TRUE;

  if (get_short((char*)"New type of spectrum {1=one-sided, 2=two-sided} > ",1,2,&spec_one_sided_flag))
    *modified=TRUE;

  bin_width=(float)(1.0/raw_epoch_len);
  if (get_short((char*)"New number of frequency bins > ",1,MAX_BIN,&spec_bin_count))
    {
      *modified=TRUE;
      sprintf(response,"y");
    }
  else c_prompt_len((char*)"New frequency bins? If yes, enter 'y' > ",response, 80);
  if(!strcmp(response,"Y") || !strcmp(response,"y"))
    {
      for (i=0; i<spec_bin_count; i++)
         {
           printf("Bin %1d:  %4.3f - %4.3f\n",i+1,spec_bin_low_freq[i],spec_bin_high_freq[i]);
           ok=0;
           while (!ok)
             {
               if (get_double((char*)"New low frequency [in Hz]   > ",0.0,1000000.0, &dbl_spec_bin_low_freq))
                 {
                 ok=1; *modified=TRUE;
                 spec_bin_low_freq[i] = (float)dbl_spec_bin_low_freq;
                 }
               else ok=1;
             }
           ok=0;
           while (!ok)
            {
              if (get_double((char*)"New high frequency [in Hz]   > ", spec_bin_low_freq[i],
                             1000000.0, &dbl_spec_bin_high_freq))
                {
                ok=1; *modified=TRUE;
                spec_bin_high_freq[i]=(float)dbl_spec_bin_high_freq;
                }
              else ok=1;
            }
         } // FOR I = 0 to SPEC BIN CNT
    } // IF USER TYPES "Y" to new freq bins?

  if (art_rej_flag==1)
    {
      printf("Artifact rejection bin: %5.2fHz - %5.2fHz\n",art_bin_low,art_bin_high);
      c_prompt_len((char*)"New artifact bin? If yes, enter 'y' > ",response, 80);
      if (!strcmp(response,"Y") || !strcmp(response,"y"))
        {
          ok=0;
          while (!ok)
            {
             if (get_double((char*)"New low frequency [in Hz]   > ", spec_bin_low_freq[0],
                            spec_bin_low_freq[spec_bin_count-1], &dbl_art_bin_low))
                {
                  if ((dbl_art_bin_low/bin_width)-floor(dbl_art_bin_low/bin_width)==0)
                    { ok=1; *modified=TRUE;
                      art_bin_low = (float)dbl_art_bin_low;
                    }
                  else printf("Error: must be a multiple of %4.2f\n",bin_width);
                }
             else ok=1;
            }
          ok=0;
          while (!ok)
            {
              if (get_double((char*)"New high frequency [in Hz]   > ",art_bin_low+bin_width,
                             spec_bin_high_freq[spec_bin_count-1], &dbl_art_bin_high))
                {
                  if ((dbl_art_bin_high/bin_width)-floor(dbl_art_bin_high/bin_width)==0)
                    {ok=1; *modified=TRUE;
                     art_bin_high = (float)dbl_art_bin_high;
                    }
                  else printf("Error: must be a multiple of %4.2f\n",bin_width);
                }
              else ok=1;
            }
        } // IF ARTIFACT REJECTION FLAG
      if (get_short((char*)"New art rej local window preceding epoch [in epochs] > ",
                     1,MAX_BIN,&art_med_pre_len))
         *modified=TRUE;

      if (get_short((char*)"New art rej local window following epoch [in epochs] > ",
                     1,MAX_BIN,&art_med_post_len))
      *modified=TRUE;
      if ( get_double((char*)"New art rej median threshold     > ",0.0,100000.0,&dbl_art_med_thresh) )
        {
         *modified=TRUE;
          art_med_thresh = (float)dbl_art_med_thresh;
        }

   }
	if(art_rej_flag != 0)
	{

		if (get_short((char*)"New number of additional artifact bands > ", 0,10,&number_of_additional_artifactbands))
			*modified=TRUE;
		  if(number_of_additional_artifactbands)
		  {
			  int index;
			  for(index = 0; index < number_of_additional_artifactbands; index++)
			  {
				  printf("For rejection band %d:\n", index);
				  if (get_float((char*)"New low frequency [in Hz]   > ", 0,
									100.0f, &additional_art_bin_low[index]))
				  {                 
					*modified=TRUE;                                                  
				  }
				  if (get_float((char*)"New High frequency [in Hz]   > ", 0,
									100.0f, &additional_art_bin_high[index]))
				  {                 
					*modified=TRUE;                                                  
				  }
				  if (get_float((char*)"New median threshold [in Hz]   > ", 0,
									100000.0f, &additional_art_medthresh[index]))
				  {                 
					*modified=TRUE;                                                  
				  }
				  if (get_short((char*)"New art rej local window preceding epoch [in epochs]> ",
                     1, 45, &additional_art_pre_len[index]))
				  {
						  *modified=TRUE;
				  }
				  if (get_short((char*)"New art rej local window following epoch [in epochs]> ",
                     1, 45, &additional_art_post_len[index]))
				  {
						  *modified=TRUE;
				  }
			  }
		  }

	}
  
  if (get_double((char*)"New calibration signal amplitude [in microvolts]  > ",1.0,100.0,&dbl_chan_calampuv))
    {
      *modified=TRUE;
       chan_calampuv = (float)dbl_chan_calampuv;
    }

  return;
}

//---------------------------------------------------------------------------------
// Private Functions

//---------------------------------------------------------------------------------
// Function name:       create_SFilename
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ private member function
// Authors:             Vasko, Batch
// Last modified:
//
// Effect on global (or member) variables: Creates the sfile name for the 
// specified pathname
//
void SFile::create_SFilename(long ID,char* label,short study,char* path)
{
  char code[8], *unknown = (char*)"?";

#ifndef WINDOWS_CP	 //rjs added sept 16 2002, since windows doesn't like the brackets
	GetPrivateProfileString( "[CHANCODES]", label, unknown, code, 8, SCC_INI );
#else
#ifdef _WC
  WCHAR wlabel[256];
  WCHAR wunknown[256];
  WCHAR wcode[4];

  GetPrivateProfileString( L"CHANCODES", wlabel, wunknown, wcode, 8, WSCC_INI );
  strcpy(label, MakeTextSmall(wlabel));
  strcpy(unknown, MakeTextSmall(wunknown));
  strcpy(code, MakeTextSmall(wcode));
#else
  if (label[0] == 'H')
      sprintf(code, "%s", label);
  else {

      GetPrivateProfileString("CHANCODES", label, unknown, code, 8, SCC_INI);
      code[1] = '\0';
  }
#endif
#endif
  if (!strcmp( code, unknown )) {
    fprintf( stderr,"Error: symbol <%s> not found in %s file.\n",label,SCC_INI);
   // exit(-1);//We have to keep going if we don't recognize the code, since new channels are always popping up. 
  }
  if (path[0] != '\0')
  {
	  int length = strlen(path);
	  //RJS  march 14 2003 checked to see if there is a trailing slash in the path name, 
	  //else we add one, so the root directory does not get mixed in with the filename
#ifdef WINDOWS_CP
	
	if(path[length - 1] != '\\' && path[length - 1] != ':' )
		strcat(path,"\\");
#else
	if(path[length - 1] != '/')
		strcat(path,"/");
#endif
    sprintf( sfilnm,"%ss%6.6ld%s.%3.3d",path,ID,code ,study );
  }
  else
    sprintf( sfilnm,"s%6.6ld%s.%3.3d",ID,code,study );

}

void _create_SFilename(long ID,char* label,short study,char* path, char sfilnm[])
{
  char code[4], *unknown = (char*)"?";

#ifndef WINDOWS_CP	 //rjs added sept 16 2002, since windows doesn't like the brackets
	GetPrivateProfileString( "[CHANCODES]", label, unknown, code, 2, SCC_INI );
#else

#ifdef _WC
  WCHAR wlabel[256];
  WCHAR wunknown[256];
  WCHAR wcode[4];

  GetPrivateProfileString(L"CHANCODES", wlabel, wunknown, wcode, 2, WSCC_INI);
  strcpy(label, MakeTextSmall(wlabel));
  strcpy(unknown, MakeTextSmall(wunknown));
  strcpy(code, MakeTextSmall(wcode));
#else
	GetPrivateProfileString( "CHANCODES", label, unknown, code, 2, SCC_INI );
#endif
#endif
  if (!strcmp( code, unknown )) {
    fprintf( stderr,"Error: symbol <%s> not found in %s file.\n",label,SCC_INI);
   // exit(-1);//We have to keep going if we don't recognize the code, since new channels are always popping up. 
  }
  if (path[0] != '\0')
  {
	  int length = strlen(path);
	  //RJS  march 14 2003 checked to see if there is a trailing slash in the path name, 
	  //else we add one, so the root directory does not get mixed in with the filename
#ifdef WINDOWS_CP
	
	if(path[length - 1] != '\\' && path[length - 1] != ':' )
		strcat(path,"\\");
#else
	if(path[length - 1] != '/')
		strcat(path,"/");
#endif
    sprintf( sfilnm,"%ss%6.6ld%1c.%3.3d",path,ID,code[0],study );
  }
  else
    sprintf( sfilnm,"s%6.6ld%1c.%3.3d",ID,code[0],study );
}
//---------------------------------------------------------------------------------
// Function name:       init_artifact
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class private member function
// Authors:             Batch,Vasko
// Last modified:
//
// Effect on global (or member) variables: The memory for the artifact buffer is
// allocated according to the raw_epoch_cnt. This value must be greater than zero.
// All of the characters in the buffer are initialized to 1 - no artifact. Returns
// FALSE if the raw epoch count has not been set or if memory can not be allocated.
//
short SFile::init_artifact()
{
        if (raw_epoch_cnt == 0) return FALSE;
        ABuffer = (short*)malloc(sizeof(short)*(int)raw_epoch_cnt);//rjs Artifact data is now signed, so anything above 0 is good
        if (ABuffer == NULL ) return FALSE;
        for (int i = 0; i < raw_epoch_cnt; i++) ABuffer[i] = 1;
        return TRUE;
}

short SFile::write_header()	// divided into two function to allow for byte switching cconrad 05102002
{
#ifdef HPUX
	return write_header_hp();
#else
	return write_header_pc();
#endif
}

//-----------------------------------------------------------------------------------
// Function name:       write_header_hp
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class priveate member function
// Authors:             Batch
// Last modified:
//
// Effect on global (or member) variables: Writes header fields, one at a time, to
// disk. Returns TRUE when all fstream operators are successful, otherwise it returns
// FALSE
//
short SFile::write_header_hp()
{
        sf.seekp(0,ios::beg);
        sf.write((const char*)&sf_hdr_version, sizeof(float));
        sf.write(bf_filename, 16);
        sf.write(bf_label, LABSIZ);
        sf.write((const char*)&bf_study, sizeof(short));
        sf.write((const char*)&bf_channel, sizeof(short));
        sf.write((const char*)&bf_samples_per_sec, sizeof(short));
        sf.write((const char*)&bf_id, sizeof(long));
        sf.write((const char*)&bf_date, sizeof(long));
        sf.write((const char*)&bf_time, sizeof(long));
        sf.write((const char*)&chan_calampuv, sizeof(float));

        sf.write((const char*)&art_rej_flag, sizeof(short));
        sf.write((const char*)&raw_epoch_len, sizeof(float));
        sf.write((const char*)&raw_epoch_cnt,  sizeof(long));
        sf.write((const char*)&art_med_thresh, sizeof(float));
        sf.write((const char*)&art_med_pre_len, sizeof(short));
        sf.write((const char*)&art_med_post_len, sizeof(short));
        sf.write((const char*)&art_bin_low, sizeof(float));
        sf.write((const char*)&art_bin_high, sizeof(float));

        sf.write((const char*)&raws_per_spec, sizeof(short));
        sf.write((const char*)&spec_bin_count, sizeof(short));
        sf.write((const char*)&spec_windowing_flag, sizeof(short));
        sf.write((const char*)&spec_unit_flag, sizeof(short));
        sf.write((const char*)&spec_one_sided_flag, sizeof(short));
        for ( int i = 0; i < MAX_BIN; i++ )
        {
                sf.write((const char*)&spec_bin_low_freq[i], sizeof(float));
                sf.write((const char*)&spec_bin_high_freq[i], sizeof(float));
        }
        // New
        sf.write((const char*)&num_fft_pts, sizeof(unsigned short));
        sf.write((const char*)&num_samp_overlap, sizeof(short));
		sf.write((const char*)&data_acquisition_equipment, sizeof(unsigned char));//rjs added sept 16 2002
		sf.write((const char*)&device_scaled_to, sizeof(unsigned char));
		sf.write((const char*)&number_of_additional_artifactbands, sizeof(short));


		for(int index = 0; index < MAX_ART_BANDS; index++)
		{
			sf.write((const char*)&additional_art_bin_low[index], sizeof(float));
			sf.write((const char*)&additional_art_bin_high[index], sizeof(float));					
			sf.write((const char*)&additional_art_medthresh[index], sizeof(float));          			
			sf.write((const char*)&additional_art_pre_len[index], sizeof(short));           			
			sf.write((const char*)&additional_art_post_len[index], sizeof(short));		
		}
		sf.write((const char*)&artifacts_uncompressed, sizeof(char));//RJS added compression flag, Feb 2007

		//RJS ADDED For 2d Artifacts March 2009
		sf.write((const char*)&NumberOf2DArtifacts, sizeof(short));	
		for(int index = 0; index < MAX_2D_ARTIFACTS; index++)
		{
	 
			sf.write((const char*)&Artifacts2D[index].StartBand, sizeof(float));
			sf.write((const char*)&Artifacts2D[index].EndBand, sizeof(float));
			sf.write((const char*)&Artifacts2D[index].StartSegment, sizeof(short));	
			sf.write((const char*)&Artifacts2D[index].EndSegment, sizeof(short));	
		}

        sf.write(junk,FILLER_SIZE);
        sf.flush();
        return sf.good();
}

//-----------------------------------------------------------------------------------B
// Function name:       write_header_pc
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class priveate member function
// Authors:             Batch
// Last modified:
//
// Effect on global (or member) variables: Writes header fields, one at a time, to
// disk. Returns TRUE when all fstream operators are successful, otherwise it returns
// FALSE
//

short SFile::write_header_pc()
{
		float f;
		short s;
		long l;
        sf.seekp(0,ios::beg);
		f = sf_hdr_version;
		switch_bytes_hp((unsigned char*)&f, sizeof(float));
        sf.write((const char*)&f, sizeof(float));
        sf.write(bf_filename, 16);
        sf.write(bf_label, LABSIZ);
		s = bf_study;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		s = bf_channel;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		s = bf_samples_per_sec;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		l = bf_id;
		switch_bytes_hp((unsigned char*)&l, sizeof(long));
        sf.write((const char*)&l, sizeof(long));
		l = bf_date;
		switch_bytes_hp((unsigned char*)&l, sizeof(long));
        sf.write((const char*)&l, sizeof(long));
		l = bf_time;
		switch_bytes_hp((unsigned char*)&l, sizeof(long));
        sf.write((const char*)&l, sizeof(long));
		f = chan_calampuv;
		switch_bytes_hp((unsigned char*)&f, sizeof(float));
        sf.write((const char*)&f, sizeof(float));
		s = art_rej_flag;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		f = raw_epoch_len;
		switch_bytes_hp((unsigned char*)&f, sizeof(float));
        sf.write((const char*)&f, sizeof(float));
		l = raw_epoch_cnt;
		switch_bytes_hp((unsigned char*)&l, sizeof(long));
        sf.write((const char*)&l, sizeof(long));
		f = art_med_thresh;
		switch_bytes_hp((unsigned char*)&f, sizeof(float));
        sf.write((const char*)&f, sizeof(float));
		s = art_med_pre_len;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		s = art_med_post_len;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		f = art_bin_low;
		switch_bytes_hp((unsigned char*)&f, sizeof(float));
        sf.write((const char*)&f, sizeof(float));
		f = art_bin_high;
		switch_bytes_hp((unsigned char*)&f, sizeof(float));
        sf.write((const char*)&f, sizeof(float));
		s = raws_per_spec;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		s = spec_bin_count;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		s = spec_windowing_flag;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		s = spec_unit_flag;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		s = spec_one_sided_flag;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
        for ( int i = 0; i < MAX_BIN; i++ )
        {
				f = spec_bin_low_freq[i];
				switch_bytes_hp((unsigned char*)&f, sizeof(float));
				sf.write((const char*)&f, sizeof(float));
                f = spec_bin_high_freq[i];
				switch_bytes_hp((unsigned char*)&f, sizeof(float));
				sf.write((const char*)&f, sizeof(float));
        }
        // New
		s = num_fft_pts;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		s = num_samp_overlap;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));

		s = data_acquisition_equipment;//rjs added sept 16 2002
		switch_bytes_hp((unsigned char*)&s, sizeof(unsigned char));
        sf.write((const char*)&s, sizeof(unsigned char));
		s = device_scaled_to;
		switch_bytes_hp((unsigned char*)&s, sizeof(unsigned char));
        sf.write((const char*)&s, sizeof(unsigned char));
		s = number_of_additional_artifactbands;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
        sf.write((const char*)&s, sizeof(short));
		for(int index = 0; index < MAX_ART_BANDS; index++)
		{
			f = additional_art_bin_low[index];
			switch_bytes_hp((unsigned char*)&f, sizeof(float));
			sf.write((const char*)&f, sizeof(float));
			f = additional_art_bin_high[index];
			switch_bytes_hp((unsigned char*)&f, sizeof(float));
			sf.write((const char*)&f, sizeof(float));
			f = additional_art_medthresh[index];
			switch_bytes_hp((unsigned char*)&f, sizeof(float));
			sf.write((const char*)&f, sizeof(float));
            s = additional_art_pre_len[index];
			switch_bytes_hp((unsigned char*)&s, sizeof(short));
			sf.write((const char*)&s, sizeof(short));
            s = additional_art_post_len[index];
			switch_bytes_hp((unsigned char*)&s, sizeof(short));
			sf.write((const char*)&s, sizeof(short));		
		}
		sf.write((const char*)&artifacts_uncompressed, sizeof(char));
//This saves out the 2D artifact data. Each one is s range of frequency band and segment times
		s = NumberOf2DArtifacts;
		switch_bytes_hp((unsigned char*)&s, sizeof(short));
		sf.write((const char*)&s, sizeof(short));	
		for(int index = 0; index < MAX_2D_ARTIFACTS; index++)
		{
			f = Artifacts2D[index].StartBand;
			switch_bytes_hp((unsigned char*)&f, sizeof(float));
			sf.write((const char*)&f, sizeof(float));
			f  = Artifacts2D[index].EndBand;
			switch_bytes_hp((unsigned char*)&f, sizeof(float));
			sf.write((const char*)&f, sizeof(float));

            s = Artifacts2D[index].StartSegment;
			switch_bytes_hp((unsigned char*)&s, sizeof(short));
			sf.write((const char*)&s, sizeof(short));	
            s = Artifacts2D[index].EndSegment;
			switch_bytes_hp((unsigned char*)&s, sizeof(short));
			sf.write((const char*)&s, sizeof(short));	
		}
	    sf.write(junk,FILLER_SIZE);	
        sf.flush();
        return sf.good();
}

//---------------------------------------------------------------------------------
// Function name:       write_artifact
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ private member function
// Authors:             Vasko, Batch
// Last modified:
//
// Effect on global (or member) variables: Writes to disk the compressed artifact
// buffer. It is written exactly at position S_H_SIZE (4096), so it may be called
// independently of the wBrite header function. The compression algorithm takes
// advantage of the zero and one values by imploding each group of 16
// values into a single unsigned short. Returns TRUE if the write was successful.
//
short SFile::write_artifact()
{

	//rjs Feb 2007 Artifact can now be uncompressed, so if the flag is set we simply write out the buffer
	

        // Compress 16 unsigned shorts into a single unsigned short
        unsigned short compressed;
        int i, cindx, sindx;

        sf.seekp(S_H_SIZE,ios::beg); // seek to artifact position
		if(artifacts_uncompressed)
		{
			//artifacts are uncompressed
			for (cindx = 0, sindx = 0; cindx < raw_epoch_cnt; cindx++)
			{
			
					// shift over artifacts in last compressed short if necessary					
					compressed = ABuffer[cindx];
#ifndef HPUX
					switch_bytes_hp((unsigned char *)&compressed, sizeof(unsigned short));
#endif
					sf.write((const char*)&compressed, sizeof(unsigned short));
			}
		}
		else
		{
			//artifacts are compressed, process normally
			for (cindx = 0, sindx = 0; cindx < raw_epoch_cnt; sindx++)
			{
					for (compressed = 0,i = 0; i < 16 && cindx < raw_epoch_cnt; i++)
							compressed = ( compressed << 1 ) + ABuffer[cindx++];
					// shift over artifacts in last compressed short if necessary
					if ((cindx == raw_epoch_cnt) && (raw_epoch_cnt%16 != 0))
					  for (i = 0; i < 16-(raw_epoch_cnt%16); i++) compressed <<= 1;
#ifndef HPUX
					switch_bytes_hp((unsigned char *)&compressed, sizeof(unsigned short) );
#endif
					sf.write((const char*)&compressed,sizeof(unsigned short));
			}
		}
        dpos = sf.tellp(); // set the data position pointer
        sf.flush();
        return sf.good();
}

//-----------------------------------------------------------------------------------
// Function name:       read_header
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ class private member function
// Authors:             Batch
// Last modified:
//
// Effect on global (or member) variables: Reads header fields from disk.
// Returns TRUE when all fstream operators are successful, otherwise it returns
// FALSE
//
short SFile::read_header()  // divided into two function to allow for byte switching cconrad 05102002
{
#ifdef HPUX
	return read_header_hp();
#else
	return read_header_pc();
#endif
}

short SFile::read_header_hp()
{
        sf.seekg(0,ios::beg); // seek to the beginning
        sf.read((char*)&sf_hdr_version, sizeof(float));
        sf.read(bf_filename,16);
        sf.read(bf_label,LABSIZ);
        sf.read((char*)&bf_study, sizeof(short));
        sf.read((char*)&bf_channel, sizeof(short));
		sf.read((char*)&bf_samples_per_sec, sizeof(short));
		sf.read((char*)&bf_id, sizeof(long));
        sf.read((char*)&bf_date, sizeof(long));
        sf.read((char*)&bf_time, sizeof(long));
        sf.read((char*)&chan_calampuv, sizeof(float));
        sf.read((char*)&art_rej_flag, sizeof(short));
        sf.read((char*)&raw_epoch_len, sizeof(float));
        sf.read((char*)&raw_epoch_cnt,  sizeof(long));
        sf.read((char*)&art_med_thresh, sizeof(float));
		sf.read((char*)&art_med_pre_len, sizeof(short));
		sf.read((char*)&art_med_post_len, sizeof(short));
		sf.read((char*)&art_bin_low, sizeof(float));      
		sf.read((char*)&art_bin_high, sizeof(float));
		sf.read((char*)&raws_per_spec, sizeof(short));
		sf.read((char*)&spec_bin_count, sizeof(short));
		sf.read((char*)&spec_windowing_flag, sizeof(short));      
		sf.read((char*)&spec_unit_flag, sizeof(short)); 
		sf.read((char*)&spec_one_sided_flag, sizeof(short));    
		for ( int i = 0; i < MAX_BIN; i++ )
        {
                sf.read((char*)&spec_bin_low_freq[i], sizeof(float));
                sf.read((char*)&spec_bin_high_freq[i], sizeof(float));
        }
		// New
        sf.read((char*)&num_fft_pts, sizeof(unsigned short));  
//		printf("The num_fft_pts : %d\n", num_fft_pts);
		sf.read((char*)&num_samp_overlap, sizeof(short));
		sf.read((char*)&data_acquisition_equipment, sizeof(unsigned char));//rjs added sept 16 2002
		sf.read((char*)&device_scaled_to, sizeof(unsigned char));

		sf.read((char*)&number_of_additional_artifactbands, sizeof(short));
		for(int index = 0; index < MAX_ART_BANDS; index++)
		{
			sf.read((char*)&additional_art_bin_low[index], sizeof(float));
			sf.read((char*)&additional_art_bin_high[index], sizeof(float));
            sf.read((char*)&additional_art_medthresh[index], sizeof(float));			
			sf.read((char*)&additional_art_pre_len[index], sizeof(short));
			sf.read((char*)&additional_art_post_len[index], sizeof(short));

		}		
		sf.read((char*)&artifacts_uncompressed, sizeof(unsigned char));
//This reads in the 2D artifact data. Each one is s range of frequency band and segment times
		sf.read((char*)&NumberOf2DArtifacts, sizeof(short));
 
		for(int index = 0; index < MAX_2D_ARTIFACTS; index++)
		{
			sf.read(( char*)&Artifacts2D[index].StartBand, sizeof(float));			 
			sf.read(( char*)&Artifacts2D[index].EndBand, sizeof(float));		 
			sf.read(( char*)&Artifacts2D[index].StartSegment, sizeof(short));		 
			sf.read(( char*)&Artifacts2D[index].EndSegment, sizeof(short));
 
		}
 

		sf.read(junk,FILLER_SIZE);
        return sf.good();
}

short SFile::read_header_pc()
{
        sf.seekg(0,ios::beg); // seek to the beginning
        sf.read((char*)&sf_hdr_version, sizeof(float));
	switch_bytes_hp((unsigned char *)&sf_hdr_version, sizeof(float) );
        sf.read(bf_filename,16);
        sf.read(bf_label,LABSIZ);
        sf.read((char*)&bf_study, sizeof(short));
	switch_bytes_hp((unsigned char *)&bf_study, sizeof(short) );
        sf.read((char*)&bf_channel, sizeof(short));
	switch_bytes_hp((unsigned char *)&bf_channel, sizeof(short) );
		sf.read((char*)&bf_samples_per_sec, sizeof(short));
	switch_bytes_hp((unsigned char *)&bf_samples_per_sec, sizeof(short) );
		sf.read((char*)&bf_id, sizeof(long));
	switch_bytes_hp((unsigned char *)&bf_id, sizeof(long) );
        sf.read((char*)&bf_date, sizeof(long));
	switch_bytes_hp((unsigned char *)&bf_date, sizeof(long) );
        sf.read((char*)&bf_time, sizeof(long));
	switch_bytes_hp((unsigned char *)&bf_time, sizeof(long) );
        sf.read((char*)&chan_calampuv, sizeof(float));
	switch_bytes_hp((unsigned char *)&chan_calampuv, sizeof(float) );
        sf.read((char*)&art_rej_flag, sizeof(short));
	switch_bytes_hp((unsigned char *)&art_rej_flag, sizeof(short) );
        sf.read((char*)&raw_epoch_len, sizeof(float));
	switch_bytes_hp((unsigned char *)&raw_epoch_len, sizeof(float) );
        sf.read((char*)&raw_epoch_cnt,  sizeof(long));
	switch_bytes_hp((unsigned char *)&raw_epoch_cnt, sizeof(long) );
        sf.read((char*)&art_med_thresh, sizeof(float));
	switch_bytes_hp((unsigned char *)&art_med_thresh, sizeof(float) );
		sf.read((char*)&art_med_pre_len, sizeof(short));
	switch_bytes_hp((unsigned char *)&art_med_pre_len, sizeof(short) );
		sf.read((char*)&art_med_post_len, sizeof(short));
	switch_bytes_hp((unsigned char *)&art_med_post_len, sizeof(short) );
		sf.read((char*)&art_bin_low, sizeof(float));
	switch_bytes_hp((unsigned char *)&art_bin_low, sizeof(float) );
		sf.read((char*)&art_bin_high, sizeof(float));
	switch_bytes_hp((unsigned char *)&art_bin_high, sizeof(float) );
		sf.read((char*)&raws_per_spec, sizeof(short));
	switch_bytes_hp((unsigned char *)&raws_per_spec, sizeof(short) );
		sf.read((char*)&spec_bin_count, sizeof(short));
	switch_bytes_hp((unsigned char *)&spec_bin_count, sizeof(short) );
		sf.read((char*)&spec_windowing_flag, sizeof(short));
	switch_bytes_hp((unsigned char *)&spec_windowing_flag, sizeof(short) );
		sf.read((char*)&spec_unit_flag, sizeof(short));
	switch_bytes_hp((unsigned char *)&spec_unit_flag, sizeof(short) );
		sf.read((char*)&spec_one_sided_flag, sizeof(short));
	switch_bytes_hp((unsigned char *)&spec_one_sided_flag, sizeof(short) );
		for ( int i = 0; i < MAX_BIN; i++ )
        {
                sf.read((char*)&spec_bin_low_freq[i], sizeof(float));
			switch_bytes_hp((unsigned char *)&spec_bin_low_freq[i], sizeof(float) );
                sf.read((char*)&spec_bin_high_freq[i], sizeof(float));
			switch_bytes_hp((unsigned char *)&spec_bin_high_freq[i], sizeof(float) );
        }
		
        sf.read((char*)&num_fft_pts, sizeof(unsigned short));
	switch_bytes_hp((unsigned char *)&num_fft_pts, sizeof(unsigned short) );
		sf.read((char*)&num_samp_overlap, sizeof(short));
	switch_bytes_hp((unsigned char *)&num_samp_overlap, sizeof(short) );

		sf.read((char*)&data_acquisition_equipment, sizeof(char));//rjs added sept 16 2002
 	switch_bytes_hp((unsigned char *)&data_acquisition_equipment, sizeof(unsigned char) );
		sf.read((char*)&device_scaled_to, sizeof(char));
 	switch_bytes_hp((unsigned char *)&device_scaled_to, sizeof(unsigned char) );

		
		sf.read((char*)&number_of_additional_artifactbands, sizeof(short));
        switch_bytes_hp((unsigned char *)&number_of_additional_artifactbands, sizeof(short) );
		for(int index = 0; index < MAX_ART_BANDS; index++)
		{
                sf.read((char*)&additional_art_bin_low[index], sizeof(float));
			switch_bytes_hp((unsigned char *)&additional_art_bin_low[index], sizeof(float) );
			    sf.read((char*)&additional_art_bin_high[index], sizeof(float));
			switch_bytes_hp((unsigned char *)&additional_art_bin_high[index], sizeof(float) );
                sf.read((char*)&additional_art_medthresh[index], sizeof(float));
			switch_bytes_hp((unsigned char *)&additional_art_medthresh[index], sizeof(float) );
				sf.read((char*)&additional_art_pre_len[index], sizeof(short));
			switch_bytes_hp((unsigned char *)&additional_art_pre_len[index], sizeof(short) );
				sf.read((char*)&additional_art_post_len[index], sizeof(short));
			switch_bytes_hp((unsigned char *)&additional_art_post_len[index], sizeof(short) );	
		}
		sf.read((char*)&artifacts_uncompressed, sizeof(unsigned char));
			 
		sf.read((char*)&NumberOf2DArtifacts, sizeof(short));
		switch_bytes_hp((unsigned char *)&NumberOf2DArtifacts, sizeof(short) );
		for(int index = 0; index < MAX_2D_ARTIFACTS; index++)
		{
			sf.read(( char*)&Artifacts2D[index].StartBand, sizeof(float));
			switch_bytes_hp((unsigned char *)&Artifacts2D[index].StartBand, sizeof(float));
			sf.read(( char*)&Artifacts2D[index].EndBand, sizeof(float));
			switch_bytes_hp((unsigned char *)&Artifacts2D[index].EndBand, sizeof(float) );

			sf.read(( char*)&Artifacts2D[index].StartSegment, sizeof(short));
			switch_bytes_hp((unsigned char *)&Artifacts2D[index].StartSegment, sizeof(short) );
			sf.read(( char*)&Artifacts2D[index].EndSegment, sizeof(short));
			switch_bytes_hp((unsigned char *)&Artifacts2D[index].EndSegment, sizeof(short) );
		}
 
		sf_hdr_version = SFILE_HEADER_VERSION;		 
	 
		sf.read(junk,FILLER_SIZE);
        return sf.good();
}

//---------------------------------------------------------------------------------
// Function name:       read_artifact
// Source file:         sfile.cpp
// Prototype file:      sfile.h
// Type:                C++ private member function
// Authors:             Vasko, Batch
// Last modified:
//
// Effect on global (or member) variables: Reads artifact values from disk into the
// artifact buffer. The values are compressed so each one is exploded. Since
// the function explicitly seeks to the S_H_SIZE position, it may be called
// indepedently of the read header function. It also sets the data position pointer
// to the position directly following the artifact read.  Returns TRUE if the read
// was successful.
//
short SFile::read_artifact()
{
		//rjs Feb 2007 Artifact can now be uncompressed, so if the flag is set we simply read in the
	//buffer and swith the bytes if need be
        ABuffer = (short*)malloc( sizeof(short)*(int)raw_epoch_cnt );
        if (ABuffer == NULL) return FALSE;

        short numshorts;
        long i, cindx, sindx;
        unsigned short temp, mask = 0x8000;


        sf.seekg(S_H_SIZE,ios::beg); // seek to artifact position
		if(artifacts_uncompressed)
		{
			//artifacts uncompressed, just read in the buffer
			for (sindx = 0; sf.good() && sindx < raw_epoch_cnt; sindx++ )
			{
				sf.read((char*)&temp,sizeof(unsigned short));
#ifndef HPUX  
				switch_bytes_hp((unsigned char*)(&temp), sizeof(unsigned short));
#endif
				ABuffer[sindx] = temp;
				
			}
		}
		else
		{
			//artifacts compressed
			numshorts = (short)((raw_epoch_cnt/16) + ((raw_epoch_cnt%16 == 0) ? 0 : 1));
			for (i = 0, cindx = 0; i < numshorts; i++)
			{
				sf.read((char*)&temp,sizeof(unsigned short));
#ifndef HPUX  
				switch_bytes_hp((unsigned char*)(&temp), sizeof(unsigned short));
#endif
				// decompress unsigned shorts one at a time
				for (sindx = 0; sf.good() && sindx < 16 && cindx < raw_epoch_cnt; sindx++ )
				{
					ABuffer[cindx++] = (unsigned short)((temp & mask) ? 1 : 0);
					temp <<= 1;
				}
			}
		}

        dpos = sf.tellg();
        return sf.good();
}

void SFile::Flush()
{
	if(sf.is_open())
		sf.flush();
}
////////////////////////////////////////////////////////////////
//Returns 1 if we are in artifact
short SFile::In2dArtifact(short segment, short bin)
{
	if(NumberOf2DArtifacts < 1)
		return 0;//no artifacts, return 0
	int index;

	for(index = 0; index < NumberOf2DArtifacts; index++)
	{
		if(Artifacts2D[index].EndBand < spec_bin_low_freq[bin] )
			continue;//we end before the low end, ignore
		if(Artifacts2D[index].StartBand > spec_bin_high_freq[bin] )
			continue;//we start after the high end, ignore
		if(Artifacts2D[index].StartSegment > segment)
			continue;//start segment ends after us, ignore
		if(Artifacts2D[index].EndSegment < segment)
			continue;//we end before the segment starts, ignore
		return 1;
	}

	return 0;
}

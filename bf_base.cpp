//------------------------------------------------------------------------------------
// File:		bf_base.cpp
// Type:		C++ Class implementation
// Definition File:	bf_base.hpp
// Author(s):	Principle original authors Debbie Kesicki, Jack Doman
// 				Adapted to class members by Kim Batch, Tim Hoffman 30-Mar-1995
// Last Modifed 20-AUG-2012 R. Seres needed error string in constructor
// Modified:	28-APR-95 K.Batch added assign_cal_constants, check_sample_count,
//				fix_up_sample_count, chan_nam, and write_comments
//				11-APR-95 K.Batch added rename_bfilnm(char newbfilnm[])
// Modified:	23-JUL-96 K.Batch added two new fields to be displayed in disp_bhdr
// Modified:	23-MAR-97 K.Batch added constructor and destructor implementations. 
//				The constructor	initializes max_position from amptable.bin
// Modified:    13-FEB-98 K.Batch removed the max_position initialization from the constructor
//              and placed it in disp_bhdr. It gets its value from scc.ini
// Modified:	04-26-00  C Pratt added assign_ad_constants()
// Modified:	05-22-02  R Seres added mfc include for windows library version
// Modified:	02-03-03  R Seres added #ifdefs for Linux compile
// Modified:	05-15-05  R Seres added ability to read in an EDF file using the bfile class
// Modified:	01-03-06  R Seres fixed some bugs with EDF reading, allowed IDs less than 100,000
//							to be considered valid. Changed EDF smoothed file designation to 'Z'
// Synopsis:	Member functions bf_base class. 
// 
// MEMBER TABLE:
//
// RENAME_BFILNM(char[])Assumes newbfilnm checked for existance. Renames bfile.
//			argument:	R->filnm		user supplied filename
// CLOSEBFL() 		Closes the open bfile of this class. Returns true.
// CHECK_BHDR()		Expects an initialized bfile header. Validates bfile header 
//			structure values. Returns true if valid, false if any values 
//			are not valid.
// DISP_BHDR()		Expects an initialized bfile header. Displays bfile header's 
//			values to screen. Returns true.
// CALC_SAMPLE_COUNT(float) Expects an existing bfile with initialized bhdr. Returns 
//			sample count using bfile size, chan_cnt and float samples per
//			byte ratio Returns -1l is fpos is corrupted.
//			argument:	R->byte_ratio
// CHECK_SAMPLE_COUNT(long) Checks if all channel counts are equal to input sample count
//			Returns FALSE on first in correct channel count.  Returns TRUE if
//			all channels are equal to sample_count.
//			arguments:	R->sample_count		#samples expected 
// FIX_UP_SAMPL_COUNT(long) Sets bfile sampl_count to sample_count for all channels
//			arguments:	R->sample_count		#samples expected  
// ASSIGN_CAL_CONSTANTS()// If bfile date is after ONE_VOLTCAL_DATE, assigns new cal 
//			constants, else asssigns old ones
// CHAN_NAM(char *[], short, short) Validates channel names with input chan names
//			arguments:	R->channels 	list of expected channel names   
//					R->min_chan 	# names in channels[] list		   
//					R->report	write to stderr only if TRUE 
// WRITE_COMMENTS(FILE*, char[], char[]) Opens and writes the generic comment lines to an 
//			output file.
//			arguments:	R->*(*f)	file pointer to output file
//					R->filename  	output file name
//					R->prog		usually program name and date
//-------------------------------------------------------------------------------------
 
#include "stdafx.h"
 
#include "local.h"

#ifndef HPUX
#ifndef LINUX
#define WINDOWS 1
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1
#endif
#endif


#ifdef WINDOWS
//rjs for windows library version
#include <windows.h>

#endif

#include "bf_base.hpp"	 // base class definition file


#include "math.h"

//-------------------------------------------------------------------------------------
// Function Name:	bf_base 
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:			C++ class constructor
// Author(s):		Kim Batch	
// Last Modified:	13-FEB-98 no longer inititalizes max_position
//
// Effect on member variable(s):  
// 
bf_base::bf_base()                
{
	IndexIntoRecord = 0;
	EdfRecordSize = 0;
	memset(DataPtrs, 0, sizeof(unsigned short) * MAX_CHANNELS_V2);
} // END OF CONSTRUCTOR

//-------------------------------------------------------------------------------------
// Function Name:	~bf_base() 
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:			C++ class destructor
// Author(s):		Kim Batch	
// Last Modified:	
//
// Effect on member variable(s):  no effect
// 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
long StringDateToYYYYMMDD_EDF(char date[])
{
	long YYYYMMDD=0;
	int limit = strlen(date);
	char substring[32];
	int index = 0;
	int sub_index = 0;

	while(date[index] == ' ' && index < limit)
	{//get rid of any leading spaces
		index++;	
	}
	//handle both '.', '-' or '/' as a divider for date
	while(date[index] != '/' && date[index] != '-' && date[index] != '.' && index < limit)
	{
		substring[sub_index] = date[index];
		index++;
		sub_index++;
	}
	//get day
	substring[sub_index] = '\0';	
	YYYYMMDD += atoi(substring);	
	sub_index = 0;
	index++;
	while(date[index] != '/' && date[index] != '-' && date[index] != '.' && index < limit)
	{
		substring[sub_index] = date[index];
		index++;
		sub_index++;
	}
	//get month
	substring[sub_index] = '\0';
	YYYYMMDD += atoi(substring) * 100;

	sub_index = 0;
	index++;
	while(index < limit)
	{
		substring[sub_index] = date[index];
		index++;
		sub_index++;
	}
	substring[sub_index] = '\0';

	short year = atoi(substring);
	YYYYMMDD += year * 10000;
	if( year < 72)//if date is not y2k complient
		YYYYMMDD += 20000000;
	else if (year < 1900)
		YYYYMMDD += 19000000;
	return YYYYMMDD;
}

bf_base::~bf_base()                
{
} // END OF DESTRUCTOR

//-------------------------------------------------------------------------------------
// Function Name:	get_bfilnm(char bfilename[]) 
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:		C++ class public void
// Author(s):		Kim Batch	
// Last Modified:	
//
// Effect on member variable(s):  no effect
// 
void bf_base::get_bfilnm(char bfilename[])                
{
	strcpy (bfilename, bfilnm);
	return;

} // END OF GET_BFILNM ROUTINE

//-------------------------------------------------------------------------------------
// Function Name:	get_bfilpath(char bfilepathname[]) 
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:		C++ class public void
// Author(s):		Kim Batch	
// Last Modified:	
//
// Effect on member variable(s):  no effect
// 
void bf_base::get_bfilpath(char bfilepathname[])                
{
	strcpy (bfilepathname, bfilpath);
	return;

} // END OF GET_BFILNM ROUTINE


//-------------------------------------------------------------------------------------
// Function Name:	set_bfilpath() 
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:		C++ class public void
// Author(s):		Kim Batch	
// Last Modified:	
//
// Effect on member variable(s):  no effect
// 
void bf_base::set_bfilpath()                
{
	getdstr(bfilnm, bfilpath);
	return;

} // END OF GET_BFILNM ROUTINE



//-------------------------------------------------------------------------------------
// Function Name:	rename_bfilnm(char newbfilnm[]) 
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:		C++ class public short
// Author(s):		Kim Batch, Jack Doman (modified section of bf_edit.cpp)	
// Last Modified:	
//
// Effect on member variable(s):  bfilnm is changed to newbfilnm (file is renamed)
// 
short bf_base::rename_bfilnm(char newbfilnm[])                
{
	if (rename( bfilnm, newbfilnm ) != 0) return( FALSE ); // if doesn't succeed
	strcpy(bfilnm, newbfilnm);
	return( TRUE );
} // END OF CHANGE_BFILNM ROUTINE

//-------------------------------------------------------------------------------------
// Function Name:	closebfl() 
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:		C++ class public virtual short
// Author(s):		Debbie Kesicki		
// Last Modified:	30-Mar-95
//
// Effect on member variable(s):  fptr is deleted
//
short bf_base::closebfl()
{
	fclose(fptr);
	return(TRUE);

}// END OF CLOSEBFL ROUTINE 

// -------------------------------------------------------------------------------------
// Function Name:	check_hdr()
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:		C++ class public short
// Author(s):		Debbie Kesicki
// Last Modified:	30-Mar-95                               
//
// Effect on member variable(s):  none
//
short bf_base::check_bhdr()
{
   if ((bhdr.id < 0) || (bhdr.id > 9999999))
     { fprintf(stderr,"ERROR - Invalid ID: %ld \n",bhdr.id);
       return(FALSE);
     }
   if ((bhdr.study < 0) || (bhdr.study > 999))
     { fprintf(stderr,"ERROR - Invalid STUDY: %d \n",bhdr.study);
       return(FALSE);
     }
   if (!ckdate(bhdr.date))
     { fprintf(stderr,"ERROR - Invalid START_DATE: %8.8ld \n",bhdr.date);
       return(FALSE);
     }
   if (!cktime(bhdr.time))
     { fprintf(stderr,"ERROR - Invalid START_TIME: %6.6ld \n",bhdr.time);
       return(FALSE);
     }
   if (!ckdate(bhdr.bfile_creation_date))
     { fprintf(stderr,"ERROR - Invalid CREATION_DATE: %8.8ld \n",bhdr.bfile_creation_date);
       return(FALSE);
     }
   if (!cktime(bhdr.bfile_creation_time))
     { fprintf(stderr,"ERROR - Invalid CREATION_TIME: %6.6ld \n",bhdr.bfile_creation_time);
       return(FALSE);
     }
   if(bhdr.prog_version <= 0)
     { fprintf(stderr,"ERROR - Invalid PROG_VERSION: %3.1f \n",bhdr.prog_version);
       return(FALSE);
     }
   if ((bhdr.input_compr_ratio!=1) && (bhdr.input_compr_ratio!=2) && 
      (bhdr.input_compr_ratio!=4) && (bhdr.input_compr_ratio!=8) &&
      (bhdr.input_compr_ratio!=16))
     { fprintf(stderr,"ERROR - Invalid INPUT_COMPR_RATIO: %d ",bhdr.input_compr_ratio);
       return(FALSE);
     }
   if ((bhdr.output_compr_alg!=0) && (bhdr.output_compr_alg!=1))
     { fprintf(stderr,"ERROR - Invalid OUTPUT_COMPR_ALG: %d \n",bhdr.output_compr_alg);
       return(FALSE);
     }
   if( (bhdr.cal_detect != CAL_DETECTED) &&
       (bhdr.cal_detect != CAL_NOT_FOUND) &&
       (bhdr.cal_detect != CAL_UNTRIED))
     { fprintf(stderr,"ERROR - Invalid CAL_DETECT: %d ",bhdr.cal_detect);
//-debug JD 6/2/99       return(FALSE);
     }
   if (bhdr.lcm_spm <=0)
     { fprintf(stderr,"ERROR - Invalid LCM_SPM: %ld \n",bhdr.lcm_spm);
       return(FALSE);
     }
   if (bhdr.chan_cnt > MAX_CHANNELS_V2)
     { fprintf(stderr,"ERROR - Invalid CHAN_CNT: %d \n",bhdr.chan_cnt);
       return(FALSE);
     }
   return(TRUE);
}// END OF CHECK_BHEADER ROUTINE 

// ---------------------------------------------------------------------------
// Function Name: 	disp_bhdr()
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:		C++ class public short
// Author(s):		Debbie Kesicki
// Last Modified:	30-Mar-95
//       		Modifed 1-14-94 T.Hoffman - Merged with BHEADER_EDIT version and added
//       		a switch HHMM to select a display format for time fields. If HHMM
//       		is TRUE then times are display HHMM, else displayed as HHMMSS
//       		Modified July 16, 1993 by Tim Hoffman - added END DATE & TIME:
//			Modified October 5, 1992 by Tim Hoffman - condensed output format
// Modified:	23_JUL-96 K.Batch added the display of two new fields, the bfile 
//				position, and the channel's amplifier serial number
// Modified:    13-FEB-98 K.Batch added local variable max_position and gets its value from
//              scc.ini through generics call GetPrivateProfileInt(...) 
// Effect on member variable(s):  none
//
short bf_base::disp_bhdr(short HHMM) 
{
  short i, max_position;
  long  enddate,endtime,nsec;

// GET VALUE OF MAXPOSITION FROM SCC.INI
#ifndef WINDOWS //windows handles profile headings differently than HPUnix and Linux
  if ( (max_position = GetPrivateProfileInt("[POSITIONS]","max",0,SCC_INI)) < bhdr.position )
#else
#ifdef _WC
  if ((max_position = GetPrivateProfileInt(L"POSITIONS", L"max", 0, WSCC_INI)) < bhdr.position)
#else
  if ((max_position = GetPrivateProfileInt("POSITIONS", "max", 0, SCC_INI)) < bhdr.position)
#endif
#endif
    {
      printf("******** DISPLAY OF POSITION AND SERIAL # IS NOT CORRECT *********\n");
      printf("Check to make sure the max position value %d in scc.ini is current.\n\n", max_position);
    }

// CALCULATE END TIME OF STUDY BASED ON START TIME AND AMT OF DATA IN BHEADERILE 

  enddate = bhdr.date;
  endtime = bhdr.time;
  nsec = (bhdr.chans[0].sampl_count) / (bhdr.lcm_spm/60);
  if (nsec>0L) increment_date_time_by_sec( &enddate, &endtime, nsec );

// DISPLAY CONTENTS OF HEADER 

  if (HHMM)   /* display any time fields as HHMM (no seconds) */
    {
     printf("Bfilename:'%s' ID:%ld Study:%d StartDate:%08ld StartTime:%04ld\n",
             bhdr.bfilename, bhdr.id,bhdr.study, bhdr.date,bhdr.time/100L );
     printf("CreationDate:%08ld CreationTime:%04ld    EndDate  :%08ld EndTime  :%04ld\n",
             bhdr.bfile_creation_date,bhdr.bfile_creation_time/100L,enddate,endtime/100L);
    }
  else       /* display times as full  HHMMSS */
    {
     printf("BfileName:'%s' ID:%ld Study:%d StartDate:%08ld StartTime:%06ld \n",
             bhdr.bfilename,bhdr.id,bhdr.study, bhdr.date,bhdr.time );
     printf("CreationDate:%08ld CreationTime:%06ld    EndDate  :%08ld EndTime  :%06ld\n",
             bhdr.bfile_creation_date,bhdr.bfile_creation_time,enddate,endtime);
    } 

  if (bhdr.position < 1 || bhdr.position > max_position) // position not set
  {
	printf("Input Compr Ratio: %d  Output Compr Alg: %d  Block: %d  Blocksize %d  Device id: %d\n",
          bhdr.input_compr_ratio,bhdr.output_compr_alg,bhdr.block,bhdr.blocksize,bhdr.device_id);
	printf("lcm_spm: %ld  Position: %d  Num Chans Collected: %d  Prog_Version: %5.2f\n",
          bhdr.lcm_spm,UNKNOWN_POSITION,bhdr.chan_cnt,bhdr.prog_version);
  }
  else
  {
	printf("Input Compr Ratio: %d  Output Compr Alg: %d  Block: %d  Blocksize %d  Device id: %d\n",
          bhdr.input_compr_ratio,bhdr.output_compr_alg,bhdr.block,bhdr.blocksize,bhdr.device_id);
	printf("lcm_spm: %ld  Position: %d  Num Chans Collected: %d  Prog_Version: %5.2f\n",
          bhdr.lcm_spm,bhdr.position,bhdr.chan_cnt,bhdr.prog_version);
  }
  printf("Cal Detection Status: ");
  if (bhdr.cal_detect==1)
    printf("CAL DETECTED\n");
  else if (bhdr.cal_detect==2)
    printf("CAL TRIED BUT NOT DETECTED\n");
  else if (bhdr.cal_detect==3)
    printf("CAL DETECT UNTRIED\n");
  else
    printf(" ?? undefined status value %d ??\n",bhdr.cal_detect);
  printf("Comment: %s\n\n",bhdr.bfile_comment);
  if (bhdr.position < 1 || bhdr.position > max_position) // and serials not set
  {
	printf("Port Label Lead_Type  SPM  TConst  Gain  Cal_Per  Cal_Amp  Serial#  Sample_Cnt\n");
	printf("------------------------------------------------------------------------------\n");
	for (i=0;i<bhdr.chan_cnt;i++)
      printf("%02d  %-4s  %-9s  %6ld  %3.1f  %3.1f  %7d  %7d  %9s  %10ld\n",
              bhdr.chans[i].ad_port_num, bhdr.chans[i].label,bhdr.chans[i].lead_type,
              bhdr.chans[i].spm,bhdr.chans[i].timeconst,bhdr.chans[i].gain,
              bhdr.chans[i].cal_per,bhdr.chans[i].cal_amp,UNKNOWN_SERIAL,
			  bhdr.chans[i].sampl_count);
  }
  else
  {
	printf("Port Label Lead_Type  SPM  TConst  Gain  Cal_Per  Cal_Amp  Serial#  Sample_Cnt\n");
	printf("------------------------------------------------------------------------------\n");
	for (i=0;i<bhdr.chan_cnt;i++)
      printf("%02d  %-4s  %-9s  %6ld  %3.1f  %3.1f  %7d  %7d  %9s  %10ld\n",
              bhdr.chans[i].ad_port_num, bhdr.chans[i].label,bhdr.chans[i].lead_type,
              bhdr.chans[i].spm,bhdr.chans[i].timeconst,bhdr.chans[i].gain,
              bhdr.chans[i].cal_per,bhdr.chans[i].cal_amp,bhdr.chans[i].serial,
			  bhdr.chans[i].sampl_count);
  }
  return(TRUE);
}// END OF DISP_BHDER ROUTINE 

//--------------------------------------------------------------------------------------
// Function Name: 	build_bhdr()
// Source File:		bf_base.cpp
// Prototype File:	bf_base.hpp
// Type:			C++ class public short
// Author(s):		Debbie Kesicki
// Last Modified:	30-Mar-95
//			25-APR-95 K.Batch modified as a class member function
//
// Effect on member variable(s):  
//-------------------------------------------------------------------------------------- 
short bf_base::build_bhdr(struct B_HEADER oldhdr, short new_chans[], short new_chan_cnt)
{ short i; // looping variable	  
//--------------------------------------------------------------------------------------
  bhdr = oldhdr;
  bhdr.chan_cnt = new_chan_cnt;
  for(i=0; i<bhdr.chan_cnt; i++)
    { bhdr.chans[i] = oldhdr.chans[new_chans[i]];
      bhdr.chans[i].sampl_count = 0; // for error condition
    }
  for (i = bhdr.chan_cnt; i<MAX_CHANNELS_V2; i++)
    { bhdr.chans[i].index = 0;
      bhdr.chans[i].label[0] = '\0';
      bhdr.chans[i].lead_type[0] = '\0';
      bhdr.chans[i].ad_port_num = 0;
      bhdr.chans[i].spm = 0;
      bhdr.chans[i].timeconst = 0.0;
      bhdr.chans[i].gain = 0.0;
      bhdr.chans[i].cal_per = 0;
      bhdr.chans[i].cal_amp = 0;
      bhdr.chans[i].sampl_count = 0;
    }
  return(TRUE);
} // END OF BUILD_BHDR ROUTINE

//--------------------------------------------------------------------------------------
// Function name:	calc_sample_count(float)
// Source file:		bf_base.cpp
// Prototype file:	bf_base.hpp
// Type:		C++ class public long
// Author(s):		Debbie Kesicki (Jack Doman?)
// Last Modified:	31-Mar-95 Modified as class member by Kim Batch & Tim Hoffman
//			29-May-96 Cast integers as float during division
//
// Effect on member variable(s):	none 
// 
long bf_base::calc_sample_count(float byte_ratio)
{ 
   long count, old_pos;
// -------------------------------------------------------------------------
   old_pos = ftell(fptr);
   if (fseek(fptr, 0l, SEEK_END))
    { fprintf(stderr,"Can't seek to end of file. File position may be corrupted.\n");
      return(-1l);
    }

   count = (long)((float)(ftell(fptr) - B_H_SIZE)/(float)bhdr.chan_cnt*byte_ratio);

   if (fseek(fptr, old_pos, SEEK_SET))
    { fprintf(stderr,"Can't reset file postition. File position may be corrupted.\n");
      return(-1l);
    }
   return(count);
}//END OF CALC_SAMPLE_COUNT ROUTINE

//--------------------------------------------------------------------------------------
// Function name:	get_byte_ratio()
// Source file:		bf_base.cpp
// Prototype file:	bf_base.hpp
// Type:		C++ class public float
// Author(s):		K.Batch
// Last Modified:	
//
// Effect on member variable(s):	none 
// 
float bf_base::get_byte_ratio()
{ 
   if (bhdr.output_compr_alg == 1) return (2.0/3.0);
   else return (0.5);

}//END OF GET_BYTE_RATIO ROUTINE

//-----------------------------------------------------------------------------------
// Function name:	chan_nam(char *channels[], short min_chan, short report)
// Source file:		bf_base.cpp
// Prototype file:	bf_base.hpp
// Type:		C++ class public short
// Authors:		Debbie Frasca
// Last modified:	28-APR-95 K.Batch moved from digsub.c to bf_base.cpp
//			and modified as class member of bf_base 
//			Jack Doman  5/20/1991
//			Defined as short function.  Added REPORT argument.
//			No longer exits if errors;  instead return FALSE
//
// Effect on global (or member) variables:	none
//
short bf_base::chan_nam(char *channels[], short min_chan, short report)
{
  short i,k; // for loop indexes                 
// ------------------------------------------------------------------------- 
  if ( bhdr.chan_cnt < min_chan )
     { if (report) fprintf(stderr,("%%Unacceptable #channels %d in %s\n",
					      bhdr.chan_cnt,bfilnm));
	return(FALSE);
     }

  for (k=0; k < min_chan; k++)      // check the names of each channel
      if (strcmp(bhdr.chans[k].label, channels[k]))
	 {
	   if (report)
		{
		fprintf(stderr,"%% Invalid channel or incorrect order\n");
		fprintf(stderr,"      Expected     Encountered\n");
		for (i=0; i <min_chan; i++)
			fprintf(stderr,"    %10s      %10s\n",
					 channels[i],bhdr.chans[i].label);
		fprintf(stderr,"\n");
		}
	   return(FALSE);
	 }
  return(TRUE);
} // END OF CHAN_NAM ROUTINE

//-----------------------------------------------------------------------------------
// Function name:	assign_cal_constants()
// Source file:		bf_base.cpp
// Prototype file:	bf_base.hpp
// Type:		C++ class private void
// Authors:		Tim Hoffman
// Modified:	28-APR-95 K.Batch moved from digsub.c to bf_base.cpp
//			and modified as class member of bf_base
// Modified:	16-SEP-00 J.Doman - Adjust for 16 bit A/D values
//
// Effect on global (or member) variables:	none
//
void bf_base::assign_cal_constants()
{
  if (bhdr.date < ONEVOLT_CAL_DATE )
    { 
      EEGCALDEF  = OLD_EEGCALDEF;
      EOG0CALDEF = OLD_EOG0CALDEF;
      EOG1CALDEF = OLD_EOG1CALDEF;
    }
  else
    {
      EEGCALDEF   = NEW_EEGCALDEF;
      EOG0CALDEF  = NEW_EOG0CALDEF;
      EOG1CALDEF  = NEW_EOG1CALDEF;
    }
  //rjs only record pc (id 0)and biosaca(id 2) are 12 bit, the rest are 16 bit
  if (bhdr.device_id != 0 && bhdr.device_id != 2)
	{// Convert cal amplitudes from 12 to 16 bit values
	  EEGCALDEF  = EEGCALDEF * 16.0;
	  EOG0CALDEF = EOG0CALDEF * 16.0;
	  EOG1CALDEF = EOG1CALDEF * 16.0;
	}
  return;
} // END OF ASSIGN_CAL_CONSTANTS

//-----------------------------------------------------------------------------------
// Function name:	assign_ad_constants()
// Source file:		bf_base.cpp
// Prototype file:	bf_base.hpp
// Type:		C++ class private void
// Authors:		Caroline Pratt
//
void bf_base::assign_ad_constants()
{
  //rjs only record pc (id 0)and biosaca(id 2) are 12 bit, the rest are 16 bit
  if (bhdr.device_id != 0 && bhdr.device_id != 2)
  {
	  ADZERO = ADZERO_16;
	  ADMAX_float = ADMAX_float_16;
  }
  else
  {
	  ADZERO = ADZERO_12;
	  ADMAX_float = ADMAX_float_12;
  }
  return;
} // END OF ASSIGN_AD_CONSTANTS

//-----------------------------------------------------------------------------------
// Function name:	short check_sample_count(long sample_count)
// Source file:		bf_base.cpp
// Prototype file:	bf_base.hpp
// Type:		C++ class public short
// Authors:		
// Last modified:	28-APR-95 K.Batch moved from digsub.c to bf_base.cpp
//			and modified as class member of bf_base
//
// Effect on global (or member) variables: 	none
//
short bf_base::check_sample_count(long sample_count)
{
   short i;

   for (i=0; i < bhdr.chan_cnt; i++)
      if ((bhdr.chans[i]).sampl_count != sample_count)
	 return(FALSE);

   return(TRUE);
} // END OF CHECK_SAMPLE_COUNT ROUTINE

//-----------------------------------------------------------------------------------
// Function name:	short fix_up_sample_count(long sample_count)
// Source file:		bf_base.cpp
// Prototype file:	bf_base.hpp
// Type:		C++ class public short
// Authors:
// Last modified:	28-APR-95 K.Batch moved from digsub.c to bf_base.cpp
//			and modified as class member of bf_base
//
// Effect on global (or member) variables:	changes each channels sampl_count in bfile 
//						bhdr to sample_count
//
short bf_base::fix_up_sample_count(long sample_count)
{
   int i;

   for (i = 0; i < bhdr.chan_cnt; i++)
      (bhdr.chans[i]).sampl_count = sample_count;

   return(TRUE);
} // END OF FIX_UP_SAMPLE_COUNT ROUTINE

//-----------------------------------------------------------------------------------
// Function name:	write_comments(FILE *(*f), char filename[], 
//					struct B_HEADER bhdr, char prog[])
// Source file:		bf_base.cpp
// Prototype file:	wpiclib.h
// Type:		C++ short
// Authors:		Jack Doman 4/27/1990
// Last modified:	28-APR-95 K.Batch moved from digsub.c to bf_base.cpp
//			1/25/92 Debbie Frasca - Fixed bug in switches used
//			9/19/91 Debbie Frasca - Added switches to comment v8.5
//			8/30/91 Debbie Frasca - Added recordpc version # to comment
//
// Effect on global variables:
//
short write_comments(FILE *(*f), char filename[], struct B_HEADER bhdr, char prog[])
{ 
  time_t  ltime; // system date and time
// ------------------------------------------------------------------------- 
  if ((  *f=fopen(filename,"w")) == NULL)    // open the output file        
     {
       printf("\n%% Error openning output file: %s\n",filename);
       return( FALSE );
     }
  time(&ltime);                              // system date and time        
  fprintf(*f,"%s created by %s on %s",filename,prog,ctime(&ltime));
  fprintf(*f,"from the binary file %s created %8.8ld %6.6ld by recordpc v%4.2f\n",
	  bhdr.bfilename,bhdr.bfile_creation_date,
	  bhdr.bfile_creation_time,bhdr.prog_version);
  fprintf(*f,"Input Compression ratio = %d \n",bhdr.input_compr_ratio);
  if(bhdr.cal_detect == CAL_DETECTED)
     fprintf(*f,"Cal signal detected \n");
  else if(bhdr.cal_detect == CAL_NOT_FOUND)
     fprintf(*f,"Cal signal not found \n");
  else if(bhdr.cal_detect == CAL_UNTRIED)
     fprintf(*f,"Cal signal detection untried \n");
  fprintf(*f,"%s\n",bhdr.bfile_comment);
  return( TRUE );
} // END OF WRITE_COMMENTS ROUTINE

void bf_base::MakeBFileHeaderFromEDF(EDF_FILE *EdfFile, long ID, short Study, short Frequency)
{
	if(EdfFile->EdfHeader.device_id != 0)
		bhdr.device_id = EdfFile->EdfHeader.device_id;
	else
		bhdr.device_id = 1;
	strcpy(bhdr.bfile_comment, "BFile was derived from an EDF");
	bhdr.prog_version = PROG_VERSION;
	bhdr.input_compr_ratio = 1;   
	bhdr.output_compr_alg = 1;  
	bhdr.lcm_spm = (Frequency) * 60;
	bhdr.block = 0;	bhdr.blocksize = 0;
	bhdr.chan_cnt = 0;

	sprintf(bhdr.bfilename,"b%6.6ldZ.%3.3d", ID, Study);

	bhdr.position = 0;
	bhdr.id = ID;
	bhdr.study  = Study;
	char datestring[9];//make a temp string, since this method will damage the original one
	memcpy(datestring, EdfFile->EdfHeader.startdate, sizeof(char) * 9);
	char *ptr = strchr(datestring, '.');
	if(ptr)
	{ //to handle 19.04.04
		char *ptr2 = ptr;
		*ptr = '\0';
		bhdr.date = atoi(datestring);//get day
		ptr2++;
		ptr = strchr(ptr2, '.');
		if(ptr)
		{		
			*ptr = '\0';
			bhdr.date += atoi(ptr2) * 100; //get month
			ptr2 = ptr;
			ptr2++;
			if(atoi(ptr2) < 100)
			{
				if(atoi(ptr2) < 60)
					bhdr.date += (2000 + atoi(ptr2)) * 10000; //get year
				else
					bhdr.date += (1900 + atoi(ptr2)) * 10000; //get year
			}
			else
				bhdr.date += atoi(ptr2) * 10000; //get year
		}
	}
	else// if there date is not in the form of YY.DD.MM then assume it is YYYYMMDD
		bhdr.date = StringDateToYYYYMMDD_EDF(EdfFile->EdfHeader.startdate);  

	char timestring[9];//make a temp string, since this method will damage the original one
	memcpy(timestring, EdfFile->EdfHeader.starttime, sizeof(char) * 9);
	ptr = strchr(timestring, '.');
	if(ptr)
	{ //to handle 19.04.04
		char *ptr2 = ptr;
		*ptr = '\0';
		bhdr.time = atoi(timestring) * 10000;//get hour
		ptr2++;
		ptr = strchr(ptr2, '.');
		if(ptr)
		{		
			*ptr = '\0';
			bhdr.time += atoi(ptr2) * 100; //get minute
			ptr2 = ptr;
			ptr2++;			 
			bhdr.time += atoi(ptr2); //get seconds
		}
	}
	else	
		bhdr.time = StringTimeToHHMMSS(EdfFile->EdfHeader.starttime);  /* time of study HHMMSS              */

	bhdr.bfile_creation_date = bhdr.date;	
	bhdr.bfile_creation_time = bhdr.time;
	StartSeconds = (60 - bhdr.time % 100) % 60;//determine how much time needs to be cut of so we can begin on an even minute
 
	int index;
	
	//adjust number of channels for channels we're not going to write out
	short channel_count = bhdr.chan_cnt;


	short channel_index = 0;
	short used_channels_index=0;
	
	//Frequency;

	for(index = 0; index < EdfFile->EdfHeader.number_of_signals; index++)
	{
		if (channel_index < MAX_CHANNELS_V2 && Frequency == EdfFile->EdfHeader.number_of_samples_in_each_record[index] / (EdfFile->EdfHeader.duration))
		{
			EdfRecordSize = EdfFile->EdfHeader.number_of_samples_in_each_record[index] ;//set the record size
			DataPtrs[channel_index] = EdfFile->samples[index].data;

 			strncpy(bhdr.chans[channel_index].label, EdfFile->EdfHeader.label[index].string, 5);
			if(bhdr.chans[channel_index].label[LABSIZ - 2] == ' ')
				bhdr.chans[channel_index].label[LABSIZ - 2] = '\0';

			bhdr.chans[channel_index].label[4] = '\0';
			strncpy(bhdr.chans[channel_index].lead_type, EdfFile->EdfHeader.label[index].string, 9);
			bhdr.chans[channel_index].label[8] = '\0';
			bhdr.chans[channel_index].ad_port_num = index;//
			bhdr.chans[channel_index].spm = (EdfFile->EdfHeader.number_of_samples_in_each_record[index] * 60 )/ EdfFile->EdfHeader.duration;//samples per minute
			bhdr.chans[channel_index].timeconst = 0.0f;
			bhdr.chans[channel_index].gain = 2.0f; //hardcoded
			bhdr.chans[channel_index].cal_per = 0;
/*
			if(EdfFile->EdfHeader.cal_amp[index] != -1)
			{
				bhdr.chans[channel_index].cal_amp = EdfFile->EdfHeader.cal_amp[index];
			}
			
			else*/ if(EdfFile->EdfHeader.physical_maximum[index] - EdfFile->EdfHeader.physical_minimum[index] != 0)
			{
				if(strcmp_nocase(bhdr.chans[channel_index].label, "EKG2") == 0)
					bhdr.chans[channel_index].cal_amp =  

					(short) (( EdfFile->EdfHeader.digital_maximum[index] - EdfFile->EdfHeader.digital_minimum[index])*50.0f) /
					((EdfFile->EdfHeader.physical_maximum[index] - EdfFile->EdfHeader.physical_minimum[index] )* 1000.0f) ; 
				else if(strcmp_nocase(bhdr.chans[channel_index].label, "Skin") == 0)
					//this cal amp value is A to D ticks per centigrade
					bhdr.chans[channel_index].cal_amp =  
					(short)((EdfFile->EdfHeader.digital_maximum[index] - EdfFile->EdfHeader.digital_minimum[index]))/
					(EdfFile->EdfHeader.physical_maximum[index] - EdfFile->EdfHeader.physical_minimum[index]); 
				else if(strcmp_nocase(bhdr.chans[channel_index].label, "Room") == 0)
				
					bhdr.chans[channel_index].cal_amp =  
					(short)((EdfFile->EdfHeader.digital_maximum[index] - EdfFile->EdfHeader.digital_minimum[index]))/
					((EdfFile->EdfHeader.physical_maximum[index] - EdfFile->EdfHeader.physical_minimum[index]) * 1000.0f); 
				else if(strcmp_nocase(bhdr.chans[channel_index].label, "Flow") == 0 || strcmp_nocase(bhdr.chans[channel_index].label, "SNOR") == 0
					|| strcmp_nocase(bhdr.chans[channel_index].label, "THOR") == 0 || strcmp_nocase(bhdr.chans[channel_index].label, "ABDO") == 0
					|| strcmp_nocase(bhdr.chans[channel_index].label, "RESP") == 0)
				
					bhdr.chans[channel_index].cal_amp =  
					(short)((EdfFile->EdfHeader.digital_maximum[index] - EdfFile->EdfHeader.digital_minimum[index]))/
					((EdfFile->EdfHeader.physical_maximum[index] - EdfFile->EdfHeader.physical_minimum[index]) ); 

				else //for readings in volts
					bhdr.chans[channel_index].cal_amp =  (short)(
					((EdfFile->EdfHeader.digital_maximum[index] - EdfFile->EdfHeader.digital_minimum[index]) * 50)/
					(EdfFile->EdfHeader.physical_maximum[index] - EdfFile->EdfHeader.physical_minimum[index]));//for 50 microvolts, 4096 for 12 bit samples
				if(bhdr.chans[channel_index].cal_amp < 0)
					bhdr.chans[channel_index].cal_amp = bhdr.chans[channel_index].cal_amp * -1;//invert if the signal is inverted
				
			}
			else
				bhdr.chans[channel_index].cal_amp = 1;//physical coordinates are equal, so set cal amp to 1 
			if(abs(bhdr.chans[channel_index].cal_amp) < 1) //check if the calamp came out to be zero, and fix it here
				bhdr.chans[channel_index].cal_amp = 1;
			bhdr.chans[channel_index].sampl_count = 
				EdfFile->EdfHeader.number_of_samples_in_each_record[channel_index] * EdfFile->EdfHeader.number_of_data - StartSeconds * Frequency;

			bhdr.chans[channel_index].serial[0] = '\0';
			channel_index++;
	
			used_channels_index++;
		}
		else
			EdfFile->ignore[index] = true;//channel is not at the frequency we are interested in
	}
	bhdr.chan_cnt = channel_index;//set the true channel count
 
	IndexIntoRecord = 0;//this forces our first read epoch
	bhdr.cal_detect = 1;
	
 
}
//-----------------------------------------------------------------------------------

void bf_base::MakeEDFHeaderFromBfile(bf_base *bfile)
{

	sprintf(EDFFile.EdfHeader.version_info, "0       ");//just 0 for our version information

	sprintf(EDFFile.EdfHeader.patient_id, "%08d%72s", bfile->bhdr.id, " ");
	memset(EDFFile.EdfHeader.recording_id, ' ', sizeof(char) * 80);
	
	//set start date
	sprintf(EDFFile.EdfHeader.startdate, "%02d.%02d.%02d", bfile->bhdr.date % 100, 
		(bfile->bhdr.date / 100)% 100, (bfile->bhdr.date / 10000) % 100);
	
	//set start time
	sprintf(EDFFile.EdfHeader.starttime, "%02d.%02d.%02d", bfile->bhdr.time / 10000, 
		(bfile->bhdr.time / 100) % 100, bfile->bhdr.time % 100);
 
	EDFFile.EdfHeader.device_id = bfile->bhdr.device_id;
	EDFFile.EdfHeader.device_id = 1;

	EDFFile.EdfHeader.duration = 1;
	//our number of sample chunks is the total number of samples divided by how many samples per second is
	//in the bfile
	EDFFile.EdfHeader.number_of_data = bfile->bhdr.chans[0].sampl_count / (bfile->bhdr.chans[0].spm/ 60);

	EDFFile.InitChannels(bfile->bhdr.chan_cnt);
	for(int index = 0; index < EDFFile.EdfHeader.number_of_signals; index++)
	{
		strcpy(EDFFile.EdfHeader.label[index].string, bfile->bhdr.chans[index].label);
		strcpy(EDFFile.EdfHeader.transducer[index].string, "uV");
		strcpy(EDFFile.EdfHeader.prefiltering[index].string, "UNKNOWN");
		EDFFile.EdfHeader.number_of_samples_in_each_record[index] = bfile->bhdr.chans[index].spm / 120;
		EDFFile.EdfHeader.cal_amp[index] = bfile->bhdr.chans[index].cal_amp;
		EDFFile.EdfHeader.digital_maximum[index] = 4096;
		EDFFile.EdfHeader.digital_minimum[index] = -4095;
		EDFFile.EdfHeader.physical_maximum[index] = (float)(4096 * 25)/(bfile->bhdr.chans[index].cal_amp );
		EDFFile.EdfHeader.physical_minimum[index] = (float)(-4095 * 25)/(bfile->bhdr.chans[index].cal_amp );
	}

}

//-----------------------------------------------------------------------------------
void bf_base::ImportEDFInfo(EDF_FILE *Source)
{

	//merely copies header information from another EDF file
	memcpy(EDFFile.EdfHeader.version_info, Source->EdfHeader.version_info, sizeof(char) * 8);
	memcpy(EDFFile.EdfHeader.patient_id, Source->EdfHeader.patient_id, sizeof(char) * 80);
	memcpy(EDFFile.EdfHeader.recording_id, Source->EdfHeader.recording_id, sizeof(char) * 80);
	memcpy(EDFFile.EdfHeader.startdate, Source->EdfHeader.startdate, sizeof(char) * 9);
	memcpy(EDFFile.EdfHeader.starttime, Source->EdfHeader.starttime, sizeof(char) * 9);		
	EDFFile.EdfHeader.sizeof_header = Source->EdfHeader.sizeof_header;
	EDFFile.EdfHeader.number_of_data = Source->EdfHeader.number_of_data;
	EDFFile.EdfHeader.duration = Source->EdfHeader.duration;
	EDFFile.EdfHeader.number_of_signals = Source->EdfHeader.number_of_signals;
	EDFFile.EdfHeader.device_id = Source->EdfHeader.device_id;
 
	EDFFile.InitChannels(Source->EdfHeader.number_of_signals);


	memcpy(EDFFile.EdfHeader.label, Source->EdfHeader.label, sizeof(string16) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.EdfHeader.transducer, Source->EdfHeader.transducer, sizeof(string80) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.EdfHeader.physical_dimension, Source->EdfHeader.physical_dimension, sizeof(string16) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.EdfHeader.physical_minimum, Source->EdfHeader.physical_minimum, sizeof(float) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.EdfHeader.physical_maximum, Source->EdfHeader.physical_maximum, sizeof(float) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.EdfHeader.digital_minimum, Source->EdfHeader.digital_minimum, sizeof(short) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.EdfHeader.digital_maximum, Source->EdfHeader.digital_maximum, sizeof(short) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.EdfHeader.prefiltering, Source->EdfHeader.prefiltering, sizeof(string80) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.EdfHeader.number_of_samples_in_each_record, Source->EdfHeader.number_of_samples_in_each_record, sizeof(int) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.EdfHeader.cal_amp, Source->EdfHeader.cal_amp, sizeof(short) * EDFFile.EdfHeader.number_of_signals);
	memcpy(EDFFile.ignore, Source->ignore, sizeof(bool) * EDFFile.EdfHeader.number_of_signals);



}
short bf_base::OpenEdfAsBFile(char bfilnm[])
{

	if(!EDFFile.OpenEDFFile(bfilnm))
		return false;
  	if(!EDFFile.ReadEDFHeader())
	{
		strcpy(LastError, EDFFile.LastError);
		strcat(LastError, " for file ");
		strcat(LastError, bfilnm);
		EDFFile.CloseEDFFile();
 		return false;
	}
	fptr = EDFFile.EdfFile;
	long  ID;
	short Study;

	char filename[80];
	strcpy(filename, bfilnm);
	char *ptr;
	ptr = strrchr(filename, '-');
#ifdef WINDOWS
	char *startptr = strrchr(filename, '\\');
#else
	char *startptr = strrchr(filename, '//');
#endif
	if(!startptr)
		startptr = filename;
	else
		startptr++;
	//if the file starts with a 'd', assume it is a decimated EDF file
	short freq = 256;
	if(*startptr == 'd' || *startptr == 'D')
		freq = 128;
	if(ptr)
	{
		//the ID is not stored in an EDF file, so derive it from the filename
		*ptr = (char)'\0';
		while((*startptr < '0' || *startptr > '9') && *startptr != '\0' && *startptr != '.')
		{
			startptr++;
		}
		int index=0;
		char idstring[7];
		while(*startptr >= '0' && *startptr <= '9' && index < 6)
		{
			idstring[index++] = *startptr;
			startptr++;
		}
		idstring[index] = '\0';
		ID = atoi(idstring);	
		ptr++;
		char *ptr2 = strchr(ptr, '.');
		if(ptr2)
		{
			*ptr2 = (char)'\0';
			Study = atoi(ptr);
			//set bfile header information from the edf file
			MakeBFileHeaderFromEDF(&EDFFile, ID, Study, freq);
			IndexIntoRecord = EdfRecordSize;//this forces our first read sample
			return 1;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------------
void bf_base::CheckFileType(char FileName[])
{
	//if file extension ends in rec or edf, treat this as an EDF file
	char *ptr;
	bIsEdfFile = false;
	ptr = strrchr(FileName, '.');
	if(!ptr)
	{	
		return;
	}
	if(strcmp_nocase(ptr, ".rec") == 0)
		bIsEdfFile = true;
	if(strcmp_nocase(ptr, ".edf") == 0)
		bIsEdfFile = true;
}
//------------------------------------------------------------------------------------
void bf_base::DecimateSampleCount(short Decimate)
{
	if(bIsEdfFile)
	{
		for(int index = 0; index < EDFFile.EdfHeader.number_of_signals; index++)
		{
			EDFFile.EdfHeader.number_of_samples_in_each_record[index] /= Decimate;
		}
	}
}
/***************************************************************************
 * OPBFHP.C Routines to process data from a binary file under HP-UX.
	    These functions are called by openbfl.c routines to correct for
	    discrepancies in alignment within structures and little-endian.
    Written by Jack Doman      2/17/1992
   Modified: 6/22/1992	Debbie Kesicki - v9.1 WPICLIB.H Function prototyping
   Modified: 07/08/93 Timothy L. Hoffman - v9.2 Compiles & links under C++
	  both the PC MS-DOS platform and the HPUX platform. Most of the
	  changes were: prototyping, explicit type conversions, and adding
	  dummy int arg to the ^C handler routine. The functionality of the
	  program has not been changed.
   Recompiled as version 9.3  9/23/93   
   Recompiled as version 9.4  2/01/94
   Modified: 8/15/2001	JPassios - Added read_bhead_pc/write_bhead_pc
                         functions to replace fread/fwrite functions
	Modified 7/7/2003   RSeres - Removed switchbyteshp and placed it in generics.cpp so
								it can be used in other progrmas without having to include the
								bfile library
	Modified 12/3/2003 RSeres added a carriage return at the end of file to suppress a compiler warning
 ***************************************************************************/
 
#include "stdafx.h"
 
#include "local.h"
#ifdef LOCAL
  #include "wpicc.h"
  #include "bfile.h"
  #include "header.h"
#else
   #include <wpicc.h>
   #include <bfile.h>
   #include <header.h>
#endif
#include <stdio.h>			       /* define i/o vals routines */
#include "wpiclib.h"		/* function prototyping */
#include "generic.h"	
 

/* ------------------------------------------------------------------------- */
short read_bhead_hp(FILE *fptr,struct B_HEADER *bfile)
 {
   int iii,jjj;
   short hold_short;
   long  hold_long;
   float hold_float;
   char  hold_char;

   fread(&(bfile->device_id),sizeof(short),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->device_id, sizeof(short) );
   fread(&(bfile->bfile_creation_date),sizeof(long),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->bfile_creation_date, sizeof(long) );
   fread(&(bfile->bfile_creation_time),sizeof(long),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->bfile_creation_time, sizeof(long) );
   fread(&(bfile->bfile_comment[0]),sizeof(char),80,fptr);
   fread(&(bfile->prog_version),sizeof(float),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->prog_version,sizeof(float));
   fread(&(bfile->input_compr_ratio),sizeof(short),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->input_compr_ratio, sizeof(short) );
   fread(&(bfile->output_compr_alg),sizeof(short),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->output_compr_alg,sizeof(short));
   fread(&(bfile->lcm_spm),sizeof(long),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->lcm_spm, sizeof(long) );
   fread(&(bfile->block),sizeof(short),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->block, sizeof(short) );
   fread(&(bfile->blocksize),sizeof(short),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->blocksize, sizeof(short) );
   fread(&(bfile->chan_cnt),sizeof(short),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->chan_cnt, sizeof(short) );
	  int LIMIT = MAX_CHANNELS_V2;
	  if (bfile->prog_version < PROG_VERSION_BEFORE_HD)
		  LIMIT = OLD_MAX_CHANNELS;
		  for (iii = 0; iii<LIMIT; iii++)
      {
      fread(&hold_short,sizeof(short),1,fptr);
	 switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
	 bfile->chans[iii].index=hold_short;
      for (jjj=0; jjj<LABSIZ; jjj++)
	 {
	 fread(&hold_char,sizeof(char),1,fptr);
	    bfile->chans[iii].label[jjj]=hold_char;
	 }
      for (jjj=0; jjj<9; jjj++)
	 {
	 fread(&hold_char,sizeof(char),1,fptr);
	    bfile->chans[iii].lead_type[jjj]=hold_char;
	 }
      fread(&hold_short,sizeof(short),1,fptr);
	 switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
	 bfile->chans[iii].ad_port_num = hold_short;
      fread(&hold_long,sizeof(long),1,fptr);
	 switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
	 bfile->chans[iii].spm = hold_long;
      fread(&hold_float,sizeof(float),1,fptr);
	 switch_bytes_hp((unsigned char *)&hold_float, sizeof(float) );
	 bfile->chans[iii].timeconst = hold_float;
      fread(&hold_float,sizeof(float),1,fptr);
	 switch_bytes_hp((unsigned char *)&hold_float, sizeof(float) );
	 bfile->chans[iii].gain = hold_float;
      fread(&hold_short,sizeof(short),1,fptr);
	 switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
	 bfile->chans[iii].cal_per = hold_short;
      fread(&hold_short,sizeof(short),1,fptr);
	 switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
	 bfile->chans[iii].cal_amp = hold_short;
      fread(&hold_long,sizeof(long),1,fptr);
	 switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
	 bfile->chans[iii].sampl_count = hold_long;
      for (jjj=0; jjj<9; jjj++)
	 {
	 fread(&hold_char,sizeof(char),1,fptr);
	    bfile->chans[iii].serial[jjj]=hold_char;
	 }
      for (jjj=0; jjj<3; jjj++)
	 {
	 fread(&hold_char,sizeof(char),1,fptr);
	    bfile->chans[iii].filler[jjj]=hold_char;
	 }
      }
   fread(&(bfile->position),sizeof(short),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->position, sizeof(short) );
   fread(&(bfile->filler),sizeof(char),82,fptr);
   fread(&(bfile->id),sizeof(long),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->id, sizeof(long) );
   fread(&(bfile->study),sizeof(short),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->study, sizeof(short) );
   fread(&(bfile->date),sizeof(long),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->date, sizeof(long) );
   fread(&(bfile->time),sizeof(long),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->time, sizeof(long) );
   fread(&(bfile->bfilename),sizeof(char),16,fptr);
   fread(&(bfile->cal_detect),sizeof(short),1,fptr);
      switch_bytes_hp((unsigned char *)&bfile->cal_detect, sizeof(short) );
   return( TRUE );
}



/* ------------------------------------------------------------------------- */
short write_bhead_hp(FILE *fptr,struct B_HEADER *bfile)
 {
   int iii;
   short hold_short;
   long  hold_long;
   float hold_float;

   hold_short = bfile->device_id;
      switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
      if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0 ) return(FALSE);
   hold_long = bfile->bfile_creation_date;
      switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
      if ( fwrite(&hold_long,sizeof(long),1,fptr) == 0)    return(FALSE);
   hold_long = bfile->bfile_creation_time;
      switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
      if ( fwrite(&hold_long,sizeof(long),1,fptr) == 0)    return(FALSE);
   if ( fwrite(&(bfile->bfile_comment[0]),sizeof(char),80,fptr) == 0)
							   return(FALSE);
   hold_float = bfile->prog_version;
      switch_bytes_hp((unsigned char *)&hold_float, sizeof(float) );
      if ( fwrite(&hold_float,sizeof(float),1,fptr) == 0)  return(FALSE);
   hold_short = bfile->input_compr_ratio;
      switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
      if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0 ) return(FALSE);
   hold_short = bfile->output_compr_alg;
      switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
      if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0 ) return(FALSE);
   hold_long = bfile->lcm_spm;
      switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
      if ( fwrite(&hold_long,sizeof(long),1,fptr) == 0)    return(FALSE);
   hold_short = bfile->block;
      switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
      if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0 ) return(FALSE);
   hold_short = bfile->blocksize;
      switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
      if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0 ) return(FALSE);
   hold_short = bfile->chan_cnt;
      switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
      if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0 ) return(FALSE);
	  int LIMIT = MAX_CHANNELS_V2;
	  if (bfile->prog_version < PROG_VERSION_BEFORE_HD)
		  LIMIT = OLD_MAX_CHANNELS;
	  for (iii = 0; iii<LIMIT; iii++)
      {
      hold_short = bfile->chans[iii].index;
	 switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
	 if (fwrite (&hold_short,sizeof(short),1,fptr) == 0 )   return(FALSE);
      if (fwrite(&bfile->chans[iii].label,sizeof(char),LABSIZ,fptr) == 0)
								return(FALSE);
      if (fwrite(&bfile->chans[iii].lead_type,sizeof(char),9,fptr) != 9)
								return(FALSE);
      hold_short = bfile->chans[iii].ad_port_num;
	 switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
	 if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0 ) return(FALSE);
      hold_long = bfile->chans[iii].spm;
	 switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
	 if ( fwrite(&hold_long,sizeof(long),1,fptr) == 0)    return(FALSE);
      hold_float = bfile->chans[iii].timeconst;
	 switch_bytes_hp((unsigned char *)&hold_float, sizeof(float) );
	 if ( fwrite(&hold_float,sizeof(float),1,fptr) == 0)  return(FALSE);
      hold_float = bfile->chans[iii].gain;
	 switch_bytes_hp((unsigned char *)&hold_float, sizeof(float) );
	 if ( fwrite(&hold_float,sizeof(float),1,fptr) == 0)  return(FALSE);
      hold_short = bfile->chans[iii].cal_per;
	 switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
	 if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0 ) return(FALSE);
      hold_short = bfile->chans[iii].cal_amp;
	 switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
	 if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0 ) return(FALSE);
      hold_long = bfile->chans[iii].sampl_count;
	 switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
	 if ( fwrite(&hold_long,sizeof(long),1,fptr) == 0)    return(FALSE);
      if (fwrite(&bfile->chans[iii].serial,sizeof(char),9,fptr) != 9)
								return(FALSE);
      if (fwrite(&bfile->chans[iii].filler,sizeof(char),3,fptr) != 3)
								return(FALSE);
      }
   hold_short = bfile->position;
      switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
      if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0)  return(FALSE);
   if (fwrite(&(bfile->filler),sizeof(char),82,fptr) != 82)   return(FALSE);
   hold_long = bfile->id;
      switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
      if ( fwrite(&hold_long,sizeof(long),1,fptr) == 0)    return(FALSE);
   hold_short = bfile->study;
      switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
      if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0)  return(FALSE);
   hold_long = bfile->date;
      switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
      if ( fwrite(&hold_long,sizeof(long),1,fptr) == 0)    return(FALSE);
   hold_long = bfile->time;
      switch_bytes_hp((unsigned char *)&hold_long, sizeof(long) );
      if ( fwrite(&hold_long,sizeof(long),1,fptr) == 0)    return(FALSE);
   if (fwrite(&(bfile->bfilename),sizeof(char),16,fptr) != 16)  return(FALSE);
   hold_short = bfile->cal_detect;
      switch_bytes_hp((unsigned char *)&hold_short, sizeof(short) );
      if ( fwrite(&hold_short,sizeof(short),1,fptr) == 0)  return(FALSE);
   return( TRUE );
}



/* ------------------------------------------------------------------------- */
short read_bhead_pc(FILE *fptr,struct B_HEADER *bfile)
 {
   int iii,jjj;

   fread(&(bfile->device_id),sizeof(short),1,fptr);
   fread(&(bfile->bfile_creation_date),sizeof(long),1,fptr);
   fread(&(bfile->bfile_creation_time),sizeof(long),1,fptr);
   fread(&(bfile->bfile_comment[0]),sizeof(char),80,fptr);
   fread(&(bfile->prog_version),sizeof(float),1,fptr);
   fread(&(bfile->input_compr_ratio),sizeof(short),1,fptr);
   fread(&(bfile->output_compr_alg),sizeof(short),1,fptr);
   fread(&(bfile->lcm_spm),sizeof(long),1,fptr);
   fread(&(bfile->block),sizeof(short),1,fptr);
   fread(&(bfile->blocksize),sizeof(short),1,fptr);
   fread(&(bfile->chan_cnt),sizeof(short),1,fptr);
   long what = sizeof(long);
   short *pptr = &(bfile->chans[511].ad_port_num);
   what = sizeof(*bfile);
   int LIMIT = MAX_CHANNELS_V2;
  if (bfile->prog_version < PROG_VERSION_BEFORE_HD)
	   LIMIT = OLD_MAX_CHANNELS;
   for (iii = 0; iii<LIMIT; iii++)
   {
      fread(&(bfile->chans[iii].index),sizeof(short),1,fptr);
	  for (jjj=0; jjj<LABSIZ; jjj++)
	  {
	    fread(&(bfile->chans[iii].label[jjj]),sizeof(char),1,fptr);
	  }
      for (jjj=0; jjj<9; jjj++)
	  {
	    fread(&(bfile->chans[iii].lead_type[jjj]),sizeof(char),1,fptr);
	  }
      fread(&(bfile->chans[iii].ad_port_num),sizeof(short),1,fptr);
	  fread(&(bfile->chans[iii].spm),sizeof(long),1,fptr);
	  fread(&(bfile->chans[iii].timeconst),sizeof(float),1,fptr);
	  fread(&(bfile->chans[iii].gain),sizeof(float),1,fptr);
	  fread(&(bfile->chans[iii].cal_per),sizeof(short),1,fptr);
	  fread(&(bfile->chans[iii].cal_amp),sizeof(short),1,fptr);
	  fread(&(bfile->chans[iii].sampl_count),sizeof(long),1,fptr);
	  for (jjj=0; jjj<9; jjj++)
	  {
	    fread(&(bfile->chans[iii].serial[jjj]),sizeof(char),1,fptr);
	  }
      for (jjj=0; jjj<3; jjj++)
	  {
	    fread(&(bfile->chans[iii].filler[jjj]),sizeof(char),1,fptr);
	  }
   }
   fread(&(bfile->position),sizeof(short),1,fptr);
   fread(&(bfile->filler),sizeof(char),82,fptr); 
   fread(&(bfile->id),sizeof(long),1,fptr);
   fread(&(bfile->study),sizeof(short),1,fptr);
   fread(&(bfile->date),sizeof(long),1,fptr);
   fread(&(bfile->time),sizeof(long),1,fptr);
   fread(&(bfile->bfilename),sizeof(char),16,fptr);
   fread(&(bfile->cal_detect),sizeof(short),1,fptr); 
   return( TRUE );
}



/* ------------------------------------------------------------------------- */
short write_bhead_pc(FILE *fptr,struct B_HEADER *bfile)
 {
   int iii;

   if ( fwrite(&bfile->device_id,sizeof(short),1,fptr) == 0 ) return(FALSE);
   if ( fwrite(&bfile->bfile_creation_date,sizeof(long),1,fptr) == 0) 
															return(FALSE);
   if ( fwrite(&bfile->bfile_creation_time,sizeof(long),1,fptr) == 0) 
															return(FALSE);
   if ( fwrite(&(bfile->bfile_comment[0]),sizeof(char),80,fptr) == 0) 
															return(FALSE);
   if ( fwrite(&bfile->prog_version,sizeof(float),1,fptr) == 0) return(FALSE);
   if ( fwrite(&bfile->input_compr_ratio,sizeof(short),1,fptr) == 0 ) 
															return(FALSE);
   if ( fwrite(&bfile->output_compr_alg,sizeof(short),1,fptr) == 0 ) 
															return(FALSE);
   if ( fwrite(&bfile->lcm_spm,sizeof(long),1,fptr) == 0)   return(FALSE);
   if ( fwrite(&bfile->block,sizeof(short),1,fptr) == 0 ) return(FALSE);
   if ( fwrite(&bfile->blocksize,sizeof(short),1,fptr) == 0 ) return(FALSE);
   if ( fwrite(&bfile->chan_cnt,sizeof(short),1,fptr) == 0 ) return(FALSE);
   int LIMIT = MAX_CHANNELS_V2;
   if (bfile->prog_version < PROG_VERSION_BEFORE_HD)
	   LIMIT = OLD_MAX_CHANNELS;
   for (iii = 0; iii<LIMIT; iii++)
   {
		if (fwrite(&bfile->chans[iii].index,sizeof(short),1,fptr) == 0 ) 
															return(FALSE);
		if (fwrite(&bfile->chans[iii].label,sizeof(char),LABSIZ,fptr) == 0) 
															return(FALSE);
		if (fwrite(&bfile->chans[iii].lead_type,sizeof(char),9,fptr) != 9)
															return(FALSE);
		if (fwrite(&bfile->chans[iii].ad_port_num,sizeof(short),1,fptr) == 0 )
															return(FALSE);
		if (fwrite(&bfile->chans[iii].spm,sizeof(long),1,fptr) == 0)   
															return(FALSE);
		if (fwrite(&bfile->chans[iii].timeconst,sizeof(float),1,fptr) == 0)
															return(FALSE);
		if (fwrite(&bfile->chans[iii].gain,sizeof(float),1,fptr) == 0) 
															return(FALSE);
   		if (fwrite(&bfile->chans[iii].cal_per,sizeof(short),1,fptr) == 0 )
															return(FALSE);
		if (fwrite(&bfile->chans[iii].cal_amp,sizeof(short),1,fptr) == 0 )
															return(FALSE);
		if (fwrite(&bfile->chans[iii].sampl_count,sizeof(long),1,fptr) == 0)
															return(FALSE);
		if (fwrite(&bfile->chans[iii].serial,sizeof(char),9,fptr) != 9)
															return(FALSE);
   		if (fwrite(&bfile->chans[iii].filler,sizeof(char),3,fptr) != 3) 
															return(FALSE);
   }
   
   if (fwrite(&bfile->position,sizeof(short),1,fptr) == 0) return(FALSE);
   if (fwrite(&(bfile->filler),sizeof(char),82,fptr) != 82) return(FALSE);
   if (fwrite(&bfile->id,sizeof(long),1,fptr) == 0) return(FALSE);
   if (fwrite(&bfile->study,sizeof(short),1,fptr) == 0) return(FALSE);
   if (fwrite(&bfile->date,sizeof(long),1,fptr) == 0) return(FALSE);
   if (fwrite(&bfile->time,sizeof(long),1,fptr) == 0) return(FALSE);
   if (fwrite(&(bfile->bfilename),sizeof(char),16,fptr) != 16) return(FALSE);
   if (fwrite(&bfile->cal_detect,sizeof(short),1,fptr) == 0) return(FALSE);

   return( TRUE );
}

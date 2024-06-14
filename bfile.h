/* ***************************************************************************
 *  BFILE.H - Definition of the bfile header structure
 *             5-MAY-1990    T. Hoffman
 * Note - MAX_CHANNELS assumed to be defined before the inclusion of this file
 * Modified: 12/07/90 Added 2 new ("Havstad" version) fields to CHAN_TYPE
 *               struct. They are: lead_type[9], and sampl_count. These fields
 *               will not be used by DELREMPC but are used exclusively by
 *               Jim Havstad of Dr. Ellers' project when we send him bfile
 * Modified: 2/19/92 JD Added B_H_SIZE define which is size of header in bytes.
 *		     Used by calc_sample_count function for HPUX portability.
 * Modified: 3/19/1992 D Frasca - Redefined bfile.cal_detect
 * Modified: 17-APR-95 K. Batch - changed cal constant vals
 * Modified: 23-JUL-96 K.Batch - Moved MAY_20_1996 definition from setamps.cpp 
 *			 to bfile.h. This date represents a change in cal factor determination.
 * Modified: 29-MAY-02 R Seres moved some function declartions here from wpic.h
 * ***************************************************************************/

#ifndef BFILE_H			/* test for previously included bfile.h */
#define BFILE_H

#include <stdio.h>	//rjs change may 29 2002 so FILE is defined for the new function declarations

#define LABSIZ  5                       /* # chars in the channel label      */
#define B_H_SIZE   26856 // 1024		/* size of B_HEADER in Microsoft-C   */
#define MAX_POSITION  17		/* max value for position */
#define UNKNOWN_POSITION 0
#define UNKNOWN_SERIAL " ......"

/* vars and consts related to cal detection */

#define CAL_DETECTED	1
#define CAL_NOT_FOUND	2
#define CAL_UNTRIED	3
#define	GOOD_CAL_DATE	 19910523	/* first date of "good" cal signal */
#define ONEVOLT_CAL_DATE 19950310       /* first date of 1V peak to peak cal */

#define MAY_20_1996  (unsigned long)19960520

#define OLD_EEGCALDEF     588.98        /* use this value prior 1V date */
#define NEW_EEGCALDEF     403.745        /* use this on or after 1V date */
#define OLD_EOG0CALDEF    592.95        /* etc. etc. */
#define NEW_EOG0CALDEF    403.745
#define OLD_EOG1CALDEF    565.60
#define NEW_EOG1CALDEF    403.745

#define PROG_VERSION_BEFORE_HD 12.0f//this is the first update that we had HD channels. 

#ifdef WPICMAIN 	   /* we are in a MAIN program. */
     #ifdef __cplusplus    /* all mainlines are C++ files */
       extern "C" {
       float EEGCALDEF;
       float EOG0CALDEF;
       float EOG1CALDEF; }
     #else		   /* but just in case */
       float EEGCALDEF;
       float EOG0CALDEF;
       float EOG1CALDEF;
     #endif
#else			  /* ELSE: we are NOT in a mainline */
     #ifdef __cplusplus   /* but we ARE in a C++ source file */
       extern "C" {
       extern float EEGCALDEF;
       extern float EOG0CALDEF;
       extern float EOG1CALDEF; }
     #else		   /* else we are in a plain C source file */
       extern float EEGCALDEF;    // changed jim
       extern float EOG0CALDEF;   // changed jim
       extern float EOG1CALDEF;   // changed jim
     #endif


#endif

/* ----------------- CHANNEL DEFINITION STRUCTURE -------------------------- */

struct CHAN_TYPE
{
	short   index;                  /* index into this array             */
	char    label[LABSIZ];          /* "EEG","EOG1", "NPT" etc.          */
	char    lead_type[9];           /* EEG lead or extra annotation      */
	short   ad_port_num;            /* port position 0..15               */
	long    spm;                    /* samples per minute ( hz*60 )      */
	float   timeconst;              /* time constant for polygraph       */
	float   gain;                   /* gain factor for amplifier         */
	short   cal_per;                /* average cal signal period         */
	short   cal_amp;                /* average cal signal amplitude      */
	long    sampl_count;            /* # samples collected               */
	char    serial[9];               // serial # of amplifier this channel 
	char    filler[3];              /* space for future fields           */

};

/* ---- DEFINE BINARY FILE HEADER STRUCTURE (USES DEV_FLAGS AS A FIELD) ---- */
struct B_HEADER
{                                       /* board & data collection params    */
	short	device_id;		/* 0=ATLAB board to collect signal   */
					/* 1-255 for future boards/vendors   */
	long    bfile_creation_date;    /* creation date of file YYYYMMDD    */
	long    bfile_creation_time;    /* creation time of file HHMMSS      */
	char    bfile_comment[80];      /* file annotation                   */
	float   prog_version;           /* versn # RECORD creating this file */
	short   input_compr_ratio;      /* input  compression: 16 = 16 to 1  */
	short   output_compr_alg;       /* output compress:0=none 1=standard */
	long    lcm_spm;                /* lcm of the channel spm's          */
	short   block;                  /* data stord in block 0=false1=true */
	short   blocksize;              /* multiple of the lcm of chann Hz's */
	short   chan_cnt;               /* # channels used in this study     */
	struct  CHAN_TYPE chans[MAX_CHANNELS_V2]; /* configs of each channel    */
	short   position;               // polygraph position
	char	filler[82];		/* space for future fields	     */

 /* ------------------- subject study parameters --------------------------- */
	long    id;                     /* upto 6 digit id                   */
	short   study;                  /* study number                      */
	long    date;                   /* date of study YYYYMMDD            */
	long    time;                   /* time of study HHMMSS              */
	char    bfilename[16];          /* B<ID>.<STUDY> output filename     */

 /* ------------------- cal signal information ----------------------------- */
		 /* 1=>cal detected, 2= cal not found 3=cal detect untried */
	short	cal_detect;
  };

//rjs change may 29 2002, moved these here from wpic.h
/* *********************************************************************** */
/* - - - - - - - - - - - - - OPBFHP FUNCTIONS  - - - - - - - - - - - - - - */
/* SWITCH_BYTES_HP -Switches bytes for HP compatibility                    */
/* READ_BHEAD_HP - Enables HP to read binary file header                   */
/* READ_BHEAD_PC - Enables PC to read binary file header                   */
/* WRITE_BHEAD_HP -Enables HP to write binary file header                  */
/* WRITE_BHEAD_PC -Enables PC to write binary file header                  */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifdef __cplusplus    /* all mainlines are C++ files */
 
#endif
	short read_bhead_hp(FILE *fptr, struct B_HEADER *bfile);
	short write_bhead_hp(FILE *fptr, struct B_HEADER *bfile);
	short read_bhead_pc(FILE *fptr, struct B_HEADER *bfile);
	short write_bhead_pc(FILE *fptr, struct B_HEADER *bfile);
#ifdef __cplusplus    /* all mainlines are C++ files */
 
#endif

#endif			/* end of #ifndef BFILE_H */

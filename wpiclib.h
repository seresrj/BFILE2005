/* *********************************************************************** */
/* WPICLIB.H - Holds function prototying for Plain C functions which are   */
/* compiled by the Plain C compiler.                                       */
/* Any C++ module that includes this file must type it as extern "C"       */
/*                                                                         */
/* Modified: 8/17/1992	Debbie Kesicki					   */
/* Modified: 6/25/1993	Jack Doman	Uses WPICMAIN to define fnct pntrs */
/*		in mainline programs and declare extern in library files.  */
/* Modified 10/14/1993 T.Hoffman    Added 4 function protypes to openbfl   */
/*		section. The prototypes are exclusively for GOETZ program  */
/* Modified 04/28/94   T.Hoffman    Added calflags[] arg to calsig()       */
/* Modified 10/28/94   T.Hoffman    Removed prototypes for DELREMP,PROCEEG */
/*                     and PROCREM. Which are C++ functions compiled by    */
/*                     the C++ compiler. Replaced C++ '//' delims.         */
/* Modified 3/14/95    K.Batch   Added assign_cal_constants to DIGSUB      */
/* Modified 5/29/02    R Seres  removed tsd and bfile functions            */
/* Modified 5/29/02    R Seres  made  getdefs and show_studydefs only compile under    */
/*					   c++, since c only defines these in the scope of the declaration */
/*					   on the g++ Linux compiler				           */
/* *********************************************************************** */

#ifndef	WPICLIB_H		/* Check for previously included wpiclib.h */
#define WPICLIB_H

#include "local.h"	//rjs change may 29 2002

#ifdef __cplusplus      //debug jp 
 
#endif /* __cplusplus */

#ifdef WPICMAIN		/* if this is a mainline declare function pointers */
	short (*get_epoch)(unsigned short value[]); /* generator fnct pntr */
	short (*abort_cal) (void);	   /* stop cal detection fnct pntr */
/*	short (*flush) ( unsigned short *buffer, short i,
			 unsigned short *offset, FILE *fptr ); */
#else
	extern short (*get_epoch)(unsigned short value[]);
	extern short (*abort_cal) (void);
/*/	extern short (*flush) ( unsigned short *buffer, short i,
				unsigned short *offset, FILE *fptr ); */

#endif
#ifndef HPUX
/* *********************************************************************** */
/* - - - - - - - - - - - - - RECORDPC FUNCTIONS  - - - - - - - - - - - - - */
/* CREATE_BATCH - Function to add bfile processing to DOSLEEP.BAT file     */
/* END_COLLECT - Disables CTRL_C and sets collect variable to false        */
/* FLUSH - Flushes i'th study buffer of data to disk and incr sampl_count  */
/* FLUSH_CLOSE_STOP_AD - Flushes all buffs, closes files, stops A/D samplng*/
/* GSTUDYDAT - Prompts for or computes info needed to populate bfile header*/
/* GET_N_PLOT_EPOCH - Calls function to plot epoch to screen               */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
short create_batch(struct B_HEADER bh);
short end_collect(void);
short local_flush(unsigned short *buffer,short i,unsigned short *offset,FILE *fptr);
short netwk_flush(unsigned short *buffer,short i,unsigned short *offset,FILE *fptr);
short flush_close_stop_ad(void);
short gstudydat(short *board_scan_cnt, short board_chan_list[],
		short board_gain_list[], long *board_samps_sec,
		long *techstartdate, long *techstarttime);
short get_n_plot_epoch(unsigned short vals[]);


/* *********************************************************************** */
/* - - - - - - - - - - - - - RECPCSUB FUNCTIONS  - - - - - - - - - - - - - */
/* GETDEFS - Opens binary file of channel and study definitions            */
/*           Returns:- a pointer to the study definitions                  */
/*                   - a pointer to the channel definitions                */
/*                   - a count of how many of each was read in             */
/* SHOW_STUDYDEFS - Displays list of study configurations                  */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifdef __cplusplus // STUDY_TYPE is not defined in the c programs that include this file
short getdefs(struct CHAN_TYPE chandefs[], struct STUDY_TYPE studydefs[],
	      short *num_channels_defined, short *num_studies_defined,
	      char err_msg[]);
short show_studydefs(struct STUDY_TYPE studydefs[],
		     struct CHAN_TYPE chandefs[], short num_studies_defined);
#endif
/************************************************************************
 - - - - - - - - - - - - - RECPCPLT FUNCTIONS  - - - - - - - - - - - - -
 ENTER_GRAPHICSE     - Puts CRT into highest resolution color graphics mode
 EXIT_GRAPHICSE      - Takes CRT back back out of graphics mode
 INIT_PLOTTING       - Defines 1 physical vport for plotting, + 2 text wins
 CALC_COORD_SYSTEM   - calcs & defines logical x,y system over plot vport
 REFRESH_PLOT_WIN    - Refreshes plot window and relevant counters etc.
 SET_WIN_STARTDATETIME Sets values of GLOBAL wsd,wst used by refresh_top_win
 GET_WIN_STARTDATETIME Gets values of above
 REFRESH_TOP_TXT_WIN - Refreshes informational text just above plot win
 REFRESH_BOT_TXT_WIN - Refreshes informational text just below plot win
 PLOT_STUDY	       - Plots single tick (epoch) of values for 1 study
 GET_PLOT_COMMAND    - reads kbd commands to update display list & win size
 DO_PLOT_COMMAND     - update current plot vars according to plot command
 GET_WINSIZ	       - returns number of seconds and ticks in next plot win
 SET_ALL_DISPLAY_FLAGS - sets all display_next_cycle flags TRUE excpt ALLCHAN study
 SET_CURR_DISPLAY_FLAG - sets all FALSE except for curr_study's flag gets set TRUE
 parse_hhmm	       - reads military hhmm from kbd char by char
 display_hhmm_str      - puts prompt for militry hhmm at bottom of crt
 ck_hhmm_str	       - validates hhmm string


*/
short enter_graphics(void); /* returns FALSE if can't set graphics board */
void  exit_graphics(void);  /* always expected to succeed */
short init_plotting(char errmsg[], long winsiz, short study_cnt); 
void  calc_coord_system(void);
void  refresh_plot_win(void);
void  set_win_startdatetime( long win_start_date, long win_start_time );
void  get_win_startdatetime( long *win_start_date, long *win_start_time );
void  refresh_top_txt_win(void);
void  refresh_bot_txt_win(void);
short plot_study( unsigned short values[] );
short get_plot_command(void);	// 12-MAY-95 K.B. - void to short
void  do_plot_command(void);
void  get_winsiz( long *secs, long *ticks );
void set_all_display_flags(void);
void set_curr_display_flag(void);
short parse_hhmm( char c, short *hhmm );
void display_hhmm_str( char hhmm_str[], short c_cnt );
short ck_hhmm_str( char hhmm_str[], short *hhmm );
short display_samps_per_sec(void);

#endif		/* end of #ifndef HPUX */

/* *********************************************************************** */
/* - - - - - - - - - - - - - PROCNPT FUNCTIONS - - - - - - - - - - - - - - */
/* PROCNPT - Accumulates "sum" variable, increments "count" variable       */
/* OPEN_NPT - Opens n output file and writes header lines                  */
/* CLOSE_NPT - Writes summary lines and closes n output file               */
/* WRITE_NPT - Writes epoch to n file, accumulates tip, base values        */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
short procnpt(unsigned short tip, unsigned short base);
short open_npt(struct B_HEADER bfile, char prog_name[]);
short close_npt(void);
short write_npt(void);

/* *********************************************************************** */
/* - - - - - - - - - - - - - BLDFLWPC FUNCTIONS  - - - - - - - - - - - - - */
/* END_BLDFLWPC - Flushes remaining buffers and closes all files           */
/* NEXT_BLD - Returns set of EKG and BLD values                            */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void	end_bldflwpc(int x);
short   next_bld(unsigned short samples[]);

/* *********************************************************************** */
/* - - - - - - - - - - - - - PROCBLD FUNCTIONS - - - - - - - - - - - - - - */
/* PROCBLD - Tests for EKG value above EKGVAL limit. Skips PAUSE values    */
/*            then stores the next MSEC BLD1 values in bld array           */
/* SMOOTH_BFL - Resets each value in bld array to diff from mean, calc max */
/* SLOPE_BLD - Returns maximum slope and index of that slope               */
/* OPEN_BLD - Opens the f output file and write header lines               */
/* CLOSE_BLD - Writes summary lines and closes f output file               */
/* WRITE_BLD - Writes epoch to f file, accumulates totals for entire study */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
short procbld(void);
short smooth_bfl(short array[], short smooth_window, long nelements,
						long *maximum, long *index);
short slope_bld(short array[], long index_of_array, float *max_slope,
						      long *indx_slope);
short open_bld(struct B_HEADER bhdr, char prog_name[], short invert);
short close_bld(void);
short write_bld(void);

/* *********************************************************************** */
/* - - - - - - - - - - - - CALSIG FUNCTIONS  - - - - - - - - - - - - - - - */
/* SET_CALFLAGS - Compares chan labels of selected studies with file of    */
/*		  chan labels to be turned off for cal detection	   */
/* CALSIG - Locates 32 consec full waves with equal periods on all chans   */
/* STOP_CAL - De-activates cal sinal detector following CTRL_C             */
/* CALSET - Opens bfile, calls calsig, sets head vars, re-writes header    */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

 void set_calflags( struct B_HEADER bf, short calflags[] );
 short calsig(short nchan, long cal_per[], long cal_amp[], long *lag1,
 			  long *lag2, short plot, long sampersec, short calflags[]);
 void stop_cal(int dummy);
 short calset(char bfilename[], short uresp);


/* *********************************************************************** */
/* - - - - - - - - - - - - - OUTBFL FUNCTIONS - - - - - - - - - - - - - -  */
/* OPENOUTBFL - Opens output binary file for writing                       */
/* OPENRWBFL - Opens output binary file for reading and writing            */
/* WRITEOUTHEAD - Writes binary file header                                */
/* PUT_EPOCH - Stores data values in local array, flushes when needed      */
/* BFLUSH - Flushes data buffer to output file                             */
/* BFLUSH_CLOSE - Flushes buffer and closes file			   */
/* CLOSEOBFL - Closes output binary file				   */
/* CALC_SAMPLE_COUNT - Calculates # of samples in each channel of a bfile  */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
short openoutbfl(struct B_HEADER *bf, char filename[]);                  
short openrwbfl(char filename[], struct B_HEADER *bfile, short check_flag);
short writeouthead(struct B_HEADER *bf);                                 
short put_epoch(unsigned short value[]);
short bflush(void);
short bflush_close(struct B_HEADER bf, char filename[]);
short closeobfl(void);
long calc_sample_count(short chans, float byte_ratio);

/* *********************************************************************** */
/* - - - - - - - - - - - - - OPENBFL FUNCTIONS - - - - - - - - - - - - - - */
/* OPENBFL - Opens binary file and sets buffer pointers                    */
/* READBUF - Reads next input buffer                                       */
/* DISKDAT - Gets one value per channel and increments pointers            */
/* CLOSEBFL - Closes binary file                                           */
/* SKIPDAT - Causes next call to diskdat to return value ticks away	   */
/* OPENGFL - GOETZ version of openbfl                                      */
/* POP_GOETZ_HDR - puts hardcoded values into header                       */
/* GOETZ_DISKDAT - reads epoch from raw GOETZ bfile and parses data	   */
/* END_GOETZ - ^C handler for GOETZ program				   */
/* 3/15/95 - K.Batch commented out the prototypes for openbfl. They are now*/
/* included in class bfile.                                                */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*short openbfl(char filename[], struct B_HEADER *bfile);
short readbuf(void);
short diskdat(unsigned short value[]);
short closebfl(void);
short skipdat(long ticks);
short opengfl(char filenm[],struct B_HEADER *bfile);
void pop_GOETZ_hdr(char filenm[],struct B_HEADER *bfile);
short GOETZ_diskdat(unsigned short value[]);
void end_GOETZ(int dummy);*/

//rjs change may 29 2002 removed the bfile function declarations here
//RJS why is there more than 1? short switch_bytes_hp(unsigned char *baddr, size_t size);

/* *********************************************************************** */
/* - - - - - - - - - - - - DIGSUB FUNCTIONS  - - - - - - - - - - - - - - - */
/* PROC_BFL - Gets bfilename and opens file                                */
/* PROC_GFL - GOETZ version of proc_bfl                                    */
/* CHAN_NAM - Validates channel names                                      */
/* START_SMOOTHING_EEG_REM - Initializes the eeg and rem smoothing arrays  */
/* SKIP_SMP - Determines # samps to skip so analysis starts at epoch start */
/* WRITE_COMMENTS - Opens and writes generic comment lines to output file  */
/* FIX_UP_SAMPLE_COUNT - Sets bfile sampl_count for all channels           */
/* CHECK_SAMPLE_COUNT - Verifies that all chan counts = input sampl_count  */
/* BUILD_HEAD - Builds new head and chan values from old head		   */
/* DISP_HEAD - Displays bfile head values				   */
/* CHECK_HEAD - Validates bfile head values				   */
/* ASSIGN_CAL_CONSTANTS - assigns new cal constants
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void assign_cal_constants (long bdate);
short proc_bfl(char bfilnm[], struct B_HEADER *bfile);
short proc_gfl(char bfilnm[], struct B_HEADER *bfile);
short chan_nam(char *channels[], short min_chan, char bfilnm[],
				 struct B_HEADER bfile, short report);
short start_smoothing_eeg_rem(long filter_len, short eeg_only);
short skip_smp(long *date,long *time,long esize,long sampsec, char bfilnm[]);
short write_comments(FILE *(*f), char filename[], struct B_HEADER bfile,
							      char prog[]);
short fix_up_sample_count(long sample_count, struct B_HEADER *bhead);
short check_sample_count(long sample_count, struct B_HEADER *bhead);
short build_head(struct B_HEADER oldhdr, struct B_HEADER *newhdr, 
		 short new_chans[], short new_chan_cnt); 
short disp_head(struct	B_HEADER  bf, short HHMM );
short check_head(struct  B_HEADER  bf);

//rjs change may 29 2002 removed the TSD file declarations here

#ifndef MAX_BYTE
   #include "header.h"	/* TSD_HEAD struct definition */
#endif



#ifdef __cplusplus
 
#endif

#endif		/* end of #ifndef WPICLIB_H */

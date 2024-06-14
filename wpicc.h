/* ***********************************************************************
 * WPICC.H - Include file for variable declarations in C programs.       *
 *  Last Modified: 5/1/91  D Frasca V8.0 Turned Pragmas off              *
 *  Last Modified: 7/23/91 D Frasca V8.3 Added CAL_CHANS, BLD_CHANS      *
 *  Last Modified: 10/2/91 D Frasca Added WNULL since Cv6.0 NULL= ptr	 *
 *  Last Modified: 7/6/92 D Kesicki Added BELL	 v9.1			 *
 *  Last Modified: 4/24/00 C Pratt Added ADMAX_float_16 and ADZERO_16	 *
 *  Last Modified: 8/07/03 RJS Added function to reference and set ADZERO *
 * ***********************************************************************/
#ifndef	WPICC_H						/* test for previously included wpicc.h */
#define	WPICC_H

#define   ADMAX_float_12	4096.0	   /* highest 12 bit value */
#define   ADZERO_12        2048       /* ZERO on th A/D card                */
#define   ADMAX_float_16	65536.0	   /* highest 12 bit value */
#define   ADZERO_16        32768       /* ZERO on th A/D card                */

#ifdef WPICMAIN 	   /* we are in a MAIN program. */
     #ifdef __cplusplus    /* all mainlines are C++ files */
       extern "C" {
       long	  ADZERO;
	   float  ADMAX_float; }
     #else		   /* but just in case */
       long	  ADZERO;
	   float  ADMAX_float;
     #endif
#else			  /* ELSE: we are NOT in a mainline */
     #ifdef __cplusplus   /* but we ARE in a C++ source file */
       extern "C" {
       extern long	  ADZERO;
	   extern float  ADMAX_float; }
     #else		   /* else we are in a plain C source file */
       extern long	  ADZERO;      // changed jim
	   extern float  ADMAX_float; // changed jim
	 #endif
#endif	/* end of #ifdef WPICMAIN	*/

#define   TRUE             1
#define   FALSE            0
#define   OLD_MAX_CHANNELS    16
#define   MAX_CHANNELS_V2  512//For high density EEG files
#define   CAL_CHANS        3        /* Required chans for cal & delrempc */
#define   BLD_CHANS        2        /* Required chans for bld            */
#define   WNULL            0        /* WPIC NULL to replace NULL pointer */
#define   BELL		   7	    /* Rings terminal bell		 */
#define   TAB		   9
#define   RETURN	  13
#define   BACKSPACE	   8

long	GetADZERO();//rjs added to avoid multiple reference bugs
void	SetADZERO(long afzero);//rjs added to avoid multiple reference bugs

#endif	/* end of #ifndef WPICC_H	*/
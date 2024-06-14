//EDFFILE.H
// Modified : Aug 20 2012 -RJS
// Added error messages for reader

//Modified : Feb 4 2004 -RJS
// Fixed some edf header types that were incorrect
// and added a function that would write out an EDF file as well

// Modified May 7th 2004 - RJS
// Rather than read the entire EDF file into memory, added some functions so 
// an application can read in only a sample set at a time. 

// Modified Jan 5th 2005 - RJS
// Added cal_amp values into header, these are only calcluated from a harmonie export. 

// Modified jan 3rd 2006 - RJS
// Added the ability to read the EDF as "chunks" of data as opposed to the entire file. 

//string structures to be used for arrays
#ifndef _EDFFILE
#define _EDFFILE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>


//#include "fstream.h"
//#include "iomanip.h"

#define CHANNEL_INFORMATION_SIZE 1044
#define MAX_EDFCHANNELS  128
#define MAX_TOTAL_CHANNELS 200

struct string16 {
	char string[16];
};

struct string80 {
	char string[80];
};
 

//sample data structure
struct SAMPLE {
	short	*data;//pointer to reading data
	long	Size;//size of allocated data
	float   Scale;
	long	Count;//number of actual data in record
	SAMPLE()
	{
		data = NULL;
		Size = 0;
		Count = 0;
		Scale = 1.0f;
	}
	void Init(int number)//creates the data array
	{
		data = new short[number];
	//	memset(data, 0, sizeof(unsigned short) * number);
		Size = number;
	}
	void Clean()//cleans the data array
	{
		if(data)
			delete [] data;
	}
	void Expand(short multiple)
	{
		short *old;
		long newsize = Size * multiple;
		old = data;
		data = new  short[newsize];
		long oldindex = 0;
		short count = 0;
		for(long index = 0; index < newsize; index++)
		{
			data[index] = old[oldindex];
			count++;
			if(count == multiple)
			{
				count = 0;
				oldindex++;
			}
		}
	}
};

struct EDF_HEADER
{
	char version_info[8];
	char patient_id[80];
	char recording_id[80];
	char startdate[9];
	char starttime[9];
	int sizeof_header;//how big the header structure is in bytes
	int number_of_data;//how many sample segments we have 
	float	duration;//length in seconds of the readings
	int number_of_signals;//how many wavforms are in the edf file
	string16 *label;//the labels of each signal
	string80 *transducer;//not used for conversion
	string16 *physical_dimension;//not used for conversion
	float *physical_minimum;//not used for conversion
	float *physical_maximum;	//not used for conversion
	short *digital_minimum;//not used for conversion
	short *digital_maximum;//not used for conversion	
	string80 *prefiltering;//not used for conversion
	int *number_of_samples_in_each_record;//this is essentially the sampling rate of the channel
	short *cal_amp;
	short device_id;

	EDF_HEADER()
	{
			//init all pointers to 0
		label = NULL;
		transducer = NULL;
		physical_dimension = NULL;
		physical_minimum = NULL;
		physical_maximum = NULL;
		digital_minimum = NULL;	
		digital_maximum = NULL;
		prefiltering = NULL;
		number_of_samples_in_each_record = NULL;
		cal_amp = NULL;
	}
	void CopyInto(EDF_HEADER *edf)
	{
		InitChannels(edf->number_of_signals);
		strncpy(version_info, edf->version_info, 8);
		strncpy(patient_id, edf->patient_id, 80);
		strncpy(recording_id, edf->recording_id, 80);
 		strncpy(startdate, edf->startdate, 9);
		strncpy(starttime, edf->starttime, 9);
  
		sizeof_header = edf->sizeof_header;
		number_of_data = edf->number_of_data;//how many sample segments we have 
	 	duration = edf->duration;//length in seconds of the readings
		number_of_signals = edf->number_of_signals;//how many wavforms are in the edf file
		device_id = edf->device_id;

		memcpy(label, edf->label, sizeof(string16) * number_of_signals);
		memcpy(transducer, edf->transducer, sizeof(string16)* number_of_signals);
		memcpy(physical_dimension, edf->physical_dimension, sizeof(string16)* number_of_signals);
		memcpy(physical_minimum, edf->physical_minimum, sizeof(float)* number_of_signals);
		memcpy(physical_maximum, edf->physical_maximum, sizeof(float)* number_of_signals);
		memcpy(digital_minimum, edf->digital_minimum, sizeof(short)* number_of_signals);
		memcpy(digital_maximum, edf->digital_maximum, sizeof(short)* number_of_signals);
		memcpy(prefiltering, edf->prefiltering, sizeof(string80)* number_of_signals);
		memcpy(number_of_samples_in_each_record, edf->number_of_samples_in_each_record, sizeof(int)* number_of_signals);
 
	}
	void InitChannels(short NumberOfSignals)
	{

		number_of_signals = NumberOfSignals;
		label = new string16[number_of_signals];
		transducer = new string80[number_of_signals];
		physical_dimension = new string16[number_of_signals]; 
		physical_minimum = new float[number_of_signals]; 
		physical_maximum = new float[number_of_signals]; 
		digital_minimum = new short[number_of_signals]; 
		digital_maximum = new short[number_of_signals]; 
		prefiltering = new string80[number_of_signals]; 
		number_of_samples_in_each_record = new int[number_of_signals]; 
		cal_amp = new short[number_of_signals]; 

	 
	}
	void Clean()
	{
		if(label)
			delete [] label;
		label = NULL;

		if(transducer)
			delete [] transducer;
		transducer = NULL;

		if(physical_dimension)
			delete [] physical_dimension;
		physical_dimension = NULL;

		if(physical_minimum)
			delete [] physical_minimum;
		physical_minimum = NULL;

		if(physical_maximum)
			delete [] physical_maximum;	
		physical_maximum = NULL;

		if(digital_minimum)
			delete [] digital_minimum;
		digital_minimum = NULL;

		if(digital_maximum)
			delete [] digital_maximum;	
		digital_maximum = NULL;

		if(prefiltering)
			delete [] prefiltering;
		prefiltering = NULL;

		if(number_of_samples_in_each_record)
			delete [] number_of_samples_in_each_record;
		number_of_samples_in_each_record = NULL;

		if(cal_amp)
			delete [] cal_amp;

	}
};

struct SAMPLE_CHUNK
{
	short NumberOfSamples;
	SAMPLE *samples; 
	void Clean()
	{
		for (int index = 0; index < NumberOfSamples; index++)
		{
			samples[index].Clean();
		}
		delete [] samples;
	}
};

class EDF_FILE
{
public :
	EDF_HEADER					EdfHeader;//edf file header
	SAMPLE						*samples; //the wavform data readings
	int							*data_indecies; //an array of indices, an index into the samples array , since each channel may have a different number of samples
 	FILE						*EdfFile; //class now keeps its own file ptr
	long						endofheader;//marker so we know where to rewind to get to the data
	float						*minf, *maxf;
	double						*mind, *maxd;
	double						slpden;
	short						LargestRecordSize;
	long						SizeOfSample;
	long						FileSize;
	bool						*ignore;//if true, ignore this channel for processing. 
	short						CurrentWriteIndex;
	char						LastError[256];
											
			EDF_FILE();
			~EDF_FILE();
	bool	ReadEDFFile(char filename[]);
	bool	ReadEDFHeader(bool bDoNotCreateBuffer = false);
	void	CloseEDFFile();
	bool	OpenEDFFile(char filename[]);
	bool	OpenEDFFileForWriting(char filename[]);
	bool	ReadEDFSample();
	bool	ReadEDFSampleChunk(SAMPLE_CHUNK *chunk);
	bool	WriteEDFEpoch(unsigned short chans[]);
	bool	WriteEDFHeader();
	bool	WriteEDFFile(char filename[]);
	bool	OpenEDFFileForReadWrite(char filename[]);
//	bool	CheckEndOfFile(fstream *file);
	void	Clean();
	void	IgnoreChannel(short index){ ignore[index] = true;};
	void	ExpandChannel(short channel, short NewSampleRate);
	void	InitChannels(short NumberOfChannels);
	void	InitSampleChunk(SAMPLE_CHUNK *chunk);
	void	Rewind();
	short   GetData(int chan, int count);
};
#endif 

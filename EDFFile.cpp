//EDFFILE.CPP

// Modified : Aug 20 2012 -RJS
// Added error messages for reader

//Modified : Feb 4 2004 -RJS
// Fixed some edf header types that were incorrect
// and added a function that would write out an EDF file as well

// Modified May 5th 2004 -RJS
// Fixed calculation of header size incase channels were ignored

// Modified May 7th 2004 - RJS
// Rather than read the entire EDF file into memory, added some functions so 
// an application can read in only a sample set at a time. 

// Modified May 15th 2005 - RJS
// Added functionality so the bfile class can read in an EDF and treat it like a 
// bfile. Also added ability to store cal-amps and device ID in an EDF file.

// Modified jan 3rd 2006 - RJS
// Added the ability to read the EDF as "chunks" of data as opposed to the entire file. 
 
#include "stdafx.h"
 
#include "EDFFile.h"
#include "string.h"
#include "generic.h"
#include "wpicc.h"

#define MESSAGEOUT(x)  fprintf(stderr,x);

EDF_FILE::EDF_FILE()
{
	minf = NULL;
	maxf = NULL;
	mind = NULL;
	maxd = NULL;
	ignore = NULL;
	EdfFile = NULL;
	LargestRecordSize = 0;
	SizeOfSample = 0;
	CurrentWriteIndex = 0;
	samples = NULL;
	data_indecies = NULL;
	endofheader = 0;
}
//////////////////////////////////////////////////////////////////////////////
EDF_FILE::~EDF_FILE()
{
	if(minf)
		delete [] minf;
	minf = NULL;
	if(maxf)
		delete [] maxf;
	maxf = NULL;
	if(mind)
		delete [] mind;
	mind = NULL;
	if(maxd)
		delete [] maxd;
	maxd = NULL;

}
//////////////////////////////////////////////////////////////////////////////
void EDF_FILE::Clean()
{
	//delete any space we may have allocated
	EdfHeader.Clean();
	if(samples)
	{
		for(int index = 0; index < EdfHeader.number_of_signals; index++)
		{
			samples[index].Clean();
		}
		delete [] samples;
		samples = NULL;
	}
	if(data_indecies)
	{
		delete [] data_indecies;
		data_indecies= NULL;
	}
	if(ignore)
		delete [] ignore;
	ignore = NULL;
}
 
//////////////////////////////////////////////////////////////////////////////
bool EDF_FILE::ReadEDFFile(char filename[])
{
 
	OpenEDFFile(filename);
	ReadEDFHeader();
	//init space for the wav form data
	int index;
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(EdfHeader.number_of_samples_in_each_record[index]  * EdfHeader.number_of_data < 1)
		{
			MESSAGEOUT("Error in EDF/REC file: number of signals negative\n")
			Clean();
			return 0;
		}
		samples[index].Init(EdfHeader.number_of_samples_in_each_record[index]  * EdfHeader.number_of_data);
	}
	//read in all the wav form data
 
 

	for(int x = 0; x < EdfHeader.number_of_data; x++)
	{
		for(index = 0; index < EdfHeader.number_of_signals; index++)
		{
			for(int data_index = 0; data_index < EdfHeader.number_of_samples_in_each_record[index]; data_index++)
			{
				fread((char*)&samples[index].data[data_indecies[index]], sizeof(char), 2, EdfFile);	 

				if((short)samples[index].data[data_indecies[index]] > (short)maxf[index])
					maxf[index] = (short)samples[index].data[data_indecies[index]];
				if((short)samples[index].data[data_indecies[index]] < (short)minf[index])
					minf[index] = (short)samples[index].data[data_indecies[index]];
 
				data_indecies[index]++;
			}
		}
	}
//close edf file
 
	fclose(EdfFile);
	return true;
}
///////////////////////////////////////////////////////////////////////////////
bool EDF_FILE::WriteEDFHeader()
{
 
	if(!EdfFile)
		return false;
	fwrite(EdfHeader.version_info, sizeof(char), 8, EdfFile);
	//EdfFile.write(EdfHeader.version_info, 8);

	fwrite(EdfHeader.patient_id, sizeof(char), 80, EdfFile);
	//EdfFile.write(EdfHeader.patient_id, 80);

	fwrite(EdfHeader.recording_id, sizeof(char), 80, EdfFile);
	//EdfFile.write(EdfHeader.recording_id, 80);

	//write start date and time
	fwrite(EdfHeader.startdate, sizeof(char), 8, EdfFile);
	//EdfFile.write(EdfHeader.startdate, 8);
 
	fwrite(EdfHeader.starttime, sizeof(char), 8, EdfFile);
 	//EdfFile.write(EdfHeader.starttime, 8);

	//write size of header and convert it to a number
	short RealCount = 0;
	 
	if(ignore)
	for(int x = 0; x < EdfHeader.number_of_signals; x++)
	{
		if(!ignore[x])
			RealCount ++;
		
	} 
	else
		RealCount = EdfHeader.number_of_signals;
	EdfHeader.sizeof_header = RealCount * 256 + 256;
	char junk[44];
	sprintf(junk, "%-8d", EdfHeader.sizeof_header);
	fwrite(junk, sizeof(char), 8, EdfFile);
 
	memset(junk, 0, sizeof(char) * 44);
	sprintf(junk, "%d ", EdfHeader.device_id);
	memset(junk, 0, sizeof(char) * 32);

	fwrite(junk,sizeof(char), 44, EdfFile);

	//convert number_of_data to a string and write it out
	sprintf(junk, "%-8d", EdfHeader.number_of_data);
	fwrite(junk, sizeof(char), 8, EdfFile);
	//EdfFile.write(junk, 8);
 
	//convert duration to a string and write it out
	sprintf(junk, "%-8d", EdfHeader.duration);
	fwrite(junk, sizeof(char), 8, EdfFile);
	//EdfFile.write(junk, 8);
 
	//convert number of signals to a string and write it out

	sprintf(junk, "%-4d", RealCount);
	fwrite(junk, sizeof(char), 4, EdfFile);
	//EdfFile.write(junk, 4);
 
	int index ;

	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore || !ignore[index])
		{
			fwrite(EdfHeader.label[index].string, sizeof(string16), 1, EdfFile);
			//EdfFile.write(EdfHeader.label[index].string, sizeof(string16));			          
		} 
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore || !ignore[index])
		{
			fwrite(EdfHeader.transducer[index].string, sizeof(string80), 1, EdfFile);
			//EdfFile.write(EdfHeader.transducer[index].string, sizeof(string80));
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore || ignore[index])
		{
			strncpy(junk, EdfHeader.physical_dimension[index].string, 8);
			sprintf(junk, "%-8s", junk);
			junk[9] = '\0';
			fwrite(junk, sizeof(char), 8, EdfFile);
			//EdfFile.write(junk, 8);
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore ||!ignore[index])
		{
			sprintf(junk, "%-8.2f", EdfHeader.physical_minimum[index]);
			fwrite(junk, sizeof(char), 8, EdfFile);
			//EdfFile.write(junk, 8);
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore ||!ignore[index])
		{
			sprintf(junk, "%-8.2f", EdfHeader.physical_maximum[index]);
			fwrite(junk, sizeof(char), 8, EdfFile);
			//EdfFile.write(junk, 8);
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore ||ignore[index])
		{
			sprintf(junk, "%-8d", EdfHeader.digital_minimum[index]);
			fwrite(junk, sizeof(char), 8, EdfFile);
			//EdfFile.write(junk, 8);
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore ||!ignore[index])
		{
			sprintf(junk, "%-8d", EdfHeader.digital_maximum[index]);
			fwrite(junk, sizeof(char), 8, EdfFile);
			//EdfFile.write(junk, 8);
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore ||!ignore[index])
			fwrite(EdfHeader.prefiltering[index].string, sizeof(string80), 1, EdfFile);
			//EdfFile.write(EdfHeader.prefiltering[index].string, sizeof(string80));
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore ||!ignore[index])
		{
			sprintf(junk, "%-8d", EdfHeader.number_of_samples_in_each_record[index]);
			fwrite(junk, sizeof(char), 8, EdfFile);
			//EdfFile.write(junk, 8);
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore ||ignore[index])
		{
			
		//	sprintf(junk, "%-32d", EdfHeader.cal_amp[index]);

			memset(junk, 0, sizeof(char) * 32);
			fwrite(junk, sizeof(char), 32, EdfFile);

		}
	}
 	
	return true;
}
//////////////////////////////////////////////////////////////////////////////
bool EDF_FILE::WriteEDFFile(char filename[])
{
 
	//its assumed that the parent program already checked if the file exists or not
	EdfFile = fopen(filename, "wb");
	
	WriteEDFHeader();
	//write out all the wav form data
 
 	memset(data_indecies, 0, sizeof(int) * EdfHeader.number_of_signals);

	int x;
	int index;

	for(x = 0; x < EdfHeader.number_of_data; x++)
	{
		for(index = 0; index < EdfHeader.number_of_signals; index++)
		{
			if(!ignore[index])
			{
				for(int data_index = 0; data_index < EdfHeader.number_of_samples_in_each_record[index]; data_index++)
				{
					fwrite((char*)&samples[index].data[data_indecies[index]], sizeof(short), 1, EdfFile);	 
					//fwrite((char*)&samples[index].data[data_indecies[index]], sizeof(char), 2, EdfFile);
					data_indecies[index]++;
				}
			}
		}
	}
//close edf file
 
	fclose(EdfFile);
	 
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void EDF_FILE::ExpandChannel(short channel, short NewSampleRate)
{
	samples[channel].Expand(NewSampleRate / EdfHeader.number_of_samples_in_each_record[channel]);
	EdfHeader.number_of_samples_in_each_record[channel] = NewSampleRate;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
short EDF_FILE::GetData(int chan, int d){
	return samples[chan].data[d];
}
bool EDF_FILE::ReadEDFHeader(bool bDoNotCreateBuffer)
{
	LastError[0] = '\0';
	endofheader = 0;
	//read header information for the edf file	
	fread(EdfHeader.version_info, sizeof(char), 8, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	
	fread(EdfHeader.patient_id, sizeof(char), 80, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	
	fread(EdfHeader.recording_id, sizeof(char), 80, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	char temp[9];
	temp[8] = '\0';
	char junk[128];
	//read start date and time
	fread(EdfHeader.startdate, sizeof(char), 8, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	EdfHeader.startdate[8] = '\0';

	fread(EdfHeader.starttime, sizeof(char), 8, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	EdfHeader.starttime[8] = '\0';

	//read size of header and convert it to a number
	fread(temp, sizeof(char), 8, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	EdfHeader.sizeof_header = atoi(temp);
		//read reserved bytes
	fread(junk, sizeof(char), 44, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	junk[2] = '\0';
	EdfHeader.device_id = atoi(junk);
	//read size of header and convert it to a number
	
	//read number of data records
	fread(temp, sizeof(char), 8, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	EdfHeader.number_of_data = atoi(temp);
 
	if(EdfHeader.number_of_data < 1)
	{
		 
	//	sprintf(LastError, "Number for data is 0");
	 
		//return 0;
	}

	//read duration of wav in seconds and convert to a number
	fread(temp, sizeof(char), 8, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	EdfHeader.duration = atof(temp);
 
	//read number of signals and convert to a number
	fread(temp, sizeof(char), 4, EdfFile);
	if(ferror(EdfFile))
	{
		return 0;
	}
	EdfHeader.number_of_signals = atoi(temp);
	if(EdfHeader.number_of_signals < 1)
	{
		MESSAGEOUT("Error in EDF/REC file: number of signals is less than 1\n")
		Clean();
		return 0;
	}
	
	minf = new float[EdfHeader.number_of_signals];
	maxf = new float[EdfHeader.number_of_signals];

	mind = new double [EdfHeader.number_of_signals];
	maxd = new double [EdfHeader.number_of_signals];
	
	ignore = new bool[EdfHeader.number_of_signals]; 
	//init all our data structures dependant on number of signals
	EdfHeader.label = new string16[EdfHeader.number_of_signals];
	EdfHeader.transducer = new string80[EdfHeader.number_of_signals];
	EdfHeader.physical_dimension = new string16[EdfHeader.number_of_signals];
	EdfHeader.physical_minimum = new float[EdfHeader.number_of_signals];
	EdfHeader.physical_maximum = new float [EdfHeader.number_of_signals];	
	EdfHeader.digital_minimum = new short[EdfHeader.number_of_signals];
	EdfHeader.digital_maximum = new short[EdfHeader.number_of_signals];
	EdfHeader.prefiltering = new string80[EdfHeader.number_of_signals];
	EdfHeader.number_of_samples_in_each_record = new int[EdfHeader.number_of_signals];
	EdfHeader.cal_amp = new short[EdfHeader.number_of_signals];
	memset(EdfHeader.cal_amp, 0, sizeof(short) * EdfHeader.number_of_signals);

	samples = new SAMPLE[EdfHeader.number_of_signals];
	data_indecies = new int[EdfHeader.number_of_signals];
	

	memset(data_indecies, 0, sizeof(int) * EdfHeader.number_of_signals);
	memset(ignore, 0, sizeof(bool) * EdfHeader.number_of_signals);


 
	int index ;

	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		//EdfFile.read(EdfHeader.label[index].string, sizeof(string16));
		fread(EdfHeader.label[index].string, sizeof(string16), 1, EdfFile);

		if(ferror(EdfFile))
		{
			return 0;
		}
		if (strlen(EdfHeader.label[index].string) > 15) {
			EdfHeader.label[index].string[15] = '\0';//cap really long label names
		}
		char *end = strchr(EdfHeader.label[index].string, ' ');

		if (strchr(EdfHeader.label[index].string, ' ')) {
			*end = '\0';
		}
		//apply an end of string delimiter after the first non space
		//from the end
		//while(*end == ' ' && end != &EdfHeader.label[index].string[0])
		//{
		//	end --;
		//}
		//*(end + 1) = '\0';

	}

	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		//EdfFile.read(EdfHeader.transducer[index].string, sizeof(string80));

		fread(EdfHeader.transducer[index].string, sizeof(string80), 1, EdfFile);
		if(ferror(EdfFile))
		{
			return 0;
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		//EdfFile.read((char*)&EdfHeader.physical_dimension[index].string, 8);
		fread((char*)&EdfHeader.physical_dimension[index].string, sizeof(char), 8, EdfFile);
		if(ferror(EdfFile))
		{
			return 0;
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		fread(temp, sizeof(char), 8, EdfFile);
		if(ferror(EdfFile))
		{
			return 0;
		}
		EdfHeader.physical_minimum[index] = (float)atof(temp);
	}


	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		fread(temp, sizeof(char), 8, EdfFile);
		if(ferror(EdfFile))
		{
			return 0;
		}
		EdfHeader.physical_maximum[index] = (float)atof(temp);
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		fread(temp, sizeof(char), 8, EdfFile);
		if(ferror(EdfFile))
		{
			return 0;
		}
		EdfHeader.digital_minimum[index] = atoi(temp);
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		fread(temp, sizeof(char), 8, EdfFile);
		if(ferror(EdfFile))
		{
			return 0;
		}
		EdfHeader.digital_maximum[index] = atoi(temp);
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
//		EdfFile.read(EdfHeader.prefiltering[index].string, sizeof(string80));
		fread(EdfHeader.prefiltering[index].string, sizeof(string80), 1, EdfFile);
		if(ferror(EdfFile))
		{
			return 0;
		}
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		//EdfFile.read(temp, 8);
		fread(temp, sizeof(char), 8, EdfFile);
		if(ferror(EdfFile))
		{
			return 0;
		}
		EdfHeader.number_of_samples_in_each_record[index] = atoi(temp);
		if(EdfHeader.number_of_samples_in_each_record[index] > LargestRecordSize)
			LargestRecordSize = EdfHeader.number_of_samples_in_each_record[index];
	}
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		fread(junk, sizeof(char), 32, EdfFile);
		short amp = atoi(junk);

		if(amp != 0 || junk[0] == '0')
			EdfHeader.cal_amp[index] = amp;
		else
			EdfHeader.cal_amp[index] = -1;
		if(ferror(EdfFile))
		{
			return 0;
		}
	}
      //init min and max value for each channel
    for (index = 0; index < EdfHeader.number_of_signals; index++)
	{
		maxf[index] = 32768;
		minf[index] = 32767;
	}
	//init space for the wav form data

	//read in all the wav form data
 
	//do some error checking on the file size
	long buffersize = 0;
	if(!bDoNotCreateBuffer) 
		for(index = 0; index < EdfHeader.number_of_signals; index++)
		{
		//	if(EdfHeader.number_of_samples_in_each_record[index]  * EdfHeader.number_of_data < 1)
			{
		//		MESSAGEOUT("Error in EDF/REC file: number of signals negative\n")
		//		Clean();
		//		return 0;
			}
			samples[index].Init(EdfHeader.number_of_samples_in_each_record[index]);
			buffersize += EdfHeader.number_of_samples_in_each_record[index];
		}
	endofheader = ftell(EdfFile);
	short bob = samples[3].data[0];
	if(EdfHeader.number_of_data < 0)
		EdfHeader.number_of_data = (FileSize - EdfHeader.sizeof_header)/EdfHeader.sizeof_header;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void EDF_FILE::CloseEDFFile()
{
	if(EdfFile)
		fclose(EdfFile);
	EdfFile = NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool EDF_FILE::OpenEDFFile(char filename[])
{
	
	if(!FileExists(filename))
	{
		char error_string[128];
		sprintf(error_string, "ERROR : File %s not found \n", filename);
		MESSAGEOUT(error_string);
		return false;
	}
	EdfFile = fopen(filename, "rb");
	if(!EdfFile)
		return false;
	fseek( EdfFile, 0, SEEK_END );
 
// get the file size
	FileSize = ftell( EdfFile );
	fclose(EdfFile);
	EdfFile = fopen(filename, "rb");
	if(!EdfFile)
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool EDF_FILE::OpenEDFFileForWriting(char filename[])
{
 
	EdfFile = fopen(filename, "wb");
	if(!EdfFile)
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////
bool EDF_FILE::ReadEDFSampleChunk(SAMPLE_CHUNK *chunk)
{
	short index;
	long count;
 
	for(index = 0; index < EdfHeader.number_of_signals ; index++)
	{
 
		if(!EdfFile)
			return 0;
		count = fread(&chunk->samples[index].data[0], sizeof(short), EdfHeader.number_of_samples_in_each_record[index], EdfFile);
		if(feof(EdfFile))
		{
			fclose(EdfFile);
			EdfFile = NULL;
			return 0;
		}
		if(count == 0)
		{
			printf("\nFata Error reading file.\n");
			return 0;
		}
 
	 
	}// end for

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////
bool EDF_FILE::ReadEDFSample()
{
	short index;
	long count;


	// Initalize the data to zero 
	memset(data_indecies, 0, sizeof(int) * EdfHeader.number_of_signals);
	//if not allocated, allocated memory for the data channels

 
	for(index = 0; index < EdfHeader.number_of_signals ; index++)
	{
		for(int data_index = 0; data_index < EdfHeader.number_of_samples_in_each_record[index]; data_index++)
		{
			if(!EdfFile)
				return 0;
			count = fread(&samples[index].data[data_indecies[index]], sizeof(short), 1, EdfFile);
		//	samples[index].data[data_indecies[index]] = rand() % 256;
			if(feof(EdfFile))
			{
				fclose(EdfFile);
				EdfFile = NULL;
				return 0;
			}
			if(count == 0)
			{
				printf("\nFata Error reading file.\n");
				return 0;
			}
		/*	if((short)samples[index].data[data_indecies[index]] > (short)maxf[index])
				maxf[index] = (short)samples[index].data[data_indecies[index]];
			if((short)samples[index].data[data_indecies[index]] < (short)minf[index])
				minf[index] = (short)samples[index].data[data_indecies[index]];*/
			data_indecies[index]++;
		}
	}// end for

	return true;
}
////////////////////////////////////////////////////////////////////////////
bool EDF_FILE::WriteEDFEpoch(unsigned short chans[])
{
	//NOTE:: For this function to work like it does in a bfile, the sampling rate of all 
	//the channels must be the same. 
	if(!EdfFile)
		return false;
	short RealIndex = 0;
	if(!samples)
	{
		samples = new SAMPLE[EdfHeader.number_of_signals];
		for(int index = 0; index < EdfHeader.number_of_signals; index++)
		{
			if(!ignore[index])
				samples[RealIndex++].Init(EdfHeader.number_of_samples_in_each_record[index]);	
		}

		RealIndex = 0;
	}

	int index;
	for(index = 0; index < EdfHeader.number_of_signals; index++)
	{
		if(!ignore[index])
#ifdef WINDOWS
			samples[RealIndex].data[CurrentWriteIndex] = (short)(chans[RealIndex++] - GetADZERO()); 
#else
			samples[RealIndex].data[CurrentWriteIndex] = (short)(chans[RealIndex++] - ADZERO);
#endif

	}

	CurrentWriteIndex ++;	

	RealIndex = 0;
	if(CurrentWriteIndex >= EdfHeader.number_of_samples_in_each_record[0])
	{
	  
		for(index = 0; index < EdfHeader.number_of_signals; index++)
			if(!ignore[index])
				if(!fwrite((char*)&samples[RealIndex++].data[0], sizeof(short), EdfHeader.number_of_samples_in_each_record[0], EdfFile))
					return false;
	 
		CurrentWriteIndex = 0;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////
void EDF_FILE::InitChannels(short NumberOfChannels)
{
	EdfHeader.number_of_signals = NumberOfChannels;
	EdfHeader.InitChannels(NumberOfChannels);
	ignore = new bool[NumberOfChannels];
	memset(ignore, 0, sizeof(bool) * NumberOfChannels);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool EDF_FILE::OpenEDFFileForReadWrite(char filename[])
{
	if(!FileExists(filename))
	{
		char error_string[128];
		sprintf(error_string, "ERROR : File %s not found \n", filename);
		MESSAGEOUT(error_string);
		return false;
	}
	EdfFile = fopen(filename, "rb+");
	if(!EdfFile)
		return false;
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  EDF_FILE::InitSampleChunk(SAMPLE_CHUNK *chunk)
{
	chunk->NumberOfSamples = EdfHeader.number_of_signals;
	chunk->samples = new SAMPLE[chunk->NumberOfSamples];
	for(int index = 0; index < EdfHeader.number_of_signals; index++)
	{
		chunk->samples[index].Init(EdfHeader.number_of_samples_in_each_record[index]);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void EDF_FILE::Rewind()
{
	if(EdfFile)
	{
		rewind(EdfFile);
		fseek(EdfFile, endofheader, SEEK_SET);
	}
}
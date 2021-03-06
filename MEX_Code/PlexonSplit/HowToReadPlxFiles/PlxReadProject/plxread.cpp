/*

  1. PLX FILE STRUCTURE
  ---------------------

  Definitions of all the structures age given in the file "Plexon.h"

  PLX file has the following general structure:
	- file header (PL_FileHeader structure)
	- a number of DSP channel headers (PL_ChanHeader structures)
	- a number external event headers (PL_EventHeader structures)
	- a number of slow channel headers (PL_SlowChannelHeader structures)
	- data blocks

  Each data block consists of either:
   - the block header (PL_DataBlockHeader structure)
   or:
   - the block header plus an array of waveform values (short integers)

  An array of waveform values follows the block header if
	PL_DataBlockHeader.NumberOfWaveforms > 0

  Here is the minimum code that reads a PLX file:

  -------------------------------------------------
  void ReadPlx(){
	PL_FileHeader fh;
	PL_ChanHeader channels[128];
	PL_EventHeader evheaders[512];
	PL_SlowChannelHeader slowheaders[64];
	PL_DataBlockHeader db;
	short buf[256];
	int nbuf;
	FILE* fp;

  	fp = fopen("test1.plx", "rb");
	// first, read file header
	fread(&fh, sizeof(fh), 1, fp); 
	// then, read channel headers
    if(fh.NumDSPChannels > 0) fread(channels, fh.NumDSPChannels*sizeof(PL_ChanHeader), 1, fp);
	if(fh.NumEventChannels> 0) fread(evheaders, fh.NumEventChannels*sizeof(PL_EventHeader), 1, fp);
	if(fh.NumSlowChannels) fread(slowheaders, fh.NumSlowChannels*sizeof(PL_SlowChannelHeader), 1, fp);
	// read data blocks
	while(feof(fp) == 0 
	     && fread(&db, sizeof(db), 1, fp) == 1){ // read the block header
		if(db.NumberOfWaveforms > 0){ // read the waveform after the block
			nbuf = db.NumberOfWaveforms*db.NumberOfWordsInWaveform;
			fread(buf, nbuf*2, 1, fp);
		}
		// analyze the data block here
	}
  }
  -------------------------------------------------------


   2. HEADERS
   ----------

   2A. PL_FileHeader

    Here are some of the fields of this structure:

	int		ADFrequency; // Timestamp frequency in hertz
	int		NumDSPChannels; // Number of DSP channel headers in the file
	int		NumEventChannels; // Number of Event channel headers in the file
	int		NumSlowChannels; // Number of A/D channel headers in the file
	int		NumPointsWave; // Number of data points in waveform
	int		NumPointsPreThr; // Number of data points before crossing the threshold
	int		Year; // when the file was created
	int		Month; // when the file was created
	int		Day; // when the file was created
	int		Hour; // when the file was created
	int		Minute; // when the file was created
	int		Second; // when the file was created

    int		TSCounts[130][5]; // number of timestamps[channel][unit]
	int		WFCounts[130][5]; // number of waveforms[channel][unit]
	int		EVCounts[512];    // number of timestamps[event_number]

	
	2B. CHANNEL HEADERS

    Each channel header contians channel name that user has specified
	in the Sort Client. DSP channel headers also contain sorting parameters
	
    A/D channel headers also store A/D frequencies and gains.

	See file Plexon.h for details. 

 
    2C. PL_DataBlockHeader

    short	Type; // 1 - spike, 4 - external event, 5 - a/d data
	short	UpperByteOf5ByteTimestamp; // upper byte of the 5-byte timestamp
	long	TimeStamp; // lower 4 bytes of the timestamp. 
				// All the timestamps are in ticks, i.e. 40000 is one second in typical configuration
	short	Channel; // channel number, interpretation depends on Type
	short	Unit; // unit number, interpretation depends on Type
	short	NumberOfWaveforms; // number of waveforms that follow this header
	short	NumberOfWordsInWaveform; // number of short integers in each waveform

		2C.1. Spike Records (Type = 1)

		TimeStamp - spike timestamp, time of threshold crossing
		Channel - 1-based DSP channel number (1 to number of DSP channels)
		Unit    - unit number, (0 to 4), 0 - invalid unit, 1,2,3,4 - units a,b,c,d
		NumberOfWaveforms - if 0, there is no waveform for this spike
		                    if 1, spike waveform follows this data block header
		NumberOfWordsInWaveform - number of raw A/D values (short integers) in the waveform

        Converting from waveform A/D values to voltages:

		voltage = (a_d_value*3./2048.)/gain

  		
		2C.2. Event Records (Type = 4)

		TimeStamp - event timestamp
		Channel - 1-based external event channel number (1 to 300)
		Unit    - if Channel = PL_StrobedExtChannel (257), unit is the strobed value
				  otherwise, Unit is 0
 
 		
		2C.3. A/D Records (Type = 5)

		TimeStamp - timestamp of the first A/D value 
					that follow this data block header
		Channel - 0-based A/D channel number (0 to 63)
		Unit    - always 0
		NumberOfWaveforms - always 1
		NumberOfWordsInWaveform - number of raw A/D values 
		                          (short integers) that follow this data block header

		Converting from A/D values to voltages:

		voltage = (a_d_value*5./2048.)/gain



	3. SAMPLE PROGRAM
	-----------------

    The program listed below does the following:
	- opens a PLX file
	- prints out the file header information
	- prints out the timestamps for DSP channel 1, unit a
	- prints out the waveforms for DSP channel 1, unit a
	- prints out the timestamps for external event channel 1
	- prints out the timestamps and voltages for A/D channel 0

*/




#include <windows.h>
#include <stdio.h>
#include "Plexon.h"

void printheaderinfo(PL_FileHeader& fh);

void main()
{
	FILE* fp = fopen("test1.plx", "rb");
	if(fp == 0){
		printf("Cannot open test1.plx!");
		exit(1);
	}
	PL_FileHeader fh;
	// first, read file header
	fread(&fh, sizeof(fh), 1, fp);

	// print file header information
	printheaderinfo(fh);

	// read channel headers
	PL_ChanHeader channels[128];
	PL_EventHeader evheaders[512];
	PL_SlowChannelHeader slowheaders[64];

	memset(channels, 0, 128*sizeof(PL_ChanHeader));
	memset(evheaders, 0, 512*sizeof(PL_EventHeader));
	memset(slowheaders, 0, 64*sizeof(PL_SlowChannelHeader));

	if(fh.NumDSPChannels > 0)
		fread(channels, fh.NumDSPChannels*sizeof(PL_ChanHeader), 1, fp);
	if(fh.NumEventChannels> 0)
		fread(evheaders, fh.NumEventChannels*sizeof(PL_EventHeader), 1, fp);
	if(fh.NumSlowChannels)
		fread(slowheaders, fh.NumSlowChannels*sizeof(PL_SlowChannelHeader), 1, fp);
	
	PL_DataBlockHeader db;
	short buf[256];
	// where the data starts
	int datastart = sizeof(fh) + fh.NumDSPChannels*sizeof(PL_ChanHeader)
						+ fh.NumEventChannels*sizeof(PL_EventHeader)
						+ fh.NumSlowChannels*sizeof(PL_SlowChannelHeader);

	int nbuf;

	// SAMPLE 1:
	// read the timestamps for a spike channel
	//

	int channel_to_extract = 1; // dsp channel numbers are 1-based
	int unit_to_extract = 1;
	int count = 1;
	printf("\nTimestamps for channel %d, unit %d\n", 
		channel_to_extract, unit_to_extract);

	while(feof(fp) == 0 && fread(&db, sizeof(db), 1, fp) == 1){ // read the block
		nbuf = 0;
		if(db.NumberOfWaveforms > 0){ // read the waveform after the block
			nbuf = db.NumberOfWaveforms*db.NumberOfWordsInWaveform;
			fread(buf, nbuf*2, 1, fp);
		}
		if(db.Type == PL_SingleWFType){ // both timestamps and waveforms have this type
			if(db.Channel == channel_to_extract && db.Unit == unit_to_extract){ 
				printf("spike: %d ticks: %d seconds: %.6f\n", count, db.TimeStamp,
						db.TimeStamp/(double)fh.ADFrequency);
				count++;
			}
		}
	}

	// SAMPLE 2:
	//  read the waveforms for a spike channel
	//

	channel_to_extract = 1;
	unit_to_extract = 1;
	count = 1;
	printf("\nWaveforms for channel %d, unit %d\n", 
		channel_to_extract, unit_to_extract);

	// rewind file to the data start!!!
	fseek(fp, datastart, SEEK_SET);
	int i;
	while(feof(fp) == 0 && fread(&db, sizeof(db), 1, fp) == 1){ // read the block
		nbuf = 0;
		if(db.NumberOfWaveforms > 0){ // read the waveform after the block
			nbuf = db.NumberOfWaveforms*db.NumberOfWordsInWaveform;
			fread(buf, nbuf*2, 1, fp);
		}
		if(db.Type == PL_SingleWFType && nbuf > 0){ 
			if(db.Channel == channel_to_extract && db.Unit == unit_to_extract){ 
				printf("spike: %d ticks: %d seconds: %.6f\n", count, db.TimeStamp,
						db.TimeStamp/(double)fh.ADFrequency);
				printf("waveform:");
				for(i=0; i<db.NumberOfWordsInWaveform; i++){
					printf(" %d,", buf[i]);
				}
				printf("\n");
				count++;
			}
		}
	}

	
	// SAMPLE 3
	//   read the timestamps for an external event channel
	//
	int event_to_extract = 1;
	count = 1;
	printf("\nTimestamps for event %d\n", 
		event_to_extract);
	
	// rewind file to the data start!!!
	fseek(fp, datastart, SEEK_SET);

	while(feof(fp) == 0 && fread(&db, sizeof(db), 1, fp) == 1){ // read the block
		nbuf = 0;
		if(db.NumberOfWaveforms > 0){ // read the waveform after the block
			nbuf = db.NumberOfWaveforms*db.NumberOfWordsInWaveform;
			fread(buf, nbuf*2, 1, fp);
		}
		if(db.Type == PL_ExtEventType){ 
			if(db.Channel == event_to_extract){ 
				printf("event: %d ticks: %d seconds: %.6f\n", count, db.TimeStamp,
					db.TimeStamp/(double)fh.ADFrequency);
				count++;
			}
		}
	}

	// SAMPLE 4
	//  read continuous data for one channel
	//   please note that a/d data does not start
	//      at time 0!!!
	//   the timestamps for the a/d data points need to be
	//      calculated
	//    converting to voltage: 5V corresponds to the a/d value of 2048 

	channel_to_extract = 0; // a/d channel numbers are zero-based!!!
	count = 1;
	printf("\nContinuous data for channel %d\n", 
		channel_to_extract);
	
	// rewind file to the data start!!!
	fseek(fp, datastart, SEEK_SET);

	// find the header for this a/d channel
	int header_num = -1;
	for(i=0; i<fh.NumSlowChannels; i++){
		if(slowheaders[i].Channel == channel_to_extract){
			header_num = i;
			break;
		}
	}
	if(header_num == -1){
		printf("No header for the specified A/D channel!\n");
		fclose(fp);
		exit(1);
	}

	int gain = slowheaders[header_num].Gain;
	int adfreq = slowheaders[header_num].ADFreq;

	if(adfreq == 0 || gain == 0){
		printf("No A/D frequency or gain!\n");
		fclose(fp);
		exit(1);
	}
	int ticks_in_adcycle = fh.ADFrequency/adfreq;

	int first_timestamp = -1;
	int ts;
	while(feof(fp) == 0 && fread(&db, sizeof(db), 1, fp) == 1){ // read the block
		nbuf = 0;
		if(db.NumberOfWaveforms > 0){ // read the waveform after the block
			nbuf = db.NumberOfWaveforms*db.NumberOfWordsInWaveform;
			fread(buf, nbuf*2, 1, fp);
		}
		if(db.Type == PL_ADDataType){ 
			if(db.Channel == channel_to_extract){
				if(first_timestamp == -1){
					printf("first data point at: %d ticks: %d seconds: %.6f\n", count, db.TimeStamp,
						db.TimeStamp/(double)fh.ADFrequency);
				first_timestamp = db.TimeStamp;
				}
				for(i=0; i<db.NumberOfWordsInWaveform; i++){
					ts = db.TimeStamp + i*ticks_in_adcycle;
					// voltage: 5V corresponds to the a/d value of 2048 
					double v = (buf[i]*5./2048.)/(double)gain;

					printf("a/d value %5d (%6.3f V) at %d ticks or %.6f seconds\n", 
						buf[i], v, ts, ts/(double)fh.ADFrequency);
				}
				count++;
			}
		}
	}

	fclose(fp);
}

void printheaderinfo(PL_FileHeader& fh)
{
	printf("File Version: %d\n", fh.Version);
	printf("File Comment: %s\n", fh.Comment);
	printf("Frequency: %d\n", fh.ADFrequency);
	printf("DSP Channels: %d\n", fh.NumDSPChannels);
	printf("Event Channels: %d\n", fh.NumEventChannels);
	printf("A/D Channels: %d\n", fh.NumSlowChannels);
	int i, j;
	printf("\nTimestamps:\n");
	for(i=0; i<130; i++){
		for(j=0; j<5; j++){
			if(fh.TSCounts[i][j] > 0){
				printf("Channel %03d Unit %d Count %d\n", i, j, fh.TSCounts[i][j]);
			}
		}
	}
	printf("\nWaveforms:\n");
	for(i=0; i<130; i++){
		for(j=0; j<5; j++){
			if(fh.WFCounts[i][j] > 0){
				printf("Channel %03d Unit %d Count %d\n", i, j, fh.TSCounts[i][j]);
			}
		}
	}
	printf("\nEvents:\n");
	for(i=0; i<299; i++){
		if(fh.EVCounts[i] > 0){
			printf("Event %03d Count %d\n", i, fh.EVCounts[i]);
		}
	}
	printf("\nA/D channels:\n");
	for(i=300; i<512; i++){
		if(fh.EVCounts[i] > 0){
			printf("channel %02d data points %d\n", i-300+1, fh.EVCounts[i]);
		}
	}
}
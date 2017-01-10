#include <stdint.h>
#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
//#include <varint.h>
using namespace std;

// ENCODE : To encode the first time stamp from the "timestamp.txt" (input) file
void write_timestamp(ofstream& outfile, uint64_t timestamp) 
{
	uint8_t bytes[8]; // storing the first timestamp in 8 bytes
	for (int i=7; i >=0;i--, timestamp >>=8)
	bytes[i] = timestamp; 
	outfile.write((char*)bytes,8); // writing the timestamps in the binary output file 
}

// ENCODE :Function to encode the various intervals and send to "output.txt"
void encode(ofstream& outfile, uint64_t diff) // Parameters are the the output file and the difference we calculated
{  
	int size = 0;
	uint8_t bytes[8];
    while (diff > 127) // while number is greater than 127, bytes to store data increase by 1
    {
    	bytes[size] = (diff & 127)| 128; // sets the MSB of the byte to 1
    	diff >>= 7; // shift the data by 7 bits 
    	size++; // 
    }
    bytes[size++] = diff&127; // One byte is added to accomodate less than 127   
//  cout << "ENCODE function SIZE value " << size << endl;
    outfile.write((char*)bytes,size);
}

// ENCODE : Utility for TimeStamps compression  
void TS_Compression()
{
    ifstream infile("Timestamps.txt"); // Creating an stream for input file
	if(! infile.is_open()) // Check if the input file is opened
	{
		cout << "Unable to open input file" << endl;
		exit(EXIT_FAILURE); // exit if error opening file
	} 
	
	ofstream outfile("Encoded.bin", ios::trunc | ios::binary); // Creating an stream for output file
	if(! outfile.is_open()) // Check if the input file is opened
	{
		cout << "Unable to open output file" << endl;
		exit(EXIT_FAILURE); // exit if error opening file
	}
	
	long double seconds; // to read data from input stream
	uint64_t timestamp;  // to store time in microseconds
	
	if(infile >> seconds) // sending data to seconds from input stream
	{
		timestamp = (seconds*1000000); // converting micro seconds timestamps to seconds
//		cout << "TS_Compression timestamp value "<< timestamp << endl; 
		write_timestamp(outfile, timestamp); // Calling function to write first timestamp in output file
	}
	
	while(! infile.eof()) // Looping till the end of file is not reached 
	{
		uint8_t bytesize = 0, bytes[4];
        uint32_t interval;
        uint8_t* buf;
        
        while (infile >> seconds) // 
        {
        	interval = (seconds*1000000) - timestamp; // Subtract from initial to get interval between 2 time stamps
        	timestamp+= interval; // Original can be recreated
        	//cout << interval << endl;
        	encode (outfile, interval); // Calling encode function to encode interval between timestamps       	
        }      
	}	
}


// DECODE : Function to read the first timestamp from the encoded file
uint64_t read_timestamp (ifstream& ifile)
{
	uint64_t timestamp= 0;
	uint8_t byte;
	for(uint8_t i=0;i<8; i++)
	{
		ifile.read((char*)&byte, 1); // read function takes n bytes from input stream and store it in "bytes"
		if(ifile.fail()) 
		return 0; //  Check if file failsif file fails
		timestamp <<= 8; // shifting 8 bits to right
		timestamp |= byte; // Here OR operation btween two bytes sums up value in the two bytes
	}

	return timestamp;
}

// DECODE : Writing the decoded timestamp in the "Decoded.txt" file
void write_seconds(ofstream& ofile, uint64_t timestamp)
{
	//long double useconds = (long double)timestamp/1000000;
	uint64_t seconds = timestamp;
//	cout << "write_seconds seconds value "<< seconds << endl;
	ofile << seconds << endl;
}

// DECODE : Utility to decode intervals
// Decodes an unsigned variable-length integer using the MSB algorithm.
uint8_t decode(ifstream& ifile, uint8_t inputsize)
{	
    uint8_t* bytes; 
    ifile.read((char*) bytes,inputsize); 
	uint8_t ret = 0;
	for (uint8_t i= 0; i< inputsize;i++)
	{
		// Every time the loop moves we will left shift bits by 7, read MSB if 1 and continute 
		ret|= (bytes[i]) << (7*i);  
		// if next byte-flag is set 
		if(!bytes[i]&128) // if MSB zero, exit loop
		{
			break;
		}
	} 

	return  ret;
}

// DECODE : To find the size of bytes
uint8_t read_size(ifstream& ifile, uint8_t *bytes)
{	
	int bytesize = 1;
	ifile.read((char*) bytes,1); // reads 'n' number of characters from file and stores in bytes
	if (ifile.fail()) // if the file fails return 0
	return 0;
	bytesize += bytes[0]>> 7; // shifting the characters by 7 left
	for (int i = 1; i<bytesize; i++)
	{
		ifile.read((char*) bytes,1);
		if (ifile.fail())
		return 0;
		
		if((bytes[0]>>7)&127) // AND Masking : IF the 8th bit is 1, increment byte size
		{
			bytesize++;			
		}		
	}
	
	return bytesize;
} 

// DECODE : Utility for decompression
void TS_Decompression()
{
	uint8_t value;
	ifstream ifile ("Encoded.bin", ios::binary); // Taking input stream from encoded file
    if (! ifile.is_open()) // Check to open input file 
	{
		cout << "Decoder : Unable to open input file " << endl;
		exit(EXIT_FAILURE); // If failure, exit
	}
    ofstream ofile ("Decoded.txt", ios::trunc); // Taking output stream from encoded file
    if (! ofile.is_open())
    {
    	cout << "Decoder: Unable to open output file" << endl; 
    	exit(EXIT_FAILURE); // If failure, exit
    }
    ofile.precision(6); 
	
	uint64_t timestamp = read_timestamp(ifile); // Decoding the first timestamp
	uint64_t dummy_store = timestamp; // To store intital timestamp 
	if(timestamp)                    // If true
	write_seconds(ofile,timestamp); // Write first timestamp to output file

	while (!ifile.eof()) // while end of file is not reached
	{		    
	    uint8_t bytes[4], delta;
	    uint8_t delta_byte_size = read_size(ifile, bytes); // getting delta byte size
		delta = decode(ifile, delta_byte_size); // Getting decoded interval values		
		value = dummy_store + delta; // Adding to inital interval 
		dummy_store = value; // Storing current timestamp value to be addded to delta 		
		cout<< "The Decoded value returned from function " << delta << endl;
		write_seconds(ofile, value); // Writing the values to output file	    
	}
	
}

// Main utility function to call the compression and decompression functions
int main()
{
	// Calling TimeStamps Compression function
	TS_Compression();
	// Calling TimeStamps Decompresion function
	TS_Decompression();
	return 0;
}

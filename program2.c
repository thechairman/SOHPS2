
//Started this in c, it we need to it is trivial to convert 
//to C++

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fctl.h>
#include<errno.h>
#include<string.h>

#define READ_BUF_SIZE 100

int main(int argc, char* argv[]){

	int inputFile;
	int numVars;
	int numTerms;
	char* readbuffer = char[READ_BUF_SIZE];
	char* start, stop;
	ssize_t bytesRead;

	//check for input file
	if(argc < 2){
		printf("No input file provided\n");
		return 1;
	}

	//open input file
	inputFile = open(argv[1], O_RDONLY);

	//check for errors
	if( inputFile < 0){
		printf("Error opening input file: %s\n", strerror(errno));
		return 1;
	}

	//read first hundered bytes of file
	bytesRead = read(inputFile, readbuffer, READ_BUF_SIZE);

	//pull the number of varibles
	start = readBuffer;
	numVars = atoi(start);

	//move to the start of next line
	while(*start != '\n'){
		++start;
	}
	++start;

	//get the number of terms
	numTerms = atoi(start);
	
	//move to the start of next line
	while(*start != '\n'){
		++start;
	}
	++start;

	//this is where we should start reading in the terms
	//not really sure how we should represent the terms


	return 0;
}



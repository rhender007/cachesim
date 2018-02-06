/*
 * sim.h
 *
 *  Created on: Apr 16, 2010
 *      Author: rhender
 */

#ifndef SIM_H_
#define SIM_H_


struct Line {
	int tag;
	char valid;
	char dirty;
	int timeStamp;/*marker of when used last*/
};
typedef struct Line* LinePtr;/*array lines*/

struct Set {
	LinePtr* set;
	unsigned int nextIndex;/*used for FIFO replacement*/
};
typedef struct Set* SetPtr;/*array sets*/

typedef SetPtr* Cache;

/*Contains information about the cache simulation*/
struct SimInfo {

	/*Input*/
	char* associativity;
	int blockSize;
	char* replacementAlgorithm;
	char* writePolicy;
	char* traceFile;
	int cacheSize;


	/*Calculated info*/
	int numberOfSets;
	int numberOfLinesPerSet;

	int cacheBits;/*needed for calculations*/
	int tagBits;
	int blockBits;/*needed for calc of the bits*/
	int setBits;

	int lineBits;


	/*Counters*/
	int cacheHits;
	int cacheMisses;
	int memoryReads;
	int memoryWrites;
	int count;


	/*Input from file pinfile*/
	unsigned int instructionLocation;
	unsigned int referencedLocation;
	char readOrWrite;

	/*Our cache*/
	Cache cache;

	int addressBits;

	int powerFlag;

};
typedef struct SimInfo* SimInfoPtr;

int SimInit(SimInfoPtr si, char* cache_size, char* assoc, char* line_size,
		char* replace_algo, char* write_policy, char* trace_file);

int RunSim(char* cache_size, char* assoc, char* line_size, char* replace_algo,
		char* write_policy, char* trace_file);


int isValidInput(SimInfoPtr si);

int LogBase2(SimInfoPtr si,int number,int flagNumber);
void ToLowerCase(char* myString);
int CalculateAndMallocIfValid(SimInfoPtr siPtr);
void Free(SimInfoPtr si);
void ReadWritePolicy(SimInfoPtr siPtr,int setNumber,int lineNumber,int tag);
FILE* OpenFile(char* file_name);
void PrintResults(SimInfoPtr si);
int CheckForHelpAndArgCountError(char* cache_sze,int argc);

#endif /* SIM_H_ */

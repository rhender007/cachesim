/*
 ============================================================================
 Name        : sim.c
 Author      : Robert Henderson
 Version     : RTM
 Copyright   : R. Henderson Rutgers University
 Description : Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "sim.h"
#include <string.h>
#include <math.h>

/*cachesize, associativity, blockSize, replacementAlgorithm, writePolicy, traceFile*/
int main(int argc, char** argv) {

	/*./sim 512 direct 32 fifo wt pinatrace.out*/

	CheckForHelpAndArgCountError(argv[1],argc);
	RunSim(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);

	return EXIT_SUCCESS;
}
/*
 * Takes in a SimInfoPtr and all the arguments from main
 * Side effects: assigns the arguments to the simInfo fields
 * If input is valid it will call CalculateAndMallocIfValid to continue the program
 * Returns 0 upon success; exit(1) upon failure
 */
int SimInit(SimInfoPtr si, char* cache_size, char* assoc, char* line_size,
		char* replace_algo, char* write_policy, char* trace_file) {

	char* string;
	char* pch = strpbrk(assoc, ":");

	si->powerFlag = 0;/*flag is true if an input is not a power of 2*/
	si->addressBits = 32;

	/*si->associativity = malloc(sizeof(assoc));*/
	si->associativity = assoc;/*direct, associative, nway associative*/
	ToLowerCase(si->associativity);
	/*si->replacementAlgorithm = malloc(sizeof(replace_algo));*/
	si->replacementAlgorithm = replace_algo;/*FIFO, LRU*/
	ToLowerCase(si->replacementAlgorithm);
	/*si->writePolicy = malloc(sizeof(write_policy));*/
	si->writePolicy = write_policy;/*write thru or write back*/
	ToLowerCase(si->writePolicy);
	/*si->traceFile = malloc(sizeof(trace_file));*/
	si ->traceFile = trace_file;/*pin output file*/

	/*Always give CS and BS*/
	si->cacheSize = strtol(cache_size, &string, 10);
	si->cacheBits = LogBase2(si, si->cacheSize, 1);

	si->blockSize = strtol(line_size, &string, 10);
	si->blockBits = LogBase2(si, si->blockSize, 2);/*log((double) si->blockSize) / log(2.0);*/
	/**********************************/

	if (pch == NULL)/*pch is null for assoc, direct*/
	{
		if (!strcmp(si->associativity, "direct"))
			si->numberOfLinesPerSet = 1;
		else
			si->numberOfLinesPerSet = si->cacheSize / si->blockSize;
	}

	else/*For the nway assoc*/
	{
		si->numberOfLinesPerSet = atoi(pch + 1);
		LogBase2(si, si->numberOfLinesPerSet, 3);
	}

	/*Check to see if inputs are valid here before other calculations are made*/
	/*isValidInput(si);*/

	if (!isValidInput(si))
		exit(EXIT_FAILURE);

	CalculateAndMallocIfValid(si);

	return 0;
}

/*
 * Opens a file
 * Takes in a file name
 * Upon failure, exit 0; Upon success, returns the file pointer
 */
FILE* OpenFile(char* file_name) {
	FILE* filePtr = fopen(file_name, "rb");

	if (filePtr == NULL) {
		fprintf(stderr, "Error: can't open file.\n");
		exit(0);
	}

	else
		return filePtr;
}

/*
 * Checks for valid input
 * Takes in a SimInfoPtr
 * Returns 0 if no error; Returns True (other than 1) if input error
 */
int isValidInput(SimInfoPtr siPtr) {
	int errorNumber;/*we'll only read out one error at a time*/
	char* pch = strpbrk(siPtr->associativity, ":");
	int returnVal;

	/*ok*/
	if ((strcmp(siPtr->associativity, "direct")) && (strcmp(
			siPtr->associativity, "assoc")) && (pch == NULL)) {
		errorNumber = 4;
	} else if ((strcmp(siPtr->replacementAlgorithm, "lru")) && (strcmp(
			siPtr->replacementAlgorithm, "fifo"))) {
		errorNumber = 5;
	} else if ((strcmp(siPtr->writePolicy, "wb")) && (strcmp(
			siPtr->writePolicy, "wt"))) {
		errorNumber = 6;
	} else if (siPtr->powerFlag) {
		errorNumber = siPtr->powerFlag;/*(1-3)*/
	} else
		errorNumber = 0;

	switch (errorNumber) {
	case 0:
		break;
	case 1:
		fprintf(stderr, "Error...cache_size is not a power of 2\n");
		break;
	case 2:
		fprintf(stderr, "Error...block_offset is not a power of 2\n");
		break;
	case 3:
		fprintf(stderr, "Error...numberOfLinesPerSet is not a power of 2\n");
		break;
	case 4:
		fprintf(stderr, "Error...associativity is not valid\n");
		break;
	case 5:
		fprintf(stderr, "Error...replacement_algorithm is not valid\n");
		break;
	case 6:
		fprintf(stderr, "Error...write_policy is not valid\n");
		break;
	default:
		fprintf(stderr, "Error...General Error\n");
		break;

	}

	if (!errorNumber)
		returnVal = 1;
	else
	{
		fprintf(stderr, "Type ./sim -h for help\n");
		returnVal = 0;
	}

	return returnVal;

}

/*Will either return the number of the next slot to be victimized or the match
 * Takes in a SimInfoPtr, tag, and a set; tag and set are generated from the address
 * No side effects
 * */
int FindReplacementIndex(SimInfoPtr si, int tag, int set) {

	int index;
	int isFound = 0;/*Boolean*/
	int forLoopIndex;/* = -1;*/
	int lruIndex = 0;

	if (!strcmp(si->associativity, "direct"))
		index = 0;

	else /*if (!strcmp(si->associativity, "assoc")) {*/
	{
		if (!strcmp(si->replacementAlgorithm, "fifo")) {
			for (forLoopIndex = 0; forLoopIndex < (si->numberOfLinesPerSet); forLoopIndex++) {
				if ((si->cache[set]->set[forLoopIndex]->tag == tag)
						&& (si->cache[set]->set[forLoopIndex]->valid == '1')) {
					isFound = 1;
					index = forLoopIndex;

				} else
					;
			}

			if (!isFound) {
				int i = (si->cache[set]->nextIndex++);
				index = (i) % (si->numberOfLinesPerSet);
			}

		} else if (!strcmp(si->replacementAlgorithm, "lru")) {
			for (forLoopIndex = 0; forLoopIndex < (si->numberOfLinesPerSet); forLoopIndex++) {
				if ((si->cache[set]->set[forLoopIndex]->timeStamp)
						< (si->cache[set]->set[lruIndex]->timeStamp))
					lruIndex = forLoopIndex;
				if ((si->cache[set]->set[forLoopIndex]->tag == tag)
						&& (si->cache[set]->set[forLoopIndex]->valid == '1')) {
					isFound = 1;
					index = forLoopIndex;
				} else
					;
			}

			if (!isFound)/*if not found return least recent index*/
			{
				index = lruIndex;
			}

		} else
			;

	}

	return index;

}
/*
 * Runs the simulation; Gets the code out of main
 * Takes in all the arguments from main
 * Calls SimInit, OpenFile, and then in a loop calls FindReplacementIndex and ReadWritePolicy
 *
 */
int RunSim(char* cache_size, char* assoc, char* line_size, char* replace_algo,
		char* write_policy, char* trace_file) {

	struct SimInfo s;
	FILE* filePtr;
	int tag;
	unsigned int setNumber;
	int lineNumber;

	SimInit(&s, cache_size, assoc, line_size, replace_algo, write_policy,
			trace_file);

	filePtr = OpenFile(trace_file);

	while (fscanf(filePtr, "%x: %c %x", &s.instructionLocation, &s.readOrWrite,
			&s.referencedLocation) != 0) {

		if (!strcmp(s.associativity, "assoc"))
			setNumber = 0;
		else
			setNumber = (s.referencedLocation << (s.tagBits)) >> (s.tagBits
					+ s.blockBits);

		tag = (s.referencedLocation >> s.blockBits);
		lineNumber = FindReplacementIndex(&s, tag, setNumber);

		ReadWritePolicy(&s, setNumber, lineNumber, tag);

	}

	fclose(filePtr);

	Free(&s);/*Frees the allocated space for the cache*/
	PrintResults(&s);

	return 0;
}

/*
 * Takes in a siminfo ptr, number of whiches power of 2 needs to be computed
 * Calculates the power of 2 and also modifies the flag checking to see if num is power of 2
 * */
int LogBase2(SimInfoPtr si, int number,/*int* power,*/int flagNumber) {
	int tempSize = number;
	int powerOf2Number = 1;
	int power = 0;
	while (tempSize > 1) {
		tempSize = tempSize >> 1;
		powerOf2Number = powerOf2Number << 1;
		power++;
	}
	if (number != powerOf2Number) {
		si->powerFlag = flagNumber;
	}

	return power;
}

/*Converts a string to uppercase using pointers to modify the original*/
void ToLowerCase(char* myString) {
	int index = 0;
	while (myString[index] != '\0') {
		myString[index] = tolower((myString[index]));
		index++;
	}
}

/*Calculates and mallocs if Input is valid*/
int CalculateAndMallocIfValid(SimInfoPtr si) {
	int i;
	int j;

	si->numberOfSets = si->cacheSize
			/ (si->blockSize * si->numberOfLinesPerSet);

	si->setBits = LogBase2(si, si->numberOfSets, 4);
	si->tagBits = si->addressBits - (si->blockBits + si->setBits);

	/*Our running counts*/
	si->cacheHits = 0;
	si->cacheMisses = 0;
	si->memoryReads = 0;
	si->memoryWrites = 0;
	si->count = 1;

	si->cache = malloc(sizeof(SetPtr) * si->numberOfSets);

	/*malloc for the set then the line*/
	for (i = 0; i < si->numberOfSets; i++) {
		si->cache[i] = malloc(sizeof(struct Set));
		si->cache[i]->set = malloc(sizeof(LinePtr) * si->numberOfLinesPerSet);
		si->cache[i]->nextIndex = 0;/*1st time thru ini to 0*/

		for (j = 0; j < si->numberOfLinesPerSet; j++) {
			si->cache[i]->set[j] = malloc(sizeof(struct Line));
			si->cache[i]->set[j]->valid = '0';
			si->cache[i]->set[j]->dirty = '0';
			si->cache[i]->set[j]->timeStamp = 0;
		}

	}

	return 0;
}
/*
 * Will modify the cache hits, cache misses, memory reads, and memory writes thru the use of pointers
 * Takes in a siminfo pointer, a set number calculated for a mem address, a line number determined by
 * FindReplacementIndex, and a tag number generated by the address
 *
 * Side effects: modifies cache hits, misses, memory reads, writes for a specific sim ptr
 * Returns nothing;void
 */

void ReadWritePolicy(SimInfoPtr siPtr, int setNumber, int lineNumber, int tag) {
	if (siPtr->cache[setNumber]->set[lineNumber]->valid != '1') {/*invalid*/

		/*cache miss*/
		siPtr->cacheMisses++;
		/*must go to memory*/
		siPtr->memoryReads++;/*read into cache*/
		siPtr->cache[setNumber]->set[lineNumber]->tag = tag;
		siPtr->cache[setNumber]->set[lineNumber]->valid = '1';

		/*if "write thru" with 'W' write into memory*/
		if (((int) siPtr->readOrWrite == (int) 'W')) {
			if (!strcmp(siPtr->writePolicy, "wt"))
				siPtr->memoryWrites++;
			else if (!strcmp(siPtr->writePolicy, "wb"))
				siPtr->cache[setNumber]->set[lineNumber]->dirty = '1';
			else
				;/*ERROR*/
		} else
			/*Just a read case*/
			;
	}

	else /*if(siPtr->cache[setNumber]->set[0]->valid = '1') Valid*/
	{
		/*valid and tag matches, we have a hit!!*/
		if (siPtr->cache[setNumber]->set[lineNumber]->tag == tag) {
			siPtr->cacheHits++;

			if ((int) siPtr->readOrWrite == (int) 'W') {

				if (!strcmp(siPtr->writePolicy, "wt"))/*for wt*/
					siPtr->memoryWrites++;

				else
					/*for wb*/
					siPtr->cache[setNumber]->set[lineNumber]->dirty = '1';/*set it to dirty*/
			}

			else
				;
			/*Read case*/

		} /*Valid,tag doesnt match*/

		else {

			siPtr->cacheMisses++;
			siPtr->memoryReads++;

			/*evicting*/
			if (siPtr->cache[setNumber]->set[lineNumber]->dirty == '1') {
				siPtr->memoryWrites++;/*write to mem*/
				siPtr->cache[setNumber]->set[lineNumber]->dirty = '0';/*set dirty to '0';*/
			}
			/*assign new tag*/
			siPtr->cache[setNumber]->set[lineNumber]->tag = tag;

			if ((int) siPtr->readOrWrite == (int) 'W') {
				if (!strcmp(siPtr->writePolicy, "wt"))
					siPtr->memoryWrites++;

				if (!strcmp(siPtr->writePolicy, "wb"))
					siPtr->cache[setNumber]->set[lineNumber]->dirty = '1';
			}
		}
	}
	siPtr->cache[setNumber]->set[lineNumber]->timeStamp = siPtr->count;
	siPtr->count++;
}

/*Frees the memory allocated with malloc*/
void Free(SimInfoPtr siPtr) {
	int j;
	int i;
	for (j = 0; j < siPtr->numberOfLinesPerSet; j++) {
		for (i = 0; i < siPtr->numberOfSets; i++) {
			free(siPtr->cache[i]->set[j]); /*free each line ptr*/
		}
		free(siPtr->cache[i]);/*free each set ptr*/
	}
	free(siPtr->cache);/*free the array of sets ptr*/
}

/*cachesize, associativity, blockSize, replacementAlgorithm, writePolicy, traceFile
 * Takes in a SiminfoPtr
 * Prints out the input and the results
 * Returns Nothing;void
 *
 * */
void PrintResults(SimInfoPtr siPtr) {
	printf("./sim %d %s %d %s %s %s\n", siPtr->cacheSize, siPtr->associativity,
			siPtr->blockSize, siPtr->replacementAlgorithm, siPtr->writePolicy,
			siPtr->traceFile);
	printf("Cache Hits: %d\n", siPtr->cacheHits);
	printf("Cache Misses: %d\n", siPtr->cacheMisses);
	printf("Memory Reads: %d\n", siPtr->memoryReads);
	printf("Memory Writes: %d\n\n", siPtr->memoryWrites);
}

/*
 * Returns 0 if no errors
 * exit 1 if errors or help
 */


int CheckForHelpAndArgCountError(char* cache_size,int argc)
{
	if(!strcmp(cache_size,"-h")||argc!=7)
		{
		fprintf(stderr,"Usage: \nsim [-h] <cache size> <associativity> <block size> <replace alg> <write policy> <trace file>\n");
		exit(1);
		}

return 0;
}


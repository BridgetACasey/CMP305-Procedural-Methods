#pragma once

#include "MarkovWord.h"

#include <vector>

class MarkovChain
{
public:
	MarkovChain(const char* fileName, int sample);
	~MarkovChain();

	void generateLookupTable(const char* fileName, int sample);

	std::string generateSentence(char* startWord, int length);

private:
	void loadTextData(const char* fileName);

	std::string sampleNextCharacter(const char* start, int sample);

	std::vector<MarkovWord> lookupTable;
	std::string data;

	int sampleSize;
};
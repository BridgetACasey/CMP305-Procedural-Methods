#pragma once

#include <vector>
#include <string>

struct MarkovWord
{
	std::pair<std::string, std::string> text;
	float inputFrequency = 1.0f;	//How often the input/first element of text appears in the table
	float matchFrequency = 1.0f;	//How often the same pair of input and output appear in the corpus
	float probability = 0.0f;	//The decimal value used to determine which output is most likely for a given input
};

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
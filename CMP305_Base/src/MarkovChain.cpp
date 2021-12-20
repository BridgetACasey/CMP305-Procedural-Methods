#include "MarkovChain.h"

#include <fstream>
#include <algorithm>

struct orderProbability
{
	inline bool operator()(const MarkovWord& first, const MarkovWord& second)
	{
		return (first.probability > second.probability);
	}
};

MarkovChain::MarkovChain(const char* fileName, int sample)
{
	sampleSize = sample;

	generateLookupTable(fileName, sampleSize);
}

MarkovChain::~MarkovChain()
{
}

std::string MarkovChain::generateSentence(char* start, int length)
{
	std::string sentence, next;
	int min = 0;

	if (start)
	{
		sentence = std::string(start);
		next = std::string(start);

		min = strlen(start);
	}

	else
	{
		//If there is no starting sample, pick a random one from the lookup table
		int index = rand() % lookupTable.size();

		sentence = lookupTable.at(index).text.first;
		next = sentence;

		min = strlen(sentence.c_str());
	}

	for (int i = min; i < length; i++)
	{
		//Create the next sequence of characters to sample with
		next += sampleNextCharacter(next.c_str(), sampleSize);
		next.erase(next.begin());

		if (next.back() == '#')
		{
			return sentence;
		}

		//Grab next predicted character and append to sentence
		sentence += next.back();
	}

	return sentence;
}

std::string MarkovChain::sampleNextCharacter(const char* start, int sample)
{
	std::string startWord = std::string(start);
	std::vector<MarkovWord> possibleNext;
	std::string next = " ";

	int length = strlen(start);

	if (sample > length)
	{
		sample = length;
	}
	
	else if (sample < length)
	{
		//Only taking into account the last characters according to the sample size
		startWord.erase(startWord.begin(), startWord.begin() + (length - sample));
	}

	for (MarkovWord& current : lookupTable)
	{
		if (startWord == current.text.first)
		{
			possibleNext.push_back(current);
		}
	}

	if (possibleNext.size() > 0)
	{
		std::sort(possibleNext.begin(), possibleNext.end(), orderProbability());

		float random = float(rand() % 100);
		float interval = 0.0f;
		random /= 100.0f;

		for (MarkovWord& word : possibleNext)
		{
			next = word.text.second;

			if (random <= (word.probability + interval))
			{
				break;
			}

			interval += word.probability;
		}
	}

	return next;
}

void MarkovChain::generateLookupTable(const char* fileName, int sample)	//How many characters to sample at a time
{
	loadTextData(fileName);

	//Create the table for the data
	for (int i = 0; i < (data.size() - sample); i++)
	{
		MarkovWord word;

		word.text.first = data.substr(i, sample);
		word.text.second = data.substr(i + sample, 1);

		bool insertPair = true;

		for (MarkovWord& previous : lookupTable)
		{
			if (word.text.first == previous.text.first)
			{
				word.inputFrequency += 1.0f;
				previous.inputFrequency += 1.0f;

				if (word.text.second == previous.text.second)
				{
					//If the current pair of input and output data already exists in the table, increment it and move on to a new pair
					word.matchFrequency += 1.0f;
					previous.matchFrequency += 1.0f;
					insertPair = false;
				}
			}
		}

		if (insertPair)
		{
			lookupTable.push_back(word);
		}
	}

	//Convert frequency values into probability percentages
	for (MarkovWord& current : lookupTable)
	{
		current.probability = current.matchFrequency / current.inputFrequency;
	}
}

void MarkovChain::loadTextData(const char* fileName)
{
	std::ifstream file(fileName);

	std::string line;

	while (std::getline(file, line))
	{
		data += line;
	}
}
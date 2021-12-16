#pragma once

#include <string>

struct MarkovWord
{
	std::pair<std::string, std::string> text;
	float inputFrequency = 1.0f;	//How often the input/first element of text appears in the table
	float matchFrequency = 1.0f;	//How often the same pair of input and output appear in the corpus
	float probability = 0.0f;	//The decimal value used to determine which output is most likely for a given input
};
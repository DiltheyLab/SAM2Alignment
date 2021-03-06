//============================================================================
// Name        : bam2pairwise.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdexcept>
#include <exception>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <assert.h>
#include <utility>

using namespace std;

std::vector<std::string> split(std::string input, std::string delimiter);
void eraseNL(string& s);
std::string seq_reverse_complement(std::string sequence);
char reverse_char_nucleotide(char c);

class alignment {
public:
	std::string Name;
	int FLAG;
	std::string rName;
	unsigned long int POS;
	unsigned int mapQ;
	std::string CIGAR;
	std::string SEQ;
	std::string QUAL;
	std::string tags;
	void fillFromLine(const std::string& line)
	{
		std::vector<std::string> line_fields = split(line, "\t");
		if(!(line_fields.size() >= 11))
		{
			std::cerr << "Wrong number of records in SAM line:\n";
			std::cerr << line_fields.size() << "\n";
			std::cerr << line << "\n" << std::flush;
		}
		assert(line_fields.size() >= 11);
		Name = line_fields.at(0);
		FLAG = std::stoi(line_fields.at(1));
		rName = line_fields.at(2);
		POS = std::stoul(line_fields.at(3));
		mapQ = std::stoul(line_fields.at(4));
		CIGAR = line_fields.at(5);
		SEQ = line_fields.at(9);
		QUAL = line_fields.at(10);
		if(QUAL != "*")
		{
			assert(QUAL.length() == SEQ.length());
		}
		if(line_fields.size() >= 12)
		{
			tags = line_fields.at(11);
		}

	}

	bool isPaired() const
	{
		return (FLAG & (int)1);
	}

	bool eachSegmentProperlyAligned() const
	{
		return (FLAG & (int)2);
	}

	bool isUnmapped() const
	{
		return (FLAG & (int)4);
	}

	bool isMapped() const
	{
		return(! isUnmapped());
	}

	bool nextSegmentUnmapped() const
	{
		return (FLAG & (int)8);
	}

	bool isReverseComplemented() const
	{
		return (FLAG & (int)16);
	}

	bool nextSegmentReverseComplemented() const
	{
		return (FLAG & (int)32);
	}

	bool isFirst() const
	{
		return (FLAG & (int)64);
	}

	bool isSecond() const
	{
		return (FLAG & (int)128);
	}

	bool isNotPrimary() const
	{
		return (FLAG & (int)256);
	}


	bool isPrimary() const
	{
		return (! isNotPrimary());
	}

	bool notPassingFilters() const
	{
		return (FLAG & (int)512);
	}

	bool isPCROrOpticalDuplicate() const
	{
		return (FLAG & (int)1024);
	}

	bool isSupplementary() const
	{
		return (FLAG & (int)2048);
	}

};
map<string, string> readFASTA(std::string file, bool fullIdentifier = false);
void processAlignments_oneRead(const std::string& readID, const std::vector<alignment>& alignments, const std::map<std::string, std::string>& referenceSequences);
void testFlags();

int main(int argc, char *argv[]) {
	std::vector<std::string> ARG (argv + 1, argv + argc + !argc);

	if(ARG.size() == 0)
	{
		//ARG.push_back("/cygdrive/d/Temp/SAMtest/testAlignments.sam");
		//ARG.push_back("/cygdrive/d/Temp/SAMtest/testGenome.fa");

		//ARG.push_back("/cygdrive/d/Temp/SAMtest/mycoTest.sam");
		//ARG.push_back("/cygdrive/d/Temp/SAMtest/FBG1E1.rc.aligned.pilon.fasta");

	}

	if(ARG.size() == 0)
	{
		std::cerr << "Error:\n\nPlease provide two arguments: a SAM file and a reference in FASTA format." << "\n\n" << std::flush;
		throw std::runtime_error("Missing arguments");
	}

	testFlags();

	std::string samFile = ARG.at(0);
	std::string refFile = ARG.at(1);

	std::map<std::string, std::string> referenceSequences = readFASTA(refFile);

	std::set<std::string> processed_readIDs;

	std::map<std::string, std::vector<alignment>> readID_2_alignments;
	auto processAlignmentStorage = [&]() -> void
	{
		for(auto readData : readID_2_alignments)
		{
			processAlignments_oneRead(readData.first, readData.second, referenceSequences);
		}
		readID_2_alignments.clear();
	};

	std::ifstream SAM;
	SAM.open(samFile.c_str());
	assert(SAM.is_open());

	std::string runningReadID;
	std::string line;
	alignment currentAlignment;
	while(std::getline(SAM, line))
	{
		eraseNL(line);
		if(line.length())
		{
			if(line.at(0) == '@')
					continue;

			currentAlignment.fillFromLine(line);

			if(currentAlignment.isUnmapped())
				continue;

			if(currentAlignment.Name != runningReadID)
			{
				processAlignmentStorage();
				processed_readIDs.insert(runningReadID);
				assert(processed_readIDs.count(currentAlignment.Name) == 0);
				runningReadID = currentAlignment.Name;
			}

			std::string readID_withMateInfo = currentAlignment.Name;
			if(currentAlignment.isPaired())
			{
				readID_withMateInfo += ("/" + std::string((currentAlignment.isFirst() ? "1" : "2")));
			}

			readID_2_alignments[readID_withMateInfo].push_back(currentAlignment);
		}
	}

	processAlignmentStorage();

	return 0;

	/*
	 *
		int alignmentStart = currentAlignment.Position;
		int alignmentStop = currentAlignment.GetEndPosition(false, true);
		assert(! currentAlignment.IsPaired());
		assert(currentAlignment.BuildCharData());


		std::string readID_no12 = currentAlignment.Name;
		int whichMate =  (readerI == 0) ? 1 : 2;

		currentAlignment.IsPrimaryAlignment()
	 */
}

std::string expandCIGAR (const std::string& compressedCIGAR, unsigned int expectedLength)
{
	assert(compressedCIGAR.length());

	std::string CIGAR_expanded;
	CIGAR_expanded.reserve(expectedLength);

	std::string CIGAR_digits;
	char CIGAR_operation = '0';

	auto processCIGARElement = [&]() {
		int CIGAR_num_ops = std::stoi(CIGAR_digits);
		assert(CIGAR_num_ops > 0);
		assert((CIGAR_operation == 'M') || (CIGAR_operation == 'I') || (CIGAR_operation == 'D') || (CIGAR_operation == 'S') || (CIGAR_operation == 'H') || (CIGAR_operation == 'P') || (CIGAR_operation == '=') || (CIGAR_operation == 'X'));
		for(int oI = 0; oI < CIGAR_num_ops; oI++)
		{
			CIGAR_expanded.push_back(CIGAR_operation);
		}
		CIGAR_digits.clear();
		CIGAR_operation = '0';
	};

	for(unsigned int i = 0; i < compressedCIGAR.length(); i++)
	{
		assert(std::isdigit(compressedCIGAR.at(i)));
		CIGAR_digits.push_back(compressedCIGAR.at(i));
		if(! std::isdigit(compressedCIGAR.at(i+1)))
		{
			CIGAR_operation = compressedCIGAR.at(i+1);
			processCIGARElement();
			i = i + 1;
		}
	}

	assert(CIGAR_expanded.length());
	return CIGAR_expanded;
}

void processOneReadAlignment(const std::string& readID, const alignment& alignment, const std::string& primaryReadSequence_originalStrand, const std::map<std::string, std::string>& referenceSequences)
{
	std::string CIGAR_expanded = expandCIGAR(alignment.CIGAR, primaryReadSequence_originalStrand.length() * 1.5);

	std::string aligned_reference;
	std::string aligned_read;

	aligned_reference.reserve(CIGAR_expanded.size());
	aligned_read.reserve(CIGAR_expanded.size());

	const std::string& referenceContig = referenceSequences.at(alignment.rName);

	std::string primarySequence_asInAlignment = (alignment.isReverseComplemented()) ? seq_reverse_complement(primaryReadSequence_originalStrand) : primaryReadSequence_originalStrand;

	unsigned int consumed_read_characters = 0;
	unsigned int consumed_read_characters_along_originalSequence = 0;
	unsigned int next_reference_character_to_be_consumed = alignment.POS-1; assert(alignment.POS > 0);

	long long first_referenceBase_in_alignment = -1;
	long long last_referenceBase_in_alignment = -1;
	long long first_readBase_in_alignment = -1;
	long long last_readBase_in_alignment = -1;
	int total_soft_clipping = 0;
	int total_hard_clipping = 0;

	for(unsigned int cigarI = 0; cigarI < CIGAR_expanded.size(); cigarI++)
	{
		// CIGAR operation
		char CIGARoperation = CIGAR_expanded.at(cigarI);

		switch(CIGARoperation)
		{
		// all of these operations take one character out of alignedBases

		case 'D':

			assert(consumed_read_characters > 0); // deletion at the beginning of alignment doesn't make sense
			aligned_reference.push_back(referenceContig.at(next_reference_character_to_be_consumed));
			aligned_read.push_back('-');
			next_reference_character_to_be_consumed++;

			break;

		case 'M':
		case '=':
		case 'X':

			if(first_referenceBase_in_alignment == -1)
			{
				first_referenceBase_in_alignment = next_reference_character_to_be_consumed;
			}
			last_referenceBase_in_alignment = next_reference_character_to_be_consumed;

			if(first_readBase_in_alignment == -1)
			{
				first_readBase_in_alignment = consumed_read_characters_along_originalSequence;
			}
			last_readBase_in_alignment = consumed_read_characters_along_originalSequence;

			aligned_reference.push_back(referenceContig.at(next_reference_character_to_be_consumed));
			aligned_read.push_back(primarySequence_asInAlignment.at(consumed_read_characters_along_originalSequence));

			next_reference_character_to_be_consumed++;
			consumed_read_characters++;
			consumed_read_characters_along_originalSequence++;

			break;

		case 'I':

			if(first_readBase_in_alignment == -1)
			{
				first_readBase_in_alignment = consumed_read_characters_along_originalSequence;
			}
			last_readBase_in_alignment = consumed_read_characters_along_originalSequence;

			aligned_reference.push_back('-');
			aligned_read.push_back(primarySequence_asInAlignment.at(consumed_read_characters_along_originalSequence));
			consumed_read_characters_along_originalSequence++;

			break;

		case 'N':

			throw std::runtime_error("N character in CIGAR - should only be the case for RNASeq data!\n");
			break;

		case 'S':

			consumed_read_characters_along_originalSequence++;
			total_soft_clipping++;
			break;

		case 'H':

			consumed_read_characters_along_originalSequence++;
			total_hard_clipping++;
			break;

		case 'P':

			throw std::runtime_error("P character in CIGAR - should have been removed in advance!\n");
			break;

		default:

			throw std::runtime_error("Unknown element of CIGAR string");
		}

	}

	int read_bases_in_alignment = last_readBase_in_alignment - first_readBase_in_alignment + 1;
	assert(read_bases_in_alignment > 0);
	if(alignment.SEQ != "*")
	{
		assert(read_bases_in_alignment == ((int)alignment.SEQ.length() - total_soft_clipping));
	}
	assert((int)primaryReadSequence_originalStrand.length() == (read_bases_in_alignment + total_soft_clipping + total_hard_clipping));
	assert((long long)alignment.POS == (first_referenceBase_in_alignment+1));
	assert(aligned_reference.length() == aligned_read.length());

	unsigned int nm = 0;
	for(unsigned int cI = 0; cI < aligned_reference.length(); cI++)
	{
		if(aligned_reference.at(cI) != aligned_read.at(cI))
			nm++;
	}
	int output_read_coordinates_1 = first_readBase_in_alignment;
	int output_read_coordinates_2 = last_readBase_in_alignment;
	if(alignment.isReverseComplemented())
	{
		output_read_coordinates_1 = primaryReadSequence_originalStrand.length() - 1 - output_read_coordinates_1;
		output_read_coordinates_2 = primaryReadSequence_originalStrand.length() - 1 - output_read_coordinates_2;
	}
	std::cout
		<< readID << " "
		<< alignment.rName << ":"
		<< first_referenceBase_in_alignment+1 << "-" << last_referenceBase_in_alignment+1 << " "
		<< "read:" << output_read_coordinates_1+1 << "-" << output_read_coordinates_2+1 << " "
		<< "(" << nm << "/" << aligned_read.length() << ")" << " "
		<< "mapQ="+std::to_string(alignment.mapQ) << " "
		<< "flags="+std::to_string(alignment.FLAG) << " "
		<< "secondary="+std::string(alignment.isNotPrimary() ? "1" : "0") << " "
		<< "supplementary="+std::string(alignment.isSupplementary() ? "1" : "0") << " "
		<< alignment.tags << "\n";

	std::cout << aligned_reference << "\n";
	std::cout << aligned_read << "\n";
}

void processAlignments_oneRead(const std::string& readID, const std::vector<alignment>& alignments, const std::map<std::string, std::string>& referenceSequences)
{
	std::string primaryReadSequence_originalStrand;
	for(const alignment& alignment : alignments)
	{
		if(! referenceSequences.count(alignment.rName))
		{
			throw std::runtime_error("Unknown reference sequence " + alignment.rName);
		}
		assert(referenceSequences.count(alignment.rName));

		if(alignment.isPrimary() && (! alignment.isSupplementary()))
		{
			std::string alignment_sequence_nonReverse = alignment.isReverseComplemented() ? seq_reverse_complement(alignment.SEQ) : alignment.SEQ;

			if(primaryReadSequence_originalStrand.length())
			{
				if(primaryReadSequence_originalStrand != alignment_sequence_nonReverse)
				{
					std::cerr << "Read " << readID << " seems to have multiple primary-aligment-associated sequences.\n";
					std::cerr << "Alternative 1: " << alignment_sequence_nonReverse << "\n";
					std::cerr << "Alternative 2: " << primaryReadSequence_originalStrand << "\n";
					std::cerr << std::flush;
				}
				assert(primaryReadSequence_originalStrand == alignment_sequence_nonReverse);
			}
			else
			{
				primaryReadSequence_originalStrand = alignment_sequence_nonReverse;
			}

			for(const auto& cigarElement : alignment.CIGAR)
			{
				assert(cigarElement != 'H');
			}
		}
	}

	if(primaryReadSequence_originalStrand.length() == 0)
	{
		throw std::runtime_error("Missing primary non-supplementary alignment for read " + readID);
	}
	assert(primaryReadSequence_originalStrand.length());

	for(const auto& oneAlignment : alignments)
	{
		processOneReadAlignment(readID, oneAlignment, primaryReadSequence_originalStrand, referenceSequences);
	}
}

void eraseNL(string& s)
{
	while (!s.empty() && ((s[s.length()-1] == '\r') || (s[s.length()-1] == '\n'))) {
	    s.erase(s.length()-1);
	}
}

map<string, string> readFASTA(std::string file, bool fullIdentifier)
{
	map<string, string> forReturn;

	std::ifstream FASTAstream;

	FASTAstream.open(file.c_str());
	if(! FASTAstream.is_open())
	{
		throw std::runtime_error("readFASTA(): Cannot open file "+file);
	}

	while(FASTAstream.good())
	{
		std::string line;
		size_t lineCounter = 0;

		std::string currentSequenceIdentifier;
		while(FASTAstream.good())
		{
			std::getline(FASTAstream, line);
			eraseNL(line);

			lineCounter++;

			if(line.substr(0, 1) == ">")
			{
				std::string ident = line.substr(1);

				if(! fullIdentifier)
				{
					for(size_t i = 0; i < ident.size(); i++)
					{
						if(ident.at(i) == ' ')
						{
							ident = ident.substr(0, i);
							break;
						}
					}
				}
				currentSequenceIdentifier = ident;
				assert(forReturn.count(ident) == 0);
			}
			else
			{
				forReturn[currentSequenceIdentifier] += line;
			}
		}
	}

	return forReturn;
}

void testFlags()
{
	alignment A;
	A.FLAG = 81;
	assert(  A.isPaired());
	assert(! A.isNotPrimary());
	assert(! A.isSupplementary());
	assert(  A.isFirst());
	assert(! A.isSecond());
	assert(A.isReverseComplemented());

	A.FLAG = 2304;
	assert(! A.isPaired());
	assert(  A.isNotPrimary());
	assert(  A.isSupplementary());
	assert(! A.isReverseComplemented());

}

std::vector<std::string> split(std::string input, std::string delimiter)
{
	std::vector<std::string> output;
	if(input.length() == 0)
	{
		return output;
	}

	if(delimiter == "")
	{
		output.reserve(input.size());
		for(unsigned int i = 0; i < input.length(); i++)
		{
			output.push_back(input.substr(i, 1));
		}
	}
	else
	{
		if(input.find(delimiter) == std::string::npos)
		{
			output.push_back(input);
		}
		else
		{
			int s = 0;
			int p = input.find(delimiter);

			do {
				output.push_back(input.substr(s, p - s));
				s = p + delimiter.size();
				p = input.find(delimiter, s);
			} while (p != (int)std::string::npos);
			output.push_back(input.substr(s));
		}
	}

	return output;
}


std::string seq_reverse_complement(std::string sequence)
{
	int length = sequence.size();
	std::string forReturn;
	forReturn.resize(length);
    for(int k=0; k < length; k++)
    {
        forReturn[k] = reverse_char_nucleotide(sequence.at(length-k-1));
    }
    return forReturn;
}


char reverse_char_nucleotide(char c)
{
    switch (c)
    {
		case 'A':
			return 'T';
		case 'C':
			return 'G';
		case 'G':
			return 'C';
		case 'T':
			return 'A';
		case 'N':
			return 'N';
		case 'a':
			return 't';
		case 'c':
			return 'g';
		case 'g':
			return 'c';
		case 't':
			return 'a';
		case 'n':
			return 'n';
		case '_':
			return '_';
		case '*':
			return '*';
		default:
			std::string errorString = "Utilities::reverse_char_nucleotide: nucleotide not existing!";
			errorString.push_back(c);
			throw std::runtime_error(errorString);
    }
}




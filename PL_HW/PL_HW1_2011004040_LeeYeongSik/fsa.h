// PL homework: hw1
// fsa.h

#ifndef _PL_HOMEWORK_FSA_H_
#define _PL_HOMEWORK_FSA_H_

#include <map>
#include <set>

struct TableElement {
  int state;
  char input_char;
  int next_state;
};

struct FiniteStateAutomaton 
{
	int													initial;
	std::set<int>										finals;

	std::set<int>										states;
	std::set<char>										inputs;
	std::multimap<std::pair<int, char>, int>			trans;
};

// Homework 1.1
bool RunFSA(const FiniteStateAutomaton* fsa, const char* str);

// Homework 1.1 and 1.2
bool BuildFSA(const TableElement* elements, int num_elements,
              const int* accept_states, int num_accept_states,
              FiniteStateAutomaton* fsa);

// Homework 1.3
bool BuildFSA(const char* regex, FiniteStateAutomaton* fsa);

#endif //_PL_HOMEWORK_FSA_H_


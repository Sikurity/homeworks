#include <map>
#include <set>
#include <vector>
#include <stack>

#ifndef _PL_HOMEWORK_REGEXP_MATCHER_H_
#define _PL_HOMEWORK_REGEXP_MATCHER_H_

using namespace std;

struct TableElement
{
	int		state;
	char	input_char;
	int		next_state;
};

struct FiniteStateAutomaton 
{
	int													initial;
	std::set<int>										finals;

	std::set<int>										states;
	std::set<char>										inputs;
	std::multimap<std::pair<int, char>, int>			trans;
};

bool RunFSA(const FiniteStateAutomaton* fsa, const char* str);
bool BuildFSA(const TableElement* elements, int num_elements, const int* accept_states, int num_accept_states, FiniteStateAutomaton* fsa);

struct RegExpMatcher
{
	FiniteStateAutomaton fsa;

	vector<TableElement> elements;
	stack< pair<char, int> > regexStack;
	
	int final;
};

bool BuildRegExpMatcher(const char* regexp, RegExpMatcher* regexp_matcher);

bool RunRegExpMatcher(const RegExpMatcher* regexp_matcher, const char* str);

#endif


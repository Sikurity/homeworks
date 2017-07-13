#include <map>
#include <set>
#include <vector>
#include <stack>
#include <string>

#ifndef _PL_HOMEWORK_LR_PARSER_H_
#define _PL_HOMEWORK_LR_PARSER_H_

using namespace std;

enum LRAction 
{
	INVALID = 0,
	SHIFT = 1,
	REDUCE = 2,
	ACCEPT = 3,
	GOTO = 4
};

struct LRTableElement 
{
	int state;
	int symbol;
	LRAction action;
	int next_state;
};

struct LRRule 
{
	int id;
	int lhs_symbol;
	int num_rhs;
};

struct LRParser 
{
	map< pair<int, int>, pair<LRAction, int> > actionTable;
	map< pair<int, int>, int > gotoTable;
	vector< pair<int, int> > ruleTable;
};

bool BuildLRParser(const LRTableElement* elements, int num_elements,
                   const LRRule* rules, int num_rules,
                   LRParser* lr_parser);

bool RunLRParser(const LRParser* lr_parser, const char* str);

#endif
#include <assert.h>

#include <iostream>
#include <vector>

#include "lr_parser.h"

#define DISABLE_LOG false
#define LOG \
    if (DISABLE_LOG) {} else std::cerr

using namespace std;

bool BuildLRParser(const LRTableElement* elements, int num_elements,
                   const LRRule* rules, int num_rules,
                   LRParser* lr_parser) 
{
	int i;

	lr_parser->ruleTable.resize(num_rules + 1);

	for( i = 0 ; i < num_elements ; i++ )
	{
		if( elements[i].action != GOTO )
			lr_parser->actionTable[make_pair(elements[i].state, elements[i].symbol)] = make_pair(elements[i].action, elements[i].next_state);
		else
			lr_parser->gotoTable[make_pair(elements[i].state, elements[i].symbol)] = elements[i].next_state;
	}

	lr_parser->ruleTable[0] = make_pair(INVALID, 0);
	for( i = 0 ; i < num_rules ; i++ )
		lr_parser->ruleTable[rules[i].id] = make_pair(rules[i].lhs_symbol, rules[i].num_rhs);

	return true;
}

bool RunLRParser(const LRParser* lr_parser, const char* str) 
{
	stack< pair<int, int> > parseStack;

	pair<int, int>	prevActionState;
	pair<int, int>	curActionState;
	pair<int, int>	nextActionState;
	pair<int, int>	tmp;
	
	int input;
	int num_rhs, pos = 0;

	parseStack.push(make_pair(INVALID, 0));

	while( true )
	{
		if( ('a' <= str[pos] && str[pos] <= 'z') || ('0' <= str[pos] && str[pos] <= '9')  || str[pos] == '.' )
			input = (int)'I';
		else
			input = (int)(str[pos]);

		if( parseStack.empty() )
			return false;
		
		curActionState = parseStack.top();

		try
		{
			nextActionState = lr_parser->actionTable.at(make_pair(curActionState.second, input));
		}
		catch(exception e)
		{
			return false;
		}

		if( nextActionState.first == SHIFT )
		{
			if( input != '$' )
			{
				parseStack.push(make_pair(input, nextActionState.second));
				pos++;
			}
			else
				break;
		}
		else if( nextActionState.first == REDUCE )
		{
			if( nextActionState.second )
			{
				try
				{
					num_rhs = lr_parser->ruleTable[nextActionState.second].second;
				}
				catch(exception e)
				{
					return false;
				}
			}
			else
				break;

			while( num_rhs-- )
			{
				if( parseStack.empty() )
					break;	
				else
					parseStack.pop();
			}

			if( parseStack.empty() )
				break;	
			else
			{
				prevActionState = parseStack.top();

				try
				{
					tmp = make_pair
						(
							lr_parser->ruleTable[nextActionState.second].first, 
							lr_parser->gotoTable.at(make_pair(prevActionState.second, lr_parser->ruleTable[nextActionState.second].first))
						);
				}
				catch( exception e )
				{
					return false;
				}
					
				parseStack.push(tmp);

			}
		}
		else if( nextActionState.first == ACCEPT )
			return true;
		else
			break;
	}

	return false;
}
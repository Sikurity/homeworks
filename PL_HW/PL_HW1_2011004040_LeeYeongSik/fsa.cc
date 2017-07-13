// PL homework: hw1
// fsa.cc

#include <iostream>
#include <queue>
#include <string.h>

#include "fsa.h"

#define DISABLE_LOG true
#define LOG \
    if (DISABLE_LOG) {} else std::cerr

const char kEps = '#';

using namespace std;

typedef multimap<pair<int, char>, int> Multi_PairIntChar_I;

bool CheckIfNFA(const TableElement* elements, int num_elements) 
{
	int i;
	bool result, **checkTable;

	checkTable = new bool *[num_elements];

	for( i = 0 ; i < num_elements ; i++ )
	{
		checkTable[i] = new bool[256];
		memset(checkTable[i], false, sizeof(bool) * 256);
	}

	for( i = 0 ; i < num_elements ; i++ )
	{
		if( elements[i].input_char == '#' )
			break;

		if( checkTable[elements[i].state][elements[i].input_char] )
			break;
		else
			checkTable[elements[i].state][elements[i].input_char] = true;
	}

	if( i == num_elements )
		result = false;
	else
		result = true;

	for( i = 0 ; i < num_elements ; i++ )
		delete[] checkTable[i];

	delete[] checkTable;

	return result;
}

bool BuildDFA(const TableElement* elements, int num_elements,
              const int* accept_states, int num_accept_states,
              FiniteStateAutomaton* fsa) 
{
	int i;

	fsa->initial = elements[0].state;

	for( i = 0 ; i < num_accept_states ;i++ )
		fsa->finals.insert(accept_states[i]);

	for( i = 0 ; i < num_elements ;i++ )
	{
		fsa->inputs.insert(elements[i].input_char);
		fsa->states.insert(elements[i].state);
		
		fsa->trans.insert
		(
			multimap<pair<int, char>, int>::value_type
			(
				make_pair(elements[i].state, elements[i].input_char), 
				elements[i].next_state
			)
		);
	}

	LOG << "num_elements: " << num_elements << endl;

	if (num_elements <= 0) 
		return false;

	return true;
}

bool BuildNFA(const TableElement* elements, int num_elements,
              const int* accept_states_array, int num_accept_states,
              FiniteStateAutomaton* fsa) 
{
	int i, j, curNum, elementsNum, acceptsNum;

	TableElement *newElements;
	int *newAcceptStates;

	queue<int>			q;
	queue< set<int> >	qS;

	set<int>	cur, next, group, accept, final;
	
	map<int, set<int> >			stEpsilon;
	map<set<int>, int>			stGroup;
	map<pair<int, char>, int>	stTrnas;
	
	set<char>::iterator				iterInput;
	set<int>::iterator				iterState;
	set<int>::iterator				iterGroup;
	Multi_PairIntChar_I::iterator	iterTrans;

	pair<Multi_PairIntChar_I::iterator, Multi_PairIntChar_I::iterator>	iterRange;

	BuildDFA(elements, num_elements, accept_states_array, num_accept_states, fsa);

	for( iterState = fsa->states.begin() ; iterState != fsa->states.end() ; iterState++ )
	{
		cur.clear();
		cur.insert(*iterState);

		q.push(*iterState);

		while( !q.empty() )
		{
			curNum = q.front();
			q.pop();

			iterRange = fsa->trans.equal_range(make_pair(curNum, '#'));

			for( iterTrans = iterRange.first; iterTrans != iterRange.second; iterTrans++ )
			{
				curNum = iterTrans->second;
				if( cur.find(curNum) == cur.end() )
				{
					cur.insert(curNum);
					q.push(curNum);
				}
			}
		}

		stEpsilon[*iterState] = cur;
	}
	
	acceptsNum = 0;
	elementsNum = 1;
	
	cur.clear();
	cur.insert(fsa->initial);

	stGroup[cur] = elementsNum++;
	if( fsa->finals.find(fsa->initial) != fsa->finals.end() )
	{
		acceptsNum++;
		accept.insert(1);
	}

	qS.push(cur);

	fsa->inputs.erase('#');

	while( !qS.empty() )
	{
		cur = qS.front();
		qS.pop();

		for( iterInput = fsa->inputs.begin() ; iterInput != fsa->inputs.end() ; iterInput++ )
		{
			/* make next state group set for the input */
			group.clear();

			for( iterState = cur.begin() ; iterState != cur.end() ; iterState++ )
			{
				next = stEpsilon[*iterState];
				for( iterGroup = next.begin() ; iterGroup != next.end() ; iterGroup++ )
					group.insert(*iterGroup);
			}

			next.clear();

			for( iterState = group.begin() ; iterState != group.end() ; iterState++ )
			{
				iterRange = fsa->trans.equal_range(make_pair(*iterState, *iterInput));

				for( iterTrans = iterRange.first; iterTrans != iterRange.second; ++iterTrans )
					next.insert(iterTrans->second);
			}

			final = next;

			for( iterState = next.begin() ; iterState != next.end() ; iterState++ )
			{
				group = stEpsilon[*iterState];
				for( iterGroup = group.begin() ; iterGroup != group.end() ; iterGroup++ )
					final.insert(*iterGroup);
			}

			if( final.empty() )
				stTrnas[make_pair(stGroup[cur], *iterInput)] = 0;
			else 
			{
				if( stGroup.find(final) == stGroup.end() )
				{
					stTrnas[make_pair(stGroup[cur], *iterInput)] = elementsNum;
					stGroup[final] = elementsNum++;

					for( iterGroup = fsa->finals.begin() ; iterGroup != fsa->finals.end() ; iterGroup++ )
					{
						if( final.find(*iterGroup) != final.end() )
						{
							acceptsNum++;
							accept.insert(stGroup[final]);

							break;
						}
					}

					qS.push(final);
				}
				else
					stTrnas[make_pair(stGroup[cur], *iterInput)] = stGroup[final];
			}
		}
	}

	newElements	= new TableElement[elementsNum * fsa->inputs.size()];

	i = 1;
	j = 0;
	while( i < elementsNum )
	{
		for( iterInput = fsa->inputs.begin() ; iterInput != fsa->inputs.end() ; iterInput++ )
		{
			newElements[j].state = i;
			newElements[j].input_char = *iterInput;
			newElements[j].next_state = stTrnas[make_pair(i, *iterInput)];

			j++;
		}
		i++;
	}
	
	elementsNum *= fsa->inputs.size();

	newAcceptStates = new int[acceptsNum];
	
	i = 0;
	for( iterState = accept.begin() ; iterState != accept.end() ; iterState++ )
		newAcceptStates[i++] = *iterState;

	fsa->finals.clear();
	fsa->initial = 0;
	fsa->inputs.clear();
	fsa->states.clear();
	fsa->trans.clear();

	BuildDFA(newElements, elementsNum, newAcceptStates, acceptsNum, fsa);
	
	delete[] newElements;
	delete[] newAcceptStates;

	return true;
}

// Homework 1.1
bool RunFSA(const FiniteStateAutomaton* fsa, const char* str) 
{
	int i, cur;
	set<int>::iterator iter;

	cur = fsa->initial;

	for( i = 0 ; str[i] != NULL ; i++ )
		cur = fsa->trans.find(make_pair(cur, str[i]))->second;

	iter = fsa->finals.find(cur);
	
	if( iter != fsa->finals.end() )
		return true;

	return false;
}

// Homework 1.1 and 1.2
bool BuildFSA(const TableElement* elements, int num_elements,
              const int* accept_states, int num_accepts,
              FiniteStateAutomaton* fsa) {
  if (CheckIfNFA(elements, num_elements)) {
    return BuildNFA(elements, num_elements, accept_states, num_accepts, fsa);
  } else {
    return BuildDFA(elements, num_elements, accept_states, num_accepts, fsa);
  }
}

// Homework 1.3
bool BuildFSA(const char* regex, FiniteStateAutomaton* fsa) {
  // Implement this function.
  return false;
}
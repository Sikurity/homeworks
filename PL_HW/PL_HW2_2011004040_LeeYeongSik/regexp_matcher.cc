#include <string.h>
#include <queue>
#include <list>
#include "regexp_matcher.h"

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
		if( elements[i].input_char == kEps )
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

bool BuildDFA(const TableElement* elements, int num_elements, const int* accept_states, int num_accept_states, FiniteStateAutomaton* fsa)
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

	if (num_elements <= 0) 
		return false;

	return true;
}

bool BuildNFA(const TableElement* elements, int num_elements, const int* accept_states_array, int num_accept_states, FiniteStateAutomaton* fsa) 
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

			iterRange = fsa->trans.equal_range(make_pair(curNum, kEps));

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

	fsa->inputs.erase(kEps);

	while( !qS.empty() )
	{
		cur = qS.front();
		qS.pop();

		for( iterInput = fsa->inputs.begin() ; iterInput != fsa->inputs.end() ; iterInput++ )
		{
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

bool RunFSA(const FiniteStateAutomaton* fsa, const char* str) 
{
	int i, cur;
	set<int>::iterator iter;

	cur = fsa->initial;

	for( i = 0 ; str[i] ; i++ )
		cur = fsa->trans.find(make_pair(cur, str[i]))->second;

	iter = fsa->finals.find(cur);
	
	if( iter != fsa->finals.end() )
		return true;

	return false;
}

bool BuildFSA(const TableElement* elements, int num_elements, const int* accept_states, int num_accepts, FiniteStateAutomaton* fsa) 
{
	return CheckIfNFA(elements, num_elements) ? BuildNFA(elements, num_elements, accept_states, num_accepts, fsa) : BuildDFA(elements, num_elements, accept_states, num_accepts, fsa);
}

bool BuildRegExpMatcher(const char* regexp, RegExpMatcher* regexp_matcher)
{
	set< pair<int, int> > epsPairSet;
	map<int, list<int> > regexMap;
	pair<char, int> prevState;
	list<int>::iterator iterList;

	TableElement element;

	char cur;
	int i, num, strPos;

	int		nextState;
	char	inputChar;

	int largePair;
	int smallPair;
	
	num = 1;
	strPos = 0;

	largePair = 0;
	smallPair = 0;
	
	element.state = num;
	element.input_char = kEps;
	element.next_state = num;

	regexp_matcher->elements.push_back(element);

	regexp_matcher->regexStack.push(make_pair('(', num++));

	while( !(regexp_matcher->regexStack.empty()) )
	{
		if( regexp[strPos] )
		{
			cur = regexp[strPos];
			
			if(	cur == '(' )
				smallPair++;
			else if( cur == ')' )
			{
				if( smallPair > 0 )
					smallPair--;
				else
					return false;
			}
			else if( cur == '[' )
			{
				if( largePair == 0 )
					largePair = 1;
				else
					return false;
			}
			else if( cur == ']' )
			{
				if( largePair == 1 )
					largePair = 0;
				else
					return false;
			}
		}
		else
			cur = ')';

		if( cur == '|' || cur == ')' )
		{
			inputChar = kEps;
			nextState = num;

			do
			{
				if( regexp_matcher->regexStack.empty() )
					return false;
				
				prevState = regexp_matcher->regexStack.top();
				regexp_matcher->regexStack.pop();

				if( inputChar == '.' )
				{
					for( i = '0' ; i <= '9' ; i++ )
					{
						element.state		= prevState.second;
						element.input_char	= i;
						element.next_state	= nextState;

						regexp_matcher->elements.push_back(element);
					}

					for( i = 'A' ; i <= 'Z' ; i++ )
					{
						element.state		= prevState.second;
						element.input_char	= i;
						element.next_state	= nextState;

						regexp_matcher->elements.push_back(element);
					}
					
					for( i = 'a' ; i <= 'z' ; i++ )
					{
						element.state		= prevState.second;
						element.input_char	= i;
						element.next_state	= nextState;

						regexp_matcher->elements.push_back(element);
					}
				}
				else if( inputChar == '*' )
				{
					if( prevState.first == '#' )
					{
						element.state		= prevState.second;
						element.input_char	= kEps;
						element.next_state	= nextState;
				
						regexp_matcher->elements.push_back(element);

						nextState = prevState.second;
				
						if( regexp_matcher->regexStack.empty() )
							return false;
						
						prevState = regexp_matcher->regexStack.top();
						regexp_matcher->regexStack.pop();

						element.state		= nextState;
						element.input_char	= kEps;
						element.next_state	= prevState.second;
				
						regexp_matcher->elements.push_back(element);

						element.state		= prevState.second;
						element.input_char	= kEps;
						element.next_state	= nextState;
				
						regexp_matcher->elements.push_back(element);
					}
					else 
					{
						element.state		= nextState;
						element.input_char	= kEps;
						element.next_state	= prevState.second;
				
						regexp_matcher->elements.push_back(element);
						
						if( prevState.first == '.' )
						{
							for( i = '0' ; i <= '9' ; i++ )
							{
								element.state		= prevState.second;
								element.input_char	= i;
								element.next_state	= prevState.second + 1;

								regexp_matcher->elements.push_back(element);
							}

							for( i = 'A' ; i <= 'Z' ; i++ )
							{
								element.state		= prevState.second;
								element.input_char	= i;
								element.next_state	= prevState.second + 1;

								regexp_matcher->elements.push_back(element);
							}
					
							for( i = 'a' ; i <= 'z' ; i++ )
							{
								element.state		= prevState.second;
								element.input_char	= i;
								element.next_state	= prevState.second + 1;

								regexp_matcher->elements.push_back(element);
							}
						}
						else
						{
							element.state		= prevState.second;
							element.input_char	= prevState.first;
							element.next_state	= prevState.second + 1;

							regexp_matcher->elements.push_back(element);
						}

						element.state		= nextState - 1;
						element.input_char	= kEps;
						element.next_state	= nextState;
				
						regexp_matcher->elements.push_back(element);

						element.state		= prevState.second;
						element.input_char	= kEps;
						element.next_state	= nextState;
				
						regexp_matcher->elements.push_back(element);

						prevState.first = kEps;
					}
				}
				else if( inputChar != '#' || prevState.first != '#' || epsPairSet.find(make_pair(prevState.second, nextState)) == epsPairSet.end() )
				{
					element.state = prevState.second;
					element.input_char = inputChar;
					element.next_state = nextState;

					regexp_matcher->elements.push_back(element);
				}
				
				inputChar = prevState.first;
				nextState = prevState.second;
			}
			while( prevState.first != '(' );
			
			regexMap[prevState.second].push_back(num++);

			if( cur == ')'  )
			{			
				for( iterList = regexMap[prevState.second].begin() ; iterList != regexMap[prevState.second].end() ; iterList++ )
				{
					element.state = *iterList;
					element.input_char = kEps;
					element.next_state = num;
				
					regexp_matcher->elements.push_back(element);
				}

				if( regexp_matcher->regexStack.empty() && !regexp[strPos] )
				{
					if( smallPair || largePair )
						return false;
					else
						break;
				}
				else
				{
					epsPairSet.insert(make_pair(prevState.second, num));
					regexp_matcher->regexStack.push(make_pair(kEps, prevState.second));
					regexp_matcher->regexStack.push(make_pair(kEps, num++));
				}
			}
			else
				regexp_matcher->regexStack.push(make_pair('(', prevState.second));
		}
		else if( cur == ']' )
		{
			if( regexp_matcher->regexStack.empty() )
				return false;
			
			prevState = regexp_matcher->regexStack.top();

			while( prevState.first != '[' )
			{
				if
				( 
					prevState.first == '#' || 
					prevState.first == '|' || 
					prevState.first == '*' || 
					prevState.first == '.' 
				)
					return false;

				if( regexp_matcher->regexStack.empty() )
					return false;
				
				regexp_matcher->regexStack.pop();
				regexMap[num].push_back(prevState.first);

				if( regexp_matcher->regexStack.empty() )
					return false;
				
				prevState = regexp_matcher->regexStack.top();
			}

			if( regexMap[num].empty() )
			{
				element.state		= prevState.second;
				element.input_char	= kEps;
				element.next_state	= num;
				
				regexp_matcher->elements.push_back(element);
			}
			else
			{
				for( iterList = regexMap[num].begin() ; iterList != regexMap[num].end() ; iterList++ )
				{
					element.state		= prevState.second;
					element.input_char	= *iterList;
					element.next_state	= num;
				
					regexp_matcher->elements.push_back(element);
				}
			}

			element.state		= num++;
			element.input_char	= kEps;
			element.next_state	= num;
				
			regexp_matcher->elements.push_back(element);

			if( regexp_matcher->regexStack.empty() )
				return false;
			
			regexp_matcher->regexStack.pop();

			epsPairSet.insert(make_pair(prevState.second, num));
			regexp_matcher->regexStack.push(make_pair(kEps, prevState.second));
			regexp_matcher->regexStack.push(make_pair(kEps, num++));
		}
		else if( cur )
		{
			if( regexp_matcher->regexStack.empty() )
				return false;
			
			prevState = regexp_matcher->regexStack.top();

			if( cur == '*' && prevState.first != '#' )
			{
				if
				(
					prevState.first == '|' ||
					prevState.first == '[' ||
					prevState.first == '(' ||
					prevState.first == '*'
				)
					return false;

				num++;
				regexp_matcher->regexStack.push(make_pair(cur, num++));
			}
			else
				regexp_matcher->regexStack.push(make_pair(cur, num++));
		}

		strPos++;
	}

	regexp_matcher->final = num;
	BuildFSA(&regexp_matcher->elements[0], regexp_matcher->elements.size(), (int *)&regexp_matcher->final, 1, &regexp_matcher->fsa);

	return true;
}

bool RunRegExpMatcher(const RegExpMatcher* regexp_matcher, const char* str)
{
	return RunFSA(&regexp_matcher->fsa, str);
}
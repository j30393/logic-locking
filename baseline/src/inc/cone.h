#pragma once

#ifndef _CONE_H
#define _CONE_H

#include<string>
#include"./node.h"
#include<set>
#include<algorithm>



class CONE;

bool compareCone(CONE *_cone1, CONE *_cone2 );

class CONE{

	public:
		CONE(){
			input.clear();
		}
		CONE(FType _ft, NODE* _node)
			:type(_ft), output(_node){
			input.clear();
		}
		CONE(FType _ft)
			:type(_ft){
			input.clear();
		}
		~CONE(){
			input.clear();
		}
		
		bool operator ==(CONE* _C);
		
		bool operator >(CONE* _A){
			return input.size() > _A->getInput().size();
		}
		bool operator <(CONE* _A){
			return input.size() < _A->getInput().size();
		}
		friend std::ostream& operator<<(std::ostream& os, CONE* _cone);
		
		std::set<NODE*>& 	getInput()						{ return input; 				}
		NODE*				getOutput()						{ return output; 				}
		void				insertInput(NODE* _node)		{ input.insert(_node); 			}
		void				setOutput(NODE* _node)			{ output = _node;				}
		void				eraseInput(NODE* _node);		//erase
		int					IntersectionSize(CONE* _cone);	//get two set insterction size
		std::set<NODE*>		IntersectionCone(std::vector<CONE*>);	//get two set insterction size
		FType				getFtype()						{ return type;				 	}
		void				setFtype(FType _type)			{ type = _type;					}
		
	private:
		NODE* output;
		std::set<NODE*>input;
		FType type;
};

#endif

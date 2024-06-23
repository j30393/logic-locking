#pragma once

#ifndef _ENCRYPTION_H
#define _ENCRYPTION_H

#include"./node.h"
#include<iostream>
#include<sstream>
#include<fstream>
#include<map>
#include<cctype>
#include<algorithm>
#include<stdio.h>
#include <random>
#include <chrono>
#include<cstring>
#include <time.h>
#include <stack>
#include <queue>
#include <cassert>
#include <cmath>
//#define bug



class encryption;

class encryption{
	public:
		encryption();
		~encryption();
			
		//operator
		const 	std::string	getKey()							{ return key;					}
		const	int			getKeylen()							{ return KEY_Ary.size();		}
				void		insertNODE(NODE *_node)				{ NODE_Ary.push_back(_node);	}
				void		insertPI(NODE *_node)				{ PI_Ary.push_back(_node);		}
				void		insertPO(NODE *_node)				{ PO_Ary.push_back(_node);		}
				void		insertKey(NODE *_node)				{ KEY_Ary.push_back(_node);		}
				void		readfile(std::string);				//read .bench file
				void		topological_sort();					// pre-process for logic cone
				void		outputfile();
				void		setOutputname(std::string _name);
				void 		graph_traverse();
				void		setDebugMode(bool _debug)			{ is_debug = _debug;			}
				// fault base 
				void		set_test_num(int _num)				{ test_num = _num;				}
		const	int			get_test_num()						{ return test_num;				}
				void		fault_impact_cal();
		float 		key_ratio;
				void		xor_encryption();
				void 		sl_one_encryption();
		// fault base example num
		std::vector<bool>	solver(std::vector<bool>);			// generate the output from the given input 
	private:
		std::vector<NODE*>				NODE_Ary;
		std::vector<NODE*>				PI_Ary;
		std::vector<NODE*>				PO_Ary;
		std::vector<NODE*>				KEY_Ary;
		std::vector<NODE*>				ENCY_Ary;
		std::vector<NODE*>				AND_Ary;
		std::vector<NODE*>				OR_Ary;
		std::map<std::string, NODE*>	name2node;
		std::string 			 		key;
		std::string	 			 		filename;	
		std::string	 			 		outputname;
		bool 					 		twolevelfile;
		bool 					 		is_debug;
		int 							keyCount;	
		std::vector<int>				visited;
		std::vector<int>				in_degree;
		std::vector<int>				hue;
		double							threshold;
		int 							surplus_area;
		int 	 						constraint;
		// fault base example num
		int 							test_num;
};

#endif

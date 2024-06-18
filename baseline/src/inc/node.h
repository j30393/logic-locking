#pragma once

#ifndef _NODE_H
#define _NODE_H

#include<string>
#include<iostream>
#include<vector>
#include<cstdint>
#include<set>

#define debug

enum class Type{ 
	Intl, 
	PI, 
	PO
};

std::ostream& operator<<(std::ostream& os, Type _t);

enum class FType{
	AND, 
	OR, 
	NOR, 
	NAND, 
	NOT, 
	BUF, 
	XOR, 
	XNOR
};

std::ostream& operator<<(std::ostream& os, FType _ft);


class NODE;


class NODE{
	public:
		NODE()
			:CO(0), enc(false), t(Type::Intl), ft(FType::BUF), name(""), path_len(0), and_counter(0), 
			or_counter(0), id(0), depth(-0xfffffff), is_stuck_faulting(false), fault_impact(0) , NoO0(0), 
			NoP0(0), NoO1(0), NoP1(0){
		}

		NODE(Type _t, FType _ft, std::string _name)
			:CO(0), t(_t), ft(_ft), name(_name), path_len(0), and_counter(0), or_counter(0), id(0), enc(false), 
			depth(-0xfffffff), is_stuck_faulting(false), fault_impact(0) , NoO0(0), NoP0(0), NoO1(0), NoP1(0){
		}

		~NODE(){
			FI_Ary.clear();
			FO_Ary.clear();
		}
		
		NODE* operator=(NODE* _n){
			NODE *tem_n;
			tem_n = _n;
			return tem_n;
		}
		//operator overloading
		bool operator ==(NODE* _A){
			//std::cout<<"test ->" <<name<<" , "<<_A->getName()<<std::endl;
			return (name == _A->getName() && ft == _A->getFtype());
		}
		bool operator ==(std::string _name){
			//std::cout<<"testB ->" <<name<<" , "<<_name<<std::endl;
			return name == _name;
		}
		
		bool operator >(NODE* _A){
			return name > _A->getName();
		}
		bool operator <(NODE* _A){
			return name < _A->getName();
		}
		friend std::ostream& operator<<(std::ostream& os, NODE* p);

		// 109062233 add 
		const 	int 		getDepth()					{ return depth;						}
				void		setDepth(int _num)			{ depth = _num; 					}
		//operator
		const 	int 		getFIlen()					{ return FI_Ary.size();				} 
		const 	int 		getFOlen()					{ return FO_Ary.size();				}
				void		setFtype(FType _ft)			{ ft = _ft;							}
				void		setType(Type _t)			{ t = _t; 							}
		const 	FType		getFtype()					{ return ft;						}
		const 	Type		getType()					{ return t;							}
		const 	std::string	getName()					{ return name;						}
			 	void		setName(std::string _name)	{ name = _name;						}
		const 	int	 		getPathlen()				{ return path_len;					}
		 		void	  	setPathlen(int _len)		{ path_len = _len;					}
				void		insertFI(NODE* _node)		{ FI_Ary.push_back(_node);			}
				void		eraseFI(NODE* _node);
				void		insertFO(NODE* _node)		{ FO_Ary.push_back(_node);			}
				void		eraseFO(NODE* _node);
		const	int		 	FIfind(NODE* _node);		//return index
		const	int		 	FOfind(NODE* _node);		//return index
		const 	int			getAndC()					{ return and_counter;				}
		const 	int			getOrC()					{ return or_counter;				}
				void		setAndC(int _num)			{ and_counter = _num; 				}
				void		setOrC(int _num)			{ or_counter = _num; 				}
		const 	int			getId()						{ return id;						}
				void		setId(int _num)				{ id = _num; 						}
		std::vector<NODE*>& getFI()						{ return FI_Ary;					}	
		std::vector<NODE*>& getFO()						{ return FO_Ary;					}	
		const	bool		isEncryption()				{ return enc;						}	
				void		setEncryption(bool _enc)	{ enc = _enc;						}
				void 		clearFI()					{ FI_Ary.clear();					}
				void 		clearFO()					{ FO_Ary.clear();					}
		const	std::string stringFType();
				NODE*		getEncNode()				{ return enc_node;					}
				void 		setEncNode(NODE* _node)		{ enc_node = _node;					}
				void		setCO (int _num)			{ CO  = _num; 						}
		const	int 		getCO ()					{ return CO; 						} 
				bool 		path3length(NODE*, int);
		// 109062233 add for stuck at fault 
		const	int			getCurrentOutput()			{ return current_output;			}
				void		setCurrentOutput(int _b)	{ current_output = _b;				}
				void		setStuckFaultValue(bool _b)	{ stuck_fault_value = _b;			}
		const	bool		getStuckFaultValue()		{ return stuck_fault_value;			}
		const	bool		is_StuckFaulting()			{ return is_stuck_faulting;			}
				void		setStuckFaulting(bool _b)	{ is_stuck_faulting = _b;			}
				// 109062233 add for stuck at fault
		int					NoP0; // detect the stuck at 0 fault
		int					NoO0; // affect bits for stuck at 0 fault	
		int					NoP1; // detect the stuck at 1 fault
		int					NoO1; // affect bits for stuck at 1 fault
		unsigned long long int	fault_impact;
	private:
		Type				t;
		FType				ft;
		std::vector<NODE*>	FI_Ary;
		std::vector<NODE*>	FO_Ary;
		std::string			name;
		int		 	 		path_len;
		int					and_counter; 
		int					or_counter;
		int 				id;
		bool				enc;
		NODE*				enc_node;
		int				current_output;
		bool 				stuck_fault_value;
		bool				is_stuck_faulting;
		int 	  			CO;				//observability 	
		int					depth;
};

bool compareCO(NODE* _node1, NODE *_node2);

#endif

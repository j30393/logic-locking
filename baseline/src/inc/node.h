#pragma once

#ifndef _NODE_H
#define _NODE_H

#include<string>
#include<iostream>
#include<vector>
#include<cstdint>
#include<set>

#define debug

#define cost(arg1,arg2) ({ \
	int flag = 0; \
	if(arg1 == FType::XNOR || arg1 == FType::XOR) {\
		flag = 3; \
	} else if(arg1 == FType::NOT || arg1 == FType::BUF){\
		flag = 1; \
	} else { \
		flag = (int)arg2-1;\
	}\
	flag; \
})

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
			:CC0(0), CC1(0), CO(0), enc(false), t(Type::Intl), ft(FType::BUF), name(""), path_len(0), and_counter(0), or_counter(0), start(0), end(0), id(0){
		}



		NODE(Type _t, FType _ft, std::string _name)
			:CC0(0), CC1(0), CO(0), t(_t), ft(_ft), name(_name), path_len(0), and_counter(0), or_counter(0), start(0), end(0), id(0), enc(false){
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

		
		//operator
		const 	int 		getCost()					{ return cost(ft, FI_Ary.size());	}
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
		const 	int			getStart()					{ return start;						}
		const 	int			getEnd()					{ return end;						}
				void		setStart(int _num)			{ start = _num; 					}
				void		setEnd(int _num)			{ end = _num; 						}
		const 	int			getId()						{ return id;						}
				void		setId(int _num)				{ id = _num; 						}
		std::vector<NODE*>& getFI()						{ return FI_Ary;					}	
		std::vector<NODE*>& getFO()						{ return FO_Ary;					}	
		const	bool		isEncryption()				{ return enc;						}	
				void		setEncryption(bool _enc)	{ enc = _enc;						}
				void 		clearFI()					{ FI_Ary.clear();					}
				void 		clearFO()					{ FO_Ary.clear();					}
		const	std::string stringFType();
		const 	FType 		getEncType()				{ return enc_type;					}
				void 		setEncType(FType _ft)		{ enc_type = _ft;					}
				NODE*		getEncNode()				{ return enc_node;					}
				void 		setEncNode(NODE* _node)		{ enc_node = _node;					}
				void		setCC0(int _num)			{ CC0 = _num; 						}
				void		setCC1(int _num)			{ CC1 = _num; 						}
				void		setCO (int _num)			{ CO  = _num; 						}
		const	int 		getCC0()					{ return CC0; 						} 
		const	int 		getCC1()					{ return CC1; 						} 
		const	int 		getCO ()					{ return CO; 						} 
				bool 		path3length(NODE*, int);
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
		int					start;
		int					end;
		bool				enc;
		FType				enc_type;
		NODE*				enc_node;
		int  				CC0;			//controllability 0
		int  				CC1;			//controllability 1
		int 	  			CO;				//observability 	
};

bool compareCO(NODE* _node1, NODE *_node2);

#endif

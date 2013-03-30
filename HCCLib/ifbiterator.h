#ifndef __CORE_BIDIRECTIONAL_STREAMBUFFER_ITERATOR_h__
#define __CORE_BIDIRECTIONAL_STREAMBUFFER_ITERATOR_h__
/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <ifbiterator.h>																   *	
*																									   *
*	CLASS-TYPE		: template<class _E, class _Tr = char_traits<_E> >								   *	
*						class bidir_istreambuf_iterator                                                *
*																									   *
*																									   *
*	DESCRIPTION		: HCC Compiler utility classes (lexer tool for reading the source units)		   *
*					  This is an implementation of a stream iterator with back capabilities.		   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/

#include <iosfwd>
#include <stack>

using namespace std;

		// TEMPLATE CLASS bidir_istreambuf_iterator (modified version of istreambuf_iterator class from <iterator>)
template<class _E, class _Tr = char_traits<_E> >
	class bidir_istreambuf_iterator
		: public iterator<bidirectional_iterator_tag, _E, typename _Tr::off_type> {
public:
	typedef bidir_istreambuf_iterator<_E, _Tr> _Myt;
	typedef _E char_type;
	typedef _Tr traits_type;
	typedef typename _Tr::int_type int_type;
	typedef basic_streambuf<_E, _Tr> streambuf_type;
	typedef basic_istream<_E, _Tr> istream_type;
	bidir_istreambuf_iterator(streambuf_type *_Sb = 0) _THROW0()
		: _Sbuf(_Sb), _Got(_Sb == 0), _Bof(true), _inPos(0), _TabSize(8) {}

	bidir_istreambuf_iterator(istream_type& _I) _THROW0()
		: _Sbuf(_I.rdbuf()), _Got(_I.rdbuf() == 0), _Bof(true), _inPos(0), _TabSize(8) {}
	//used for temporary only (ie.: *_F++)
	bidir_istreambuf_iterator(const bidir_istreambuf_iterator<_E, _Tr>& _BIt) _THROW0() 
		: _Sbuf(_BIt._Sbuf), _Val(_BIt._Val), _Got(true), _Bof(_BIt._Bof), _inPos(_BIt._inPos), _TabSize(8) {}
	const _E& operator*() const
		{if(!_Got)
			((_Myt*)(this))->_Peek();
		return (_Val);}
	const _E *operator->() const
		{return (&**this); }
	_Myt& operator++()//prefix
	{	if(_backwrd.empty()){			
			if (!_Got)
				_Peek();
			_Inc();
		}
		else{
			_Val = _backwrd.top();
			_backwrd.pop();		
			_forward.push(_Val);			
			if(!_backwrd.empty()){
				_Val = _backwrd.top();
				_IncPos();
			}
			else
				_Inc();			
			_Got = true;
			}
		return (*this);}
	_Myt operator++(int)//postfix
		{if(_backwrd.empty()){
			if (!_Got)
				_Peek();			
			_Myt _Tmp = *this;
			_Inc();
			return (_Tmp);
		}
		else{
			_Val = _backwrd.top();
			_Myt _Tmp = *this;
			_backwrd.pop();		
			_forward.push(_Val);			
			if(!_backwrd.empty()){								
				_Val = _backwrd.top();				
				_IncPos();
			}else
				_Inc();
			_Got = true;
			return (_Tmp);
		}
	}	
	_Myt& operator--()//prefix
		{if(!_Bof){
			_Dec();
			_Bof = _forward.empty();
			if(_forward.size()==1){
				_Val = _forward.top();
				_forward.pop();
				_backwrd.push(_Val);
				_inPos = 1;
				_Bof = true;				
			}
		}
		return (*this);}
	_Myt operator--(int)//postfix
		{_Myt _Tmp = *this;
		_Dec();
		if((_Bof = _forward.empty()))
			_inPos = 1;
		return (_Tmp);}	
	bool equal(const _Myt& _X) const
		{if (!_Got)
			((_Myt *)this)->_Peek();
		if (!_X._Got)
			((_Myt *)&_X)->_Peek();
		return (_Sbuf == 0 && _X._Sbuf == 0
			|| _Sbuf != 0 && _X._Sbuf != 0); }	
	long inputpos() const
		{return _inPos;}
	bool is_bof() const
		{return _Bof;}
private:
	void _Inc()
		{if (_Sbuf == 0
			|| _Tr::eq_int_type(_Tr::eof(), _Sbuf->sbumpc())){
			_Sbuf = 0, _Got = true;			
		}
		else{
			_E _Now = _Peek();
			if(_Now!=0){
				_forward.push(_Now);
				_IncPos();
				
			}_Got = true;}
		}
	void _Dec()
	{if(!_forward.empty()){			
			_Val = _forward.top();			
			_backwrd.push(_Val);
			_forward.pop();		
			_DecPos();
			if(!_forward.empty()){				
				_Val = _forward.top();				
			}
		}else _Got = true;}
	_E _Peek()
		{bool _Init = (_backwrd.empty() && _forward.empty());
			{int_type _C;
				if (_Sbuf == 0
					|| _Tr::eq_int_type(_Tr::eof(), _C = _Sbuf->sgetc())){
					_Sbuf = 0;_Val = (_E)(0);}
				else{
					_Val = _Tr::to_char_type(_C);
					if(_Init){
						_IncPos();
						_forward.push(_Val);						
					}
				}
			}
		_Got = true;
		return (_Val);}
	void _IncPos()
	{_inPos++;_Bof = false;
		if(_Val=='\t' || _Val==L'\t')
			_inPos += _TabSize - _inPos%_TabSize;}
	void _DecPos()
	{_inPos--;if(_Val=='\t' || _Val==L'\t')
			_inPos -= _TabSize - _inPos%_TabSize;}
private:
	streambuf_type *_Sbuf;
	bool _Got;
	bool _Bof;
	_E _Val;
	stack<_E> _backwrd;
	stack<_E> _forward;
	long _inPos;
	char _TabSize;

	};
template<class _E, class _Tr> inline
	bool __cdecl operator==(const bidir_istreambuf_iterator<_E, _Tr>& _X,
		const bidir_istreambuf_iterator<_E, _Tr>& _Y)
	{return (_X.equal(_Y)); }
template<class _E, class _Tr> inline
	bool __cdecl operator!=(const bidir_istreambuf_iterator<_E, _Tr>& _X,
		const bidir_istreambuf_iterator<_E, _Tr>& _Y)
	{return (!(_X == _Y)); }
/*
 * Copyright (c) 1995 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Copyright (c) 1994
 * Hewlett-Packard Company
 */

#endif //__CORE_BIDIRECTIONAL_STREAMBUFFER_ITERATOR_h__

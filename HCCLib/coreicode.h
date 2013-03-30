#ifndef __HCC_INTERMEDIATE_CODE_h__
#define __HCC_INTERMEDIATE_CODE_h__
/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <coreicode.h>																	   *	
*																									   *
*	DESCRIPTION		: HCC Compiler Intermediate Code Generator (icode_generator, TODO)				   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/

#include "coresymbols.h"
#include <set>

using namespace std;

class icode_generator
{	
	icode_generator& operator=(const icode_generator&);
	enum{ CODE_SEGMENT_MAX_SIZE = 65536 /*64kb of segment size per function*/};
	char *code_ptr;
	char *cursor;
	Symbol* symbol_ptr;
	int line_number;

	void check_bounds(__uint sz);
	Symbol* get_symbol();
	source_buffer* source_ptr;
	bool IsDataType(HCC_TOKEN_TYPE type);
public:
	icode_generator(unsigned int nCodeSegmentSize = CODE_SEGMENT_MAX_SIZE) : source_ptr(0), line_number(0)
		{cursor = code_ptr = new char[nCodeSegmentSize];}	
	~icode_generator()
		{delete code_ptr;}
	icode_generator(const icode_generator&);

	void append(HCC_TOKEN_TYPE type);
	void append(const Symbol* symbol);
	void insert_line_marker(void);
	HCC_TOKEN* getToken(void);

	void reset()
		{cursor=code_ptr;}
	void set_pos(int offset)
		{cursor = code_ptr + offset;}
	const int get_pos() const
		{return cursor - code_ptr;}	

	int currentLineNumber()
		{return line_number;}

	const HCC_TOKEN_TYPE get_last_token() const
		{if(cursor)
			return static_cast<HCC_TOKEN_TYPE>(*(((wchar_t*)cursor)-1));
		 else
			return HCC_TOKEN_ERROR;}

	bool put_back()
	{
		if(get_last_token()==HCC_IDENTIFIER || get_last_token()==HCC_SEMICOLON)
		{
			cursor -= sizeof(wchar_t);
			return true;
		}
		return false;
	}

	Symbol* symbol() const
	{return symbol_ptr;
	}
	void set_source_buffer(source_buffer* _source_ptr = 0)
		{source_ptr = _source_ptr;}
private:
	bool IsStorageSpec(HCC_TOKEN_TYPE type);
};

#endif //__HCC_INTERMEDIATE_CODE_h__
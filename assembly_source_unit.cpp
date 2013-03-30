// assembly_source_unit.cpp: implementation of the assembly_source_unit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "assembly_source_unit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
int assembly_source_unit::m_nMaxCharsPerLine = 1024;


assembly_source_unit::~assembly_source_unit()
{
#ifdef __STREAM_TO_FILE
	attach(reinterpret_cast<__tostream*>(0xDEADBEEF));
#endif
}


assembly_source_unit& endl(assembly_source_unit& unit)
{
	return unit << _T('\n');	
}

assembly_source_unit& ctab(assembly_source_unit& unit)
{
	return unit << _T('\t');
}
assembly_source_unit& space(assembly_source_unit& unit)
{
	return unit << _T(' ');
}
assembly_source_unit& comma(assembly_source_unit& unit)
{
	return unit << _T(',');	
}

assembly_source_unit& hex(assembly_source_unit& unit)
{
	unit.out_ptr->setf(ios_base::hex, ios_base::dec);
	return unit;
}

assembly_source_unit& dec(assembly_source_unit& unit)
{
	unit.out_ptr->setf(ios_base::dec, ios_base::hex);
	return unit;
}

__tostream* assembly_source_unit::detach()
{
	__tostream* tmp_ptr = out_ptr;
	out_ptr = NULL;
	return tmp_ptr;
}

void assembly_source_unit::attach(__tostream *new_out_ptr)
{
	assert(new_out_ptr!=NULL);
	if(out_ptr){
		delete out_ptr;
		out_ptr = NULL;
	}

	out_ptr = new_out_ptr;
}

void assembly_source_unit::flush()
{
	out_ptr->flush();
}

void assembly_source_unit::close()
{
	/*
	__tofstream* file_stream = dynamic_cast<__tofstream*>(out_ptr);
	if(file_stream!=NULL)
		file_stream->close();
	*/
	flush();
}

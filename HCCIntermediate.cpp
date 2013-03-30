// HCCIntermediate.cpp: implementation of the HCCIntermediate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HCCIntermediate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//the map that control all the stack frames icode objects for every function in every class...
map<Symbol*, icode_generator*> map_all_function_icodes;
//the map that control all the class icode objects for every class...
map<TypeSpecifier*, icode_generator*> map_all_class_icodes;


ICodeResponsible::~ICodeResponsible()
{
	map<Symbol*, icode_generator*>::iterator it_icode = map_all_function_icodes.begin();
	while(it_icode!= map_all_function_icodes.end())
	{
		icode_generator* the_icode_ptr = it_icode->second;
		delete the_icode_ptr;
		it_icode++;
	}

	map<TypeSpecifier*, icode_generator*>::iterator it_cls_icode = map_all_class_icodes.begin();
	while(it_cls_icode!= map_all_class_icodes.end())
	{
		icode_generator* the_icode_ptr = it_cls_icode->second;
		delete the_icode_ptr;
		it_cls_icode++;
	}
}

icode_generator* ICodeResponsible::getFunctionICode(Symbol* function_ptr, bool bCreate)
{
	map<Symbol*, icode_generator*>::iterator it_icode = map_all_function_icodes.find(function_ptr);
	if(it_icode!=map_all_function_icodes.end())
		return it_icode->second;
	if(bCreate)
	{
		pair<map<Symbol*, icode_generator*>::iterator,
			bool> inserted = map_all_function_icodes.insert(pair<Symbol*, icode_generator*>(function_ptr, NULL));
		if(inserted.second)
			return (inserted.first->second = new icode_generator());
	}
	return 0;
}

icode_generator* ICodeResponsible::getClassICode(TypeSpecifier* class_type_ptr, bool bCreate)
{
	map<TypeSpecifier*, icode_generator*>::iterator it_icode = map_all_class_icodes.find(class_type_ptr);
	if(it_icode!=map_all_class_icodes.end())
		return it_icode->second;
	if(bCreate)
	{
		pair<map<TypeSpecifier*, icode_generator*>::iterator, bool> inserted = 
				map_all_class_icodes.insert(pair<TypeSpecifier*, icode_generator*>(class_type_ptr, NULL));
		if(inserted.second)
			return (inserted.first->second = new icode_generator(8192));
	}
	return 0;
}


ICodeResponsible theICodeResponsible;
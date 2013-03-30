// HCCIntermediate.h: interface for the HCCIntermediate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HCCINTERMEDIATE_H__5D0AFC1E_62EE_4BCD_8DC7_09A52A3F32C0__INCLUDED_)
#define AFX_HCCINTERMEDIATE_H__5D0AFC1E_62EE_4BCD_8DC7_09A52A3F32C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HCCLib\coresymbols.h"
#include "HCCLib\coreicode.h"

class ICodeResponsible
{
	ICodeResponsible(const ICodeResponsible&);
	ICodeResponsible& operator=(const ICodeResponsible&);
public:
	~ICodeResponsible();
	ICodeResponsible(){}
	icode_generator* getFunctionICode(Symbol* function_ptr, bool bCreate = true);
	icode_generator* getClassICode(TypeSpecifier* class_type_ptr, bool bCreate = true);
};

#endif // !defined(AFX_HCCINTERMEDIATE_H__5D0AFC1E_62EE_4BCD_8DC7_09A52A3F32C0__INCLUDED_)

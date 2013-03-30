// HCCTools.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

using namespace std;

#include "..\HCCLib\corelisting.h"
#include "..\HCCLib\errors.h"

int main_test_bidir_iterator(int argc, char* argv[])
{
	__tistring_stream test(_T("hello \tworld \tmarzan!"));
	__tifstream_iterator _F(test), _L;

	TCHAR text[256];

//#define _TEST_POSTFIX_OPERATORS

#ifdef _TEST_POSTFIX_OPERATORS
	int x = 0;
	//forward

	text[x++] =  *_F;
	_F++;
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	text[x++] =  _T('\0');

	x = 0;
	//backward
	text[x++] =  *_F--;
	text[x++] =  *_F--;
	text[x++] =  *_F--;
	text[x++] =  *_F--;
	text[x++] =  *_F--;
	text[x++] =  *_F--;
	text[x++] =  _T('\0');

	x = 0;
	//forward again
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	//extra
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	text[x++] =  *_F++;
	text[x++] =  *_F++;

	text[x++] =  _T('\0');
	
	x = 0;
	//backward again
	text[x++] =  *_F--;
	text[x++] =  *_F--;
	text[x++] =  *_F--;
	text[x++] =  *_F--;
	text[x++] =  *_F--;
	text[x++] =  *_F--;	
	
	while(!_F.is_bof())
		text[x++] =  *_F--;	

	text[x++] =  _T('\0');	

	x = 0;
	while(_F!=_L)
		text[x++] = *_F++;

	text[x++] =  _T('\0');	
#else
	int x = 0;
	//forward

	text[x++] =  *_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  _T('\0');

	x = 0;
	//backward
	text[x++] =  *--_F;
	text[x++] =  *--_F;
	text[x++] =  *--_F;
	text[x++] =  *--_F;
	text[x++] =  *--_F;	
	text[x++] =  _T('\0');

	x = 0;
	//forward again
	text[x++] =  *_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	//extra
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;
	text[x++] =  *++_F;

	text[x++] =  _T('\0');
	
	x = 0;
	//backward again
	text[x++] =  *--_F;
	text[x++] =  *--_F;
	text[x++] =  *--_F;
	text[x++] =  *--_F;
	text[x++] =  *--_F;
	text[x++] =  *--_F;	
	
	while(!_F.is_bof())
		text[x++] =  *--_F;		

	text[x++] =  _T('\0');	

	x = 0;
	text[x++] =  *_F;
	while(_F!=_L)
		text[x++] = *++_F;

	text[x++] =  _T('\0');	
#endif 

	return 0;
}

int main(int argc, char* argv[])
{	
	//return main_test_bidir_iterator(argc, argv);
	if(argc!=2)
	{
		cerr << _T("Usage: list <source file>") << endl;
		HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
	}else{
		source_buffer reader(argv[1], true);
		source_buffer::iterator& _F = reader.begin();
		source_buffer::iterator _L = reader.end();
/*
		TCHAR text[20];
		
		int x = 0;
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 


		x = 0;
		text[x++] = *_F--; 
		text[x++] = *_F--; 
		text[x++] = *_F--; 
		text[x++] = *_F--; 
		text[x++] = *_F--; 
		text[x++] = *_F--; 


		x = 0;
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
		text[x++] = *_F++; 
*/


				
		while(_F!=_L){ 
			TCHAR chr = *_F++; 
			//cout << chr;
		}

		listing << _T("one ") << _T("two!");
	}
	return 0;
}

// HCC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <ctime>
#include <iomanip>

#include "HCCParser.h"
#include "HCCCodeGenerator.h"

#pragma warning(disable:4786)
#include <set>
#include <string>

#include <windows.h>
/*
#include <comdef.h>
#pragma comment(lib, "comsupp.lib")
*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif


using namespace std;

set<string> isearch_path;

#define CODE_WAS_ASSEMBLED 0
#define PE_EXE_FILE_LINKED 0

#define __COMPACT_HCC_SOURCE__
 #undef __COMPACT_HCC_SOURCE__

#define __HCC_TOKEN_DEBUG_OUTPUT__
#undef __HCC_TOKEN_DEBUG_OUTPUT__

#define __HCC_EXECUTE_STATEMENTS__
#undef __HCC_EXECUTE_STATEMENTS__

bool bShowListing					= false;
bool bWarningsDisabled				= false;
bool bPatternInitStackVariables		= false;
bool bPatternInitHeapVariables		= false;
bool bSourceAnnotation				= false;
bool bOptimizePrologAndEpilog		= false;
bool bOptimizeEpilogOnly			= false;
bool bForcesVirtualDestructors		= false;

int g_program_stack_size = 1024 * 1024; //1mb by default for every process thread

__tstring getDriverVersion(char** szLegalInfo);

char* lpData = NULL;
void ShowHccLogo()
{
	char* szLegalInfo = NULL;
	//The H++ compiler information...
	cout << _T("H++ Compiler (R) by Harold L. Marzan Version ") << getDriverVersion(&szLegalInfo) << _T(" (Beta) ")
		 << endl;
	if(szLegalInfo!=NULL){
		cout << szLegalInfo
		 << endl
		 << endl;
		//delete []szLegalInfo;
	}else{
		 cout << _T("Copyright    (C) Harold L. Marzan 2003-*. All rights reserved.")
		 << endl
		 << endl;
	}
	delete []lpData;
}

void ShowCompilerUsage()
{
	ShowHccLogo();
	cerr << _T("Usage: hcc <source file> [options]") 
		 << endl
		 << endl
		 << _T("Compiler Options:")
		 << endl
		 << _T("/x\t\t-display symbolic cross reference")			<< endl
		 << _T("/c\t\t-compile only, no link (default)")			<< endl
		 << _T("/cl\t\t-compile and Link")							<< endl
		 << _T("/nologo\t\t-suppress copyright message")			<< endl
		 << _T("/w\t\t-disable all warnings")						<< endl
		 << _T("/I:[search-path]\t\t-add import search path for hpp sources")		<< endl
		 << endl
		 << _T("/Fa:[file]\t-name assembly listing file")			<< endl
		 << _T("/Fo:[file]\t-name object file")						<< endl
		 << _T("/Fe:[file]\t-name executable file")					<< endl
		 << _T("/GZ\t\t-use \'CCh\' pattern for initialization of stack variables")			<< endl
		 << _T("/GH\t\t-use \'CDh\' pattern for initialization of heap variables")			<< endl
		 << _T("/FS:[size]\t-specifies the total stack allocation in virtual memory for a function (1 MB default)")	<< endl
		 << _T("/Ox\t\t-specifies a size optimization for function prolog and epilog")		<< endl
		 << _T("/Oy\t\t-specifies a size optimization for function epilog only")			<< endl
		 << _T("/A\t\t-add source annotation to the generated assembly code")				<< endl
		 << _T("/Fvd\t\t-forces converting to virtual destructors when declared non-virtuals") << endl
		 << endl
		 << _T("/L[console|windows]\t-creates a Console|Windows application")				<< endl		 
		 << _T("/Vl\t\t-verbose output while linking. Use with /cl option.")				<< endl
		 << _T("/Pdb:[file]\t\t-generate a Program Database for debugging symbols")			<< endl
		 << _T("/S\t\t-show H++ sources for every translation unit while compiling")		<< endl
		 << _T("/?\t\t-show this usage help")						<< endl
		 << endl;
}

int __cdecl main(int argc, char* argv[])
{
	if(argc < 2){
		ShowCompilerUsage();
		HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
	}else{
		//to check for memory leaks
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		/*
#ifdef __COMPACT_HCC_SOURCE__
		HccErrorManager::HideArrowOnErrors();
		bShowListing = false;
#else
		bShowListing = true;
#endif
		*/

		if(argv[1][1]=='?')
		{
			ShowCompilerUsage();
			exit(0);
		}

		HccErrorManager::HideArrowOnErrors();

		string __asm_file,
			   __obj_file,
			   __exe_file,
			   __sub_system = "CONSOLE";

		volatile bool bCompileOnly = true,
					  bSuppressLogo = false,
					  bXREFInfo = false;

		string linker_options;

		for(int i=2; i< argc; i++)
		{
			if(argv[i][0]=='/' || argv[i][0]=='-')
			{
				switch(argv[i][1])
				{
				case 'x':
						//to enable the printing of the XREF line numbers...		
						HCCParser::EnableCrossReference();
						bXREFInfo = true;
					break;
				case 'c':
					{
						//if false, we must compile the assembly file, and link it...
						bCompileOnly = (argv[i][2]!='l');						
					}
					break;
				case 'n':
					{
						bSuppressLogo = (strncmp(&argv[i][1], "nologo", 6)==0);
					}
					break;
				case 'w':
						bWarningsDisabled = true;
					break;
				case 'I':
					{
						//to resolve all import hpp sources, when not resolved from current directory

						//add a include/import search path...
						isearch_path.insert(&argv[i][3]);
					}
					break;
				case 'F':
					{
						switch(argv[i][2])
						{
						case 'a':	//assembly file name
							__asm_file = &argv[i][4];
							break;
						case 'o':	//object file name
							__obj_file = &argv[i][4];
							break;
						case 'e':
							__exe_file = &argv[i][4];
							break;
						case 'S':
							g_program_stack_size = atoi(&argv[i][4]);
							if(g_program_stack_size < 4096)
							{
								cerr << _T("Invalid stack size: \'") << &argv[i][3] << _T("\'.") 
									    _T("Use at least 4096 as the minimum. Default 1mb.") << endl;
								HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
							}else if(g_program_stack_size % 4096)
							{
								cerr << _T("Invalid stack size: \'") << &argv[i][3] << _T("\'.") 
									    _T("Use multiples of 4096 for the stack size. Default 1mb.") << endl;
								HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
							}
							break;
						case 'v':
							{
								if(argv[i][3]=='d')
									bForcesVirtualDestructors = true;
								else{
									cerr << _T("Invalid output option or unknown: \'") << &argv[i][0] << _T("\'.") << endl;
									HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
								}

							}
							break;
						default:
							{
								cerr << _T("Invalid output option or unknown: \'") << &argv[i][0] << _T("\'.") << endl;
								HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
							}
							break;
						}
					}
					break;
				case 'G':
					{
						if(argv[i][2]=='Z')
							bPatternInitStackVariables = true;
						else if(argv[i][2]=='H')
							bPatternInitHeapVariables = true;
						else
						{
							cerr << _T("Invalid option or unknown: \'") << &argv[i][0] << _T("\'.") << endl;
							HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
						}
						
					}
					break;
				case 'L':
					{
						if(strcmp(&argv[i][2], "console")==0)
							__sub_system = "CONSOLE";
						else if(strcmp(&argv[i][2], "windows")==0)
							__sub_system = "WINDOWS";
						else{
							cerr << _T("Invalid option or unknown: \'") << &argv[i][2] << _T("\'.") << endl;
							HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
						}
					}
					break;
				case 'A':
					{
						bSourceAnnotation = true;
					}
					break;
				case 'O':
					{
						if(argv[i][2]=='y')
						{
							bOptimizeEpilogOnly = true;
						}
						//else if(strcmp(&argv[i][2], "xy")==0)
						else if(argv[i][2]=='x')
						{
							bOptimizePrologAndEpilog = true;
						}
					}
					break;
				case 'S':
					{
						bShowListing = true;
						HccErrorManager::ShowArrowOnErrors();
					}
					break;
				case 'V':
					{
						if(argv[i][2]=='l')
							linker_options += "/verbose ";
					}
					break;
				case 'P':
					{
						if(_strnicmp(&argv[i][1], "Pdb", 3)==0){
							linker_options += "/PDB:";
							if(argv[i][4]==':' && strlen(&argv[i][5]) > 0)
							{
								linker_options += &argv[i][5];
								linker_options += ".pdb ";
							}else{
								linker_options += "NONE ";
							}
						}

					}
					break;
				case '?':
					{
						ShowCompilerUsage();
						exit(0);
					}
					break;
				default:
					{
						cerr << _T("Invalid option or unknown: \'") << &argv[i][0] << _T("\'.") << endl;
						HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
					}
					break;
				}
			}else{
				cerr << _T("Invalid switch specifier: \'") << &argv[i][0] << _T("\'.") << endl;
				HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
			}
		}

		if(false==bSuppressLogo){
			ShowHccLogo();
			if(false==bXREFInfo)
			{
				cout << _T("Compiling...") << endl;
			}
		}

		string first_unit = argv[1];
		//locate the dot before the file extension
		int ndot = first_unit.rfind('.');
		if(ndot!=string::npos)
		{
			string ext = first_unit.substr(ndot);
			if(ext!=".hpp" && ext!=".hcc")
			{				
				cerr << _T("error: Source H++ files must have \'.hpp\' | \'.hcc\' extensions;") 
					 << endl
					 << _T("your file: \'")
					 << argv[1] << _T("\'")
					 << endl;
				HccErrorManager::AbortTranslation(HccErrorManager::abortInvalidCommandLineArgs);
			}
		}

		//if no assembly output file was specified, then assign a compiler provided one...
		if(__asm_file.length()==0)
		{
			/*this impl. takes just the filename from the source filepath;
			int nslash = first_unit.rfind('\\');
			if(nslash==string::npos)
				//try with a slash
				nslash = first_unit.rfind('/');
			nslash++;
			if(ndot!=string::npos)
				__asm_file = first_unit.substr(nslash, ndot-nslash);
			else
				__asm_file = first_unit.substr(nslash);
			*/

			//this impl. takes the file-path and filename from the spec: file-path\filename.ext
			//and adds the asm extension.
			if(ndot!=string::npos)
				__asm_file = first_unit.substr(0, ndot);
			else
				__asm_file = first_unit;

			//add the file extension...
			__asm_file += ".asm";
		}

		
		source_buffer* unit = new source_buffer(argv[1], bShowListing);
		HCCParser parser(unit);		
		clock_t start = clock();
#ifdef __COMPACT_HCC_SOURCE__
		parser.CompactUnit();
#else
		parser.Parse();
		//This member call is intended for testing the Compiler's Lexer/Scanner.
		//parser.TestLexer();

		if(HCCParser::getMainEntryPoint()==NULL)
		{
			HccErrorManager::Error(HccErrorManager::errMissingMain, _T(" Compilation aborted.\n"));
			HccErrorManager::AbortTranslation(HccErrorManager::abortEntryPointNotFound);
		}else
		{
			if(HccErrorManager::globalErrorCount()==0)			
			{
				//for now, we are dealing with just one input file; but in a near future,
				//we'll support multiple file specification...
				//by the way, when I'm talking about us, I refer as a humble, to me (for the surprised one!).
				//				
				//for now we depend on switch /Fa[file.asm]
				assert(__asm_file.length() > 0);
				//1. generate code...
				HCCCodeGenerator code_gen(__asm_file.c_str());
				code_gen.setHppSourceFile(argv[1]);

				cout << _T("Generating Code...") << endl;
				if(code_gen.Generate(parser.getMainEntryPoint()))
				{
					//2. Invoke the assembly compiler...
					string ml_cmd = "ML /nologo -Zi -c -Fl -Sg -coff /Cx ";
					ml_cmd += __asm_file;
					ml_cmd += " stdhpp\\hcclib32.asm";
					//
					int nMASMExitCode = system(ml_cmd.c_str());
					//
					if(CODE_WAS_ASSEMBLED==nMASMExitCode && false==bCompileOnly)
					{
						//this impl. takes the file-path and filename from the spec: file-path\filename.ext
						//and adds the exe extension.
						if(__exe_file.length()==0)
						{
							if(ndot!=string::npos)
								__exe_file = first_unit.substr(0, ndot);
							else
								__exe_file = first_unit;

							__exe_file += _T(".exe");
						}else if(__exe_file.rfind(".exe")==string::npos)
							__exe_file += _T(".exe");

						int npos = __asm_file.rfind('.');
						assert(npos!=string::npos);
						if(npos!=string::npos)
						{
							__obj_file = __asm_file.substr(0, npos);
							__obj_file += ".obj";
							__obj_file += " hcclib32.obj";
							//
							char linker_cmd[1024];
							sprintf(linker_cmd, "LINK32 %s kernel32.lib user32.lib shell32.lib /nologo /SUBSYSTEM:%s /DEBUG /MAP %s /COMMENT:\"H++ Compiler by Harold Marzan\"", 
									__obj_file.c_str(),
									__sub_system.c_str(),
									(linker_options.length() > 0 ? linker_options.c_str() : ""));						
							//3. Invoke the linker...
							cout << _T("Linking...") << endl;
							int nLINKERExitCode = system(linker_cmd);
							//
							if(PE_EXE_FILE_LINKED==nLINKERExitCode)
							{
								cout << __exe_file << _T(" - ") 
												   << HccErrorManager::globalErrorCount()
												   << _T(" error(s), ")
												   << HccWarningManager::getWarningsCount() << _T(" warning(s).") 
												   << endl;
							}
						}
					}
				}
			}else{
				cerr << endl 
					 << _T("Summary: ") 
					 << endl
					 << HccErrorManager::globalErrorCount() << _T(" error(s).") 
					 << endl
					 << HccWarningManager::getWarningsCount() << _T(" warning(s).")
					 << endl
					 << endl;
			}
		}		
#endif
		
		delete unit;
		clock_t end = clock();
		double seconds = (double)(end - start) / CLOCKS_PER_SEC;
		cout << endl
			 << _T("Compilation time: ") 
			 << setprecision(2)
			 << seconds << _T(" secs.") << endl;
	}
	return 0;
}


__tstring getDriverVersion(char** szLegalInfo)
{

	__tstring version = _T("(unknown)");
	DWORD dwHandle = -1;
	TCHAR lpszFileName[MAX_PATH];
	DWORD dwLen = ::GetModuleFileName(::GetModuleHandle(NULL), lpszFileName, MAX_PATH);
	lpszFileName[dwLen] = _T('\0');

	dwLen = GetFileVersionInfoSize(lpszFileName, &dwHandle);
 
	if(dwLen > 0)
	{
		lpData = new char[dwLen];
		if(GetFileVersionInfo(lpszFileName, dwHandle, dwLen, lpData))
		{
			UINT uiLen = 0;

			struct LANGANDCODEPAGE {
			 WORD wLanguage;
			 WORD wCodePage;
			} *lpTranslate;


			if(VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), (void**)&lpTranslate, &uiLen))
			{ 
				VS_FIXEDFILEINFO *verInfo = NULL;    

				if(VerQueryValue(lpData, _T("\\"), (void**)&verInfo, &uiLen))
				{
				   TCHAR lpszVersion[32];
				   _stprintf(lpszVersion, _T("%d.%d.%d.%d"),
							(short)(verInfo->dwFileVersionMS >> 0x10),
							(short)verInfo->dwFileVersionMS,
				   			(short)(verInfo->dwFileVersionLS >> 0x10),
							(short)verInfo->dwFileVersionLS);

				   version = lpszVersion;

					//Query for Legal Copyright...
				   if(szLegalInfo!=NULL)
				   {
					   UINT uiLen = MAX_PATH + 1;
					   char szFmtSubBlock[64];
					   sprintf(szFmtSubBlock, "\\StringFileInfo\\%04x%04x\\LegalCopyright", 
										lpTranslate[0].wLanguage,
										lpTranslate[0].wCodePage);

						char *szLegalCopyData = NULL;
						if(VerQueryValue(lpData, szFmtSubBlock, (void**)&szLegalCopyData, &uiLen))
						{
							szLegalCopyData[uiLen] = 0;
							*szLegalInfo = szLegalCopyData;
							
						}
				   }

				  /*
				 long lVersion = (short)verInfo->dwFileVersionLS;

				 lVersion |= ((char)(verInfo->dwFileVersionMS)) << 0x10;
				 lVersion |= ((char)(verInfo->dwFileVersionMS >> 0x10)) << 0x18;
				 */
				}
			}
		}
	}

	return version;
}
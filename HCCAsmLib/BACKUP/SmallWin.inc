.NOLIST
; Include file for Calling Windows API Functions      (SmallWin.inc)

; Recent updates:
;  10/25/2002 (added HeapFree and GetProcessHeap)
;  11/10/2002: Added GetLastError
;  11/13/2002: Added SetLocalTime and GetKeyState; changed all "lp..." parameter names to "p..."
;  11/19/2002: Corrected the CONSOLE_CURSOR_INFO structure
;  12/11/2002: Changed names of procedure parameters relating to input and output handles,
;              to make them more consistent.

; To enable columns to line up properly, set your text editor's Tab
; settings to 5, 35, and 40.

;.386P
;.MODEL flat, stdcall
;.STACK 1024 * 1024

; Win32 Console handles
STD_INPUT_HANDLE EQU -10
STD_OUTPUT_HANDLE EQU -11		; predefined Win API constant

;  Input Mode flags (used by GetConsoleMode & SetConsoleMode):
ENABLE_PROCESSED_INPUT = 1
ENABLE_LINE_INPUT      = 2
ENABLE_ECHO_INPUT      = 4
ENABLE_WINDOW_INPUT    = 8
ENABLE_MOUSE_INPUT     = 16

; Event constants
KEY_EVENT       =  1
MOUSE_EVENT     =  2
WINDOW_BUFFER_SIZE_EVENT = 4          ; window change event record
MENU_EVENT      = 8                   ; menu event record
FOCUS_EVENT     = 16                  ; focus change

;  Output mode flags  (used by GetConsoleMode & SetConsoleMode):
ENABLE_PROCESSED_OUTPUT    = 1
ENABLE_WRAP_AT_EOL_OUTPUT  = 2

; Constants found in WINNT.H
FILE_SHARE_READ                 = 1
FILE_SHARE_WRITE                = 2
FILE_SHARE_DELETE               = 4

FILE_ATTRIBUTE_READONLY         = 1
FILE_ATTRIBUTE_HIDDEN           = 2
FILE_ATTRIBUTE_SYSTEM           = 4
FILE_ATTRIBUTE_DIRECTORY        = 10h
FILE_ATTRIBUTE_ARCHIVE          = 20h
FILE_ATTRIBUTE_DEVICE           = 40h
FILE_ATTRIBUTE_NORMAL           = 80h
FILE_ATTRIBUTE_TEMPORARY        = 100h
FILE_ATTRIBUTE_SPARSE_FILE      = 200h
FILE_ATTRIBUTE_REPARSE_POINT    = 400h
FILE_ATTRIBUTE_COMPRESSED       = 800h
FILE_ATTRIBUTE_OFFLINE          =    1000h
FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 2000h
FILE_ATTRIBUTE_ENCRYPTED           = 4000h

FILE_READ_DATA = 1
FILE_WRITE_DATA = 2
FILE_APPEND_DATA = 4
FILE_DELETE_CHILD = 40h

GENERIC_READ     = 80000000h
GENERIC_WRITE    = 40000000h
GENERIC_EXECUTE  = 20000000h
GENERIC_ALL      = 10000000h

; Memory allocation constants (added 10/13/2002)
HEAP_NO_SERIALIZE              = 00000001h
HEAP_GROWABLE                  = 00000002h
HEAP_GENERATE_EXCEPTIONS       = 00000004h
HEAP_ZERO_MEMORY               = 00000008h
HEAP_REALLOC_IN_PLACE_ONLY     = 00000010h

;------------------ From winbase.h ----------------------------
CREATE_NEW         = 1
CREATE_ALWAYS      = 2
OPEN_EXISTING      = 3
OPEN_ALWAYS        = 4
TRUNCATE_EXISTING  = 5

FILE_BEGIN         =  0
FILE_CURRENT       =  1
FILE_END           =  2

INVALID_HANDLE_VALUE = -1

exit EQU <INVOKE ExitProcess,0>		; exit program

;======================================================
;                     ALIASES
;======================================================
; The following Win32 API functions have an
; extra "A" at the end of their name, so they are
; redefined here with text macros:

CreateFile EQU <CreateFileA>
GetCommandLine EQU <GetCommandLineA>
PeekConsoleInput EQU <PeekConsoleInputA>
ReadConsole EQU <ReadConsoleA>
ReadConsoleInput EQU <ReadConsoleInputA>
SetConsoleTitle EQU <SetConsoleTitleA>
WriteConsole EQU <WriteConsoleA>
WriteConsoleOutputCharacter EQU <WriteConsoleOutputCharacterA>

;=======================================================
; Standard Windows Structure defintions:
;=======================================================

COORD STRUCT
	X WORD ?
	Y WORD ?
COORD ENDS

SYSTEMTIME STRUCT
	wYear WORD ?
	wMonth WORD ?
	wDayOfWeek WORD ?
	wDay WORD ?
	wHour WORD ?
	wMinute WORD ?
	wSecond WORD ?
	wMilliseconds WORD ?
SYSTEMTIME ENDS

; Date and time, measured in 100ns intervals
; since January 1, 1601.
FILETIME STRUCT
    loDateTime DWORD ?
    hiDateTime DWORD ?
FILETIME ENDS

SMALL_RECT STRUCT
  Left     WORD ?
  Top      WORD ?
  Right    WORD ?
  Bottom   WORD ?
SMALL_RECT ENDS

CONSOLE_CURSOR_INFO STRUCT
  dwSize   DWORD ?
  bVisible DWORD ?
CONSOLE_CURSOR_INFO ENDS

CONSOLE_SCREEN_BUFFER_INFO STRUCT
  dwSize           COORD <>
  dwCursorPos      COORD <>
  wAttributes      WORD ?
  srWindow         SMALL_RECT <>
  maxWinSize       COORD <>
CONSOLE_SCREEN_BUFFER_INFO ENDS

COMMENT !  (not used at present)
INPUT_RECORD STRUCT
	EventType WORD ?
	UNION Event
	  KEY_EVENT_RECORD <>
	  MOUSE_EVENT_RECORD <>
	  WINDOW_BUFFER_SIZE_RECORD <>
	  MENU_EVENT_RECORD <>
	  FOCUS_EVENT_RECORD <>
	ENDS
INPUT_RECORD ENDS
!

;==========================================================
;                   FUNCTION PROTOTYPES
;==========================================================

CloseHandle PROTO,		; close file handle
	handle:DWORD

CreateFile PROTO,		; create new file
	pFilename:PTR BYTE,		; ptr to filename
	desiredAccess:DWORD,		; access mode
	shareMode:DWORD,		; share mode
	pSecurity:DWORD,  		; pointer to security attributes
	creationDisposition:DWORD,		; file creation options
	flagsAndAttributes:DWORD,		; file attributes
	htemplate:DWORD		; handle to template file

ExitProcess PROTO,                  	; exit program
	dwExitCode:DWORD		; return code

FlushConsoleInputBuffer PROTO,		; flush the input buffer
	inHandle:DWORD		; standard input handle

GetCommandLine PROTO		; returns pointer to command-line string

GetConsoleCP PROTO		; returns code page ID in eax

GetConsoleCursorInfo PROTO,
	outHandle:DWORD,			; console output handle
	pCursorInfo:PTR CONSOLE_CURSOR_INFO	; cursor information

GetConsoleMode PROTO,
	ioHandle:DWORD,		; input or output handle
	pMode:PTR DWORD		; pointer to dword containing flags

GetConsoleScreenBufferInfo PROTO,
	outHandle:DWORD,		; handle to screen buffer
	pBufferInfo:PTR CONSOLE_SCREEN_BUFFER_INFO

GetKeyState PROTO, nVirtKey:DWORD		; get state of keyboard keys (see Ch11\keybd.asm)

GetLastError PROTO		; get most recent error return value

GetLocalTime PROTO,		; system time, adjusted for local time zone
	pSystemTime:PTR SYSTEMTIME		; ptr to SYSTEMTIME object

GetNumberOfConsoleInputEvents PROTO,	; get number of unread records
	inHandle:DWORD,		; input handle
	pCount:PTR DWORD		; pointer to counter

GetStdHandle PROTO,               	; get standard handle
	nStdHandle:DWORD  		; type of console handle

GetSystemTime PROTO,		; returns the system time
	pSystemTime:PTR SYSTEMTIME		; ptr to SYSTEMTIME object

GetTickCount PROTO		; get elapsed milliseconds
			; since computer turned on

GetProcessHeap PROTO   ; Get the current process heap handle

HeapAlloc PROTO,
    hHeap:DWORD,		; handle to private heap block
    dwFlags:DWORD,		; heap allocation control flags
    dwBytes:DWORD		; number of bytes to allocate
;
; The HeapAlloc function allocates a block of memory from a heap that has
; already been created by a call to HeapCreate. If the function succeeds,
; the return value is a pointer to the allocated memory block. If the function
; fails and you have not specified HEAP_GENERATE_EXCEPTIONS, the return
; value is NULL.

HeapCreate PROTO,
    flOptions:DWORD,		; heap allocation options
    dwInitialSize:DWORD,		; initial heap size, in bytes
    dwMaximumSize:DWORD		; maximum heap size, in bytes
;
; Creates a heap object that can be used by the calling process. It
; reserves space in the virtual address space of the process and allocates
; physical storage for a specified initial portion of this block.
; If the function succeeds, it returns a handle to the new heap in EAX.
; If it fails, the return value is NULL.

HeapDestroy PROTO,
    hHeap:DWORD		; pointer to the heap
;
; The HeapDestroy function destroys the specified heap object.
; HeapDestroy decommits and releases all the pages of a private heap
; object, and it invalidates the handle to the heap.
; If the function succeeds, the return value is nonzero. If the
; function fails, the return value is zero

HeapFree PROTO,
    hHeap:DWORD,
    dwFlags:DWORD,
    pMem:DWORD
;
; The HeapFree function ...

HeapSize PROTO,
    hHeap:DWORD,		; handle to heap
    dwFlags:DWORD,		; heap size options
    pMem:DWORD		; pointer to memory block
;
; The HeapSize function returns the size, in bytes, of a memory block
; allocated from a heap by the HeapAlloc or HeapReAlloc function.
; If the function succeeds, the return value is the size, in bytes,
; of the allocated memory block. If the function fails, the return
; value is -1.

PeekConsoleInput PROTO,
	inHandle:DWORD,		; input handle
	pBuffer:PTR BYTE,		; pointer to buffer
	nNumberOfBytesToRead:DWORD,    	; number of chars to read
	pNumEventsRead:PTR DWORD  		; ptr to number of events read

ReadConsole PROTO,
	inHandle:DWORD,                    ; input handle
	pBuffer:PTR BYTE,                	; pointer to buffer
	nNumberOfBytesToRead:DWORD,    	; number of chars to read
	pNumBytesRead:PTR DWORD,  		; ptr to number of bytes read
	pReserved:DWORD               	; (not used)

ReadConsoleInput PROTO,
	inHandle:DWORD,		; input handle
	pInputRec:PTR INPUT_RECORD,		; ptr to input record
	numRecs:DWORD,		; request number of recs
	pNumRead:PTR DWORD		; ptr to number of bytes read

ReadFile PROTO,		; read buffer from input file
	inHandle:DWORD,		; input handle
	pBuffer:PTR BYTE,		; ptr to buffer
	nBufsize:DWORD,		; num bytes to read
	pBytesRead:PTR DWORD,		; bytes actually read
	pOverlapped:PTR DWORD		; ptr to asynchronous info

SetConsoleCursorInfo PROTO,
	outHandle:DWORD,		 ; output handle
	pCursorInfo:PTR CONSOLE_CURSOR_INFO	; cursor information

SetConsoleCursorPosition PROTO,
	outHandle:DWORD,		; output handle
	coords:COORD		; screen X,Y coordinates

SetConsoleMode PROTO,
	outHandle:DWORD,		; output handle
	dwMode:DWORD		; console mode flags

SetConsoleScreenBufferSize PROTO,
	outHandle:DWORD,		; handle to screen buffer
	dwSize:COORD		; new screen buffer size

SetConsoleTextAttribute PROTO,
	outHandle:DWORD,		; output handle
	nColor:DWORD		; color attribute

SetConsoleTitle PROTO,		; set console window title
	pString:PTR BYTE		; points to string

SetConsoleWindowInfo PROTO,		; set position of console window
	outHandle:DWORD,		; screen buffer handle
	bAbsolute:DWORD,		; coordinate type
	pConsoleRect:PTR SMALL_RECT		; ptr to window rectangle

SetFilePointer PROTO,		; set the file pointer position
	fileHandle:DWORD,		; file handle
	nDistanceLo:SDWORD,		; bytes to move pointer
	pDistanceHi:PTR SDWORD,		; ptr to bytes to move, high
	moveMethod:DWORD		; starting point

SetLocalTime PROTO,		; set the system's local date and time
	pSystemTime:PTR SYSTEMTIME		; ptr to SYSTEMTIME structure

Sleep PROTO,	   	; sleeep for n milliseconds
	dwMilliseconds:DWORD

SystemTimeToFileTime PROTO,		; Convert a SYSTEMTIME structure
	pSystemTime:PTR SYSTEMTIME,		; to a FILETIME structure
	pFileTime:PTR FILETIME

WriteFile PROTO,		; write buffer to output file
	outHandle:DWORD,		; output handle
	pBuffer:PTR BYTE,		; pointer to buffer
	nBufsize:DWORD,		; size of buffer
	pBytesWritten:PTR DWORD,		; num bytes written
	pOverlapped:PTR DWORD		; ptr to asynchronous info

WriteConsole PROTO,		; write a buffer to the console
	outHandle:DWORD,		; output handle
	pBuffer:PTR BYTE,		; pointer to buffer
	bufsize:DWORD,		; size of buffer
	pCount:PTR DWORD,		; ptr to number of bytes written
	pReserved:DWORD		; (not used)

WriteConsoleOutputCharacter PROTO,
	outHandle:DWORD,		; console output handle
	pBuffer:PTR BYTE,		; pointer to buffer
	bufsize:DWORD,		; size of buffer
	xyPos:COORD,		; first cell coordinates
	pCount:PTR DWORD		; output count

WriteConsoleOutputAttribute PROTO,
	outHandle:DWORD,		; output handle
	pAttribute:PTR WORD,		; write attributes
	nLength:DWORD,		; number of cells
	xyCoord:COORD,		; first cell coordinates
	pCount:PTR DWORD		; number of cells written

.LIST
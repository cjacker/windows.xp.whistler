/****************************************************************************

    PROTOTYPES DECLARATION FOR UTIL MODULE

****************************************************************************/

#include <commdlg.h>

BOOL RegisterAndMapLocalHotKey(HWND, int, UINT, UINT);
void UnregisterLocalHotKey(int);

// Function building a pathname derived form the program running
void MakePathNameFromProg(LPSTR extension, LPSTR pathName);
void MakeFileNameFromProg(LPSTR extension, LPSTR fileName);

//Checks to see if a file exists with the path/filename
BOOL FileExist(LPSTR fileName);

//Checks to see if a file exists with the path/filename
//searching in the path if only filename and extension are provided
BOOL FileExistCheckingPath(LPSTR fileName);

//Check if file is Valid
BOOL PASCAL ValidFilename(
    LPSTR lpszName,
    BOOL fWildOkay);

// Opens a standard error Dialog Box
int CDECL ErrorBox(int wErrorFormat, ...);

// Opens a standard error Dialog Box (Parent is hwnd)
int CDECL ErrorBox2(
    HWND hwnd,
    UINT type,
    WORD wErrorFormat,
    ...);

// Opens an Internal error Dialog Box
int CDECL InternalErrorBox(WORD wDescript);

// Opens a standard information Dialog Box
void CDECL InformationBox(int wInfoFormat, ...);

// Opens a message box with the QCWin title
int PASCAL MsgBox(HWND hwndParent, LPSTR szText, UINT wType);

// Opens a message box with the QCWin title
int CDECL VarMsgBox(
    HWND hwndParent,
    WORD wFormat,
    UINT wType,
    ...);

// Opens a standard error Dialog Box and stop execution
void FatalErrorBox(
    WORD line1,
    LPSTR line2);

// Standard function to prompt an Assertion False
void ShowAssert(LPSTR condition, UINT line, LPSTR file);

// Loads and execute dialog box 'rcDlgNb' with 'dlgProc' function
BOOL StartDialog(int rcDlgNb, DLGPROC dlgProc);

// Invalidate the rectangle area of child 'ctlID' in 'hDlg' Dialog Box
void InvDlgCtlIdRect(HWND hDlg, WPARAM ctlID);

// Loads a resource string from resource file
void LoadResourceString(
    WORD wStrId,
    LPSTR lpszStrBuffer);

//Opens a standard question box containing combination
//of : Yes, No, Cancel
int CDECL QuestionBox(
    WORD wMsgFormat,
    UINT wType,
    ...);

//Opens a standard question box containing combination
//of : Yes, No, Cancel
int CDECL QuestionBox2(HWND hwnd, WORD wMsgFormat, UINT wType, ...);

//Gets the text associated with the passed environment variable,
//checking against any internally held environment strings first
BOOL PASCAL GetQCQPEnvStr(LPSTR lpszVar, LPSTR lpszDestEnv, WORD wDestSize);

// Scans a text string to a long value, checking limits
// and allowing 0, 0x, and $ representations
BOOL PASCAL fScanAnyLong(
    LPSTR lpszSource,
    WORD wLanguage,
    long *plDest,
    long lMin,
    long lMax);

//Gets sizes of the installed fonts
void GetFontSizes(
    HWND hWnd,
    int currentFont);

//Gets names of the installed fonts
void LoadFonts(
    HWND hWnd);

//Opens Cpu, Watch or Locals Window under MDI
int PASCAL OpenDebugWindow(int winType, LPWININFO lpWinInfo, int Preference);

int GotoLine(int view, int lineNbr, BOOL fDebugger);
BOOL    QueryLineStatus(int doc, int lineNbr, UINT state);

//Change the status of a given line
void LineStatus(
    int doc,
    int lineNbr,
    WORD state,
    LINESTATUSACTION action,
    BOOL positionInFirstView,
    BOOL redraw);

//Given a file Name, find corresponding doc
BOOL FindDoc(
    LPSTR fileName,
    int *doc,
    BOOL docOnly);

char *
GetFileName(
    char * szPath);

//Clear all visual info of a doc (Tags or BreakPoints or Compile Errors)
void ClearDocStatus(
    int doc,
    WORD state);

//Clear all visual info of all docs (Tags or BreakPoints or Compile Errors)
void ClearAllDocStatus(
    WORD state);

//Returns height (in lines) of passed list box
WORD VisibleListboxLines(
    HWND hListbox);

//Alloc and lock MOVABLE global memory.
LPSTR Xalloc(UINT bytes);

//Reallocate memory previously allocated through Xalloc
LPSTR Xrealloc(LPSTR curblock, UINT bytes);

//Free global memory.
BOOL Xfree(LPSTR lPtr);

//Hash code for tokens
void Convert(LPSTR tok);

//Adjust Full Path name to fit in a specified len string
void AdjustFullPathName(
    PSTR fullPath,
    PSTR adjustedPath,
    int len);

//Read ini file
BOOL ReadIni(int hFile, LPSTR lpBuffer, int wBytes);

//Write file and add to checksum
BOOL WriteAndSum(int hFile, LPSTR lpBuffer, int wBytes);

// Utility function
BOOL FAR PASCAL IsMonoModeProc (void) ;

// Load a DLL (qcqp.exe path tried first)
HANDLE PASCAL LoadQCQPLibrary(LPSTR LibName, BOOL DoErrorBox);

// WinExec a prog (qcqp.exe path tried first)
WORD PASCAL WinQCQPExec(LPSTR CmdLine, int nCmdShow, BOOL DoErrorBox);

//Try to lock a global memory handle
BOOL FailGlobalLock(
    HANDLE h,
    LPSTR *p);

//Retrieve in view the word at cursor, or the first line
//of text if selection is active
BOOL GetCurrentText(
    int view,
    BOOL *lookAround,
    LPSTR pText,
    int maxSize,
    LPINT xLeft,
    LPINT xRight);

//Checks if a string contains a DOS wildcard : "*" or "?"
BOOL PASCAL fIsWild(
    PSTR psz);

#define HUNDREDTHS_IN_A_DAY (((24UL*60)*60)*100)

//Returns current time in 100ths of a second
ULONG PASCAL TimeIn100ths(
    void);

//Returns current time + delay time in 100ths of a second
ULONG PASCAL StopTimeIn100ths(
    ULONG DelayIn100ths);

// Process a message received by QCQP
void PASCAL ProcessQCQPMessage(LPMSG lpMsg);

//Initialize files filters for dialog boxes using commonfile DLL
void InitFilterString(WORD id, LPSTR filter, int maxLen);

//Appends a '\' to a string if none found
void AppendBackslashIfNone(
    LPSTR path);

//Get rid of accelerator mark
void RemoveMnemonic(
    LPSTR sWith,
    LPSTR sWithout);

//Status bar operations
void CDECL  StatusText(int newTextId, WORD msgType, BOOL forceUpdate, ...);
void PASCAL StatusMultiKey(BOOL newValue);
void PASCAL StatusOvertype(BOOL newValue);
void PASCAL StatusReadOnly(BOOL newValue);
void PASCAL StatusNumLock(BOOL newValue);
void PASCAL StatusCapsLock(BOOL newValue);
void PASCAL StatusLineColumn(int newLine, int newColumn);
void PASCAL StatusSrc(BOOL newValue);
void PASCAL StatusPidTid(UINT ipid, UINT itid);

//Check if keyboard hit is NUMLOCK, CAPSLOCK or INSERT
LRESULT EXPORT KeyboardHook( int iCode, WPARAM wParam, LPARAM lParam );

//Returns the handle of one of the debug windows
HWND GetDebugWindowHandle(WORD type);

/****************************************************************************/
// Set this to 1 to include special code to allow automatic test
// suites to run.
#define TESTSUITES  1
#ifdef DEBUGGING
//Makes a printf style command for output on debug console

//#define REMOVEAUXPRINTFS  TRUE
#define REMOVEAUXPRINTFS    FALSE
#if REMOVEAUXPRINTFS
#define AuxPrintf(level, dummy) FALSE
#else
BOOL AuxPrintf(int iDebugLevel, LPSTR text, ...);
#endif //REMOVEAUXPRINTFS

//Swiss Made Chronograph
void ShowElapsedTime(
    void);
void StartTimer(
    void);

void StopTimer(
    void);

//Opens a Dialog box with a title and accepting a printf style for text
int InfoBox(
    LPSTR text,
    ...);

PSTR GetWindebugTxt(long lDbg);

#endif

/****************************************************************************

    MACROS:

****************************************************************************/
// Dbg have to be used for every assertion during Debugging time.
// If false Dbg Opens a fatal error message Box and Stops program


// First, a sanity check
#ifdef Dbg
#undef Dbg
#endif

#if DBG
#ifdef WIN32
#ifdef MIPS_C
#define Dbg(condition) \
{ \
    if (!(condition)) { \
    static char szFileName[] = __FILE__;\
    static char szExp[] = "condition"; \
    ShowAssert(szExp, __LINE__, szFileName); \
    }\
}
#else // ! MIPS_C
#define Dbg(condition)          \
    {                   \
        if (!(condition))           \
        {                   \
        static char szFileName[] = __FILE__;     \
        static char szExp[] = #condition;    \
        ShowAssert(szExp, __LINE__, szFileName); \
        }                   \
    }
#endif
#else // !WIN32
#define Dbg(condition)  \
    {                               \
        if (!(condition))    \
        {                    \
            static char _based(_segname("SZASSERT")) szFileName[] = __FILE__; \
            static char _based(_segname("SZASSERT")) szExp[] = #condition;  \
            ShowAssert(szExp,__LINE__,szFileName);  \
        }                           \
    }
#endif
#else // !DBG
#define Dbg(condition) condition
#endif

#ifdef NTBUG
#define DbgX(condition) Dbg(condition)
#else
#define DbgX(condition) condition
#endif

#ifdef ALIGN
#define AssertAligned(address)  Assert((((long)address) & 3) == 0)
#else
#define AssertAligned(address)
#endif


// Assert are assertions that will stay in final Release.
// If false Assert Opens a fatal error message Box and Stops program
#ifdef WIN32
#ifdef MIPS_C
#define RAssert(condition) \
{ \
    if (!(condition)) { \
        static char szFileName[] = __FILE__;\
        static char szExp[[] = "condition"; \
        ShowAssert(szExp, __LINE__, szFileName); \
    } \
}
#else  // MIPS_C
#define RAssert(condition)   \
    {                                   \
        if (!(condition))       \
        {                       \
            static char szFileName[] = __FILE__; \
            static char szExp[] = #condition;   \
            ShowAssert(szExp,__LINE__,szFileName);  \
        }   \
    }
#endif
#else  // WIN32
#define RAssert(condition)   \
    {                                   \
        if (!(condition))       \
        {                       \
            static char _based(_segname("SZASSERT")) szFileName[] = __FILE__; \
            static char _based(_segname("SZASSERT")) szExp[] = #condition;  \
            ShowAssert(szExp,__LINE__,szFileName);  \
        }   \
    }
#endif

#if DBG
#define DAssert(cnd) RAssert(cnd)
#define Assert(cnd)  RAssert(cnd)
#else
#define DAssert(cnd)
#define Assert(cnd)  RAssert(cnd)
#endif


// Bitwise Operations Macros

// These work on integral operands...
#define SET(i, mask)        ((i) |= (mask))
#define RESET(i, mask)      ((i) &= ~(mask))
#define TOGGLE(i, mask)     ((i) ^= (mask))

// These work on arrays of integral operands...
// bit_idx is the bit number to set in array a so
// SET_ARRAY(37, ucBuffer) does a SET(ucBuffer[4], 0x20), whereas
// SET_ARRAY[37, lBuffer) does a SET(lBuffer[1], 0x20L)
#define SHIFT_OPERAND(bit_idx, a)   ((sizeof(*(a)) > sizeof(int) ? 1L : 1) << (bit_idx%(sizeof(*(a))*8)))
#define SET_ARRAY(bit_idx, a)           SET(a[bit_idx/(sizeof(*(a))*8)], SHIFT_OPERAND(bit_idx, a))
#define RESET_ARRAY(bit_idx, a)     RESET(a[bit_idx/(sizeof(*(a))*8)], SHIFT_OPERAND(bit_idx, a))
#define TOGGLE_ARRAY(bit_idx, a)        TOGGLE(a[bit_idx/(sizeof(*(a))*8)], SHIFT_OPERAND(bit_idx, a))
#define TEST_ARRAY(bit_idx, a)      (a[bit_idx/(sizeof(*(a))*8)] & SHIFT_OPERAND(bit_idx, a))

//To test if a character belong to the C/Pascal Alphanumeric set
#define CHARINALPHASET(c) (IsCharAlphaNumeric(c) || c == '_')

void SetVerticalScrollBar(
    int view,
    BOOL propagate);

BOOL FAR PASCAL QueryCloseAllDocs(
    void);

BOOL FAR PASCAL StartFileDlg(HWND hwnd, int titleId, int defExtId,
    int helpId, int templateId, LPSTR fileName, DWORD *pFlags,
    LPOFNHOOKPROC lpfnHook);

//Start the Edit Project dialog box
void PASCAL StartEditProjDlg(
    HWND hParent);

//Update status bar
void UpdateStatus(
    WORD action,
    NPRECT pClientRect) ;

//Update ribbon
void UpdateRibbon(
    WORD action,
    NPRECT pClientRect);

void AddWindowMenuItem(
    int doc,
    int view);

void DeleteWindowMenuItem(
    int view);

void PASCAL OpenProject(
    PSTR ProjectName,
    HWND ParentWnd,
    BOOL TryForWorkspace);

void PASCAL EditProject(
    HWND ParentWnd);

void PASCAL CloseProject(
    void);

int FindWindowMenuId(
    WORD type,
    int viewLimit,
    BOOL sendDocMenuId);

BOOL DestroyView(
    int view);

#define ERC_COMPILE         0x0002
#define ERC_BUILD           0x0004
#define ERC_BREAKPT         0x0008
#define ERC_QWATCH          0x0010
#define ERC_TRACESTEP       0x0020
#define ERC_ALL             0xFFFF

BOOL IntOutOfRange(
    PSTR text,
    int far *val,
    int min,
    int max,
    HWND hDlg,
    int dlgItem,
    int errMsg);

BOOL LongOutOfRange(
    PSTR text,
    long far *val,
    long min,
    long max,
    HWND hDlg,
    int dlgItem,
    int errMsg);

int SetLanguage(int doc);

BOOL GetWordAtXY(
    int view,
    int x,
    int y,
    BOOL selection,
    BOOL *lookAround,
    BOOL includeRightSpace,
    LPSTR pWord,
    int maxSize,
    LPINT leftCol,
    LPINT rightCol);

void PASCAL EnsureScrollBars(
    int view,
    BOOL force);

void PASCAL NewFontInView(
    int view);

void PASCAL EscapeAmpersands(
    LPSTR AmpStr,
    int MaxLen);

void PASCAL UnescapeAmpersands(
    LPSTR AmpStr,
    int MaxLen);

void PASCAL BuildTitleBar(LPSTR TitleBarStr, UINT MaxLen);

void PASCAL UpdateTitleBar(
    TITLEBARMODE Mode,
    BOOL Repaint);

void PASCAL DlgEnsureTitleBar(void);

void PASCAL TimedUpdateTitleBar(TITLEBARMODE Mode, BOOL Repaint);

void FAR PASCAL FileNotSaved(int doc);

int FAR PASCAL ConvertPosX(int x);

void FAR PASCAL FlushKeyboard(void);

void PASCAL EnableRibbonControls(int Updates, BOOL LaunchingDebuggee);

BOOL FindLineStatus(int view, BYTE target, BOOL forward, int *line);

//ANSI/ASCII compatible isspace() test
#define whitespace(c) (c == ' ' || c == '\t' || c == '\n' || c == '\r')

BOOL    SetDriveAndDir(PSTR st);
VOID    GetBaseName( LPSTR , LPSTR  );
LPSTR   AllocateMultiString( DWORD );
BOOLEAN DeallocateMultiString( LPSTR );
BOOLEAN AddToMultiString( LPSTR *, DWORD *, LPSTR );
LPSTR   GetNextStringFromMultiString( LPSTR, DWORD, DWORD * );
BOOL    FindNameOn(LSZ, UINT, LSZ, LSZ);
void    Egg(void);
VOID    InvalidateAllWindows(VOID);

typedef struct
   {
   POINT from;
   POINT to;
   BOOL  bValid;
   }
LINEX;

typedef struct
   {
   HBITMAP CutMemoryBitMap;            /* Used to store cut and paste         */
   int CutHeight;                      /* current cut height                  */
   int CutWidth;                       /* current cut width                   */
   long CutFlag;                     /* flag to signal something in cut buffer*/
   }
CUTMAP;

class TMyFileWindow : public TFileWindow
   {
 public:

   NODE *args_list;
   HGLOBAL hEditHeap;
   TModule *pm;
//   HMENU hMenu;
   HFONT hEdtFont;

   TMyFileWindow(TWindow *, const LPSTR, const LPSTR, NODE *, TModule *, HGLOBAL);
   ~TMyFileWindow();

   bool CanClose();
   int EndEdit();

   void EvDestroy();
   LRESULT DefWindowProc(UINT msg, WPARAM wParam, LPARAM lParam);

   void CMExit();

   void CMTest();
   void CMSaveAndExit();
   void CMSaveToWorkspace();

   void CMEditSetFont();

   void CMHelp();
   void CMHelpEdit();
   void CMHelpEditTopic();

   virtual void SetupWindow();

   DECLARE_RESPONSE_TABLE(TMyFileWindow);
   }
;

class TMyEditboxWindow : public TEdit
   {
 public:

   TMyEditboxWindow(TWindow *, int, WORD);
   ~TMyEditboxWindow();

   void EvKeyDown(UINT, UINT, UINT);
   void EvRButtonUp(UINT, TPoint &);

   DECLARE_RESPONSE_TABLE(TMyEditboxWindow);
   }
;

class TMyListboxWindow : public TEdit
   {
 public:

   TMyListboxWindow(TWindow *, int, WORD, TModule *);
   ~TMyListboxWindow();

   void EvKeyDown(UINT, UINT, UINT);
   void EvLButtonDown(UINT modKeys, TPoint & point);
   void EvLButtonDblClk(UINT modKeys, TPoint & point);
   void EvRButtonUp(UINT, TPoint &);

   DECLARE_RESPONSE_TABLE(TMyListboxWindow);
   }
;

class TMyCommandWindow : public TDialog
   {
 public:

   TMyCommandWindow(TWindow *, LPSTR);
   ~TMyCommandWindow();

   void SetupWindow();
   void GetWindowClass(WNDCLASS & WndClass);
   char far *GetClassName();

   void EvDestroy();
   void EvSize(UINT, TSize &);
   void CmCancel()
      {
      }
   ;
   void CmOk()
      {
      }
   ;
   void EvClose()
      {
      }
   ;
   void DoListBox(UINT);
   void DoEditBox(UINT);
   void DoButtonExecute(UINT);
   void DoButtonEdall(UINT);
   void DoButtonHalt(UINT);
   void DoButtonStatus(UINT);
   void DoButtonYield(UINT);
   void DoButtonPause(UINT);
   void DoButtonTrace(UINT);
   void DoButtonReset(UINT);

   DECLARE_RESPONSE_TABLE(TMyCommandWindow);
   }
;

class TMyStatusWindow : public TDialog
   {
 public:

   TMyStatusWindow(TWindow *, LPSTR);
   ~TMyStatusWindow();
   }
;

class TMyPrinterAreaWindow : public TDialog
   {
 public:

   TMyPrinterAreaWindow(TWindow *, LPSTR);
   ~TMyPrinterAreaWindow();

   void SetupWindow();
   void CloseWindow(int);

   void ResetWindow();

   void DoReset(UINT);

   DECLARE_RESPONSE_TABLE(TMyPrinterAreaWindow);
   }
;

class TMyFileEditWindow : public TDialog
   {
 public:

   int FileEditAll;

   TMyFileEditWindow(TWindow *, LPSTR);
   ~TMyFileEditWindow();

   bool CanClose();
   void SetupWindow();

   void DoAll(UINT);
   void DoCombo(UINT);

   DECLARE_RESPONSE_TABLE(TMyFileEditWindow);
   }
;

class TMyApp : public TApplication
   {
 public:

   RECT RR;

   TMyApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
      : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow)
      {
      }
   ;
   ~TMyApp();

   void InitMainWindow();
   void InitInstance();
   bool IdleAction(long idleCount);
   bool ProcessAppMsg(MSG & msg);

   void EvSysColorChange();

   DECLARE_RESPONSE_TABLE(TMyApp);
   }
;

_CLASSDEF(TMyWindow)
class TMyWindow : public TFrameWindow
   {
 public:

   WORD size;
   HGLOBAL hListboxHeap;
   HICON HIcon;
   HDC ScreenDC;
   HDC MemDC;
   HDC PrintMemDC;
   HBITMAP OldBitmap;
   HBITMAP OldBitmap2;
   HPALETTE OldPalette;
   HPALETTE OldPalette2;
   HFONT hCmdFont;
   RECT R;
   RECT RR;
   long ii;
   int PenSize;
   char FileName[MAXPATH];
   char BitmapName[MAXPATH];
   BOOL IsNewFile;
   BOOL IsNewBitmap;

   TModule *pm;
   TPrinter *Printer;
   TMyFileWindow *EditWindow;
   //   TRichEdit *EditWindow;
   TMyCommandWindow *CommandWindow;
   TMyEditboxWindow *EditboxWindow;
   TMyListboxWindow *ListboxWindow;
   TMyStatusWindow *StatusWindow;
   TMyPrinterAreaWindow *PrinterAreaWindow;
   TMyFileEditWindow *FileEditWindow;

   TMyWindow(TWindow * AParent, LPSTR ATitle);
   ~TMyWindow();

   void MyPopupEdit(char *FileName, NODE * args);
   void MyPopupCommander();
   void MyPopupStatus();
   void MyPopupStatusKill();
   int MyPopupInput(char *str, char *pmt);
   BOOL LoadBitmapFile(LPSTR, DWORD &, DWORD &);
   BOOL OpenDIB(int, DWORD &, DWORD &);
   void GetBitmapData(int TheFile, HANDLE, long);
   BOOL DumpBitmapFile(LPSTR, int);
   BOOL WriteDIB(int TheFile, int MaxBitCount);
   void PutBitmapData(int TheFile, HANDLE, long);
   void Printit(TDC & DC);
   void SaveFile();
   void SaveFileAs();
   void SaveBitmap();
   void SaveBitmapAs();

   void SetupWindow();
   bool CanClose();
   void GetWindowClass(WNDCLASS & WndClass);
   void Paint(TDC &, bool, TRect &);

   void EvDestroy();
   void EvKeyDown(UINT, UINT, UINT);
   void EvKeyUp(UINT, UINT, UINT);
   void EvChar(UINT, UINT, UINT);
   void EvLButtonDown(UINT, TPoint &);
   void EvLButtonUp(UINT, TPoint &);
   void EvRButtonDown(UINT, TPoint &);
   void EvRButtonUp(UINT, TPoint &);
   void EvMouseMove(UINT, TPoint &);
   void EvTimer(UINT);
   void EvSize(UINT, TSize &);
   void EvVScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
   void EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);

   void CMFileNew();
   void CMFileOpen();
   void CMFileSave();
   void CMFileSaveAs();
   void CMFileEdit();
   void CMFileErase();
   void CMExit();

   void CMBitmapNew();
   void CMBitmapOpen();
   void CMBitmapSave();
   void CMBitmapSaveAs();
   void CMBitmapPrint();
   void CMBitmapPrinterSetup();
   void CMBitmapPrinterArea();

   void CMHelp();
   void CMHelpMCI();
   void CMHelpHelp();
   void CMHelpDemo();
   void CMHelpExamples();
   void CMHelpReleaseNotes();
   void CMHelpTutorial();
   void CMHelpAbout();
   void CMHelpAboutMS();

   void CMControlExecute();

   void CMSetFont();
   void CMSetCommanderFont();
   void CMSetPenSize();
   void CMSetPenColor();
   void CMSetFloodColor();
   void CMSetScreenColor();

   void CMZoomIn();
   void CMZoomOut();
   void CMZoomNormal();

   LRESULT MMMCINotify(WPARAM, LPARAM);
   LRESULT WMCheckQueue(WPARAM, LPARAM);
   LRESULT OnNetworkConnectSendAck(WPARAM, LPARAM);
   LRESULT OnNetworkConnectSendFinish(WPARAM, LPARAM);
   LRESULT OnNetworkListenReceiveAck(WPARAM, LPARAM);
   LRESULT OnNetworkListenReceiveFinish(WPARAM, LPARAM);

 protected:
   TMyWindow(StreamableInit) : TFrameWindow(streamableInit)
      {
      }
   ;

 private:
   //   const Pchar streamableName() const { return "TMyWindow"; }

   DECLARE_RESPONSE_TABLE(TMyWindow);
   }
;

_CLASSDEF(TRulerOut)
class TRulerOut : public TPrintout
   {
 public:

   TRulerOut(Pchar ATitle) : TPrintout(ATitle)
      {
      }
   void PrintPage(int Page, TRect & Rect, UINT Flags);

   void GetDialogInfo(int &minPage, int &maxPage, int &selFromPage, int &selToPage)
      {
      minPage = 1;
      maxPage = 1;
      selFromPage = 1;
      selToPage = 1;
      }

   void SetBanding(BOOL b)
      {
      Banding = b;
      }

   bool HasPage(int pageNumber)
      {
      return pageNumber == 1;
      }
   }
;

_CLASSDEF(TMyScroller)
class TMyScroller : public TScroller
   {
 public:
   long XPos;
   long YPos;
   }
;

typedef void *ent;

class qlink
   {
   friend class qlist;

   qlink *next;
   qlink *prev;
   int type;
   ent e;

   qlink(ent a, qlink * n, qlink * p, int t)
      {
      e = a;
      next = n;
      prev = p;
      type = t;
      }
   }
;

class qlist
   {
   qlink *last;
 public:
   void insert(ent a, int t);
   ent get();
   void zap();
   void clear();

   qlist()
      {
      last = NULL;
      }

   qlist(ent a, int t)
      {
      last = new qlink(a, NULL, NULL, t);
      last->next = last;
      last->prev = last;
      }

   ~qlist()
      {
      clear();
      }
   }
;

typedef struct callthing
   {
   char *func;
   int arg1;
   int arg2;
   int kind;
   }
_callthing
   ;

struct calllist : qlist
   {
   void insert(callthing *a, int t)
      {
      qlist::insert(a, t);
      }
   callthing *get()
      {
      return (callthing *) qlist::get();
      }
   }
;

//extern HANDLE HashHandle;
//extern HANDLE StackHandle;

extern calllist calllists;

extern HBITMAP MemoryBitMap;
extern HBITMAP AreaMemoryBitMap;
extern HBITMAP OldBitmap;
extern HBITMAP OldBitmap2;

extern HICON hCursor;
extern HICON hCursorWait;
extern HICON hCursorArrow;

extern HPALETTE ThePalette;

extern LPLOGPALETTE MyLogPalette;

extern NUMBER the_zoom;
extern NUMBER ibmoldx;
extern NUMBER ibmoldy;
extern NUMBER ibmoldz;

extern NODE *current_line;

extern char LibPathName[];
extern char TempPathName[];
extern char TempBmpName[];
extern char TempClipName[];
extern char szHelpFileName[];
extern char MCIHelpFileName[];
extern char SelectedText[];
extern char YABuffer[];

extern HINSTANCE ModulehInstance;

extern HWND MainHWindow;
extern HWND CmdHWindow;
extern HWND StatusHWindow;
extern HWND EdtHWindow;

extern HWND ListHWindow;
extern HWND EditHWindow;
extern HWND ExecuteHWindow;
extern HWND EdallHWindow;
extern HWND HaltHWindow;
extern HWND TraceHWindow;
extern HWND PauseHWindow;
extern HWND StatHWindow;
extern HWND YieldHWindow;
extern HWND ResetHWindow;

extern HPALETTE OldPalette;
extern HPALETTE OldPalette2;

extern HPEN OldPen;

extern HBRUSH OldBrush;

extern HFONT OldFont;

extern LOGFONT FontRec;

extern LOGPEN NormalPen;
extern LOGPEN ErasePen;

extern LOGBRUSH FloodBrush;
extern LOGBRUSH ScreenBrush;

extern RECT FullRect;
extern RECT TempRect;

extern TMyWindow *MainWindowx;

extern int PrinterAreaXLow;
extern int PrinterAreaXHigh;
extern int PrinterAreaYLow;
extern int PrinterAreaYHigh;
extern int PrinterAreaPixels;
extern int TPrinterAreaXLow;
extern int TPrinterAreaXHigh;
extern int TPrinterAreaYLow;
extern int TPrinterAreaYHigh;
extern int TPrinterAreaPixels;

extern CUTMAP *CutBmp;
extern int CutIndex;
extern int dpenr;
extern int dpeng;
extern int dpenb;
extern int dfldr;
extern int dfldg;
extern int dfldb;
extern int dscnr;
extern int dscng;
extern int dscnb;
extern int IsDirty;
extern int NumFonts;
extern int BitMapWidth;
extern int BitMapHeight;
extern int WorldWidth;
extern int WorldHeight;
extern int WorldDepth;
extern int EnablePalette;
extern int PrinterCustomFlag;
extern int TPrinterCustomFlag;
extern int Command_OK;
extern int halt_flag;
extern int traceflag;
extern int stepflag;
extern int pause_flag;
extern int MaxWidth;
extern int MaxHeight;
extern int JustDidEdit;
extern int Time_To_Pause;
extern int Time_To_Halt;
extern int MaxX;
extern int MaxY;
extern int BaseUnitsx;
extern int BaseUnitsy;
extern int current_write_mode;

extern long scolor;
extern long fcolor;
extern long pcolor;
extern long width;
extern long zoom_flag;
extern long MaxColors;
extern long first_init;

extern LINEX TurtlePoints[][4];

extern void MakeHelpPathName(char *szFileName, char *);
extern BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG);
extern void ContextHelp(HWND hWindow, TEdit *Editor);


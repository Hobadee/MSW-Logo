//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1993 by Borland International
//   Definition of class TEditWindow.  This defines the OWL 1.0 compatible
//   text edit window
//----------------------------------------------------------------------------
#ifndef __EDITWND_H
#define __EDITWND_H

#ifndef __EDIT_H
#include "owl\edit.h"
#endif
#ifndef __CHECKBOX_H
#include "owl\checkbox.h"
#endif
#ifndef __DIALOG_H
#include "owl\dialog.h"
#endif

#pragma option -Vo-
#if     defined(__BCOPT__) && !defined(_ALLOW_po) && !defined(__WIN32__)
#pragma option -po-
#endif

//
//  struct TSearchStruct
//  ------ -------------
//
struct _CLASSTYPE TSearchStruct
   {
   char  SearchText[81];
   unsigned short CaseSensitive;
   char  ReplaceText[81];
   unsigned short ReplaceAll;
   unsigned short PromptOnReplace;
   }
;

//_CLASSDEF(TEditWindow)
//_CLASSDEF(TSearchDialog)

//
//  class TEditWindow
//  ----- -----------
//
class _EXPCLASS TEditWindow : public TWindow
   {
 public:
#if !defined(__WIN32__)
   TEdit *Editor;
#else
   TRichEdit *Editor;
#endif

   TSearchStruct  SearchStruct;
   BOOL           IsReplaceOp; // TRUE if search and replace search

   TEditWindow(TWindow * parent, const char far * title, TModule * module = 0);

   void DoSearch();

 protected:
   //
   // message response functions
   //
   void          EvSize(UINT sizeType, TSize &);
   void          EvRButtonUp(UINT, TPoint &);
   void          EvSetFocus(HWND lostFocus);

   //
   // menu command handlers
   //
   void          CmEditFind();      // CM_EDITFIND
   void          CmEditFindNext();  // CM_EDITFINDNEXT
   void          CmEditReplace();   // CM_EDITREPLACE

   void          CmFilePrint();     // CM_FILEPRINT

   DECLARE_RESPONSE_TABLE(TEditWindow);
   DECLARE_STREAMABLE(_EXPCLASS, TEditWindow, 1);
   }
;


//
//  class TSearchDialog
//  ----- -------------
//
class _EXPCLASS TSearchDialog : public TDialog
   {
 public:
   TSearchDialog(TWindow * parent,
   int             resourceId,
   TSearchStruct & searchStruct,
   TModule * module = 0);
   }
;

#pragma option -Vo.
#if     defined(__BCOPT__) && !defined(_ALLOW_po) && !defined(__WIN32__)
#pragma option -po.
#endif

#endif //__EDITWND_H

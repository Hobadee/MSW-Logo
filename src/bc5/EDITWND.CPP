/*
*
*       Copyright (C) 1995 by the Regents of the University of California
*       Copyright (C) 1995 by George Mills
*
*      This program is free software; you can redistribute it and/or modify
*      it under the terms of the GNU General Public License as published by
*      the Free Software Foundation; either version 2 of the License, or
*      (at your option) any later version.
*
*      This program is distributed in the hope that it will be useful,
*      but WITHOUT ANY WARRANTY; without even the implied warranty of
*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*      GNU General Public License for more details.
*
*      You should have received a copy of the GNU General Public License
*      along with this program; if not, write to the Free Software
*      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*/

#include "allwind.h"

//__link(RegEdit)

TSearchDialog::TSearchDialog(TWindow *parent,
   int resourceId,
   TSearchStruct &searchStruct,
   TModule *module) : TWindow(parent, 0, module), TDialog(parent, resourceId, module)
   {
   new TEdit(this, ID_SEARCHTEXT, sizeof(searchStruct.SearchText));
   new TCheckBox(this, ID_CASESENSITIVE);

   if (resourceId == SD_REPLACE)
      {
      new TEdit(this, ID_REPLACETEXT, sizeof(searchStruct.ReplaceText));
      new TCheckBox(this, ID_REPLACEALL);
      new TCheckBox(this, ID_PROMPTONREPLACE);
      }

   TransferBuffer = &searchStruct;
   }

DEFINE_RESPONSE_TABLE1(TEditWindow, TWindow)
EV_WM_SIZE,
EV_WM_SETFOCUS,
EV_WM_RBUTTONUP,
EV_COMMAND(CM_FILEPRINT, CmFilePrint),
EV_COMMAND(CM_EDITFIND, CmEditFind),
EV_COMMAND(CM_EDITFINDNEXT, CmEditFindNext),
EV_COMMAND(CM_EDITREPLACE, CmEditReplace),
END_RESPONSE_TABLE;

IMPLEMENT_STREAMABLE1(TEditWindow, TWindow);

//
// constructor for a TEditWindow
//
// initializes its data fields using passed parameters and default values
// constructs its child edit control
//
TEditWindow::TEditWindow(TWindow *parent, const char *title, TModule *module) :
TWindow(parent, title, module)
   {
#if !defined(__WIN32__)
   Editor = new TEdit(this, ID_EDITOR, 0, 0, 0, 0, 0, 0, TRUE, GetModule());
#else
   Editor = new TRichEdit(this, ID_EDITOR, 0, 0, 0, 0, 0, 0, GetModule());
   Editor->Attr.ExStyle |= WS_EX_RIGHTSCROLLBAR;
#endif
   Editor->Attr.Style |= ES_NOHIDESEL | ES_AUTOHSCROLL | ES_AUTOVSCROLL;
   IsReplaceOp = FALSE;
   memset(&SearchStruct, 0, sizeof(SearchStruct));
   }

//
// responds to an incoming WM_SIZE message by resizing the child edit
// control according to the size of the TEditWindow's client area
//
void TEditWindow::EvSize(UINT sizeType, TSize &size)
   {
   TWindow::EvSize(sizeType, size);
   Editor->SetWindowPos(0, -1, -1, size.cx + 2, size.cy + 2, SWP_NOZORDER);
   }

//
// responds to an incoming WM_SETFOCUS message by setting the focus to
// the child edit control
//
void TEditWindow::EvSetFocus(HWND)
   {
   Editor->SetFocus();
   }

void TEditWindow::EvRButtonUp(UINT, TPoint &)
   {
   DefaultProcessing();
   }

void TEditWindow::DoSearch()
   {
   int rslt;
   BOOL textFoundSinceUserInput = FALSE;

   do
      {
      rslt = Editor->Search(-1, SearchStruct.SearchText, SearchStruct.CaseSensitive);

      if (rslt == -1)
         {
         if (!IsReplaceOp || !SearchStruct.ReplaceAll || !textFoundSinceUserInput)
            {
            char err[81];

            wsprintf(err, "\"%0.60s\" not found.", SearchStruct.SearchText);
            MessageBox(err, "Find error", MB_OK | MB_ICONEXCLAMATION);
            }

         }
      else if (IsReplaceOp)
         {
         if (SearchStruct.PromptOnReplace)
            {
            rslt = MessageBox("Replace this occurrence?", "Search/Replace", MB_YESNOCANCEL | MB_ICONQUESTION);

            if (rslt == IDYES)
               {
               Editor->Insert(SearchStruct.ReplaceText);
               }
            else
               {
               if (rslt == IDCANCEL) return;
               }

            textFoundSinceUserInput = FALSE;
            }
         else
            {
            textFoundSinceUserInput = TRUE;
            Editor->Insert(SearchStruct.ReplaceText);
            }
         }
      }
   while (rslt != -1 && SearchStruct.ReplaceAll && IsReplaceOp);
   }

void TEditWindow::CmEditFind()
   {
   if (TSearchDialog(this, SD_SEARCH, SearchStruct).Execute() == IDOK)
      {
      IsReplaceOp = FALSE;
      DoSearch();
      }
   }

void TEditWindow::CmEditFindNext()
   {
   DoSearch();
   }

void TEditWindow::CmEditReplace()
   {
   if (TSearchDialog(this, SD_REPLACE, SearchStruct).Execute() == IDOK)
      {
      IsReplaceOp = TRUE;
      DoSearch();
      }
   }

void TEditWindow::CmFilePrint()
   {
#if !defined(__WIN32__)
   MainWindowx->CommandWindow->MessageBox("This feature only available in Win32 version", "Information");
#else
   // Create Printout window and set characteristics.
   //
   TRichEditPrintout printout(*MainWindowx->Printer, *Editor, "Logo");
   printout.SetBanding(false);

   // Bring up the Print dialog and print the document.
   //
   MainWindowx->Printer->Print(this, printout, true);
#endif
   }

//
// reads an instance of TEditWindow from the passed ipstream
//
void *TEditWindow::Streamer::Read(ipstream &is, uint32 /*version*/)const
   {
   TEditWindow *o = GetObject();
   ReadBaseObject((TWindow *) o, is);

   is >> (TEditFile *) o->Editor;

   is.readBytes(o->SearchStruct.SearchText, sizeof o->SearchStruct.SearchText);
   is >> o->SearchStruct.CaseSensitive;
   is.readBytes(o->SearchStruct.ReplaceText, sizeof o->SearchStruct.ReplaceText);
   is >> o->SearchStruct.ReplaceAll;
   is >> o->SearchStruct.PromptOnReplace;
   is >> o->IsReplaceOp;

   return o;
   }

//
// writes the TEditWindow to the passed opstream
//
void TEditWindow::Streamer::Write(opstream &os)const
   {
   TEditWindow *o = GetObject();
   WriteBaseObject((TWindow *) o, os);
   os << o->Editor;

   os.writeBytes(o->SearchStruct.SearchText, sizeof o->SearchStruct.SearchText);
   os << o->SearchStruct.CaseSensitive;
   os.writeBytes(o->SearchStruct.ReplaceText, sizeof o->SearchStruct.ReplaceText);
   os << o->SearchStruct.ReplaceAll;
   os << o->SearchStruct.PromptOnReplace;
   os << o->IsReplaceOp;
   }


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

DEFINE_RESPONSE_TABLE1(TFileWindow, TEditWindow)
EV_COMMAND(CM_FILENEW, CmFileNew),
EV_COMMAND(CM_FILEOPEN, CmFileOpen),
EV_COMMAND(CM_FILESAVE, CmFileSave),
EV_COMMAND(CM_FILESAVEAS, CmFileSaveAs),
EV_COMMAND(CM_EDITCUT, CmEditCut),
EV_COMMAND(CM_EDITCOPY, CmEditCopy),
EV_COMMAND(CM_EDITPASTE, CmEditPaste),
EV_COMMAND(CM_EDITUNDO, CmEditUndo),
EV_COMMAND(CM_EDITCLEAR, CmEditClear),
EV_COMMAND(CM_EDITDELETE, CmEditDelete),
END_RESPONSE_TABLE;

IMPLEMENT_STREAMABLE1(TFileWindow, TEditWindow);

#define Min(a, b)    ((a) <(b) ?(a) :(b))

//
// constructor for a TFileWindow
//
// initializes its data members using passed parameters and default values
//
TFileWindow::TFileWindow(TWindow *parent,
   const char *title,
   const char *fileName,
   TModule *module) : TEditWindow(parent, title, module)
   {
   FileName = fileName ? strnewdup(fileName) : 0;
   }

//
// dispose of the file name
//
TFileWindow::~TFileWindow()
   {
   if (!HWindow) return;

   HMENU oldMenu = GetMenu();

   if (oldMenu)::DestroyMenu(oldMenu);

   delete FileName;
   }

//
// performs setup for a TFileWindow, appending 'Untitled' to its caption
//
void TFileWindow::SetupWindow()
   {
   TEditWindow::SetupWindow();
   SetFileName(FileName);

   if (FileName && !Read()) SetFileName(0);
   }

//
// sets the file name of the window and updates the caption
//
void TFileWindow::SetFileName(const char *fileName)
   {
   if (FileName != fileName)
      {
      delete FileName;
      FileName = fileName ? strnewdup(fileName) : 0;
      }

   const char *p = FileName ? (const char *) FileName : "(Untitled)";

   if (!Title || !*Title) SetWindowText(p);

   else
      {
      char newCaption[81];

      wsprintf(newCaption, "%s - %s", Title, p);
      SetWindowText(newCaption);
      }
   }

void TFileWindow::StripClipboard()
	{
/*
	LPSTR lpText;
	LPSTR lpNewText;
	HGLOBAL HText;
	HGLOBAL HNewText;
	
	OpenClipboard();
	
	HText = GetClipboardData(CF_TEXT);
	
	if (HText)
		{
		HNewText = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, GlobalSize(HText));
		
		if (HNewText)
			{
			lpText = (LPSTR) GlobalLock(HText);
			lpNewText = (LPSTR) GlobalLock(HNewText);
			
			memcpy(lpNewText, lpText, GlobalSize(HText));
			
			GlobalUnlock(HText);
			GlobalUnlock(HNewText);
			
			EmptyClipboard();
			
			SetClipboardData(CF_TEXT, HNewText);
			}
		}
	
	CloseClipboard();

	Editor->Paste();
*/
	LPSTR lpText;
	HGLOBAL HText;
	
	OpenClipboard();
	
	HText = GetClipboardData(CF_TEXT);
	
	if (HText)
		{
		lpText = (LPSTR) GlobalLock(HText);
		
		Editor->Insert(lpText);
		
		GlobalUnlock(HText);
		}
	
	CloseClipboard();
	}

//
// begins the edit of a new file, after determining that it is Ok to
// clear the TEdit's text
//
void TFileWindow::NewFile()
   {
   if (CanClear())
      {
      Editor->Clear();
      Editor->Invalidate();
      Editor->ClearModify();
      SetFileName(0);
      }
   }

//
// replaces the current file with the given file
//
void TFileWindow::ReplaceWith(const char *fileName)
   {
   if (Read(fileName))
      {
      Editor->Invalidate();
      SetFileName(fileName);
      }
   }

//
// brings up a dialog allowing the user to open a file into this
// window
//
// same as selecting File|Open from the menus
//
void TFileWindow::Open()
   {
   //  char  tmpName[MAXPATH];

   //  if (CanClear() &&
   //      TFileDialog(this, SD_FILEOPEN, strcpy(tmpName, "*.*")).Execute() == IDOK)
   //    ReplaceWith(tmpName);
   }

//
// reads the contents of a previously-specified file into the TEdit
// child control
//
BOOL TFileWindow::Read(const char *fileName)
   {
#if defined(__WIN32__)
   Editor->LimitText(0x7FFFFFF);
#endif

   if (!fileName)
      if (FileName) fileName = FileName;
      else return FALSE;

   BOOL success = FALSE;
   int file = _lopen(fileName, OF_READ);

   if (file != -1)
      {
      long charsToRead = _llseek(file, 0, SEEK_END);
      _llseek(file, 0, SEEK_SET);

      if (charsToRead < INT_MAX && charsToRead > 0)
         {
         Editor->Clear();
         //
         // Lock and resize Editor's buffer to the size of the file
         // Then if OK, read the file into editBuffer
         //
#if !defined(__WIN32__)
         char *editBuffer = Editor->LockBuffer(int (charsToRead) +1);
#else
         char *editBuffer = (char *) malloc(int (charsToRead) +1);
#endif
         if (editBuffer)
            {
            if (_lread(file, editBuffer, int (charsToRead)) == charsToRead)
               {
               //
               // 0 terminate Editor's buffer
               //
               editBuffer[ (int) charsToRead] = 0;
               success = TRUE;
#if defined(__WIN32__)
               Editor->SetWindowText(editBuffer);
#endif
               Editor->ClearModify();
               }
#if !defined(__WIN32__)
            Editor->UnlockBuffer(editBuffer, TRUE);
#else
            free(editBuffer);
#endif
            }
         }
      _lclose(file);
      }

   if (!success)
      {
      char err[MAXPATH + 33];

      wsprintf(err, "Unable to read file \"%s\" from disk", FileName);
      MessageBox(err, GetModule()->GetName(), MB_ICONEXCLAMATION | MB_OK);
      }
   return success;
   }

//
// saves the contents of the TEdit child control into the file currently
// being editted
//
// returns true if the file was saved or Editor->IsModified returns FALSE
//(contents already saved)
//
BOOL TFileWindow::Save()
   {
   if (Editor->IsModified())
      if (!FileName) return SaveAs();

      else return Write();

   else return TRUE;                   // editor's contents haven't been changed
   }

//
// saves the contents of the TEdit child control into a file whose name
// is retrieved from the user, through execution of a "Save" file dialog
//
// returns true if the file was saved
//
BOOL TFileWindow::SaveAs()
   {
   return FALSE;
   }

//
// writes the contents of the TEdit child control to a previously-specified
// file
//
BOOL
   TFileWindow::Write(const char *fileName)
   {
   if (!fileName)
      if (FileName) fileName = FileName;
      else return FALSE;

   int file = _lcreat(fileName, 0);
   if (file == -1)
      {
      char msg[MAXPATH + 33];

      wsprintf(msg, "Unable to write file \"%s\" to disk", FileName);
      MessageBox(msg, GetModule()->GetName(), MB_ICONEXCLAMATION | MB_OK);
      return FALSE;
      }

   BOOL success = FALSE;
#if !defined(__WIN32__)
   char *editBuffer = Editor->LockBuffer();
#else
   char *editBuffer = (char *) malloc(Editor->GetWindowTextLength() + 4);
   memset(editBuffer, 0, Editor->GetWindowTextLength() + 4);
#endif
   if (editBuffer)
      {
#if !defined(__WIN32__)
      success = _lwrite(file, editBuffer, strlen(editBuffer)) != (WORD) - 1;
#else
      long iLength = Editor->GetWindowTextLength();
      Editor->GetWindowText(editBuffer, iLength + 1);
      if (editBuffer[iLength - 1] != '\n')
         {
         editBuffer[iLength + 0] = '\r';
         editBuffer[iLength + 1] = '\n';
         editBuffer[iLength + 2] = '\0';
         }
      success = _lwrite(file, editBuffer, Editor->GetWindowTextLength() + 4) != (WORD) - 1;
      free(editBuffer);
#endif
      if (success)
         Editor->ClearModify();
      }
   _lclose(file);

   return success;
   }

//
// returns a BOOL value indicating whether or not it is Ok to clear
// the TEdit's text
//
// returns TRUE if the text has not been changed, or if the user Oks the
// clearing of the text
//
BOOL
   TFileWindow::CanClear()
   {
   if (Editor->IsModified())
      {
      int result;
      char msg[MAXPATH + 28];

      if (!FileName) strcpy(msg, "Untitled file has changed.  Save?");
      else wsprintf(msg, "File \"%s\" has changed.  Save?", FileName);

      result = MessageBox(msg, "File Changed", MB_YESNOCANCEL | MB_ICONQUESTION);

      return result == IDYES ? Save() : result != IDCANCEL;
      }
   return TRUE;
   }

//
// reads an instance of TFileWindow from the passed ipstream
//
void *TFileWindow::Streamer::Read(ipstream &is, uint32 /*version*/)const
   {
   TFileWindow *o = GetObject();
   ReadBaseObject((TEditWindow *) o, is);
   o->FileName = is.freadString();
   return o;
   }

//
// writes the TFileWindow to the passed opstream
//
void TFileWindow::Streamer::Write(opstream &os)const
   {
   TFileWindow *o = GetObject();
   WriteBaseObject((TEditWindow *) o, os);
   os.fwriteString(o->FileName ? o->FileName : "");
   }

BOOL TFileWindow::AssignMenu(TResId menuResId)
   {
   if (menuResId != Attr.Menu)
      {
      if (Attr.Menu.IsString()) delete (char *) Attr.Menu;

      Attr.Menu = menuResId.IsString() ? strnewdup(menuResId) : (char *) menuResId;
      }

   //
   // if the window has been created then load and set the new menu and destroy
   // the old menu
   //
   if (!HWindow) return TRUE;

   HMENU oldMenu = GetMenu();
   HMENU newMenu = GetModule()->LoadMenu(Attr.Menu);

   if (IsFlagSet(wfMainWindow)) GetApplication()->PreProcessMenu(newMenu);

   if (!SetMenu(newMenu)) return FALSE;

   if (oldMenu)::DestroyMenu(oldMenu);

   return TRUE;
   }


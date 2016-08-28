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

/* File edit Window members */

TMyFileWindow::TMyFileWindow(TWindow *AParent, const LPSTR ATitle, const LPSTR AFileName, NODE *args, TModule *AModule, HGLOBAL AHeap) :
TFileWindow(AParent, ATitle, AFileName, AModule)
   {
   args_list = args;

   hEditHeap = AHeap;
   pm = AModule;

   AssignMenu("FILECOMMANDS");
   Attr.AccelTable = "FILECOMMANDS";
   }

TMyFileWindow::~TMyFileWindow()
   {
//   DestroyMenu(hMenu);
#if !defined(__WIN32__)
   delete pm;
   GlobalFree(hEditHeap);
#endif
   if (hEdtFont) DeleteObject(hEdtFont);
   }

void TMyFileWindow::CMExit()
   {

   /* here on FILE-EXIT in editor */

   SendMessage(WM_CLOSE);
   }

void TMyFileWindow::CMSaveAndExit()
   {
   Save();
   CMExit();
   }

void TMyFileWindow::CMSaveToWorkspace()
   {
   if (!bExpert)
      {
      MainWindowx->CommandWindow->MessageBox("This feature is for expert mode only (use Save and Exit)", "Save to Workspace", MB_OK);
      return;
      }

   Save();

   if (args_list != NULL)
      {
      error_happen = 0;

      EndEdit();

      unlink(TempPathName);
      IsDirty = 1;

      if (error_happen)
         {
         MainWindowx->CommandWindow->MessageBox("The cursor will be positioned just after last successful definition.\nCheck Commander Window for possible error Message.", "Your Edit has FAILED to load", MB_OK);
         error_happen = 0;
         Editor->SetSelection(0, 0);
         Editor->Insert(" ");
         Editor->DeleteSubText(0, 1);
         int iLine = Editor->GetLineFromPos(LinesLoadedOnEdit);
         Editor->Scroll(0, iLine);
         Editor->SetSelection(LinesLoadedOnEdit, LinesLoadedOnEdit);
         }
      }
   }

void TMyFileWindow::CMHelpEdit()
   {
   do_help("Editor");
   }

void TMyFileWindow::CMHelp()
   {
   MainWindowx->CMHelp();
   //   WinHelp(szHelpFileName,HELP_INDEX,0L);
   }

void TMyFileWindow::CMHelpEditTopic()
   {
   char theText[100];
   char *ptr;
   UINT start;
   UINT end;
   int i;

   // get the keyword selected

   Editor->GetSelection(start, end);
   if ((start < end) && (end-start < 80))
      {
      Editor->GetSubText((LPSTR) theText, start, end);

      // clean up line before passing to help
		if (strlen(theText) > 0)
			{
			i = strlen(theText) - 1;
			while ((i > 0) && (theText[i] == ' ')) theText[i--] = '\0';
			
			if (strlen(theText) > 0)
				{
				i = 0;
				while ((i < strlen(theText)) && (theText[i] == ' ')) i++;
				
				ptr = theText + i;
				
				if (strlen(ptr) > 0)
					{
					if (strchr(ptr,'\r')) *strchr(ptr,'\r') = '\0';
					
					if (strlen(ptr) > 0)
						{
						if (strchr(ptr,'\n')) *strchr(ptr,'\n') = '\0';
						
						if (strlen(ptr) > 0)
							{
							do_help(ptr);
							}
						else
							{
							do_help(NULL);
							}
						}
					else
						{
						do_help(NULL);
						}
					}
				else
					{
					do_help(NULL);
					}
				}
			else
				{
				do_help(NULL);
				}
			}
		else
			{
			do_help(NULL);
			}
      }
   else
      {
      do_help(NULL);
      }
   }

void TMyFileWindow::CMTest()
   {
   char *theText;
   char *ptr;
   char *ptr2;
   char *ptr3;
   UINT start;
   UINT end;
   int more;

   // get the code selected

   Editor->GetSelection(start, end);
   theText = (char *) malloc(abs(end - start) + 1);
   Editor->GetSubText((LPSTR) theText, start, end);
   ptr = theText;

   // strip comments

   more = 1;
   while (more)
      {
      ptr2 = strchr(ptr, ';');

      if (ptr2 != NULL)
         {
         *ptr2 = ' ';
         while ((*ptr2 != 10) && (*ptr2 != 13) && (*ptr2 != '~') && (*ptr2 != 10))
            {
            *ptr2++ = ' ';
            }
         }
      else
         {
         more = 0;
         }
      }

   // paste continuation

   more = 1;
   while (more)
      {
      ptr2 = strchr(ptr, '~');

      if (ptr2 != NULL)
         {
         *ptr2 = ' ';
         ptr3 = strchr(ptr2, '\n');
         if (ptr3 != NULL)
            {
            *ptr3 = ' ';
            *(ptr3 - 1) = ' ';
            }
         }
      else
         {
         more = 0;
         }
      }

   // for each real line left execute it

   more = 1;
   while (more)
      {
      ptr2 = strchr(ptr, '\n');

      if (ptr2 != NULL)
         {
         *ptr2 = '\0';
         *(ptr2 - 1) = '\0';
         }

      putcombobox(ptr);
      do_execution(ptr);

      if (ptr2 != NULL)
         {
         ptr = ptr2 + 1;
         }
      else
         {
         more = 0;
         }
      }

   free(theText);

   }

void TMyFileWindow::SetupWindow()
   {
   LOGFONT lf;

   TFileWindow::SetupWindow();

   _fmemset(&lf, 0, sizeof(LOGFONT));

   GetPrivateProfileString("EditFont", "FaceName", "Courier", (LPSTR) & lf.lfFaceName[0], LF_FACESIZE, "Logo.ini");
   lf.lfHeight = GetPrivateProfileInt("EditFont", "Height", -13, "Logo.ini");
   lf.lfWeight = GetPrivateProfileInt("EditFont", "Weight", 400, "Logo.ini");
   lf.lfItalic = GetPrivateProfileInt("EditFont", "Italic", 0, "Logo.ini");
   lf.lfCharSet = GetPrivateProfileInt("EditFont", "CharSet", 0, "Logo.ini");
   lf.lfOutPrecision = GetPrivateProfileInt("EditFont", "OutPrecision", 1, "Logo.ini");
   lf.lfClipPrecision = GetPrivateProfileInt("EditFont", "ClipPrecision", 2, "Logo.ini");
   lf.lfQuality = GetPrivateProfileInt("EditFont", "Quality", 1, "Logo.ini");
   lf.lfPitchAndFamily = GetPrivateProfileInt("EditFont", "PitchAndFamily", 49, "Logo.ini");

   hEdtFont = CreateFontIndirect(&lf);
   Editor->SetWindowFont(hEdtFont, TRUE);
   }

void TMyFileWindow::CMEditSetFont()
   {
   static CHOOSEFONT CF;

   LOGFONT lf;

   _fmemset(&lf, 0, sizeof(LOGFONT));

   GetPrivateProfileString("EditFont", "FaceName", "Courier", (LPSTR) & lf.lfFaceName[0], LF_FACESIZE, "Logo.ini");
   lf.lfHeight = GetPrivateProfileInt("EditFont", "Height", -13, "Logo.ini");
   lf.lfWeight = GetPrivateProfileInt("EditFont", "Weight", 400, "Logo.ini");
   lf.lfItalic = GetPrivateProfileInt("EditFont", "Italic", 0, "Logo.ini");
   lf.lfCharSet = GetPrivateProfileInt("EditFont", "CharSet", 0, "Logo.ini");
   lf.lfOutPrecision = GetPrivateProfileInt("EditFont", "OutPrecision", 1, "Logo.ini");
   lf.lfClipPrecision = GetPrivateProfileInt("EditFont", "ClipPrecision", 2, "Logo.ini");
   lf.lfQuality = GetPrivateProfileInt("EditFont", "Quality", 1, "Logo.ini");
   lf.lfPitchAndFamily = GetPrivateProfileInt("EditFont", "PitchAndFamily", 49, "Logo.ini");

   // clear the struct

   memset(&CF, 0, sizeof(CF));

   // fill it with the right stuff

   CF.lStructSize = sizeof(CF);
   CF.hwndOwner = MainHWindow;
   CF.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
   CF.nFontType = SCREEN_FONTTYPE;
   CF.lpLogFont = &lf;
   CF.hInstance = 0;

   // FontRec is not trashed in case of cancel

   if (ChooseFont(&CF))
      {
      char tmp[16];
      HFONT hFont;

      WritePrivateProfileString("EditFont", "FaceName", (LPSTR) & lf.lfFaceName[0], "Logo.ini");
      wsprintf(tmp, "%d", lf.lfHeight); WritePrivateProfileString("EditFont", "Height", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfWeight); WritePrivateProfileString("EditFont", "Weight", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfItalic); WritePrivateProfileString("EditFont", "Italic", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfCharSet); WritePrivateProfileString("EditFont", "CharSet", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfOutPrecision); WritePrivateProfileString("EditFont", "OutPrecision", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfClipPrecision); WritePrivateProfileString("EditFont", "ClipPrecision", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfQuality); WritePrivateProfileString("EditFont", "Quality", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfPitchAndFamily); WritePrivateProfileString("EditFont", "PitchAndFamily", (LPSTR) tmp, "Logo.ini");

      hFont = CreateFontIndirect(&lf);
      Editor->SetWindowFont(hFont, TRUE);

		if (hEdtFont) DeleteObject(hEdtFont);
		hEdtFont = hFont;

      TRect rect;
      GetWindowRect(rect);
      SetWindowPos(0, rect.Left(), rect.Top(), rect.Width() + 1, rect.Height(), SWP_NOZORDER);
      SetWindowPos(0, rect, SWP_NOZORDER);
      }

   }

int TMyFileWindow::EndEdit()
   {

   int realsave = lendedit();

   // check for error

   for (int i = 0; i < 1; i++)
      {
      if (stopping_flag == THROWING)
         {
         if (compare_node(throw_node, Error, TRUE) == 0)
            {
            err_print();
            error_happen = 1;
            }
         else if (compare_node(throw_node, System, TRUE) == 0) break;
         else if (compare_node(throw_node, Toplevel, TRUE) != 0)
            {
            err_logo(NO_CATCH_TAG, throw_node);
            err_print();
            error_happen = 1;
            }
         stopping_flag = RUN;
         }
      if (stopping_flag == STOP || stopping_flag == OUTPUT)
         {
         print_node(stdout, make_static_strnode("You must be in a procedure to use OUTPUT or STOP.\n"));
         stopping_flag = RUN;
         }
      }

   return realsave;
   }

void TMyFileWindow::EvDestroy()
   {
   RECT wrect;
   char szWinLocStr[WININISIZ];
   int w, h;
   int realsave;
   callthing *callevent;

   // if args_list specified no user callabled editor

   if (args_list != NULL)
      {
      error_happen = 0;

      realsave = EndEdit();
      
      // if error the ask user to reedit

      if (error_happen)
         {
         if (MainWindowx->CommandWindow->MessageBox("The cursor will be positioned just after last successful definition.\nCheck Commander Window for possible error Message.\n\nReturn to edit?", "Your Edit has FAILED to load", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
            {
            if (TMyWindow_MyPopupEdit(TempPathName, args_list))
               {
               MainWindowx->CommandWindow->MessageBox("Cannot restart editor", "Fatal", MB_OK | MB_ICONQUESTION);
               }
            else
               {
               unlink(TempPathName);
               IsDirty = 1;
               }
            }
         else
            {
            error_happen = 0;
            ::SetFocus(EditHWindow);
            }
         }
      else
         {
         // check for quit before erasing

         if (realsave)
            {
            lerase(args_list);

            // Since we erased we must load again, but no errors

            lendedit();
            }

         // free up args_list

         args_list = reref(args_list, NIL);

         unlink(TempPathName);

         ::SetFocus(EditHWindow);
         }
      }

   // else execute callback for user callable editor

   else
      {
      callevent = new callthing;

      callevent->func = edit_editexit;
      callevent->kind = 3;

      calllists.insert(callevent, 3);
      }


   // Get location and size of our window on the screen so we can
   // come back up in the same spot next time we are invoked.

   if (!::IsIconic(HWindow))
      {
      ::GetWindowRect(HWindow, (LPRECT) & wrect);
      w = wrect.right - wrect.left;
      h = wrect.bottom - wrect.top;

      // Make a string with our window location and size.

      sprintf(szWinLocStr, "%d,%d,%d,%d", wrect.left, wrect.top, w, h);

      // Save in WIN.INI file.

      WritePrivateProfileString(
         "LOGO",
         "Editor",
         szWinLocStr,
         "LOGO.INI");
      }

   TFileWindow::DefaultProcessing();
   }

LRESULT TMyFileWindow::DefWindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
   {
   // if right button do context sensitive help

//   if ((msg == WM_PARENTNOTIFY) && (wParam == WM_RBUTTONDOWN))
//      {
//      if (args_list != NULL) 
//			{
//			CMHelpEditTopic();
//			return 0L;
//			}
//      }

   return (TFileWindow::DefWindowProc(msg, wParam, lParam));
   }

bool TMyFileWindow::CanClose()
   {
   char S[MAXPATH + 28];
   int Rslt;

   // if changed better ask user

   if (Editor->IsModified())
      {
      strcpy(S, "Contents have changed. Save to workspace?");

      Rslt = MessageBox(S, "Contents Changed", MB_YESNOCANCEL | MB_ICONQUESTION);
      if (Rslt == IDYES)
         return Save();
      else
         return Rslt != IDCANCEL;
      }
   return TRUE;
   }

DEFINE_RESPONSE_TABLE1(TMyFileWindow, TFileWindow)
EV_COMMAND(CM_EXIT, CMExit),
EV_COMMAND(CM_FILESAVETOWORKSPACE, CMSaveToWorkspace),
EV_COMMAND(CM_FILESAVEANDEXIT, CMSaveAndExit),
EV_COMMAND(CM_HELP, CMHelp),
EV_COMMAND(CM_HELPEDIT, CMHelpEdit),
EV_COMMAND(CM_HELPEDIT_TOPIC, CMHelpEditTopic),
EV_COMMAND(CM_TEST, CMTest),
EV_COMMAND(CM_EDITSETFONT, CMEditSetFont),
EV_WM_RBUTTONUP,
EV_WM_RBUTTONDOWN,
EV_WM_DESTROY,
END_RESPONSE_TABLE;

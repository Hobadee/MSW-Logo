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

TMyCommandWindow::TMyCommandWindow(TWindow *AParent, LPSTR ATitle)
: TDialog(AParent, ATitle)
   {
   }

TMyCommandWindow::~TMyCommandWindow()
   {
   }

void TMyCommandWindow::SetupWindow()
   {
   TDialog::SetupWindow();
#ifdef CTL3D
   Ctl3dSubclassDlgEx(HWindow, CTL3D_ALL);
#endif
   }

void TMyCommandWindow::GetWindowClass(WNDCLASS &WndClass)
   {
   TDialog::GetWindowClass(WndClass);
   WndClass.hIcon = GetApplication()->LoadIcon("LOGOICON");
   WndClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
   WndClass.hInstance = GetApplication()->GetInstance();
   }

char *TMyCommandWindow::GetClassName()
   {
   return ((LPSTR) "LogoCommander");
   }

void TMyCommandWindow::EvSize(UINT, TSize &)
   {
   RECT TraceRect;
   TRect ThisRect;
   RECT EditRect;

   int Xr;
   int Yr;
   int Ye;
   int Yb;
   int Xb;

   /* scale and pos. each sub-window in commander window based on its size */

   ::GetClientRect(TraceHWindow, &TraceRect);
   GetClientRect(ThisRect);
   ::GetClientRect(EditHWindow, &EditRect);

   Xr = ThisRect.right - ThisRect.left;
   Yr = ThisRect.bottom - ThisRect.top;
   Ye = EditRect.bottom - EditRect.top;
   Yb = TraceRect.bottom - TraceRect.top;
   Xb = TraceRect.right - TraceRect.left;

   ::SetWindowPos(ListHWindow, NULL, 4, 1, Xr - Xb * 2 - 10, Yr - Ye - 7, 0);
   ::SetWindowPos(EditHWindow, NULL, 4, Yr - Ye - 5, Xr - Xb * 2 - 10, Yb + 2, 0);

   ::SetWindowPos(HaltHWindow, NULL, Xr - Xb * 2 - 4, Yb * 0 + 1, Xb * 1, Yb, 0);
   ::SetWindowPos(TraceHWindow, NULL, Xr - Xb * 1 - 4, Yb * 0 + 1, Xb * 1, Yb, 0);
   ::SetWindowPos(PauseHWindow, NULL, Xr - Xb * 2 - 4, Yb * 1 + 1, Xb * 1, Yb, 0);
   ::SetWindowPos(StatusHWindow, NULL, Xr - Xb * 1 - 4, Yb * 1 + 1, Xb * 1, Yb, 0);
   ::SetWindowPos(YieldHWindow, NULL, Xr - Xb * 2 - 4, Yb * 2 + 1, Xb * 1, Yb, 0);
   ::SetWindowPos(ResetHWindow, NULL, Xr - Xb * 1 - 4, Yb * 2 + 1, Xb * 1, Yb, 0);

   ::SetWindowPos(ExecuteHWindow, NULL, Xr - Xb * 2 - 4, Yr - Ye - 4, Xb * 1, Yb, 0);
   ::SetWindowPos(EdallHWindow, NULL, Xr - Xb * 1 - 4, Yr - Ye - 4, Xb * 1, Yb, 0);

   }

void TMyCommandWindow::DoEditBox(UINT)
   {
   }

void TMyCommandWindow::DoListBox(UINT)
   {
   }

void do_execution(char *SelectedText2)
   {
   NODE *exec_list = NIL;
   NODETYPES this_type;
   int i;
   char *c;

   /* if something there continue */

   if (strlen(SelectedText2) != 0)
      {

      /* if executing then it's ok to halt */

      halt_flag++;
      if (halt_flag < 1) halt_flag = 1;

      /* this code emulates the TTY model used in UCBLOGO main loop */

      this_type = STRING;
      c = SelectedText2;

      /* do control character processing processing */

      for (c = SelectedText2; *c != '\0'; c++)
         {
         if (*c == '\\')
            {
            strcpy(c, c + 1);
            if (*c)
               {
               if (*c == 'n') *c = '\n';
               *c = setparity(*c);
               }
            this_type = BACKSLASH_STRING;
            }
         }

      check_reserve_tank();

      /* turn text into a NODE and parse it */

      current_line = reref(current_line, make_strnode(SelectedText2, (char *) NULL, (int) strlen(SelectedText2), this_type, strnzcpy));

      exec_list = reref(exec_list, parser(current_line, TRUE));

      /* now process it */

      val_status = 0;
      if (exec_list != NIL) eval_driver(exec_list);

      /* process special conditions */

      for (i = 0; i < 1; i++)
         {
         if (stopping_flag == THROWING)
            {
            if (compare_node(throw_node, Error, TRUE) == 0)
               {
               err_print();
               }
            else if (compare_node(throw_node, System, TRUE) == 0)
               {
               break;
               }
            else if (compare_node(throw_node, Toplevel, TRUE) != 0)
               {
               err_logo(NO_CATCH_TAG, throw_node);
               err_print();
               }
            stopping_flag = RUN;
            }

         if (stopping_flag == STOP || stopping_flag == OUTPUT)
            {
            print_node(stdout, make_static_strnode(
                  "You must be in a procedure to use OUTPUT or STOP.\n"));
            stopping_flag = RUN;
            }
         }

      /* deallocate the line */

      current_line = reref(current_line, NIL);

      /* this is a hack to force garbage collector to properly clean up */

      if (exec_list != NIL)
         {
         settype(exec_list, CONS);
         exec_list = reref(exec_list, NIL);
         }

      /* not ok to halt now */

      halt_flag--;
      if (halt_flag < 0) halt_flag = 0;
      }

   }

void TMyCommandWindow::DoButtonExecute(UINT)
   {
   char SelectedText2[MAX_BUFFER_SIZE];

   SelectedText2[0] = '\0';

   /* get what's in the edit box */

   getcombobox(SelectedText2);

   /* if real do something with it */

   if (strlen(SelectedText2))
      {

      /* copy to list box for command recall */

//      if (!bSilent)
//         {
         putcombobox(SelectedText2);
//         }

      /* if dribble then dribble */

      if (dribblestream != NULL) fprintf(dribblestream, "%s\n", SelectedText2);

      /* reset erract loop error history */
		erract_errtype = FATAL;

      /* reset evaluation counter (call counter) and execute */

      eval_count = 0;
      vector_count = 0;

      if (status_flag) update_status_evals();
      if (status_flag) update_status_vectors();

      do_execution(SelectedText2);

//      if (combo_buff)
//         {
//         if (strlen(combo_buff)) putcombochar('\n');
//         }
      }

   /* if we just poped up editor then don't set focus to commander input */

   if (!JustDidEdit)::SetFocus(EditHWindow);
   JustDidEdit = 0;

   }

void TMyCommandWindow::DoButtonEdall(UINT)
   {
   do_execution("Edallbtn");
   }

void TMyCommandWindow::DoButtonStatus(UINT)
   {

   /* toggle status state along with poping up and killing the status window */

   if (status_flag)
      {
      MainWindowx->MyPopupStatusKill();
      ::SetFocus(EditHWindow);
      }
   else
      {
      MainWindowx->MyPopupStatus();
      }
   }

void TMyCommandWindow::DoButtonReset(UINT)
   {
   /* just do a clear screen and return focus */

   lclearscreen();
   windowdelete_helper();
   ::SetFocus(EditHWindow);
   }

void TMyCommandWindow::DoButtonYield(UINT)
   {
   /* toggle yield state */

   /*
      if (yield_flag)
         {
         yield_flag = 0;
         SendDlgItemMsg(ID_YIELD, WM_SETTEXT, 0, (DWORD) "Yield");
         }
      else
         {
         yield_flag = 1;
         SendDlgItemMsg(ID_YIELD, WM_SETTEXT, 0, (DWORD) "NoYield");
         }
   */
   if (stepflag)
      {
      stepflag = 0;
      SendDlgItemMsg(ID_YIELD, WM_SETTEXT, 0, (DWORD) "Step");
      }
   else
      {
      stepflag = 1;
      SendDlgItemMsg(ID_YIELD, WM_SETTEXT, 0, (DWORD) "UnStep");
      }

   ::SetFocus(EditHWindow);
   }

void TMyCommandWindow::DoButtonPause(UINT)
   {

   /* if ok to halt then it's ok to pause if we get here */

   ::SetFocus(EditHWindow);
   if (halt_flag != 0) Time_To_Pause = 1;
   }

// Will put pack later
//
//void do_pause_update(long arg)
//   {
//   if (arg)
//      {
//      sprintf(YABuffer,"Pause-%d",(int)arg);
//      }
//   else
//      {
//      sprintf(YABuffer,"Pause");
//      }
//   MainWindowx->CommandWindow->
//   SendDlgItemMsg(ID_PAUSE, WM_SETTEXT, 0, (DWORD)YABuffer);
//   }

void TMyCommandWindow::DoButtonTrace(UINT)
   {

   /* toggle trace state */

   if (traceflag)
      {
      traceflag = 0;
      SendDlgItemMsg(ID_TRACE, WM_SETTEXT, 0, (DWORD) "Trace");
      }
   else
      {
      traceflag = 1;
      SendDlgItemMsg(ID_TRACE, WM_SETTEXT, 0, (DWORD) "UnTrace");
      }
   ::SetFocus(EditHWindow);
   }

void TMyCommandWindow::DoButtonHalt(UINT)
   {
   int i;

   for (i = 1; i < 32; i++) MainWindowx->KillTimer(i);

   /* if ok to halt and we get here then halt */

   ::SetFocus(EditHWindow);
   if (halt_flag != 0) Time_To_Halt = 1;
   }

void TMyCommandWindow::EvDestroy()
   {
   TRect wrect;
   char szWinLocStr[WININISIZ];
   int w, h;

   // Get location and size of our window on the screen so we can
   // come back up in the same spot next time we are invoked.

   if (!IsIconic())
      {
      GetWindowRect(wrect);
      w = wrect.right - wrect.left;
      h = wrect.bottom - wrect.top;

      // Make a string with our window location and size.
      sprintf(szWinLocStr, "%d,%d,%d,%d", wrect.left, wrect.top, w, h);

      // Save in WIN.INI file.
      WritePrivateProfileString(
         "LOGO",
         "Commander",
         szWinLocStr,
         "LOGO.INI");
      }

   TWindow::EvDestroy();
   }

void do_trace_update(long arg)
   {

   /* update trace button for when logo trace command is issued */

   if (arg)
      {
      sprintf(YABuffer, "UnTrace");
      }
   else
      {
      sprintf(YABuffer, "Trace");
      }
   MainWindowx->CommandWindow->
      SendDlgItemMsg(ID_TRACE, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

NODE *lhalt()
   {
//   if (halt_flag != 0) Time_To_Halt = 1;
   MainWindowx->CommandWindow->PostMessage(WM_COMMAND, ID_HALT, ID_HALT);
   return (UNBOUND);
   }

NODE *lyield()
   {

   // set flag and update button label

   yield_flag = 1;
   //   MainWindowx->CommandWindow->
   //   SendDlgItemMsg(ID_YIELD, WM_SETTEXT, 0, (DWORD) "NoYield");
   return (UNBOUND);
   }

NODE *lnoyield()
   {

   // clear flag and update button label

   yield_flag = 0;
   //   MainWindowx->CommandWindow->
   //   SendDlgItemMsg(ID_YIELD, WM_SETTEXT, 0, (DWORD) "Yield");
   return (UNBOUND);
   }

/* Editbox members */

TMyEditboxWindow::TMyEditboxWindow(TWindow *AParent, int AId, WORD Alen)
: TEdit(AParent, AId, Alen)
   {
   }

TMyEditboxWindow::~TMyEditboxWindow()
   {
   }

void TMyEditboxWindow::EvKeyDown(UINT, UINT, UINT)
   {
   TMessage Msg = __GetTMessage();

   // if up/down arrow keys then focus to list box

   switch (Msg.WParam)
      {
       case VK_UP:
           {
              ::SetFocus(ListHWindow);

              break;
           }
       case VK_F1:
           {
              ContextHelp(MainWindowx->CommandWindow->HWindow, this);
              break;
           }
      }

   DefaultProcessing();
   }

void TMyEditboxWindow::EvRButtonUp(UINT, TPoint &)
   {
//   ContextHelp(MainWindowx->CommandWindow->HWindow, this);
   DefaultProcessing();
   }

/* Listbox members */

TMyListboxWindow::TMyListboxWindow(TWindow *AParent, int AId, WORD Alen, TModule *Amodule)
: TEdit(AParent, AId, Alen, Amodule)
   {
   }

TMyListboxWindow::~TMyListboxWindow()
   {
   }

void TMyListboxWindow::EvKeyDown(UINT, UINT, UINT)
   {
   TMessage Msg = __GetTMessage();

   // if Down off bottom then focus to edit box

   if ((Msg.WParam == VK_DOWN))
      {
      UINT from;
      UINT to;

      GetSelection(from, to);
      if (GetTextLen() == to)
         {
         ::SetFocus(EditHWindow);
         return;
         }
      }

   DefaultProcessing();

   switch (Msg.WParam)
      {
       case VK_UP:
       case VK_DOWN:
       case VK_LEFT:
       case VK_RIGHT:
           {
              char buf[MAX_BUFFER_SIZE];

              GetLine(buf, MAX_BUFFER_SIZE, GetLineFromPos(-1));
              MainWindowx->EditboxWindow->SetText(buf);

              break;
           }
       case VK_F1:
           {
              ContextHelp(MainWindowx->CommandWindow->HWindow, this);

              break;
           }
      }

   }

void TMyListboxWindow::EvLButtonDown(UINT /* modKeys */, TPoint & /* point */)
   {
   char buf[MAX_BUFFER_SIZE];

   DefaultProcessing();

   GetLine(buf, MAX_BUFFER_SIZE, GetLineFromPos(-1));
   MainWindowx->EditboxWindow->SetText(buf);
   }

void TMyListboxWindow::EvLButtonDblClk(UINT /* modKeys */, TPoint & /* point */)
   {
   char buf[MAX_BUFFER_SIZE];

   DefaultProcessing();

   GetLine(buf, MAX_BUFFER_SIZE, GetLineFromPos(-1));
   MainWindowx->EditboxWindow->SetText(buf);
   MainWindowx->CommandWindow->DoButtonExecute(0);
   }

void TMyListboxWindow::EvRButtonUp(UINT, TPoint &)
   {
//   ContextHelp(MainWindowx->CommandWindow->HWindow, this);
   DefaultProcessing();
   }

DEFINE_RESPONSE_TABLE1(TMyEditboxWindow, TEdit)
EV_WM_KEYDOWN,
EV_WM_RBUTTONUP,
END_RESPONSE_TABLE;

DEFINE_RESPONSE_TABLE1(TMyListboxWindow, TEdit)
EV_WM_KEYDOWN,
EV_WM_LBUTTONDOWN,
EV_WM_LBUTTONDBLCLK,
EV_WM_RBUTTONUP,
END_RESPONSE_TABLE;

DEFINE_RESPONSE_TABLE1(TMyCommandWindow, TDialog)
EV_WM_SIZE,
EV_WM_DESTROY,
EV_WM_CLOSE,
EV_CHILD_NOTIFY_ALL_CODES(ID_LISTBOX, DoListBox),
EV_CHILD_NOTIFY_ALL_CODES(ID_EDITINPUT, DoEditBox),
EV_CHILD_NOTIFY_ALL_CODES(ID_EXECUTE, DoButtonExecute),
EV_CHILD_NOTIFY_ALL_CODES(ID_EDALL, DoButtonEdall),
EV_CHILD_NOTIFY_ALL_CODES(ID_HALT, DoButtonHalt),
EV_CHILD_NOTIFY_ALL_CODES(ID_STATUS, DoButtonStatus),
EV_CHILD_NOTIFY_ALL_CODES(ID_YIELD, DoButtonYield),
EV_CHILD_NOTIFY_ALL_CODES(ID_PAUSE, DoButtonPause),
EV_CHILD_NOTIFY_ALL_CODES(ID_TRACE, DoButtonTrace),
EV_CHILD_NOTIFY_ALL_CODES(ID_RESET, DoButtonReset),
EV_COMMAND(IDCANCEL, CmCancel),
EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;


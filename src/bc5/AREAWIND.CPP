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

TMyPrinterAreaWindow::TMyPrinterAreaWindow(TWindow *AParent, LPSTR ATitle)
: TDialog(AParent, ATitle)
   {
   }

TMyPrinterAreaWindow::~TMyPrinterAreaWindow()
   {
   }

/* These next 4 routines fetch the active area dimensions */

void TMyPrinterAreaWindow::CloseWindow(int arg)
   {
   SelectedText[0] = '\0';
   SendDlgItemMsg(ID_XLOW, WM_GETTEXT, MAX_BUFFER_SIZE, (DWORD) SelectedText);
   sscanf(SelectedText, "%d", &TPrinterAreaXLow);
   SelectedText[0] = '\0';
   SendDlgItemMsg(ID_XHIGH, WM_GETTEXT, MAX_BUFFER_SIZE, (DWORD) SelectedText);
   sscanf(SelectedText, "%d", &TPrinterAreaXHigh);
   SelectedText[0] = '\0';
   SendDlgItemMsg(ID_YLOW, WM_GETTEXT, MAX_BUFFER_SIZE, (DWORD) SelectedText);
   sscanf(SelectedText, "%d", &TPrinterAreaYLow);
   SelectedText[0] = '\0';
   SendDlgItemMsg(ID_YHIGH, WM_GETTEXT, MAX_BUFFER_SIZE, (DWORD) SelectedText);
   sscanf(SelectedText, "%d", &TPrinterAreaYHigh);
   SelectedText[0] = '\0';
   SendDlgItemMsg(ID_PIXELS, WM_GETTEXT, MAX_BUFFER_SIZE, (DWORD) SelectedText);
   sscanf(SelectedText, "%d", &TPrinterAreaPixels);

   if (
         (TPrinterAreaXLow  == -BitMapWidth  / 2) &&
         (TPrinterAreaXHigh == +BitMapWidth  / 2) &&
         (TPrinterAreaYLow  == -BitMapHeight / 2) &&
         (TPrinterAreaYHigh == +BitMapHeight / 2)) TPrinterCustomFlag = 0; else TPrinterCustomFlag = 1;

   TDialog::CloseWindow(arg);
   }

void TMyPrinterAreaWindow::ResetWindow()
   {

   /* if active area display active parameters */

   sprintf(YABuffer, "%d", TPrinterAreaXLow);
   SendDlgItemMsg(ID_XLOW, WM_SETTEXT, 0, (DWORD) YABuffer);
   sprintf(YABuffer, "%d", TPrinterAreaXHigh);
   SendDlgItemMsg(ID_XHIGH, WM_SETTEXT, 0, (DWORD) YABuffer);
   sprintf(YABuffer, "%d", TPrinterAreaYLow);
   SendDlgItemMsg(ID_YLOW, WM_SETTEXT, 0, (DWORD) YABuffer);
   sprintf(YABuffer, "%d", TPrinterAreaYHigh);
   SendDlgItemMsg(ID_YHIGH, WM_SETTEXT, 0, (DWORD) YABuffer);

   sprintf(YABuffer, "%d", TPrinterAreaPixels);
   SendDlgItemMsg(ID_PIXELS, WM_SETTEXT, 0, (DWORD) YABuffer);

   }

void TMyPrinterAreaWindow::SetupWindow()
   {

   /* if active area display active parameters */

   ResetWindow();
   TDialog::SetupWindow();

   }

void TMyPrinterAreaWindow::DoReset(UINT)
   {

   /* hack to init things */

   TPrinterAreaXLow  = -BitMapWidth / 2;
   TPrinterAreaXHigh = +BitMapWidth / 2;
   TPrinterAreaYLow  = -BitMapHeight / 2;
   TPrinterAreaYHigh = +BitMapHeight / 2;

   TPrinterAreaPixels = max(BitMapWidth, BitMapHeight) / 8;

   ResetWindow();

   }

NODE *lsetactivearea(NODE *arg)
   {
   NODE *args;
   NODE *xlow;
   NODE *ylow;
   NODE *xhigh;
   NODE *yhigh;
   char szWinLocStr[WININISIZ];

   args = vector_4_arg(arg);

   // better be a list

   if (NOT_THROWING)
      {

      // apply all args that are given

      xlow  = car(args);
      ylow  = car(cdr(args));
      xhigh = car(cdr(cdr(args)));
      yhigh = car(cdr(cdr(cdr(args))));

      PrinterAreaXLow  = ((nodetype(xlow)  == FLOAT) ? getfloat(xlow)  : (FLONUM) getint(xlow));
      PrinterAreaYLow  = ((nodetype(ylow)  == FLOAT) ? getfloat(ylow)  : (FLONUM) getint(ylow));
      PrinterAreaXHigh = ((nodetype(xhigh) == FLOAT) ? getfloat(xhigh) : (FLONUM) getint(xhigh));
      PrinterAreaYHigh = ((nodetype(yhigh) == FLOAT) ? getfloat(yhigh) : (FLONUM) getint(yhigh));

      if ((PrinterAreaXLow >= PrinterAreaXHigh) || (PrinterAreaYLow >= PrinterAreaYHigh))
			{
			MainWindowx->CommandWindow->MessageBox("Bad argument", "Active Area");
			err_logo(STOP_ERROR, NIL);
			return (UNBOUND);
			}

      sprintf(szWinLocStr, "%d", PrinterAreaXLow);
      WritePrivateProfileString("Printer", "XLow", szWinLocStr, "LOGO.INI");
      sprintf(szWinLocStr, "%d", PrinterAreaXHigh);
      WritePrivateProfileString("Printer", "XHigh", szWinLocStr, "LOGO.INI");
      sprintf(szWinLocStr, "%d", PrinterAreaYLow);
      WritePrivateProfileString("Printer", "YLow", szWinLocStr, "LOGO.INI");
      sprintf(szWinLocStr, "%d", PrinterAreaYHigh);
      WritePrivateProfileString("Printer", "YHigh", szWinLocStr, "LOGO.INI");

      if (
            (PrinterAreaXLow  == -BitMapWidth  / 2) &&
            (PrinterAreaXHigh == +BitMapWidth  / 2) &&
            (PrinterAreaYLow  == -BitMapHeight / 2) &&
            (PrinterAreaYHigh == +BitMapHeight / 2)) PrinterCustomFlag = 0; else PrinterCustomFlag = 1;
      }

   return (UNBOUND);
   }

NODE *lactivearea(void)
   {
   // if custom return settings

   return (
      cons(make_intnode((FIXNUM) PrinterAreaXLow),
         cons(make_intnode((FIXNUM) PrinterAreaYLow),
            cons(make_intnode((FIXNUM) PrinterAreaXHigh),
               cons(make_intnode((FIXNUM) PrinterAreaYHigh),
                  NIL
               )))));

   }


DEFINE_RESPONSE_TABLE1(TMyPrinterAreaWindow, TDialog)
EV_CHILD_NOTIFY_ALL_CODES(ID_RESETEXTENT, DoReset),
END_RESPONSE_TABLE;


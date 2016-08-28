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

/* status window members */

TMyStatusWindow::TMyStatusWindow(TWindow *AParent, LPSTR ATitle)
: TDialog(AParent, ATitle)
   {
   }

TMyStatusWindow::~TMyStatusWindow()
   {
   }

void update_status_turtleposition(void)
   {
   sprintf(YABuffer, "%1.0f,%1.0f,%1.0f", turtle_p[turtle_which].x, turtle_p[turtle_which].y, turtle_p[turtle_which].z);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_TURTLEPOSITION, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_pencolor(void)
   {
   sprintf(YABuffer, "%d,%d,%d", (int) dpenr, (int) dpeng, (int) dpenb);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_PENCOLOR, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_floodcolor(void)
   {
   sprintf(YABuffer, "%d,%d,%d", (int) dfldr, (int) dfldg, (int) dfldb);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_FLOODCOLOR, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_screencolor(void)
   {
   sprintf(YABuffer, "%d,%d,%d", (int) dscnr, (int) dscng, (int) dscnb);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_SCREENCOLOR, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_paletteuse(void)
   {
   if (EnablePalette)
      {
      sprintf(YABuffer, "%d", MyLogPalette->palNumEntries);
      }
   else
      {
      sprintf(YABuffer, "%s", "N/A");
      }
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_PALETTEUSE, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_penwidth(void)
   {
   sprintf(YABuffer, "%d", width);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_PENWIDTH, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_turtleheading(void)
   {
   if (current_mode == perspectivemode)
      sprintf(YABuffer, "%1.2f", rotation_z());
   else
      sprintf(YABuffer, "%1.2f", turtle_heading[turtle_which]);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_TURTLEHEADING, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_penstyle(void)
   {
   if (current_write_mode == XOR_PUT) sprintf(YABuffer, "Reverse"); else sprintf(YABuffer, "Normal");
   if (in_erase_mode) sprintf(YABuffer, "Erase");
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_PENSTYLE, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_pencontact(void)
   {
   if (pen_vis == 0) sprintf(YABuffer, "Down"); else sprintf(YABuffer, "Up");
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_PENCONTACT, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_turtlevisability(void)
   {
   if (turtle_shown[turtle_which]) sprintf(YABuffer, "Shown"); else sprintf(YABuffer, "Hidden");
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_TURTLEVISABILITY, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_turtlepitch(void)
   {
   if (current_mode == perspectivemode)
      sprintf(YABuffer, "%1.2f", rotation_x());
   else
      sprintf(YABuffer, "%1.2f", 0.0);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_TURTLEPITCH, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_turtleroll(void)
   {
   if (current_mode == perspectivemode)
      sprintf(YABuffer, "%1.2f", rotation_y());
   else
      sprintf(YABuffer, "%1.2f", 0.0);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_TURTLEROLL, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_turtlewhich(void)
   {
   if (turtle_which >= TURTLES - TURTLEN)
      sprintf(YABuffer, "%ld", (long) (-(turtle_which - (TURTLES - (TURTLEN+1)))));
   else
      sprintf(YABuffer, "%ld", (long) turtle_which);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_TURTLEWHICH, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_evals(void)
   {
   sprintf(YABuffer, "%ld", eval_count);
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_EVALS, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_vectors(void)
   {
   if (ThreeD.iPolyCount)
		{
#ifdef NOASM
		sprintf(YABuffer, "%ld/%ld", ThreeD.iPolyCount, ThreeD.iSplitPolyCount);
#else
		sprintf(YABuffer, "%ld", ThreeD.iPolyCount);
#endif
      MainWindowx->StatusWindow->SendDlgItemMsg(ID_THINGS, WM_SETTEXT, 0, (DWORD) (LPCSTR) "Polygons:");
		}
	else
		{
		sprintf(YABuffer, "%ld", vector_count);
      MainWindowx->StatusWindow->SendDlgItemMsg(ID_THINGS, WM_SETTEXT, 0, (DWORD) (LPCSTR) "Vectors:");
		}

   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_VECTORS, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

void update_status_memory(void)
   {
   sprintf(YABuffer, "%ld Nodes", (long) ((long) memory_count * (long) SEG_SIZE));
   MainWindowx->StatusWindow->
      SendDlgItemMsg(ID_MEMORY, WM_SETTEXT, 0, (DWORD) YABuffer);
   }

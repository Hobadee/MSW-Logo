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

extern COLORREF colortable[];
extern BOOL bIndexMode;

extern HBITMAP DIBToBitmap(HANDLE hDIB, HPALETTE hPal);
extern HANDLE BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal);
extern int gbmBmpToGif(char *BmpName, char *GifName);
extern int gbmGifToBmp(char *GifName, char *BmpName);

char *tempfont;
int found;
int printflag;
long bitmode = SRCCOPY;
int LL = 1;
int iDelay;
int bAppendMode;
int iLoop;
int iTrans;

void gifsave_helper(char *textbuf, int iDelay_, int bAppendMode_, int iLoop_, int iTrans_, int iMaxColorDepth)
   {
#if !defined(__WIN32__)
   MainWindowx->CommandWindow->MessageBox("GIFSAVE is only supported in 32bit versions of MSWLogo", "Error");
   err_logo(STOP_ERROR, NIL);
#else
   iDelay      = iDelay_;
   bAppendMode = bAppendMode_;
   iLoop       = iLoop_;
	iTrans      = iTrans_;
   
   MainWindowx->DumpBitmapFile(TempBmpName, iMaxColorDepth);
   lsetcursorwait();
   if (gbmBmpToGif(TempBmpName, textbuf) != 0)
      {
      MainWindowx->CommandWindow->MessageBox("Problem Generating GIF, check memory and diskspace", "Error");
      err_logo(STOP_ERROR, NIL);
      }
   lsetcursorarrow();
   unlink(TempBmpName);
#endif
   }

COLORREF GetRGBorIndexColor(NODE* &args)
	{
   NODE *cnode;
   COLORREF color;
   int icolor;
   NODE *arg;

   color = (COLORREF) -1;

   // get args
	
   if (is_list(car(args)))
		{
		
		arg = pos_int_vector_3_arg(args);
		
		if (NOT_THROWING)
			{
			if (EnablePalette)
				{
				color = LoadColor(
					((nodetype(     car(arg) ) == FLOAT) ? (FIXNUM) getfloat(     car(arg) ) : getint(     car(arg)) ),
					((nodetype(    cadr(arg) ) == FLOAT) ? (FIXNUM) getfloat(    cadr(arg) ) : getint(    cadr(arg)) ),
					((nodetype(cadr(cdr(arg))) == FLOAT) ? (FIXNUM) getfloat(cadr(cdr(arg))) : getint(cadr(cdr(arg)))));
				}
			else
				{
				color = RGB(
					((nodetype(     car(arg) ) == FLOAT) ? (FIXNUM) getfloat(     car(arg) ) : getint(     car(arg)) ),
					((nodetype(    cadr(arg) ) == FLOAT) ? (FIXNUM) getfloat(    cadr(arg) ) : getint(    cadr(arg)) ),
					((nodetype(cadr(cdr(arg))) == FLOAT) ? (FIXNUM) getfloat(cadr(cdr(arg))) : getint(cadr(cdr(arg)))));
				}
			}

      bIndexMode = FALSE;
		}
	else
		{
		cnode = numeric_arg(args);
		
		if (NOT_THROWING)
			{
			icolor = (nodetype(cnode) == FLOAT) ? (FIXNUM) getfloat(cnode) : getint(cnode);
			icolor = icolor % 16;
			if (EnablePalette)
				{
				color = LoadColor(GetRValue(colortable[icolor]), GetGValue(colortable[icolor]), GetBValue(colortable[icolor]));
				}
			else
				{
				color = colortable[icolor];
				}
			}

      bIndexMode = TRUE;
		}

	return color;
	}

NODE *lgifsave(NODE *args)
   {
   char textbuf[MAX_BUFFER_SIZE];
   NODE *val1;
   NODE *val3;
   NODE *val4;
   int iMaxColorDepth;
   int iDelay;
   int bAppendMode;
   int iLoop;
   int iTrans;

   /* same as BITMAP-SAVE but gets file name from logo command */

   cnv_strnode_string(textbuf, args);

   /* check for optional callback routine */

   iDelay = -1;
   bAppendMode = 0;
   iLoop = -1;
   iMaxColorDepth = 8;
	iTrans = -1;

   if (cdr(args) != NIL)
      {
      val1 = integer_arg(cdr(args));
      if (NOT_THROWING)
         {
         iDelay = getint(val1);
         if (cdr(cdr(args)) != NIL)
            {
            bAppendMode = torf_arg(cdr(cdr(args)));
            if (NOT_THROWING)
               {
               if (cdr(cdr(cdr(args))) != NIL)
                  {
                  val3 = integer_arg(cdr(cdr(cdr(args))));
                  if (NOT_THROWING)
                     {
                     iLoop = getint(val3);
                     if (cdr(cdr(cdr(cdr(args)))) != NIL)
                        {
                        val4 = integer_arg(cdr(cdr(cdr(cdr(args)))));
                        if (NOT_THROWING)
                           {
                           iMaxColorDepth = getint(val4);
                           if ((iMaxColorDepth != 4) && (iMaxColorDepth != 1))
										iMaxColorDepth = 8;
									if (cdr(cdr(cdr(cdr(cdr(args))))) != NIL)
										{
										iTrans = GetRGBorIndexColor(cdr(cdr(cdr(cdr(cdr(args))))));
										}
                           }
                        }
                     }
                  }
               }
            }
         }
      }

   if (NOT_THROWING)
      {
      gifsave_helper(textbuf, iDelay, bAppendMode, iLoop, iTrans, iMaxColorDepth);
      }

   return (UNBOUND);
   }

NODE *lbitsave(NODE *args)
   {
   char textbuf[MAX_BUFFER_SIZE];
   int iMaxBitCount;
   NODE *val1;

   /* same as BITMAP-SAVE but gets file name from logo command */

   cnv_strnode_string(textbuf, args);

   if (NOT_THROWING)
      {
      iMaxBitCount = 32;
      
      if (cdr(args) != NIL)
         {
         val1 = integer_arg(cdr(args));
         if (NOT_THROWING)
            {
            iMaxBitCount = getint(val1);
            }
         }
      
      MainWindowx->DumpBitmapFile(textbuf, iMaxBitCount);
      }

   return (UNBOUND);
   }

void gifload_helper(char *textbuf, DWORD &dwPixelWidth, DWORD &dwPixelHeight)
   {
#if !defined(__WIN32__)
   MainWindowx->CommandWindow->MessageBox("GIFLOAD is only supported in 32bit versions of MSWLogo", "Error");
   err_logo(STOP_ERROR, NIL);
#else
   lsetcursorwait();
   if (gbmGifToBmp(textbuf, TempBmpName) != 0)
      {
      MainWindowx->CommandWindow->MessageBox("Problem Reading GIF, check memory and diskspace", "Error");
      err_logo(STOP_ERROR, NIL);
      }
   lsetcursorarrow();
   MainWindowx->LoadBitmapFile(TempBmpName, dwPixelWidth, dwPixelHeight);
   unlink(TempBmpName);
#endif
   }

NODE *lgifload(NODE *arg)
   {
   char textbuf[MAX_BUFFER_SIZE];
   DWORD dwPixelWidth = 1;
   DWORD dwPixelHeight = 1;
   
   /* same as BITMAP-SAVE but gets file name from logo command */

   cnv_strnode_string(textbuf, arg);
   
   if (NOT_THROWING)
      {
      gifload_helper(textbuf, dwPixelWidth, dwPixelHeight);
      }
   
   return (UNBOUND);
   }

NODE *lgifsize(NODE *args)
   {
   char textbuf[MAX_BUFFER_SIZE];
   DWORD dwPixelWidth = 0;
   DWORD dwPixelHeight = 0;
   
   /* same as BITMAP-SAVE but gets file name from logo command */
   
   cnv_strnode_string(textbuf, args);
   
   if (NOT_THROWING)
      {
      gifload_helper(textbuf, dwPixelWidth, dwPixelHeight);
      
      return (
         cons(make_intnode((FIXNUM) dwPixelWidth),
         cons(make_intnode((FIXNUM) dwPixelHeight),
         NIL
         )));
      }
   
   return (UNBOUND);
   }

NODE *lbitload(NODE *arg)
   {
   char textbuf[MAX_BUFFER_SIZE];
   DWORD dwPixelWidth = 1;
   DWORD dwPixelHeight = 1;

   /* same as BITMAP-LOAD except callable from logo command */

   cnv_strnode_string(textbuf, arg);

   if (NOT_THROWING)
      {
      MainWindowx->LoadBitmapFile(textbuf, dwPixelWidth, dwPixelHeight);
      }

   return (UNBOUND);
   }

NODE *lbitloadsize(NODE *arg)
   {
   char textbuf[MAX_BUFFER_SIZE];
   DWORD dwPixelWidth = 0;
   DWORD dwPixelHeight = 0;

   /* same as BITMAP-LOAD except callable from logo command */

   cnv_strnode_string(textbuf, arg);

   if (NOT_THROWING)
      {
      MainWindowx->LoadBitmapFile(textbuf, dwPixelWidth, dwPixelHeight);
      
      return (
         cons(make_intnode((FIXNUM) dwPixelWidth),
         cons(make_intnode((FIXNUM) dwPixelHeight),
         NIL
         )));
      }

   return (UNBOUND);
   }

NODE *lbitsize(void)
   {
   BITMAP temp;
	
   HANDLE hDIB;
	HBITMAP hMemoryBitMap;
   LPBITMAPINFOHEADER lpDIBHdr;
	
	temp.bmWidth  = 0;
	temp.bmHeight = 0;
	
	// If ClipBoard check with ClipBoard only
	
	if (CutIndex == 0)
		{
		
		OpenClipboard(MainHWindow);
		
		// Try a DIB first
		
		hDIB = (HBITMAP) GetClipboardData(CF_DIB);
		
		// If it exists get the size
		
		if (hDIB)
			{
			lpDIBHdr = (LPBITMAPINFOHEADER) GlobalLock(hDIB);

			if (lpDIBHdr)
				{
				temp.bmWidth  = lpDIBHdr->biWidth;
				temp.bmHeight = lpDIBHdr->biHeight;
				GlobalUnlock(hDIB);
				}
			}
		
		// else try for a bitmap
		
		else
			{
			hMemoryBitMap = (HBITMAP) GetClipboardData(CF_BITMAP);
			
		   // If it exists get the size
					
			if (hMemoryBitMap)
				{
				GetObject(hMemoryBitMap, sizeof(BITMAP), (LPSTR) &temp);
				}
			}
		
		// we have everything we need
		
		CloseClipboard();
		
		}
	else
		{
		// if we have something fetch its size
		
		if (CutBmp[CutIndex].CutFlag)
			{
			GetObject(CutBmp[CutIndex].CutMemoryBitMap, sizeof(BITMAP), (LPSTR) &temp);
			}
		}
	
	return (
		cons(make_intnode((FIXNUM) temp.bmWidth),
		cons(make_intnode((FIXNUM) temp.bmHeight),
		NIL
		)));
   }

NODE *lsetpixel(NODE *args)
   {
   HDC ScreenDC;
   HDC MemDC;
   NODE *arg;
   NODE *cnode;
   COLORREF color;
   int icolor;

   POINT dest;

   if (current_mode == perspectivemode)
      {
      VECTOR from3d;

      from3d.x = turtle_p[turtle_which].x / WorldWidth;
      from3d.y = turtle_p[turtle_which].y / WorldHeight;
      from3d.z = turtle_p[turtle_which].z / WorldDepth;

      if (!ThreeD.TransformPoint(from3d, dest)) return (UNBOUND);
      }
   else
      {
      dest.x = g_round(turtle_p[turtle_which].x);
      dest.y = g_round(turtle_p[turtle_which].y);
      }

   // get args
	
   if (is_list(car(args)))
		{
		
		arg = pos_int_vector_3_arg(args);
		
		if (NOT_THROWING)
			{
			if (EnablePalette)
				{
				color = LoadColor(
					((nodetype(     car(arg) ) == FLOAT) ? (FIXNUM) getfloat(     car(arg) ) : getint(     car(arg)) ),
					((nodetype(    cadr(arg) ) == FLOAT) ? (FIXNUM) getfloat(    cadr(arg) ) : getint(    cadr(arg)) ),
					((nodetype(cadr(cdr(arg))) == FLOAT) ? (FIXNUM) getfloat(cadr(cdr(arg))) : getint(cadr(cdr(arg)))));
				}
			else
				{
				color = RGB(
					((nodetype(     car(arg) ) == FLOAT) ? (FIXNUM) getfloat(     car(arg) ) : getint(     car(arg)) ),
					((nodetype(    cadr(arg) ) == FLOAT) ? (FIXNUM) getfloat(    cadr(arg) ) : getint(    cadr(arg)) ),
					((nodetype(cadr(cdr(arg))) == FLOAT) ? (FIXNUM) getfloat(cadr(cdr(arg))) : getint(cadr(cdr(arg)))));
				}
			}

      bIndexMode = FALSE;
		}
	else
		{
		cnode = numeric_arg(args);
		
		if (NOT_THROWING)
			{
			icolor = (nodetype(cnode) == FLOAT) ? (FIXNUM) getfloat(cnode) : getint(cnode);
			icolor = icolor % 16;
			if (EnablePalette)
				{
				color = LoadColor(GetRValue(colortable[icolor]), GetGValue(colortable[icolor]), GetBValue(colortable[icolor]));
				}
			else
				{
				color = colortable[icolor];
				}
			}

      bIndexMode = TRUE;
		}
	
	if (NOT_THROWING)
		{

      ScreenDC = GetDC(MainHWindow);

      // memory

      MemDC = CreateCompatibleDC(ScreenDC);
      OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

      if (EnablePalette)
         {
         OldPalette = SelectPalette(MemDC, ThePalette, FALSE);
         RealizePalette(MemDC);
         }

      SetPixel(MemDC,
      +dest.x + xoffset,
      -dest.y + yoffset,
      color);

      SelectObject(MemDC, OldBitmap);
      if (EnablePalette)
         {
         SelectPalette(MemDC, OldPalette, FALSE);
         }
      DeleteDC(MemDC);

      //screen

      draw_turtle(0);

      if (EnablePalette)
         {
         OldPalette2 = SelectPalette(ScreenDC, ThePalette, FALSE);
         RealizePalette(ScreenDC);
         }

      if (zoom_flag)
         {
         TRect TempRect;

         SetRect(&TempRect,
            (+dest.x - MainWindowx->Scroller->XPos / the_zoom + xoffset) * the_zoom,
            (-dest.y - MainWindowx->Scroller->YPos / the_zoom + yoffset) * the_zoom,
            (+dest.x - MainWindowx->Scroller->XPos / the_zoom + xoffset) * the_zoom,
            (-dest.y - MainWindowx->Scroller->YPos / the_zoom + yoffset) * the_zoom);

         TempRect.Inflate(1+the_zoom,1+the_zoom);

         InvalidateRect(MainHWindow, &TempRect, FALSE);
         }
      else
         {
         SetPixel(ScreenDC,
            +dest.x - MainWindowx->Scroller->XPos + xoffset,
            -dest.y - MainWindowx->Scroller->YPos + yoffset,
            color);
         }

      if (EnablePalette)
         {
         SelectPalette(ScreenDC, OldPalette2, FALSE);
         }

      ReleaseDC(MainHWindow, ScreenDC);

      draw_turtle(1);

      }

   return (UNBOUND);
   }

int getindexcolor(COLORREF color)
   {
   for (int i=0;i<16;i++) if (color == colortable[i]) return i;
   return -1;
   }

// function that returns the RGB vector of the pixel the turtle is on top of

NODE *lpixel()
   {
   HDC MemDC;
   HDC ScreenDC;
   COLORREF the_color;

   POINT dest;

   if (current_mode == perspectivemode)
      {
      VECTOR from3d;

      from3d.x = turtle_p[turtle_which].x / WorldWidth;
      from3d.y = turtle_p[turtle_which].y / WorldHeight;
      from3d.z = turtle_p[turtle_which].z / WorldDepth;

      if (!ThreeD.TransformPoint(from3d, dest))
         {
         return (
            cons(make_intnode((FIXNUM) - 1),
               cons(make_intnode((FIXNUM) - 1),
                  cons(make_intnode((FIXNUM) - 1),
                     NIL
                  ))));
         }
      }
   else
      {
      dest.x = g_round(turtle_p[turtle_which].x);
      dest.y = g_round(turtle_p[turtle_which].y);
      }

   ScreenDC = GetDC(MainHWindow);

   // memory

   MemDC = CreateCompatibleDC(ScreenDC);
   OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

   if (EnablePalette)
      {
      OldPalette = SelectPalette(MemDC, ThePalette, FALSE);
      RealizePalette(MemDC);
      }

   the_color = GetPixel(MemDC, dest.x + xoffset, -dest.y + yoffset);

   if (EnablePalette)
      {
      SelectPalette(MemDC, OldPalette, FALSE);
      }

   DeleteDC(MemDC);
   ReleaseDC(MainHWindow, ScreenDC);

   if (bIndexMode)
      {
      int icolor = getindexcolor(the_color);
      if (icolor >= 0) return(make_intnode(icolor));
      }

	return (
      cons(make_intnode((FIXNUM) GetRValue(the_color)),
		cons(make_intnode((FIXNUM) GetGValue(the_color)),
		cons(make_intnode((FIXNUM) GetBValue(the_color)),
		NIL
		))));
   }

void logofill(BOOL bOld)
   {
   HDC ScreenDC;
   HDC MemDC;
   HBRUSH JunkBrush;

   POINT dest;

   if (current_mode == perspectivemode)
      {
      VECTOR from3d;

      from3d.x = turtle_p[turtle_which].x / WorldWidth;
      from3d.y = turtle_p[turtle_which].y / WorldHeight;
      from3d.z = turtle_p[turtle_which].z / WorldDepth;

      if (!ThreeD.TransformPoint(from3d, dest)) return;
      }
   else
      {
      dest.x = g_round(turtle_p[turtle_which].x);
      dest.y = g_round(turtle_p[turtle_which].y);
      }

   ScreenDC = GetDC(MainHWindow);

   JunkBrush = CreateBrushIndirect(&FloodBrush);

   // memory

   MemDC = CreateCompatibleDC(ScreenDC);
   OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

   if (EnablePalette)
      {
      OldPalette = SelectPalette(MemDC, ThePalette, FALSE);
      RealizePalette(MemDC);
      }

   SetTextColor(MemDC, pcolor);

   OldBrush = (HBRUSH) SelectObject(MemDC, JunkBrush);

//#if !defined(__WIN32__)
//   FloodFill(MemDC,
//      +dest.x + xoffset,
//      -dest.y + yoffset, pcolor);
//#else
   if (bOld)
		{
		ExtFloodFill(MemDC,
			+dest.x + xoffset,
			-dest.y + yoffset, pcolor, FLOODFILLBORDER);
		}
   else
		{
		COLORREF tcolor = GetPixel(MemDC, dest.x + xoffset, -dest.y + yoffset) | 0x02000000;
		ExtFloodFill(MemDC,
			+dest.x + xoffset,
			-dest.y + yoffset, tcolor, FLOODFILLSURFACE);
		}
//#endif

   if (EnablePalette)
      {
      SelectPalette(MemDC, OldPalette, FALSE);
      }

   SelectObject(MemDC, OldBrush);
   SelectObject(MemDC, OldBitmap);

   DeleteDC(MemDC);
   DeleteObject(JunkBrush);
   ReleaseDC(MainHWindow, ScreenDC);
   }

NODE *lpencolor()
   {
   if (bIndexMode)
      {
      int icolor = getindexcolor(RGB(dpenr, dpeng, dpenb));
      if (icolor >= 0) return(make_intnode(icolor));
      }
      
   return (
      cons(make_intnode((FIXNUM) dpenr),
      cons(make_intnode((FIXNUM) dpeng),
      cons(make_intnode((FIXNUM) dpenb),
      NIL
      ))));
   }

// funtion to set the pen color while updating palette if need be

void thepencolor(int r, int g, int b)
   {

   dpenr = r;
   dpeng = g;
   dpenb = b;

   if (EnablePalette)
      {
      pcolor = LoadColor((int) dpenr, (int) dpeng, (int) dpenb);
      }
   else
      {
      pcolor = RGB(dpenr, dpeng, dpenb);
      }

   NormalPen.lopnStyle = PS_INSIDEFRAME;
   NormalPen.lopnWidth.x = width;
   NormalPen.lopnColor = pcolor;
   }

// function to return flood color as a RGB list

NODE *lfloodcolor()
   {
   if (bIndexMode)
      {
      int icolor = getindexcolor(RGB(dfldr, dfldg, dfldb));
      if (icolor >= 0) return(make_intnode(icolor));
      }
      
   return (
      cons(make_intnode((FIXNUM) dfldr),
      cons(make_intnode((FIXNUM) dfldg),
      cons(make_intnode((FIXNUM) dfldb),
      NIL
      ))));
   }

// funtion to set the flood color while updating palette if need be

void thefloodcolor(int r, int g, int b)
   {

   dfldr = r;
   dfldg = g;
   dfldb = b;

   if (EnablePalette)
      {
      fcolor = LoadColor((int) dfldr, (int) dfldg, (int) dfldb);
      }
   else
      {
      fcolor = RGB(dfldr, dfldg, dfldb);
      }

   FloodBrush.lbStyle = BS_SOLID;
   FloodBrush.lbColor = fcolor;
   FloodBrush.lbHatch = HS_VERTICAL;
   }

// function to return screen color as a RGB list

NODE *lscreencolor()
   {
   if (bIndexMode)
      {
      int icolor = getindexcolor(RGB(dscnr, dscng, dscnb));
      if (icolor >= 0) return(make_intnode(icolor));
      }
   
   return (
      cons(make_intnode((FIXNUM) dscnr),
      cons(make_intnode((FIXNUM) dscng),
      cons(make_intnode((FIXNUM) dscnb),
      NIL
      ))));
   }

// funtion to set the screen color while updating palette if need be

void thescreencolor(int r, int g, int b)
   {
   HDC ScreenDC;
   HDC MemDC;

   HBRUSH TempBrush;

   dscnr = r;
   dscng = g;
   dscnb = b;

   if (EnablePalette)
      {
      scolor = LoadColor((int) dscnr, (int) dscng, (int) dscnb);
      }
   else
      {
      scolor = RGB(dscnr, dscng, dscnb);
      }

   ScreenBrush.lbStyle = BS_SOLID;
   ScreenBrush.lbColor = scolor;
   ScreenBrush.lbHatch = HS_VERTICAL;

   // When the screen changes we change the erase pen which basically
   // writes the screen color

   ErasePen.lopnStyle = PS_INSIDEFRAME;
   ErasePen.lopnWidth.x = width;
   ErasePen.lopnColor = scolor;

   TempBrush = CreateBrushIndirect(&ScreenBrush);

   ScreenDC = GetDC(MainHWindow);

   // memory

   MemDC = CreateCompatibleDC(ScreenDC);
   OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

   if (EnablePalette)
      {
      OldPalette = SelectPalette(MemDC, ThePalette, FALSE);
      RealizePalette(MemDC);
      }

   FillRect(MemDC, &FullRect, TempBrush);

   if (EnablePalette)
      {
      SelectPalette(MemDC, OldPalette, FALSE);
      }

   SelectObject(MemDC, OldBitmap);
   DeleteDC(MemDC);
   ReleaseDC(MainHWindow, ScreenDC);

   DeleteObject(TempBrush);

   ::InvalidateRect(MainWindowx->HWindow, NULL, TRUE);
   }

int get_ibm_pen_width()
   {
   int w;

   w = width;

   return (w);
   }

void set_ibm_pen_width(int w)
   {
   width = w;

   // we erase with the same pen width as we write

   NormalPen.lopnStyle = PS_INSIDEFRAME;
   NormalPen.lopnWidth.x = width;
   NormalPen.lopnColor = pcolor;

   ErasePen.lopnStyle = PS_INSIDEFRAME;
   ErasePen.lopnWidth.x = width;
   ErasePen.lopnColor = scolor;
   }

NODE *lclearpalette(void)
   {

   // kill the palette and recreate it with just black and white

   if (NOT_THROWING)
      {
      if (EnablePalette)
         {
         DeleteObject(ThePalette);

         MyLogPalette->palNumEntries = 2;

         if (status_flag) update_status_paletteuse();

         ThePalette = CreatePalette(MyLogPalette);

         ::InvalidateRect(MainWindowx->HWindow, NULL, TRUE);
         }
      }

   return (UNBOUND);
   }

NODE *lstatus(void)
   {

   // if status not running then run it

   if (!status_flag)
      {
      MainWindowx->MyPopupStatus();
      JustDidEdit = 1;
      }
   return (UNBOUND);
   }

NODE *lnostatus(void)
   {

   // if running then kill it

   if (status_flag)
      {
      MainWindowx->MyPopupStatusKill();
      }
   return (UNBOUND);
   }


void UpdateZoomControlFlag()
   {
   if ((the_zoom != 1.0) || CheckOnScreenControls())
      {
      zoom_flag = 1;
      }
   else
      {
      zoom_flag = 0;
      }

   if (the_zoom != 1.0)
      {
      MainWindowx->Scroller->TrackMode = FALSE;
      }
   else
      {
      MainWindowx->Scroller->TrackMode = TRUE;
      }
   }

// The real work of zooming is done in Paint this just adjusts the scroller
// to something reasonable so that what was basically in the center of the
// screen still is. It also readjusts the ranges on the scrollers.

void zoom_helper(NUMBER temp_zoom)
   {
   NUMBER XRatio;
   NUMBER YRatio;

   int Xr;
   int Yr;

   TRect MainRect;

   if (the_zoom != temp_zoom)
      {

      the_zoom = temp_zoom;

      MainWindowx->GetClientRect(MainRect);

      // calculate new scroller ranges

      Xr = (BitMapWidth * the_zoom) - MainRect.right;
      Yr = (BitMapHeight * the_zoom) - MainRect.bottom;

      if (Xr < 0) Xr = 0;
      if (Yr < 0) Yr = 0;

      // find out where we are (percentage of pos/range for x and y)

      if (MainWindowx->Scroller->XRange <= 0)
         {
         XRatio = 0.5;
         }
      else
         {
         XRatio = (NUMBER) MainWindowx->Scroller->XPos / (NUMBER) MainWindowx->Scroller->XRange;
         }

      if (MainWindowx->Scroller->YRange <= 0)
         {
         YRatio = 0.5;
         }
      else
         {
         YRatio = (NUMBER) MainWindowx->Scroller->YPos / (NUMBER) MainWindowx->Scroller->YRange;
         }

      // set the new ranges

      MainWindowx->Scroller->SetRange(Xr, Yr);

      // Position to the same percentage down the scroll bars as before

      MainWindowx->Scroller->ScrollTo(XRatio * Xr, YRatio * Yr);

      // hide turtle while we do this

      draw_turtle(0);

      UpdateZoomControlFlag();
      
      draw_turtle(1);

      // paint

      ::InvalidateRect(MainWindowx->HWindow, NULL, TRUE);

      }

   }

NODE *lzoom(NODE *arg)
   {
   NUMBER temp_zoom;

   NODE *val;

   // get arg

   val = numeric_arg(arg);

   if (nodetype(val) == INT) temp_zoom = (NUMBER) getint(val);
   else temp_zoom = getfloat(val);

   if (temp_zoom <= 0.0) temp_zoom = 1.0;

   zoom_helper(temp_zoom);

   return (UNBOUND);
   }


NODE *lbitblock(NODE *arg)
   {
   HDC ScreenDC;
   HDC MemDC;
   HBRUSH TempBrush;
   RECT TempRect;
   int CutWidth;
   int CutHeight;

   POINT dest;

   if (current_mode == perspectivemode)
      {
      VECTOR from3d;

      from3d.x = turtle_p[turtle_which].x / WorldWidth;
      from3d.y = turtle_p[turtle_which].y / WorldHeight;
      from3d.z = turtle_p[turtle_which].z / WorldDepth;

      if (!ThreeD.TransformPoint(from3d, dest)) return (UNBOUND);
      }
   else
      {
      dest.x = g_round(turtle_p[turtle_which].x);
      dest.y = g_round(turtle_p[turtle_which].y);
      }

   // get args

   CutWidth = getint(pos_int_arg(arg));
   CutHeight = getint(pos_int_arg(cdr(arg)));

   if (NOT_THROWING)
      {

      // only if a surface was specified continue or UAEs big time

      if ((CutWidth != 0) && (CutHeight != 0))
         {

         TempBrush = CreateBrushIndirect(&FloodBrush);

         ScreenDC = GetDC(MainHWindow);

         // memory

         MemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

         if (EnablePalette)
            {
            OldPalette = SelectPalette(MemDC, ThePalette, FALSE);
            RealizePalette(MemDC);
            }

         SetRect(&TempRect,
            +dest.x + xoffset,
            -dest.y + yoffset + LL - CutHeight,
            +dest.x + xoffset + CutWidth,
            -dest.y + yoffset + LL);

         FillRect(MemDC, &TempRect, TempBrush);

         SelectObject(MemDC, OldBitmap);
         if (EnablePalette)
            {
            SelectPalette(MemDC, OldPalette, FALSE);
            }
         DeleteDC(MemDC);

         //screen

         draw_turtle(0);

         if (EnablePalette)
            {
            OldPalette2 = SelectPalette(ScreenDC, ThePalette, FALSE);
            RealizePalette(ScreenDC);
            }

         if (zoom_flag)
            {
//            TRect temp;
//
//            temp.Set(
//               (+dest.x - MainWindowx->Scroller->XPos / the_zoom + xoffset                 ) * the_zoom,
//               (-dest.y - MainWindowx->Scroller->YPos / the_zoom + yoffset + LL - CutHeight) * the_zoom,
//               (+dest.x - MainWindowx->Scroller->XPos / the_zoom + xoffset + CutWidth      ) * the_zoom,
//               (-dest.y - MainWindowx->Scroller->YPos / the_zoom + yoffset + LL            ) * the_zoom);
//            
//            temp.Normalize();
//            temp.Inflate(1+the_zoom,1+the_zoom);
            
            InvalidateRect(MainHWindow,  /* &temp */ NULL, FALSE);
            }
         else
            {
            SetRect(&TempRect,
               +dest.x - MainWindowx->Scroller->XPos + xoffset,
               -dest.y - MainWindowx->Scroller->YPos + yoffset + LL - CutHeight,
               +dest.x - MainWindowx->Scroller->XPos + xoffset + CutWidth,
               -dest.y - MainWindowx->Scroller->YPos + yoffset + LL);
            
            FillRect(ScreenDC, &TempRect, TempBrush);
            }

         if (EnablePalette)
            {
            SelectPalette(ScreenDC, OldPalette2, FALSE);
            }

         ReleaseDC(MainHWindow, ScreenDC);

         DeleteObject(TempBrush);

         draw_turtle(1);

         }

      }
   return (UNBOUND);
   }

NODE *lbitmode(void)
   {
   int temp;

   // return the logo "code" for the bit mode

   switch (bitmode)
      {
       case SRCCOPY: temp = 1; break;
       case SRCPAINT: temp = 2; break;
       case SRCAND: temp = 3; break;
       case SRCINVERT: temp = 4; break;
       case SRCERASE: temp = 5; break;
       case NOTSRCCOPY: temp = 6; break;
       case NOTSRCERASE: temp = 7; break;
       case MERGEPAINT: temp = 8; break;
       case DSTINVERT: temp = 9; break;
      }

   return (make_intnode((FIXNUM) temp));
   }

NODE *lsetbitmode(NODE *arg)
   {

   // convert from logo "code" to Windows constants

   switch (int_arg(arg))
      {
       case 1: bitmode = SRCCOPY; break;
       case 2: bitmode = SRCPAINT; break;
       case 3: bitmode = SRCAND; break;
       case 4: bitmode = SRCINVERT; break;
       case 5: bitmode = SRCERASE; break;
       case 6: bitmode = NOTSRCCOPY; break;
       case 7: bitmode = NOTSRCERASE; break;
       case 8: bitmode = MERGEPAINT; break;
       case 9: bitmode = DSTINVERT; break;
       default:
           {
              MainWindowx->CommandWindow->MessageBox("Illegal Bitmode", "Error");
              err_logo(STOP_ERROR, NIL);
           }
      }

   return (UNBOUND);
   }

NODE *lturtlemode(void)
   {
   int temp;

   // return the logo "code" for the bit mode

   switch (turtle_bitmap[turtle_which])
      {
       case 0:
       case SRCCOPY: temp = 1; break;
       case SRCPAINT: temp = 2; break;
       case SRCAND: temp = 3; break;
       case SRCINVERT: temp = 4; break;
       case SRCERASE: temp = 5; break;
       case NOTSRCCOPY: temp = 6; break;
       case NOTSRCERASE: temp = 7; break;
       case MERGEPAINT: temp = 8; break;
       case DSTINVERT: temp = 9; break;
      }

   return (make_intnode((FIXNUM) temp));
   }

NODE *lsetturtlemode(NODE *arg)
   {

   // convert from logo "code" to Windows constants

   if (turtle_bitmap[turtle_which])
      {
      prepare_to_draw;
      draw_turtle(0);

      switch (int_arg(arg))
         {
          case 1: turtle_bitmap[turtle_which] = SRCCOPY; break;
          case 2: turtle_bitmap[turtle_which] = SRCPAINT; break;
          case 3: turtle_bitmap[turtle_which] = SRCAND; break;
          case 4: turtle_bitmap[turtle_which] = SRCINVERT; break;
          case 5: turtle_bitmap[turtle_which] = SRCERASE; break;
          case 6: turtle_bitmap[turtle_which] = NOTSRCCOPY; break;
          case 7: turtle_bitmap[turtle_which] = NOTSRCERASE; break;
          case 8: turtle_bitmap[turtle_which] = MERGEPAINT; break;
          case 9: turtle_bitmap[turtle_which] = DSTINVERT; break;
          default:
              {
                 MainWindowx->CommandWindow->MessageBox("Illegal Bitmode", "Error");
                 err_logo(STOP_ERROR, NIL);
              }
         }

      draw_turtle(1);
      done_drawing;
      }

   return (UNBOUND);
   }

NODE *lbitindex(void)
   {
   // return the current bitmap index

   return (make_intnode((FIXNUM) CutIndex));
   }

NODE *lsetbitindex(NODE *arg)
   {
   int i;

   // set the current bitmap index if within range

   i = getint(pos_int_arg(arg));

   if (i < MaxBitCuts)
      {
      CutIndex = i;
      }
   else
      {
      MainWindowx->CommandWindow->MessageBox("BitMap Index out of range", "Error");
      err_logo(STOP_ERROR, NIL);
      }

   return (UNBOUND);
   }

NODE *lbitcut(NODE *arg)
   {
   HDC ScreenDC;
   HDC MemDC;
   HDC TempMemDC;

   HBRUSH TempBrush;

   RECT TempRect;

   int TempWidth;
   int TempHeight;
   int havebitmap;

   POINT dest;

   if (current_mode == perspectivemode)
      {
      VECTOR from3d;

      from3d.x = turtle_p[turtle_which].x / WorldWidth;
      from3d.y = turtle_p[turtle_which].y / WorldHeight;
      from3d.z = turtle_p[turtle_which].z / WorldDepth;

      if (!ThreeD.TransformPoint(from3d, dest)) return (UNBOUND);
      }
   else
      {
      dest.x = g_round(turtle_p[turtle_which].x);
      dest.y = g_round(turtle_p[turtle_which].y);
      }

   havebitmap = 0;

   TempWidth = getint(pos_int_arg(arg));
   TempHeight = getint(pos_int_arg(cdr(arg)));

   if (NOT_THROWING)
      {

      // if we had a old cut get rid of it, we won't go in for clipboard

      if (CutBmp[CutIndex].CutFlag)
         {

         // if same size reuse the bitmap

         if ((TempWidth == CutBmp[CutIndex].CutWidth) && (TempHeight == CutBmp[CutIndex].CutHeight))
            {
            havebitmap = 1;
            }

         // else get rid of it and make a new one later

         else
            {
            DeleteObject(CutBmp[CutIndex].CutMemoryBitMap);
            }
         CutBmp[CutIndex].CutFlag = 0;
         }

      CutBmp[CutIndex].CutWidth = TempWidth;
      CutBmp[CutIndex].CutHeight = TempHeight;

      // only if we have a surface continue

      if ((CutBmp[CutIndex].CutWidth != 0) && (CutBmp[CutIndex].CutHeight != 0))
         {

         // flag it so we will delete it

         CutBmp[CutIndex].CutFlag = 1;

         ScreenDC = GetDC(MainHWindow);

         MemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

         if (!havebitmap) CutBmp[CutIndex].CutMemoryBitMap = CreateCompatibleBitmap(ScreenDC, (int) (CutBmp[CutIndex].CutWidth), (int) (CutBmp[CutIndex].CutHeight));
         if (!CutBmp[CutIndex].CutMemoryBitMap)
            {
            MainWindowx->CommandWindow->MessageBox("Cut failed, Possibly not enough Memory", "Error");
            err_logo(STOP_ERROR, NIL);
            return (UNBOUND);
            }

         TempMemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap2 = (HBITMAP) SelectObject(TempMemDC, CutBmp[CutIndex].CutMemoryBitMap);

         BitBlt(TempMemDC, 0, 0, (int) (CutBmp[CutIndex].CutWidth), (int) (CutBmp[CutIndex].CutHeight), MemDC,
            +dest.x + xoffset,
            -dest.y + yoffset + LL - CutBmp[CutIndex].CutHeight, SRCCOPY);

         SelectObject(TempMemDC, OldBitmap2);
         DeleteDC(TempMemDC);

         // memory

         SetRect(&TempRect,
            +dest.x + xoffset,
            -dest.y + yoffset + LL - CutBmp[CutIndex].CutHeight,
            +dest.x + xoffset + CutBmp[CutIndex].CutWidth,
            -dest.y + yoffset + LL);

         TempBrush = CreateBrushIndirect(&ScreenBrush);

         FillRect(MemDC, &TempRect, TempBrush);

         SelectObject(MemDC, OldBitmap);
         DeleteDC(MemDC);

         //screen

         draw_turtle(0);

         if (zoom_flag)
            {
//            TRect temp;
//
//            temp.Set(
//               (+turtle_p[turtle_which].x - MainWindowx->Scroller->XPos / the_zoom + xoffset                                  ) * the_zoom,
//               (-turtle_p[turtle_which].y - MainWindowx->Scroller->YPos / the_zoom + yoffset + LL - CutBmp[CutIndex].CutHeight) * the_zoom,
//               (+turtle_p[turtle_which].x - MainWindowx->Scroller->XPos / the_zoom + xoffset + CutBmp[CutIndex].CutWidth      ) * the_zoom,
//               (-turtle_p[turtle_which].y - MainWindowx->Scroller->YPos / the_zoom + yoffset + LL                             ) * the_zoom);
//            
//            temp.Normalize();
//            temp.Inflate(1+the_zoom,1+the_zoom);

            InvalidateRect(MainHWindow,  /* &temp */ NULL, FALSE);
            }
         else
            {
            SetRect(&TempRect,
               +turtle_p[turtle_which].x - MainWindowx->Scroller->XPos + xoffset,
               -turtle_p[turtle_which].y - MainWindowx->Scroller->YPos + yoffset + LL - CutBmp[CutIndex].CutHeight,
               +turtle_p[turtle_which].x - MainWindowx->Scroller->XPos + xoffset + CutBmp[CutIndex].CutWidth,
               -turtle_p[turtle_which].y - MainWindowx->Scroller->YPos + yoffset + LL);
            
            FillRect(ScreenDC, &TempRect, TempBrush);
            }

         ReleaseDC(MainHWindow, ScreenDC);

         DeleteObject(TempBrush);

         draw_turtle(1);

         // if CutIndex == 0 then do Clipboard

         if (CutIndex == 0)
            {

            // Open, dump what's in there and give him the Bitmap

            OpenClipboard(MainHWindow);
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, CutBmp[CutIndex].CutMemoryBitMap);

            // If we have a palette given him a DIB and a palette too

            if (EnablePalette)
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, ThePalette));
               SetClipboardData(CF_PALETTE, CreatePalette(MyLogPalette));
               }

            // else give hime a DIB using system palette

            else
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, NULL));
               }

            CloseClipboard();

            // Never mung with bitmaps that belong to ClipBoard

            CutBmp[CutIndex].CutFlag = 0;
            }

         }

      }
   return (UNBOUND);
   }

NODE *lbitcopy(NODE *arg)
   {
   HDC ScreenDC;
   HDC MemDC;
   HDC TempMemDC;
   int TempWidth;
   int TempHeight;
   int havebitmap;

   POINT dest;

   if (current_mode == perspectivemode)
      {
      VECTOR from3d;

      from3d.x = turtle_p[turtle_which].x / WorldWidth;
      from3d.y = turtle_p[turtle_which].y / WorldHeight;
      from3d.z = turtle_p[turtle_which].z / WorldDepth;

      if (!ThreeD.TransformPoint(from3d, dest)) return (UNBOUND);
      }
   else
      {
      dest.x = g_round(turtle_p[turtle_which].x);
      dest.y = g_round(turtle_p[turtle_which].y);
      }

   havebitmap = 0;

   TempWidth = getint(pos_int_arg(arg));
   TempHeight = getint(pos_int_arg(cdr(arg)));

   if (NOT_THROWING)
      {

      // if we had a old cut get rid of it, we won't go in for clipboard

      if (CutBmp[CutIndex].CutFlag)
         {

         // if same size reuse the bitmap

         if ((TempWidth == CutBmp[CutIndex].CutWidth) && (TempHeight == CutBmp[CutIndex].CutHeight))
            {
            havebitmap = 1;
            }

         // else get rid of it and make a new one later

         else
            {
            DeleteObject(CutBmp[CutIndex].CutMemoryBitMap);
            }
         CutBmp[CutIndex].CutFlag = 0;
         }

      CutBmp[CutIndex].CutWidth = TempWidth;
      CutBmp[CutIndex].CutHeight = TempHeight;

      // only if we have a surface continue

      if ((CutBmp[CutIndex].CutWidth != 0) && (CutBmp[CutIndex].CutHeight != 0))
         {

         // flag it so we will delete it

         CutBmp[CutIndex].CutFlag = 1;

         ScreenDC = GetDC(MainHWindow);

         MemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

         if (!havebitmap) CutBmp[CutIndex].CutMemoryBitMap = CreateCompatibleBitmap(ScreenDC, (int) (CutBmp[CutIndex].CutWidth), (int) (CutBmp[CutIndex].CutHeight));
         if (!CutBmp[CutIndex].CutMemoryBitMap)
            {
            MainWindowx->CommandWindow->MessageBox("Cut failed, Possibly not enough Memory", "Error");
            err_logo(STOP_ERROR, NIL);
            return (UNBOUND);
            }

         TempMemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap2 = (HBITMAP) SelectObject(TempMemDC, CutBmp[CutIndex].CutMemoryBitMap);

         BitBlt(TempMemDC, 0, 0, (int) (CutBmp[CutIndex].CutWidth), (int) (CutBmp[CutIndex].CutHeight), MemDC,
            +dest.x + xoffset,
            -dest.y + yoffset + LL - CutBmp[CutIndex].CutHeight, SRCCOPY);

         SelectObject(TempMemDC, OldBitmap2);
         DeleteDC(TempMemDC);

         SelectObject(MemDC, OldBitmap);
         DeleteDC(MemDC);

         ReleaseDC(MainHWindow, ScreenDC);

         // if CutIndex == 0 then do Clipboard

         if (CutIndex == 0)
            {

            // Open, dump what's in there and give him the Bitmap

            OpenClipboard(MainHWindow);
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, CutBmp[CutIndex].CutMemoryBitMap);

            // If we have a palette given him a DIB and a palette too

            if (EnablePalette)
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, ThePalette));
               SetClipboardData(CF_PALETTE, CreatePalette(MyLogPalette));
               }

            // else give hime a DIB using system palette

            else
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, NULL));
               }
            CloseClipboard();

            // Never mung with bitmaps that belong to ClipBoard

            CutBmp[CutIndex].CutFlag = 0;
            }
         }
      }
   return (UNBOUND);
   }

NODE *lbitfit(NODE *arg)
   {
   HDC ScreenDC;
   HDC MemDC;
   HDC TempMemDC;

   NUMBER FitHeight;
   NUMBER FitWidth;

   HBITMAP TempMemoryBitMap;

   BITMAP temp;
   HANDLE TempDIB;
   HPALETTE TempPal;

   FitWidth = getint(pos_int_arg(arg));
   FitHeight = getint(pos_int_arg(cdr(arg)));

   if (NOT_THROWING)
      {

      // If ClipBoard check with ClipBoard only

      if (CutIndex == 0)
         {
         OpenClipboard(MainHWindow);

         // Try a DIB first

         TempDIB = (HBITMAP) GetClipboardData(CF_DIB);

         // If Success try for a palette too

         if (TempDIB != NULL)
            {
            TempPal = (HPALETTE) GetClipboardData(CF_PALETTE);

            // we have everything we need

            CloseClipboard();

            // we work in bmps here

            CutBmp[CutIndex].CutMemoryBitMap = DIBToBitmap(TempDIB, TempPal);

            // Fill our logical palette with the Palette from the clipboard

            if (EnablePalette && (TempPal != NULL))
               {
               MyLogPalette->palNumEntries = GetPaletteEntries(TempPal, 0, 256, &(MyLogPalette->palPalEntry[0]));

               // now rebuild palette

               DeleteObject(ThePalette);
               ThePalette = CreatePalette(MyLogPalette);
               if (status_flag) update_status_paletteuse();
               }

            // Let code know below that we have something

            CutBmp[CutIndex].CutFlag = 1;

            // note we do not have to delete bitmap here because we
            // going to immediately give it back to clipboard.
            }

         // else try for a bitmap

         else
            {

            CutBmp[CutIndex].CutMemoryBitMap = (HBITMAP) GetClipboardData(CF_BITMAP);

            // where done whether they have it or not

            CloseClipboard();

            // flag that we have one if it exists, no need to delete the
            // bitmap here because clipboard still owns it.

            if (CutBmp[CutIndex].CutMemoryBitMap != NULL)
               {
               CutBmp[CutIndex].CutFlag = 1;
               }
            else
               {
               CutBmp[CutIndex].CutFlag = 0;
               }
            }

         // if we have something fetch its size

         if (CutBmp[CutIndex].CutFlag)
            {
            GetObject(CutBmp[CutIndex].CutMemoryBitMap, sizeof(BITMAP), (LPSTR) & temp);
            CutBmp[CutIndex].CutWidth = temp.bmWidth;
            CutBmp[CutIndex].CutHeight = temp.bmHeight;
            }
         }

      // only if we have a surface to fit to and from continue

      if ((FitWidth != 0) && (FitHeight != 0) && CutBmp[CutIndex].CutFlag)
         {

         ScreenDC = GetDC(MainHWindow);

         MemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap = (HBITMAP) SelectObject(MemDC, CutBmp[CutIndex].CutMemoryBitMap);

         if (EnablePalette)
            {
            OldPalette2 = SelectPalette(ScreenDC, ThePalette, FALSE);
            RealizePalette(ScreenDC);

            OldPalette = SelectPalette(MemDC, ThePalette, FALSE);
            RealizePalette(MemDC);
            }

         TempMemoryBitMap = CreateCompatibleBitmap(ScreenDC, (int) (FitWidth), (int) (FitHeight));
         if (!TempMemoryBitMap)
            {
            MainWindowx->CommandWindow->MessageBox("Fit failed, Possibly not enough Memory", "Error");
            err_logo(STOP_ERROR, NIL);
            return (UNBOUND);
            }

         TempMemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap2 = (HBITMAP) SelectObject(TempMemDC, TempMemoryBitMap);

         if (EnablePalette)
            {
            SelectPalette(ScreenDC, OldPalette2, FALSE);
            }

         ReleaseDC(MainHWindow, ScreenDC);

         if (EnablePalette)
            {
            OldPalette2 = SelectPalette(TempMemDC, ThePalette, FALSE);
            RealizePalette(TempMemDC);
            }

         SetStretchBltMode(TempMemDC, COLORONCOLOR);

         // Load hour-glass cursor.
         hCursor =::SetCursor(hCursorWait);

         StretchBlt(TempMemDC,
            0,
            0,
            FitWidth,
            FitHeight,
            MemDC,
            0,
            0,
            CutBmp[CutIndex].CutWidth,
            CutBmp[CutIndex].CutHeight,
            SRCCOPY);

         // Reload arrow cursor.
         ::SetCursor(hCursor);

         if (EnablePalette)
            {
            SelectPalette(MemDC, OldPalette, FALSE);
            SelectPalette(TempMemDC, OldPalette2, FALSE);
            }

         SelectObject(TempMemDC, OldBitmap2);
         DeleteDC(TempMemDC);

         SelectObject(MemDC, OldBitmap);
         DeleteDC(MemDC);

         DeleteObject(CutBmp[CutIndex].CutMemoryBitMap);
         CutBmp[CutIndex].CutMemoryBitMap = TempMemoryBitMap;

         CutBmp[CutIndex].CutWidth = FitWidth;
         CutBmp[CutIndex].CutHeight = FitHeight;

         if (CutIndex == 0)
            {
            OpenClipboard(MainHWindow);
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, CutBmp[CutIndex].CutMemoryBitMap);
            if (EnablePalette)
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, ThePalette));
               SetClipboardData(CF_PALETTE, CreatePalette(MyLogPalette));
               }
            else
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, NULL));
               }
            CloseClipboard();
            CutBmp[CutIndex].CutFlag = 0;
            }
         }
      }
   return (UNBOUND);
   }

NODE *lbitpaste(void)
   {
   HDC ScreenDC;
   HDC MemDC;
   HDC TempMemDC;

   BITMAP temp;

   HANDLE TempDIB;

   HPALETTE TempPal;

#if !defined(__WIN32__)
   int DeleteBitMapWhenDone;
#endif

   POINT dest;

   if (current_mode == perspectivemode)
      {
      VECTOR from3d;

      from3d.x = turtle_p[turtle_which].x / WorldWidth;
      from3d.y = turtle_p[turtle_which].y / WorldHeight;
      from3d.z = turtle_p[turtle_which].z / WorldDepth;

      if (!ThreeD.TransformPoint(from3d, dest)) return (UNBOUND);
      }
   else
      {
      dest.x = g_round(turtle_p[turtle_which].x);
      dest.y = g_round(turtle_p[turtle_which].y);
      }

   if (NOT_THROWING)
      {

#if !defined(__WIN32__)
      DeleteBitMapWhenDone = 0;
#endif

      // If ClipBoard check with ClipBoard only

      if (CutIndex == 0)
         {

         OpenClipboard(MainHWindow);

         // Try a DIB first

         TempDIB = (HBITMAP) GetClipboardData(CF_DIB);

         // If Success try for a palette too

         if (TempDIB != NULL)
            {
            TempPal = (HPALETTE) GetClipboardData(CF_PALETTE);

            // we work in bmps here

            CutBmp[CutIndex].CutMemoryBitMap = DIBToBitmap(TempDIB, TempPal);

            // Fill our logical palette with the Palette from the clipboard

            if (EnablePalette && (TempPal != NULL))
               {
               MyLogPalette->palNumEntries = GetPaletteEntries(TempPal, 0, 256, &(MyLogPalette->palPalEntry[0]));

               // now rebuild palette

               DeleteObject(ThePalette);
               ThePalette = CreatePalette(MyLogPalette);
               if (status_flag) update_status_paletteuse();
               }

            // Let code know below that we have something

            CutBmp[CutIndex].CutFlag = 1;

            // We created a BitMap from the DIB that we only need for the
            // purpose of this "paste", next paste could be something new.
            // so get rid of it once we have pasted it here.

#if !defined(__WIN32__)
            DeleteBitMapWhenDone = 1;
#endif

#if defined(__WIN32__)
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, CutBmp[CutIndex].CutMemoryBitMap);

            // If we have a palette given him a DIB and a palette too

            if (EnablePalette)
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, ThePalette));
               SetClipboardData(CF_PALETTE, CreatePalette(MyLogPalette));
               }

            // else give hime a DIB using system palette

            else
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, NULL));
               }
#endif
            }

         // else try for a bitmap

         else
            {
            CutBmp[CutIndex].CutMemoryBitMap = (HBITMAP) GetClipboardData(CF_BITMAP);

            // flag that we have one if it exists, no need to delete the
            // bitmap here because clipboard still owns it.

            if (CutBmp[CutIndex].CutMemoryBitMap != NULL)
               {
               CutBmp[CutIndex].CutFlag = 1;
               }
            else
               {
               CutBmp[CutIndex].CutFlag = 0;
               }
            }

         // if we have something fetch its size

         if (CutBmp[CutIndex].CutFlag)
            {
            GetObject(CutBmp[CutIndex].CutMemoryBitMap, sizeof(BITMAP), (LPSTR) & temp);
            CutBmp[CutIndex].CutWidth = temp.bmWidth;
            CutBmp[CutIndex].CutHeight = temp.bmHeight;
            }

         // we have everything we need

         CloseClipboard();

         }

      // only if we have something to paste

      if (CutBmp[CutIndex].CutFlag)
         {

         // if clipboard then never leave Cut Flag true

         if (CutIndex == 0) CutBmp[CutIndex].CutFlag = 0;

         ScreenDC = GetDC(MainHWindow);

         TempMemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap2 = (HBITMAP) SelectObject(TempMemDC, CutBmp[CutIndex].CutMemoryBitMap);

         //memory

         MemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

         BitBlt(MemDC,
            +dest.x + xoffset,
            -dest.y + yoffset + LL - CutBmp[CutIndex].CutHeight,
            (int) (CutBmp[CutIndex].CutWidth),
            (int) (CutBmp[CutIndex].CutHeight),
            TempMemDC, 0, 0, bitmode);

         SelectObject(MemDC, OldBitmap);
         DeleteDC(MemDC);

         //screen

         draw_turtle(0);

         if (zoom_flag)
            {
//            TRect temp;
//
//            temp.Set(
//               (+turtle_p[turtle_which].x - MainWindowx->Scroller->XPos / the_zoom + xoffset                                  ) * the_zoom,
//               (-turtle_p[turtle_which].y - MainWindowx->Scroller->YPos / the_zoom + yoffset + LL - CutBmp[CutIndex].CutHeight) * the_zoom,
//               (+turtle_p[turtle_which].x - MainWindowx->Scroller->XPos / the_zoom + xoffset + CutBmp[CutIndex].CutWidth      ) * the_zoom,
//               (-turtle_p[turtle_which].y - MainWindowx->Scroller->YPos / the_zoom + yoffset + LL                             ) * the_zoom);
//            
//            temp.Normalize();
//            temp.Inflate(1+the_zoom,1+the_zoom);

            InvalidateRect(MainHWindow,  /* &temp */ NULL, FALSE);
            }
         else
            {
            BitBlt(ScreenDC,
               +dest.x - MainWindowx->Scroller->XPos + xoffset,
               -dest.y - MainWindowx->Scroller->YPos + yoffset + LL - CutBmp[CutIndex].CutHeight,
               (int) (CutBmp[CutIndex].CutWidth),
               (int) (CutBmp[CutIndex].CutHeight),
               TempMemDC, 0, 0, bitmode);
            }
         
         ReleaseDC(MainHWindow, ScreenDC);

         draw_turtle(1);

         SelectObject(TempMemDC, OldBitmap2);
         DeleteDC(TempMemDC);

         // Clipboard owns what we paste in not what we converted

#if !defined(__WIN32__)
         if (DeleteBitMapWhenDone) DeleteObject(CutBmp[CutIndex].CutMemoryBitMap);
#endif
         }
      else
         {
         MainWindowx->CommandWindow->MessageBox("Nothing to Paste", "Error");
         err_logo(STOP_ERROR, NIL);
         }
      }
   return (UNBOUND);
   }

NODE *lbitpastetoindex(NODE *arg)
   {
   HDC ScreenDC;
   HDC MemDC;
   HDC TempMemDC;

   BITMAP temp;

   HANDLE TempDIB;

   HPALETTE TempPal;

#if !defined(__WIN32__)
   int DeleteBitMapWhenDone;
#endif

   int i;
   int x;
   int y;

   // set the current bitmap index if within range

   i = getint(pos_int_arg(arg));
   x = getint(pos_int_arg(cdr(arg)));
   y = getint(pos_int_arg(cdr(cdr(arg))));

   if (i < MaxBitCuts)
      {
      //      CutIndex = i;
      }
   else
      {
      MainWindowx->CommandWindow->MessageBox("BitMap Index out of range", "Error");
      err_logo(STOP_ERROR, NIL);
      return (UNBOUND);
      }

   if (!CutBmp[i].CutFlag)
      {
      MainWindowx->CommandWindow->MessageBox("BitMap at Index must be initialized with a bitmap", "Error");
      err_logo(STOP_ERROR, NIL);
      return (UNBOUND);
      }

   if (NOT_THROWING)
      {

#if !defined(__WIN32__)
      DeleteBitMapWhenDone = 0;
#endif

      // If ClipBoard check with ClipBoard only

      if (CutIndex == 0)
         {

         OpenClipboard(MainHWindow);

         // Try a DIB first

         TempDIB = (HBITMAP) GetClipboardData(CF_DIB);

         // If Success try for a palette too

         if (TempDIB != NULL)
            {
            TempPal = (HPALETTE) GetClipboardData(CF_PALETTE);

            // we work in bmps here

            CutBmp[CutIndex].CutMemoryBitMap = DIBToBitmap(TempDIB, TempPal);

            // Fill our logical palette with the Palette from the clipboard

            if (EnablePalette && (TempPal != NULL))
               {
               MyLogPalette->palNumEntries = GetPaletteEntries(TempPal, 0, 256, &(MyLogPalette->palPalEntry[0]));

               // now rebuild palette

               DeleteObject(ThePalette);
               ThePalette = CreatePalette(MyLogPalette);
               if (status_flag) update_status_paletteuse();
               }

            // Let code know below that we have something

            CutBmp[CutIndex].CutFlag = 1;

            // We created a BitMap from the DIB that we only need for the
            // purpose of this "paste", next paste could be something new.
            // so get rid of it once we have pasted it here.

#if !defined(__WIN32__)
            DeleteBitMapWhenDone = 1;
#endif

#if defined(__WIN32__)
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, CutBmp[CutIndex].CutMemoryBitMap);

            // If we have a palette given him a DIB and a palette too

            if (EnablePalette)
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, ThePalette));
               SetClipboardData(CF_PALETTE, CreatePalette(MyLogPalette));
               }

            // else give hime a DIB using system palette

            else
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[CutIndex].CutMemoryBitMap, NULL));
               }
#endif
            }

         // else try for a bitmap

         else
            {
            CutBmp[CutIndex].CutMemoryBitMap = (HBITMAP) GetClipboardData(CF_BITMAP);

            // flag that we have one if it exists, no need to delete the
            // bitmap here because clipboard still owns it.

            if (CutBmp[CutIndex].CutMemoryBitMap != NULL)
               {
               CutBmp[CutIndex].CutFlag = 1;
               }
            else
               {
               CutBmp[CutIndex].CutFlag = 0;
               }
            }

         // if we have something fetch its size

         if (CutBmp[CutIndex].CutFlag)
            {
            GetObject(CutBmp[CutIndex].CutMemoryBitMap, sizeof(BITMAP), (LPSTR) & temp);
            CutBmp[CutIndex].CutWidth = temp.bmWidth;
            CutBmp[CutIndex].CutHeight = temp.bmHeight;
            }

         // we have everything we need

         CloseClipboard();

         }

      // only if we have something to paste

      if (CutBmp[CutIndex].CutFlag)
         {

         // if clipboard then never leave Cut Flag true

         if (CutIndex == 0) CutBmp[CutIndex].CutFlag = 0;

         ScreenDC = GetDC(MainHWindow);

         TempMemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap2 = (HBITMAP) SelectObject(TempMemDC, CutBmp[CutIndex].CutMemoryBitMap);

         //memory

         MemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap = (HBITMAP) SelectObject(MemDC, CutBmp[i].CutMemoryBitMap);

         BitBlt(MemDC,
            +x,
            CutBmp[i].CutHeight - y - CutBmp[CutIndex].CutHeight,
            (int) (CutBmp[CutIndex].CutWidth),
            (int) (CutBmp[CutIndex].CutHeight),
            TempMemDC, 0, 0, bitmode);

         SelectObject(MemDC, OldBitmap);
         DeleteDC(MemDC);

         ReleaseDC(MainHWindow, ScreenDC);

         SelectObject(TempMemDC, OldBitmap2);
         DeleteDC(TempMemDC);

         // Clipboard owns what we paste in not what we converted

#if !defined(__WIN32__)
         if (DeleteBitMapWhenDone) DeleteObject(CutBmp[CutIndex].CutMemoryBitMap);
#endif
         }
      else
         {
         MainWindowx->CommandWindow->MessageBox("Nothing to Paste", "Error");
         err_logo(STOP_ERROR, NIL);
         }
      }
   return (UNBOUND);
   }

NODE *lsetturtle(NODE *arg)
   {
   NODE *val;
   int temp;

   val = numeric_arg(arg);

   if (NOT_THROWING)
      {
      draw_turtles(0);
      temp = getint(val);
      if ((temp >= (TURTLES - TURTLEN)) || (temp < -TURTLEN))
         {
         MainWindowx->CommandWindow->MessageBox("Bad Turtle Id", "Error");
         err_logo(STOP_ERROR, NIL);
         }
      else
         {

         turtle_which = temp;

         if (turtle_which > turtle_max)
            {
            for (int i = turtle_max + 1; i <= turtle_which; i++)
               {
               turtle_p[i].x = 0.0;
               turtle_p[i].y = 0.0;
               turtle_p[i].z = 0.0;
               turtle_heading[i] = 0.0;
               turtle_shown[i] = TRUE;
               turtle_bitmap[i] = 0;
               turtle_matrix[i].e11 = 1.0;
               turtle_matrix[i].e12 = 0.0;
               turtle_matrix[i].e13 = 0.0;
               turtle_matrix[i].e21 = 0.0;
               turtle_matrix[i].e22 = 1.0;
               turtle_matrix[i].e23 = 0.0;
               turtle_matrix[i].e31 = 0.0;
               turtle_matrix[i].e32 = 0.0;
               turtle_matrix[i].e33 = 1.0;
               }
            turtle_max = turtle_which;
            }

         if (turtle_which < 0) turtle_which = (TURTLES - (TURTLEN+1)) - turtle_which;

         if (status_flag)
            {
            update_status_turtlevisability();
            update_status_turtleposition();
            update_status_turtleheading();
            update_status_turtleroll();
            update_status_turtlepitch();
            update_status_pencontact();
            update_status_turtlewhich();
            }

extern FLONUM wanna_x;
extern FLONUM wanna_y;
extern FLONUM wanna_z;

         wanna_x = turtle_p[turtle_which].x;
         wanna_y = turtle_p[turtle_which].y;
         wanna_z = turtle_p[turtle_which].z;

         draw_turtles(1);
         }
      }

   return (UNBOUND);
   }

NODE *lturtle()
   {
   if (turtle_which >= TURTLES - TURTLEN)
      return (make_intnode(-(turtle_which - (TURTLES - (TURTLEN+1)))));
   else
      return (make_intnode(turtle_which));
   }

NODE *lturtles()
   {
   return (make_intnode(turtle_max));
   }

void turtlepaste(int turtle_which)
   {
   HDC ScreenDC;
   HDC TempMemDC;

   BITMAP temp;

   HANDLE TempDIB;

   HPALETTE TempPal;

#if !defined(__WIN32__)
   int DeleteBitMapWhenDone;
#endif

   POINT dest;

   if (current_mode == perspectivemode)
      {
      VECTOR from3d;

      from3d.x = turtle_p[turtle_which].x / WorldWidth;
      from3d.y = turtle_p[turtle_which].y / WorldHeight;
      from3d.z = turtle_p[turtle_which].z / WorldDepth;

      if (!ThreeD.TransformPoint(from3d, dest)) return;
      }
   else
      {
      dest.x = g_round(turtle_p[turtle_which].x);
      dest.y = g_round(turtle_p[turtle_which].y);
      }

   if (NOT_THROWING)
      {

#if !defined(__WIN32__)
      DeleteBitMapWhenDone = 0;
#endif

      // If ClipBoard check with ClipBoard only

      if (turtle_which == 0)
         {

         OpenClipboard(MainHWindow);

         // Try a DIB first

         TempDIB = (HBITMAP) GetClipboardData(CF_DIB);

         // If Success try for a palette too

         if (TempDIB != NULL)
            {
            TempPal = (HPALETTE) GetClipboardData(CF_PALETTE);

            // we work in bmps here

            CutBmp[turtle_which].CutMemoryBitMap = DIBToBitmap(TempDIB, TempPal);

            // Fill our logical palette with the Palette from the clipboard

            if (EnablePalette && (TempPal != NULL))
               {
               MyLogPalette->palNumEntries = GetPaletteEntries(TempPal, 0, 256, &(MyLogPalette->palPalEntry[0]));

               // now rebuild palette

               DeleteObject(ThePalette);
               ThePalette = CreatePalette(MyLogPalette);
               if (status_flag) update_status_paletteuse();
               }

            // Let code know below that we have something

            CutBmp[turtle_which].CutFlag = 1;

            // We created a BitMap from the DIB that we only need for the
            // purpose of this "paste", next paste could be something new.
            // so get rid of it once we have pasted it here.

#if !defined(__WIN32__)
            DeleteBitMapWhenDone = 1;
#endif

#if defined(__WIN32__)
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, CutBmp[turtle_which].CutMemoryBitMap);

            // If we have a palette given him a DIB and a palette too

            if (EnablePalette)
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[turtle_which].CutMemoryBitMap, ThePalette));
               SetClipboardData(CF_PALETTE, CreatePalette(MyLogPalette));
               }

            // else give hime a DIB using system palette

            else
               {
               SetClipboardData(CF_DIB, BitmapToDIB(CutBmp[turtle_which].CutMemoryBitMap, NULL));
               }
#endif
            }

         // else try for a bitmap

         else
            {
            CutBmp[turtle_which].CutMemoryBitMap = (HBITMAP) GetClipboardData(CF_BITMAP);

            // flag that we have one if it exists, no need to delete the
            // bitmap here because clipboard still owns it.

            if (CutBmp[turtle_which].CutMemoryBitMap != NULL)
               {
               CutBmp[turtle_which].CutFlag = 1;
               }
            else
               {
               CutBmp[turtle_which].CutFlag = 0;
               }
            }

         // if we have something fetch its size

         if (CutBmp[turtle_which].CutFlag)
            {
            GetObject(CutBmp[turtle_which].CutMemoryBitMap, sizeof(BITMAP), (LPSTR) & temp);
            CutBmp[turtle_which].CutWidth = temp.bmWidth;
            CutBmp[turtle_which].CutHeight = temp.bmHeight;
            }

         // we have everything we need

         CloseClipboard();

         }

      // only if we have something to paste

      if (CutBmp[turtle_which].CutFlag)
         {

         // if clipboard then never leave Cut Flag true

         if (turtle_which == 0) CutBmp[turtle_which].CutFlag = 0;

         ScreenDC = GetDC(MainHWindow);

         TempMemDC = CreateCompatibleDC(ScreenDC);
         OldBitmap2 = (HBITMAP) SelectObject(TempMemDC, CutBmp[turtle_which].CutMemoryBitMap);

         //screen

         if (zoom_flag)
            {
            SetMapMode(ScreenDC, MM_ANISOTROPIC);
            SetWindowOrgEx(ScreenDC, 0, 0, 0);
            SetWindowExtEx(ScreenDC, BitMapWidth, BitMapHeight, 0);
            SetViewportOrgEx(ScreenDC, 0, 0, 0);
            SetViewportExtEx(ScreenDC, (int) (BitMapWidth * the_zoom), (int) (BitMapHeight * the_zoom), 0);
            
            BitBlt(ScreenDC,
               +dest.x - MainWindowx->Scroller->XPos / the_zoom + xoffset,
               -dest.y - MainWindowx->Scroller->YPos / the_zoom + yoffset + LL - CutBmp[turtle_which].CutHeight,
               (int) (CutBmp[turtle_which].CutWidth),
               (int) (CutBmp[turtle_which].CutHeight),
               TempMemDC, 0, 0, turtle_bitmap[turtle_which]);
            }
         else
            {
            BitBlt(ScreenDC,
               +dest.x - MainWindowx->Scroller->XPos + xoffset,
               -dest.y - MainWindowx->Scroller->YPos + yoffset + LL - CutBmp[turtle_which].CutHeight,
               (int) (CutBmp[turtle_which].CutWidth),
               (int) (CutBmp[turtle_which].CutHeight),
               TempMemDC, 0, 0, turtle_bitmap[turtle_which]);
            }
         
         ReleaseDC(MainHWindow, ScreenDC);

         SelectObject(TempMemDC, OldBitmap2);
         DeleteDC(TempMemDC);

         // Clipboard owns what we paste in not what we converted

#if !defined(__WIN32__)
         if (DeleteBitMapWhenDone) DeleteObject(CutBmp[turtle_which].CutMemoryBitMap);
#endif
         }
      else
         {
         turtle_bitmap[turtle_which] = 0;
         char szWinLocStr[WININISIZ];
         sprintf(szWinLocStr, "Turtle %d has no picture, will Halt", turtle_which);
         MainWindowx->CommandWindow->MessageBox(szWinLocStr, "Error");
         err_logo(STOP_ERROR, NIL);
         }
      }
   }

NODE *lscrollx(NODE *arg)
   {
   NODE *args;
   int delta;

   // get args and scroll the scroller

   args = numeric_arg(arg);

   if (NOT_THROWING)
      {
      delta = getint(args);

      if (delta == 0)
         {
         TRect RR;

         MainWindowx->GetClientRect(RR);
         MainWindowx->Scroller->ScrollTo(((BitMapWidth * the_zoom) / 2) - (0.5 * RR.right), MainWindowx->Scroller->YPos);
         }
      else
         {
         MainWindowx->Scroller->ScrollTo(
            MainWindowx->Scroller->XPos + delta,
            MainWindowx->Scroller->YPos);
         }
      }

   return (UNBOUND);
   }

NODE *lscrolly(NODE *arg)
   {
   NODE *args;

   int delta;

   // get args and scroll the scroller

   args = numeric_arg(arg);

   if (NOT_THROWING)
      {
      delta = getint(args);

      if (delta == 0)
         {
         TRect RR;

         MainWindowx->GetClientRect(RR);
         MainWindowx->Scroller->ScrollTo(MainWindowx->Scroller->XPos, ((BitMapHeight * the_zoom)/ 2) - (0.5 * RR.bottom));
         }
      else
         {
         MainWindowx->Scroller->ScrollTo(
            MainWindowx->Scroller->XPos,
            MainWindowx->Scroller->YPos + delta);
         }
      }

   return (UNBOUND);
   }

NODE *lsetfocus(NODE *arg)
   {
   char textbuf[MAX_BUFFER_SIZE];

   HWND EditH;

   cnv_strnode_string(textbuf, arg);

   // get handle to Window with arg as Caption

   EditH = FindWindow(NULL, textbuf);

   // Now set focus to it, if it exists

   if (EditH != NULL)
      {
      ::SetFocus(EditH);
      }

   JustDidEdit = 1;

   return (UNBOUND);
   }

NODE *lgetfocus(void)
   {
   NODE *arg;
   NODE *val;
   char textbuf[MAX_BUFFER_SIZE];

   //int i;

   HWND TempH;

   //

   // Loop Optimize Win16 bogus
   //   for (i=0;i<MAX_BUFFER_SIZE;i++) textbuf[i] = '\0';
   memset(textbuf, 0, MAX_BUFFER_SIZE);

   // Get handle to active window

   TempH = GetActiveWindow();

   // It better exist, get it's caption

   if (TempH != NULL)
      {
      ::GetWindowText(TempH, textbuf, MAX_BUFFER_SIZE);
      }

   JustDidEdit = 1;

   // Return caption as a list

   arg = make_strnode(textbuf, NULL, strlen(textbuf), STRING, strnzcpy);
   val = arg; // parser(arg, FALSE);
   return (val);

   }

NODE *lwindowset(NODE *args)
   {
   char textbuf[MAX_BUFFER_SIZE];
   int value;

   HWND EditH;

   cnv_strnode_string(textbuf, args);
   value = getint(pos_int_arg(cdr(args)));

   // get handle to Window with arg as Caption

   EditH = FindWindow(NULL, textbuf);

   // if it exists icon it.

   if (EditH != NULL)
      {
      ShowWindow(EditH, value);
      }

   JustDidEdit = 1;

   return (UNBOUND);
   }

void ibm_clear_screen(void)
   {
   HDC ScreenDC;
   HDC MemDC;

   HBRUSH TempBrush;

   TempBrush = CreateBrushIndirect(&ScreenBrush);
   ScreenDC = GetDC(MainHWindow);

//   // screen
//   
//   if (zoom_flag)
//      {
//      InvalidateRect(MainHWindow, NULL, FALSE);
//      }
//   else
//      {
//      FillRect(ScreenDC, &TempRect, TempBrush);
//      }
//   
//   SetBkColor(ScreenDC, scolor);
//   SetBkMode(ScreenDC, TRANSPARENT);

   // memory

   MemDC = CreateCompatibleDC(ScreenDC);
   OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

   FillRect(MemDC, &FullRect, TempBrush);

   SetBkColor(MemDC, scolor);
   SetBkMode(MemDC, TRANSPARENT);

   SelectObject(MemDC, OldBitmap);
   DeleteDC(MemDC);

   ReleaseDC(MainHWindow, ScreenDC);
   DeleteObject(TempBrush);

   InvalidateRect(MainHWindow, NULL, FALSE);
   }

void option_helper(short *var, NODE *arg)
   {
   NODE *val;

   val = integer_arg(arg);
   if (NOT_THROWING)
      *var = (short) getint(val);
   }

int FAR PASCAL _export EnumerateFont(LPLOGFONT LogFont, LPTEXTMETRIC, short, LPSTR)
   {

   // if printflag just print the enumerated font

   if (printflag == 1)
      {
      printfx("[%s]\n", LogFont->lfFaceName);
      }

   // else chcek for a match

   else
      {
      if (stricmp(tempfont, LogFont->lfFaceName) == 0)
         {

         // take a copy, flag one was found and return

         memcpy(&FontRec, LogFont, sizeof(FontRec));
         found = 1;
         return 0;
         }
      }

/*
   NumFonts++;

   if (NumFonts > MaxNumFonts)
      {
      return 0;                        / * Don't send any more information     * /
      }
   else
      {
      return 1;                        / * Send more information if available  * /
      }
*/
   return 1;                        /* Send more information if available  */
   }

/* Collect all of the system fonts */

void GetFontInfo()
   {
   HDC TheDC;

   OLDFONTENUMPROC EnumProc;

   TheDC = GetDC(::GetFocus());
   NumFonts = 0;

   /* Create an instance of the call back function.  This allows
      our program to refer to an exported function.  Otherwise the
      Data segment will not be correct. */

   EnumProc = (OLDFONTENUMPROC) MakeProcInstance((FARPROC) EnumerateFont, GetApplicationObject()->GetInstance());

   /* Gather information about all fonts that are allowable in our window (DC) */

   EnumFonts(TheDC, NULL, EnumProc, NULL);

   /* Free the instance of our call back function */

   FreeProcInstance((FARPROC) EnumProc);
   ReleaseDC(::GetFocus(), TheDC);

   }
;

void setfont(char *s)
   {
   found = 0;
   printflag = 0;

   // set global font to compare to (FontRec will get filled if found)

   tempfont = s;

   GetFontInfo();

   // if not found enumerate again with printing enabled

   if (!found)
      {
      printfx("Sorry, %s Font does not exist, choose one of the following:\n\n", s);
      printflag = 1;
      GetFontInfo();
      }

   }

void do_help(char *arg)
   {

   /* if arg NULL then jump to index else try to lookup key */

   if (arg == NULL) MainWindowx->WinHelp(szHelpFileName, HELP_INDEX, 0L);
   else MainWindowx->WinHelp(szHelpFileName, HELP_PARTIALKEY, (DWORD) arg);
   JustDidEdit = 1;
   }

NODE *lhelp(NODE *arg)
   {
   char textbuf[MAX_BUFFER_SIZE];

   // if arg then pass to do_help

   if (arg != NIL)
      {
      cnv_strnode_string(textbuf, arg);

      do_help(textbuf);
      }

   // else just pop up help

   else
      {
      do_help(NULL);
      }

   return (UNBOUND);
   }

NODE *lwinhelp(NODE *arg)
   {
   char textbuf[MAX_BUFFER_SIZE];
   char textbuf2[MAX_BUFFER_SIZE];

   cnv_strnode_string(textbuf, arg);

   // if 2nd arg then pass to winhelp

   if (cdr(arg) != NIL)
      {
      cnv_strnode_string(textbuf2, cdr(arg));
      MainWindowx->WinHelp(textbuf, HELP_PARTIALKEY, (DWORD) textbuf2);
      }

   // else just give help on file (arg 1)

   else
      {
      MainWindowx->WinHelp(textbuf, HELP_INDEX, 0L);
      }

   return (UNBOUND);
   }

NODE *lsetlabelfont(NODE *arg)
   {
   NODE *args;
   char textbuf[MAX_BUFFER_SIZE];

   args = list_arg(arg);
   if (car(args) == NIL) return (NIL);
   if (NOT_THROWING)
      {
      cnv_strnode_string(textbuf, args);

      // Get the FontRec filled with a match to textbuff

      setfont(textbuf);

      // now fill in the rest of the fields if given

      if (cdr(args) != NIL)
         FontRec.lfHeight = int_arg(args = cdr(args));
      if (cdr(args) != NIL)
         FontRec.lfWidth = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfOrientation = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfWeight = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfItalic = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfUnderline = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfStrikeOut = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfCharSet = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfOutPrecision = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfClipPrecision = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfQuality = getint(pos_int_arg(args = cdr(args)));
      if (cdr(args) != NIL)
         FontRec.lfPitchAndFamily = getint(pos_int_arg(args = cdr(args)));

      // update status window

      //      if (status_flag)
      //         {
      //         update_status_fontwieght();
      //         update_status_fontsize();
      //         update_status_fontname();
      //         }
      }

   return (UNBOUND);
   }

NODE *llabelfont(void)
   {
   NODE *val;
   NODE *targ;

   // put the Font name in a list

   targ = make_strnode(FontRec.lfFaceName, NULL, strlen(FontRec.lfFaceName), STRING, strnzcpy);
   val = parser(targ, FALSE);

   // now return the whole thing as a list

   return (
      cons(val,
         cons(make_intnode((FIXNUM) FontRec.lfHeight),
            cons(make_intnode((FIXNUM) FontRec.lfWidth),
               cons(make_intnode((FIXNUM) FontRec.lfOrientation),
                  cons(make_intnode((FIXNUM) FontRec.lfWeight),
                     cons(make_intnode((FIXNUM) FontRec.lfItalic),
                        cons(make_intnode((FIXNUM) FontRec.lfUnderline),
                           cons(make_intnode((FIXNUM) FontRec.lfStrikeOut),
                              cons(make_intnode((FIXNUM) FontRec.lfCharSet),
                                 cons(make_intnode((FIXNUM) FontRec.lfOutPrecision),
                                    cons(make_intnode((FIXNUM) FontRec.lfClipPrecision),
                                       cons(make_intnode((FIXNUM) FontRec.lfQuality),
                                          cons(make_intnode((FIXNUM) FontRec.lfPitchAndFamily),
                                             NIL
                                          )
                                       )
                                    )
                                 )
                              )
                           )
                        )
                     )
                  )
               )
            )
         )
      )
   );

   }

NODE *lmachine(void)
   {
   HDC TempDC;
   int ScreenWidth;
   int ScreenHeight;

   // build list with system specific information
	
   TempDC = GetDC(0);

   ScreenWidth  = GetDeviceCaps(TempDC, HORZRES);
   ScreenHeight = GetDeviceCaps(TempDC, VERTRES);

   ReleaseDC(0, TempDC);

   TRect wrect;

   MainWindowx->GetWindowRect(wrect);

	return (
      cons(make_intnode((FIXNUM) 1),
#if !defined(__WIN32__)
		cons(make_intnode((FIXNUM) 16),
#else
		cons(make_intnode((FIXNUM) 32),
#endif
		cons(make_intnode((FIXNUM) BitMapWidth),
		cons(make_intnode((FIXNUM) BitMapHeight),
		cons(make_intnode((FIXNUM) EnablePalette),
		cons(make_intnode((FIXNUM) (LOBYTE(LOWORD(GetVersion())))),
		cons(make_intnode((FIXNUM) (HIBYTE(LOWORD(GetVersion())))),
		cons(make_intnode((FIXNUM) MaxWidth),
		cons(make_intnode((FIXNUM) MaxHeight),
		cons(make_intnode((FIXNUM) ScreenWidth),
		cons(make_intnode((FIXNUM) ScreenHeight),
		cons(make_intnode((FIXNUM) (wrect.right - wrect.left)),
		cons(make_intnode((FIXNUM) (wrect.bottom - wrect.top)),
		NIL))))))))))))));
	
   }

SIZE labelsize(char *s)
   {
   HDC ScreenDC;

   HFONT TempFont;

   SIZE size;

   ScreenDC = GetDC(MainHWindow);

   TempFont = CreateFontIndirect(&FontRec);

   OldFont = (HFONT) SelectObject(ScreenDC, TempFont);

   GetTextExtentPoint(ScreenDC, s, strlen(s), &size);
 
   SelectObject(ScreenDC, OldFont);

   DeleteObject(TempFont);

   ReleaseDC(MainHWindow, ScreenDC);

   return (size);
   }

void label(char *s)
   {
   HDC ScreenDC;
   HDC MemDC;

   HFONT TempFont;

   POINT dest;

   if (current_mode == perspectivemode)
      {
      VECTOR from3d;

      from3d.x = turtle_p[turtle_which].x / WorldWidth;
      from3d.y = turtle_p[turtle_which].y / WorldHeight;
      from3d.z = turtle_p[turtle_which].z / WorldDepth;

      if (!ThreeD.TransformPoint(from3d, dest)) return;
      }
   else
      {
      dest.x = g_round(turtle_p[turtle_which].x);
      dest.y = g_round(turtle_p[turtle_which].y);
      }

   ScreenDC = GetDC(MainHWindow);

   FontRec.lfEscapement = (360.0 - (turtle_heading[turtle_which] - 90.0)) * 10;
   TempFont = CreateFontIndirect(&FontRec);

   // memory

   MemDC = CreateCompatibleDC(ScreenDC);
   OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

   if (EnablePalette)
      {
      OldPalette = SelectPalette(MemDC, ThePalette, FALSE);
      RealizePalette(MemDC);
      }

   SetBkColor(MemDC, scolor);
   SetBkMode(MemDC, TRANSPARENT);

	if (in_erase_mode)
		{
		SetTextColor(MemDC, scolor);
		}
	else
		{
		SetTextColor(MemDC, pcolor);
		}
	
   OldFont = (HFONT) SelectObject(MemDC, TempFont);

   TextOut(MemDC,
      +dest.x + xoffset,
      -dest.y + yoffset,
      s, strlen(s));

   if (EnablePalette)
      {
      SelectPalette(MemDC, OldPalette, FALSE);
      }

   SelectObject(MemDC, OldFont);
   SelectObject(MemDC, OldBitmap);
   DeleteDC(MemDC);

   //screen

   if (EnablePalette)
      {
      OldPalette = SelectPalette(ScreenDC, ThePalette, FALSE);
      RealizePalette(ScreenDC);
      }

   SetBkColor(ScreenDC, scolor);
   SetBkMode(ScreenDC, TRANSPARENT);

	if (in_erase_mode)
		{
		SetTextColor(ScreenDC, scolor);
		}
	else
		{
		SetTextColor(ScreenDC, pcolor);
		}

   OldFont = (HFONT) SelectObject(ScreenDC, TempFont);

   if (zoom_flag)
      {
//      TRect temp;
//      SIZE size;
//      
//      GetTextExtentPoint(ScreenDC, s, strlen(s), &size);
//      
//      temp.Set(
//            (+dest.x - MainWindowx->Scroller->XPos / the_zoom + xoffset) * the_zoom,
//            (-dest.y - MainWindowx->Scroller->YPos / the_zoom + yoffset) * the_zoom,
//            (+dest.x - MainWindowx->Scroller->XPos / the_zoom + xoffset) * the_zoom,
//            (-dest.y - MainWindowx->Scroller->YPos / the_zoom + yoffset) * the_zoom);
//      
//      temp.Normalize();
//      
//      temp.Inflate((size.cx + size.cy)*the_zoom, (size.cx + size.cy)*the_zoom);
      
      InvalidateRect(MainHWindow,  /* &temp */ NULL, FALSE);
      }
   else
      {
      TextOut(ScreenDC,
         +dest.x - MainWindowx->Scroller->XPos + xoffset,
         -dest.y - MainWindowx->Scroller->YPos + yoffset,
         s, strlen(s));
      }
   
   SelectObject(ScreenDC, OldFont);

   if (EnablePalette)
      {
      SelectPalette(ScreenDC, OldPalette, FALSE);
      }

   DeleteObject(TempFont);

   ReleaseDC(MainHWindow, ScreenDC);

   }

void exit_program(void)
   {
   if (halt_flag != 0) Time_To_Halt = 1;
   Time_To_Exit = 1;
   }


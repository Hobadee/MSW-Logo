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

#define MAX_PENDING_CONNECTS 4         /* The backlog allowed for listen()    */

int critical = 0;
TColor CustColors[32];

void qlist::insert(ent a, int t)
   {

   /* class "event list queue" member to insert event */

   qlink *h;
   qlink *ph;

   if (last)
      {
      ph = last->next;
      h = new qlink(a, NULL, NULL, t);
      last->next = h;
      h->prev = last;
      h->next = ph;
      ph->prev = h;
      last = last->next;
      }
   else
      {
      last = new qlink(a, NULL, NULL, t);
      last->next = last;
      last->prev = last;
      }
   }

ent qlist::get(void)
   {

   /* class "event list queue" member to get event */

   if (last == NULL) return NULL;

   return (last->next->e);
   }

void qlist::zap(void)
   {

   /* class "event list queue" member to zap all events */

   qlink *p;

   if (last == NULL) return;

   p = last->next;

   if (last == p)
      {
      last = NULL;
      }
   else
      {
      p->prev->next = p->next;
      p->next->prev = p->prev;
      }

   delete p;

   }

void qlist::clear()
   {
   qlink *l = last;

   if (l == NULL) return;

   do
      {
      qlink *ll = l;
      l = l->next;
      delete ll;
      }
   while (l != last);
   }

calllist calllists;

TMyWindow::TMyWindow(TWindow *AParent, LPSTR ATitle)
: TFrameWindow(AParent, ATitle)
   {
   /* main window initialization */

   AssignMenu("COMMANDS");

   PenSize = 1;

   /* build the bitmap */

   /* check if a palette exists */

   ScreenDC = CreateDC("DISPLAY", NULL, NULL, NULL);

   if ((GetDeviceCaps(ScreenDC, RASTERCAPS) & RC_PALETTE) == 0)
      {
      EnablePalette = 0;
      }
   else
      {
      EnablePalette = 1;
      }

   DeleteDC(ScreenDC);

   /* create printer object */

   Printer = new TPrinter;

   /* flag that screen clean */

   IsNewFile = TRUE;
   IsNewBitmap = TRUE;

   /* setup scrollers (LOTS of code for these things) */

   if (!bFixed)
      Attr.Style |= WS_VSCROLL | WS_HSCROLL;
   Scroller = new TScroller(this, 1, 1, BitMapWidth, BitMapHeight);

   /* If palette then build one */

   if (EnablePalette)
      {
      MyLogPalette = (LPLOGPALETTE) new char[sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * MaxColors];
      MyLogPalette->palVersion = 0x300;
      MyLogPalette->palNumEntries = 2;

      MyLogPalette->palPalEntry[0].peRed = 0;
      MyLogPalette->palPalEntry[0].peGreen = 0;
      MyLogPalette->palPalEntry[0].peBlue = 0;
      MyLogPalette->palPalEntry[0].peFlags = 0;

      MyLogPalette->palPalEntry[1].peRed = 255;
      MyLogPalette->palPalEntry[1].peGreen = 255;
      MyLogPalette->palPalEntry[1].peBlue = 255;
      MyLogPalette->palPalEntry[1].peFlags = 0;

      ThePalette = CreatePalette(MyLogPalette);
      }

   strcpy(BitmapName, "Logo.Bmp");
   strcpy(FileName, "Logo.Lgo");
   }

TMyWindow::~TMyWindow()
   {
   int i;

   /* clean things up */

   delete Printer;
   delete Scroller;
   DeleteObject(MemoryBitMap);
   delete EditboxWindow;
   delete ListboxWindow;
   delete CommandWindow;

#if !defined(__WIN32__)
   GlobalFree(hListboxHeap);
   delete pm;
#endif

   /* if palette clean it too */

   if (EnablePalette)
      {
      DeleteObject(ThePalette);
      delete MyLogPalette;
      }

   free(gcstack);
   free(hash_table);

   /* Note Bitmap index 0 belongs to CLipboard */

   for (i = 1; i < MaxBitCuts; i++)
      if (CutBmp[i].CutFlag) DeleteObject(CutBmp[i].CutMemoryBitMap);
   free(CutBmp);

   if (hCursorWait) DestroyCursor(hCursorWait);
   if (hCursorArrow) DestroyCursor(hCursorArrow);
   if (hCmdFont) DeleteObject(hCmdFont);
   }

void TMyWindow::EvKeyDown(UINT, UINT, UINT)
   {
   TMessage Msg = __GetTMessage();

   callthing *callevent;

   // if keyboard was on and up and down is enabled then continue

   if (keyboard_on == 2)
      {

      // if key is down skip it

//    if (!(Msg.LParam & 0x40000000L))
         {
         callevent = new callthing;

         callevent->func = keyboard_keydown;
         callevent->arg1 = Msg.WParam;
         callevent->kind = 2;

         calllists.insert(callevent, 2);
         checkqueue();
         }

		// Don't do scrolling

		return;
      }

   // scroll main window with arrow keys

   switch (Msg.WParam)
      {
       case VK_UP:
           {
              Scroller->ScrollBy(0, -Scroller->YLine);
              break;
           }
       case VK_DOWN:
           {
              Scroller->ScrollBy(0, Scroller->YLine);
              break;
           }
       case VK_LEFT:
           {
              Scroller->ScrollBy(-Scroller->XLine, 0);
              break;
           }
       case VK_RIGHT:
           {
              Scroller->ScrollBy(Scroller->XLine, 0);
              break;
           }

       // else do your normal stuff

       default:
           {
              DefaultProcessing();
              break;
           }
      }
   }

void TMyWindow::EvKeyUp(UINT, UINT, UINT)
   {
   TMessage Msg = __GetTMessage();

   callthing *callevent;

   // if keyboard was on and up and down is enabled then continue

   if (keyboard_on == 2)
      {
      callevent = new callthing;

      callevent->func = keyboard_keyup;
      callevent->arg1 = Msg.WParam;
      callevent->kind = 2;

      calllists.insert(callevent, 2);
      checkqueue();
      }

   // else do your normal stuff

   else
      {
      DefaultProcessing();
      }
   }

void TMyWindow::EvChar(UINT, UINT, UINT)
   {
   TMessage Msg = __GetTMessage();

   callthing *callevent;

   // if keyboard was on and NOT up and down is enabled then continue

   if (keyboard_on == 1)
      {
      callevent = new callthing;

      callevent->func = keyboard_keyup;
      callevent->arg1 = Msg.WParam;
      callevent->kind = 2;

      calllists.insert(callevent, 2);
      checkqueue();
      }

   // else do your normal stuff

   else
      {
      DefaultProcessing();
      }
   }

void TMyWindow::GetWindowClass(WNDCLASS &WndClass)
   {
   TFrameWindow::GetWindowClass(WndClass);
   WndClass.lpszMenuName = "Logo";
   WndClass.hIcon = LoadIcon(GetApplication()->GetInstance(), "LogoIcon");
   }

void TMyWindow::Paint(TDC &PaintDC, bool /* erase */, TRect &PaintRect)
   {

   /*
      This is a compromise between speed and memory (as is most code).
      All drawing is written to the backing store 1 to 1 even when zoomed.
      When zoomed all drawing and painting is scaled to the display on the fly.
      Painting can be a bit slow while zoomed. It also can be inaccurate when
      mixing scaled painting and scaled drawing. Printing is never zoomed.
      User can use Bitfit if he/she wants data scaled.

      Must of rewrote this routine 20 times, at least.
    */

   int i;
   int j;

   /* grab the backing store (a bitmap) */

   ScreenDC = GetDC(0);
   MemDC = CreateCompatibleDC(ScreenDC);

   OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

   /* if palette allocate it */

   if (EnablePalette)
      {
      OldPalette = SelectPalette(PaintDC, ThePalette, FALSE);
      RealizePalette(PaintDC);

      OldPalette2 = SelectPalette(MemDC, ThePalette, FALSE);
      RealizePalette(MemDC);
      }

   /* if 1 to 1 the just do normal paint */

   if (the_zoom == 1.0)
      {
      BitBlt(PaintDC,
         PaintRect.Left(),
         PaintRect.Top(),
         PaintRect.Width(),
         PaintRect.Height(),
         MemDC,
         PaintRect.Left(),
         PaintRect.Top(),
         SRCCOPY);
      }

   /* else compute scaling and then display */

   else if (the_zoom > 1.0)
      {
      TRect TempRect;

      TempRect = PaintRect;

      TempRect.Inflate(((int) (the_zoom+0.5))*2, ((int) (the_zoom+0.5))*2);

      TempRect.left   /= the_zoom;
      TempRect.top    /= the_zoom;
      TempRect.right  /= the_zoom;
      TempRect.bottom /= the_zoom;

		if (TempRect.left   <            0) TempRect.left   = 0;
		if (TempRect.top    <            0) TempRect.top    = 0;
		if (TempRect.right  >  BitMapWidth) TempRect.right  = BitMapWidth;
		if (TempRect.bottom > BitMapHeight) TempRect.bottom = BitMapHeight;

      SetStretchBltMode(PaintDC, COLORONCOLOR);

		StretchBlt(PaintDC,
         TempRect.Left()   * the_zoom,
         TempRect.Top()    * the_zoom,
         TempRect.Width()  * the_zoom,
         TempRect.Height() * the_zoom,
         MemDC,
         TempRect.Left()  ,
         TempRect.Top()   ,
         TempRect.Width() ,
         TempRect.Height(),
         SRCCOPY);
      }

   /* else compute scaling and then display */

   else
      {
      SetStretchBltMode(PaintDC, COLORONCOLOR);

		StretchBlt(PaintDC,
         0,
         0,
         BitMapWidth * the_zoom,
         BitMapHeight * the_zoom,
         MemDC,
         0,
         0,
         BitMapWidth,
         BitMapHeight,
         SRCCOPY);
      }

   /* restore resources */

   if (EnablePalette)
      {
      SelectPalette(MemDC, OldPalette2, FALSE);
      SelectPalette(PaintDC, OldPalette, FALSE);
      }

   SelectObject(MemDC, OldBitmap);
   DeleteDC(MemDC);
   ReleaseDC(0, ScreenDC);

   /* if turtle do it */

   SetROP2(PaintDC, R2_NOT);

   for (j = 0; j <= turtle_max; j++)
      {
      if (turtle_shown[j])
         {
         if (turtle_bitmap[j])
            {
            turtlepaste(j);
            }
         else
            {
            for (i = 0; i < 4; i++)
               {
               if (TurtlePoints[j][i].bValid)
                  {
                  MoveToEx(PaintDC,
                     TurtlePoints[j][i].from.x * the_zoom,
                     TurtlePoints[j][i].from.y * the_zoom, 0);
                  LineTo(PaintDC,
                     TurtlePoints[j][i].to.x * the_zoom,
                     TurtlePoints[j][i].to.y * the_zoom);
                  }
               }
            }
         }
      }
   }

void TMyWindow::Printit(TDC &PrintDC)
   {

   /*
      Must of rewrote this at least 25 times :-) and it still does not
      work in some situations. This is just the "Paint" of printing.
      See the print module for all the other stuff.
    */

   HDC ScreenDC;

   long TempWidth;
   long TempHeight;
   long ScanLines;
	long Status;

   LPSTR BitsPtr;
   HBITMAP BitsHandle;

   BITMAPINFO *PrintBitmapInfo;

   WORD PrintbitCount;
   WORD ScreenbitCount;
   WORD size;
   
   /* do we even have a chance ? */

   if ((GetDeviceCaps(PrintDC, RASTERCAPS) & RC_STRETCHDIB) == 0)
      {
      MessageBox("Print driver does not support this function", "Error");
      return;
      }

   PrintbitCount = GetDeviceCaps(PrintDC, BITSPIXEL);
   PrintbitCount *= GetDeviceCaps(PrintDC, PLANES);

   // If a mono printer lets let it try to dither a 256 grey scale image
   if (PrintbitCount == 1) PrintbitCount = 8;

   // Get screen bitCount

   ScreenDC = GetDC(0);
   
   ScreenbitCount = GetDeviceCaps(ScreenDC, BITSPIXEL);
   ScreenbitCount *= GetDeviceCaps(ScreenDC, PLANES);

   // Don't bother creating a DIB with more colors than we have
   if (ScreenbitCount < PrintbitCount) PrintbitCount = ScreenbitCount;

   // Round to nearest legal bitmap color depth
   if      (                        (PrintbitCount <  1)) PrintbitCount =  1;
   else if ((PrintbitCount >  1) && (PrintbitCount <  4)) PrintbitCount =  4;
   else if ((PrintbitCount >  4) && (PrintbitCount <  8)) PrintbitCount =  8;
   else if ((PrintbitCount >  8) && (PrintbitCount < 16)) PrintbitCount = 16;
   else if ((PrintbitCount > 16) && (PrintbitCount < 24)) PrintbitCount = 24;
   else if ((PrintbitCount > 24)                        ) PrintbitCount = 32;

   PrintbitCount = GetPrivateProfileInt("LOGO", "PrintColorDepth", PrintbitCount, "LOGO.INI");

	if (PrintbitCount <= 8)
      {
      size = sizeof(BITMAPINFOHEADER) + ((1 << PrintbitCount) * sizeof(RGBQUAD));
      }
   else
      {
      size = sizeof(BITMAPINFOHEADER);
      }

   PrintBitmapInfo = (BITMAPINFO *) new char[size];

   PrintBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   PrintBitmapInfo->bmiHeader.biWidth = BitMapWidth;
   PrintBitmapInfo->bmiHeader.biHeight = BitMapHeight;
   PrintBitmapInfo->bmiHeader.biPlanes = 1;
   PrintBitmapInfo->bmiHeader.biBitCount = PrintbitCount;
   PrintBitmapInfo->bmiHeader.biCompression = BI_RGB;
   PrintBitmapInfo->bmiHeader.biSizeImage = ((((PrintBitmapInfo->bmiHeader.biWidth * PrintBitmapInfo->bmiHeader.biBitCount) + 31) / 32) * 4) * PrintBitmapInfo->bmiHeader.biHeight;
   PrintBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
   PrintBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
   PrintBitmapInfo->bmiHeader.biClrUsed = 0;
   PrintBitmapInfo->bmiHeader.biClrImportant = 0;

   /* we don't need hour glass here because print module takes care of it */

   BitsHandle = (HBITMAP) GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, PrintBitmapInfo->bmiHeader.biSizeImage);

   /* If fail try again after a compact */

   if (!BitsHandle)
      {
      GlobalCompact(-1);
      BitsHandle = (HBITMAP) GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, PrintBitmapInfo->bmiHeader.biSizeImage);
      }

   /* if we got the memory continue */

   if (BitsHandle)
      {

      /* get printer size per inch */

      TempWidth = GetDeviceCaps(PrintDC, LOGPIXELSX);
      TempHeight = GetDeviceCaps(PrintDC, LOGPIXELSY);

      /* if palette allocate it */

      if (EnablePalette)
         {
         OldPalette = SelectPalette(ScreenDC, ThePalette, FALSE);
         RealizePalette(ScreenDC);
         }

      /* set up an assured contrast ? */

      SetTextColor(PrintDC, 0x00000000L);
      SetBkColor(PrintDC, 0x00ffffffL);

      /* where is that memory anyway */

      BitsPtr = (LPSTR) GlobalLock((HGLOBAL) BitsHandle);

      ScanLines = GetDIBits(ScreenDC, MemoryBitMap, 0, BitMapHeight, BitsPtr, PrintBitmapInfo, DIB_RGB_COLORS);

      /* check we got something */

		if (ScanLines != 0)
			{

			/* if "active area" just print that */

			if (PrinterCustomFlag)
				{
				Status = StretchDIBits(PrintDC,
					0,
					0,
					(TempWidth * (PrinterAreaXHigh - PrinterAreaXLow)) / PrinterAreaPixels,
					(TempHeight * (PrinterAreaYHigh - PrinterAreaYLow)) / PrinterAreaPixels,
					+PrinterAreaXLow + xoffset,
					BitMapHeight - (-PrinterAreaYLow + yoffset),
					PrinterAreaXHigh - PrinterAreaXLow,
					PrinterAreaYHigh - PrinterAreaYLow,
					BitsPtr, PrintBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
				}

			/* else print the whole thing */

			else
				{
				Status = StretchDIBits(PrintDC,
					0,
					0,
					(TempWidth * BitMapWidth) / PrinterAreaPixels,
					(TempHeight * BitMapHeight) / PrinterAreaPixels,
					0,
					0,
					BitMapWidth,
					BitMapHeight, BitsPtr, PrintBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
				}

			if (Status <= 0)
				{
				MessageBox("Cannot draw image", "Error");
#if defined(__WIN32__)
            char Buffer[64];
            MessageBox(itoa(GetLastError(), Buffer, 10), "Sub Code");
#endif
				}
			}

		/* can't do it */

		else
			{
			MessageBox("Cannot extract image", "Error");
			}
      
      /* restore resources */
      
      if (EnablePalette)
         {
         SelectPalette(ScreenDC, OldPalette, FALSE);
         }
      
      GlobalUnlock(BitsHandle);
      
      GlobalFree(BitsHandle);
      }
   
   /* can't do it */
   
   else
      {
      MessageBox("No Memory to Print", "Error");
      }
   
   ReleaseDC(0, ScreenDC);

   delete PrintBitmapInfo;
   
   }

bool TMyWindow::CanClose()
   {
   HWND EditH;

   EditH = ::FindWindow(NULL, "Editor");

   // if editor is running could loose changes
   
	if (EditH)
      {
      ::ShowWindow(EditH, SW_SHOWNORMAL);
      ::SetWindowPos(EditH, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      JustDidEdit = 1;

      if (MessageBox("Changes in this edit session may be lost.\n\nDo you really want to exit MSWLogo?", "An Edit session is running", MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return FALSE;
      }

   // if not halted warn user and give chance to abort shutdown

   if (halt_flag != 0)
		{
		// if we already tried warn user of doom
      if (Time_To_Halt)
			{
			if (MessageBox("MSWLogo does not like exiting while not halted.\n\nDo you really want to exit MSWLogo?", "Logo is not Halted", MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return FALSE;
			}
		// else let the user optionally halt first
		else
			{
			if (MessageBox("MSWLogo does not like exiting while not halted.\n\nDo you really want to Halt MSWLogo?", "Logo is not Halted", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
				{
				CommandWindow->DoButtonHalt(0);
				}
       	return FALSE;
			}
		}

   // if dirty warn user and give chance to abort shutdown

   if (IsDirty)
		{
		if (MessageBox("You will lose any changes if you don't save your workspace to disk.\n\nDo you really want to exit MSWLogo?", "You have not saved to disk", MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return FALSE;
		}
	
	// if we made it here we are OK

	return TRUE;
   }

void TMyWindow::CMExit()
   {

   /* here on FILE-EXIT main window (screen) */

   if (halt_flag != 0) Time_To_Halt = 1;
   Time_To_Exit = 1;
   }

BOOL TMyWindow::WriteDIB(int TheFile, int MaxBitCount)
   {
   WORD SavebitCount;
   WORD size;

   HDC TempMemDC;
   HDC MemDC;

   LPSTR BitsPtr;
   HBITMAP BitsHandle;
   BITMAPFILEHEADER BitmapFileHeader;
   BITMAPINFO *SaveBitmapInfo;

   /* grab a DC */

   ScreenDC = CreateDC("DISPLAY", NULL, NULL, NULL);

   /* hard code to screen mode */

   SavebitCount = GetDeviceCaps(ScreenDC, BITSPIXEL);
   SavebitCount *= GetDeviceCaps(ScreenDC, PLANES);

   if (MaxBitCount < SavebitCount)
      {
      if ((MaxBitCount == 1) || (MaxBitCount == 4) || (MaxBitCount == 8))
         {
         SavebitCount = MaxBitCount;
         }
      }
   
   /* compute size of bitmap */

   if (SavebitCount == 16) SavebitCount = 24;
   if (SavebitCount == 32) SavebitCount = 24;

   if (SavebitCount <= 8)
      {
      size = sizeof(BITMAPINFOHEADER) + ((1 << SavebitCount) * sizeof(RGBQUAD));
      }
   else
      {
      size = sizeof(BITMAPINFOHEADER);
      }

   SaveBitmapInfo = (BITMAPINFO *) new char[size];

   SaveBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   SaveBitmapInfo->bmiHeader.biWidth = BitMapWidth;
   SaveBitmapInfo->bmiHeader.biHeight = BitMapHeight;
   SaveBitmapInfo->bmiHeader.biPlanes = 1;
   SaveBitmapInfo->bmiHeader.biBitCount = SavebitCount;
   SaveBitmapInfo->bmiHeader.biCompression = BI_RGB;
   SaveBitmapInfo->bmiHeader.biSizeImage = ((((SaveBitmapInfo->bmiHeader.biWidth * SaveBitmapInfo->bmiHeader.biBitCount) + 31) / 32) * 4) * SaveBitmapInfo->bmiHeader.biHeight;
   SaveBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
   SaveBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
   SaveBitmapInfo->bmiHeader.biClrUsed = 0;
   SaveBitmapInfo->bmiHeader.biClrImportant = 0;

   if (PrinterCustomFlag)
      {
      SaveBitmapInfo->bmiHeader.biWidth = PrinterAreaXHigh - PrinterAreaXLow;
      SaveBitmapInfo->bmiHeader.biHeight = PrinterAreaYHigh - PrinterAreaYLow;
      }

   SaveBitmapInfo->bmiHeader.biSizeImage = ((((SaveBitmapInfo->bmiHeader.biWidth * SaveBitmapInfo->bmiHeader.biBitCount) + 31) / 32) * 4) * SaveBitmapInfo->bmiHeader.biHeight;

   // allocate space for the raw DIB data

   BitsHandle = (HBITMAP) GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, SaveBitmapInfo->bmiHeader.biSizeImage);

   /* bummer */

   if (!BitsHandle)
      {
      DeleteDC(ScreenDC);

      return (FALSE);
      }

   /* go find it */

   BitsPtr = (LPSTR) GlobalLock((HGLOBAL) BitsHandle);

   /* if palette yank it in */

   if (EnablePalette)
      {
      OldPalette2 = SelectPalette(ScreenDC, ThePalette, FALSE);
      RealizePalette(ScreenDC);
      }

   /* if custom then use custom dimensions */

   if (PrinterCustomFlag)
      {
      AreaMemoryBitMap = CreateCompatibleBitmap(ScreenDC,
         PrinterAreaXHigh - PrinterAreaXLow,
         PrinterAreaYHigh - PrinterAreaYLow);

      if (!AreaMemoryBitMap)
         {
         MessageBox("Write failed, Possibly no Memory", "Error");
         err_logo(STOP_ERROR, NIL);
         }

      MemDC = CreateCompatibleDC(ScreenDC);
      OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);

      TempMemDC = CreateCompatibleDC(ScreenDC);
      OldBitmap2 = (HBITMAP) SelectObject(TempMemDC, AreaMemoryBitMap);

      BitBlt(TempMemDC,
         0,
         0,
         PrinterAreaXHigh - PrinterAreaXLow,
         PrinterAreaYHigh - PrinterAreaYLow,
         MemDC,
         +PrinterAreaXLow + xoffset,
         -PrinterAreaYHigh + yoffset,
         SRCCOPY);

      SelectObject(TempMemDC, OldBitmap2);
      DeleteDC(TempMemDC);

      SelectObject(MemDC, OldBitmap);
      DeleteDC(MemDC);

      // convert logo bitmap to raw DIB in BitsPtr
      GetDIBits(ScreenDC, AreaMemoryBitMap, 0, PrinterAreaYHigh - PrinterAreaYLow, BitsPtr, SaveBitmapInfo, DIB_RGB_COLORS);

      DeleteObject(AreaMemoryBitMap);
      }

   /* else do whole thing */

   else
      {
      // convert logo bitmap to raw DIB in BitsPtr
      GetDIBits(ScreenDC, MemoryBitMap, 0, BitMapHeight, BitsPtr, SaveBitmapInfo, DIB_RGB_COLORS);
      }

   /* restore some of the resourese */

   if (EnablePalette)
      {
      SelectPalette(ScreenDC, OldPalette2, FALSE);
      }

   DeleteDC(ScreenDC);

   /* build header */

   BitmapFileHeader.bfType = 19778;
   BitmapFileHeader.bfSize = size + sizeof(BITMAPFILEHEADER) + (int) (SaveBitmapInfo->bmiHeader.biWidth * SaveBitmapInfo->bmiHeader.biHeight * (SavebitCount / 8));
   BitmapFileHeader.bfReserved1 = 0;
   BitmapFileHeader.bfReserved2 = 0;
   BitmapFileHeader.bfOffBits = size + sizeof(BITMAPFILEHEADER);

   // write header

   _lwrite(TheFile, (LPSTR) &BitmapFileHeader, sizeof(BitmapFileHeader));

   _lwrite(TheFile, (LPSTR) SaveBitmapInfo, size);

   // write out raw DIB data to file

   GlobalUnlock(BitsHandle);

   PutBitmapData(TheFile, BitsHandle, SaveBitmapInfo->bmiHeader.biSizeImage);

   GlobalFree(BitsHandle);

   delete SaveBitmapInfo;

   return TRUE;
   }

BOOL TMyWindow::DumpBitmapFile(LPSTR Name, int MaxBitCount)
   {
   int TheFile;

   /* open and check if ok */

   TheFile = _lcreat(Name, 0);
   if (TheFile != -1)
      {

      // Load hour-glass cursor.
      hCursor =::SetCursor(hCursorWait);

      /* do it and if error then let user know */

      if (!WriteDIB(TheFile, MaxBitCount))
         {
         MessageBox("Could not Write .BMP", "Error");
         err_logo(STOP_ERROR, NIL);
         }

      // Reload arrow cursor.
      ::SetCursor(hCursor);

      _lclose(TheFile);
      }

   /* else file never opened */

   else
      {
      MessageBox("Could not Open .BMP", "Error");
      err_logo(STOP_ERROR, NIL);
      }

   return TRUE;
   }

/* Attempt to open a Windows 3.0 device independent bitmap. */

BOOL TMyWindow::OpenDIB(int TheFile, DWORD &dwPixelWidth, DWORD &dwPixelHeight)
   {
   WORD ReadbitCount;

   long longWidth;
   HDC DCHandle;
   LPSTR BitsPtr;
   BITMAPINFO *ReadBitmapInfo;

   HBITMAP BitsHandle;
   HBITMAP NewBitmapHandle;

   DWORD NewPixelWidth;
   DWORD NewPixelHeight;

   BITMAPFILEHEADER BitmapFileHeader;
   int i;
   int j;
   int k;

   /* get header */
   
   _llseek(TheFile, 0, 0);
   _lread(TheFile, (LPSTR) & BitmapFileHeader, sizeof(BitmapFileHeader));
   
   /* bfOffbits should be equal to BitmapInfoHeader */
   
   ReadBitmapInfo = (BITMAPINFO *) new char[BitmapFileHeader.bfOffBits - sizeof(BitmapFileHeader)];
   
   _lread(TheFile, (LPSTR) ReadBitmapInfo, BitmapFileHeader.bfOffBits - sizeof(BitmapFileHeader));
   
   if (dwPixelWidth)
      {
      
      /* do what the spec says */
      
      if (ReadBitmapInfo->bmiHeader.biClrUsed == 0) ReadBitmapInfo->bmiHeader.biClrUsed = 1 << ReadBitmapInfo->bmiHeader.biBitCount;
      
      /* only allow bitcount equal to display capability */
      
      ReadbitCount = ReadBitmapInfo->bmiHeader.biBitCount;
            
      /* if palette load up palette from bitmap color table */
      
      if (EnablePalette)
         {
         if (ReadbitCount == 8)
            {
            for (i = 0; i < ReadBitmapInfo->bmiHeader.biClrUsed; i++) LoadColor(
               ReadBitmapInfo->bmiColors[i].rgbRed,
               ReadBitmapInfo->bmiColors[i].rgbGreen,
               ReadBitmapInfo->bmiColors[i].rgbBlue);
            }
         
         /* fill palette with a wide range */
         
         else
            {
            for (i = 0; i < 5; i++)
               for (j = 0; j < 5; j++)
               for (k = 0; k < 5; k++) LoadColor(i * 42, j * 42, k * 42);
            }
         }
      
      /* save some typing */
      
      NewPixelWidth = ReadBitmapInfo->bmiHeader.biWidth;
      NewPixelHeight = ReadBitmapInfo->bmiHeader.biHeight;
      
      /* compute image size if not set */
      
      if (ReadBitmapInfo->bmiHeader.biSizeImage == 0)
         {
         longWidth = (((NewPixelWidth * ReadbitCount) + 31) / 32) * 4;
         
         ReadBitmapInfo->bmiHeader.biSizeImage = longWidth * NewPixelHeight;
         }
      
      /* pack and allocate */
      
      BitsHandle = (HBITMAP) GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, ReadBitmapInfo->bmiHeader.biSizeImage);
      
      /* sorry */
      
      if (!BitsHandle)
         {
         delete ReadBitmapInfo;
         return FALSE;
         }
      
      /* read the file into the bitmap */
      
      GetBitmapData(TheFile, BitsHandle, ReadBitmapInfo->bmiHeader.biSizeImage);
      
      /* Create DC comaptible with screen */
      
      ScreenDC = GetDC(MainHWindow);
      
      MemDC = CreateCompatibleDC(ScreenDC);
      DCHandle = CreateCompatibleDC(ScreenDC);
      
      /* if palette yank it */
      
      if (EnablePalette)
         {
         OldPalette2 = SelectPalette(ScreenDC, ThePalette, FALSE);
         RealizePalette(ScreenDC);
         
         OldPalette = SelectPalette(MemDC, ThePalette, FALSE);
         RealizePalette(MemDC);
         }
      
      /* find it */
      
      BitsPtr = (LPSTR) GlobalLock((HGLOBAL) BitsHandle);
      
      /* now create the bitmap with the bits just loaded */
      
      NewBitmapHandle = CreateDIBitmap(ScreenDC, &(ReadBitmapInfo->bmiHeader), CBM_INIT, BitsPtr, ReadBitmapInfo, 0);
      
      /* now dump the bits */
      
      GlobalUnlock(BitsHandle);
      GlobalFree(BitsHandle);
      
      delete ReadBitmapInfo;
      
      if (EnablePalette)
         {
         SelectPalette(ScreenDC, OldPalette2, FALSE);
         }
      
      ReleaseDC(MainHWindow, ScreenDC);
      
      /* now that things are clean we can check if we are ok */
      
      if (!NewBitmapHandle) return FALSE;
      
      /*
      We've now made a bonafied bitmap. But we want to copy it into the
      existing backing store.
      */
      
      if (EnablePalette)
         {
         OldPalette2 = SelectPalette(DCHandle, ThePalette, FALSE);
         RealizePalette(DCHandle);
         }
      
      OldBitmap2 = (HBITMAP) SelectObject(DCHandle, NewBitmapHandle);
      OldBitmap = (HBITMAP) SelectObject(MemDC, MemoryBitMap);
      
      /* if either dimension is more than half then put in corner */
      
      if ((NewPixelWidth > (BitMapWidth / 2)) || (NewPixelHeight > (BitMapHeight / 2)))
         {
         BitBlt(MemDC, 0, 0, NewPixelWidth, NewPixelHeight, DCHandle, 0, 0, SRCCOPY);
         }
      
      /* else put at turtle */
      
      else
         {
         POINT dest;
         
         if (current_mode == perspectivemode)
            {
            VECTOR from3d;
            
            from3d.x = turtle_p[turtle_which].x / WorldWidth;
            from3d.y = turtle_p[turtle_which].y / WorldHeight;
            from3d.z = turtle_p[turtle_which].z / WorldDepth;
            
            if (!ThreeD.TransformPoint(from3d, dest)) return (TRUE);
            }
         else
            {
            dest.x = g_round(turtle_p[turtle_which].x);
            dest.y = g_round(turtle_p[turtle_which].y);
            }
         
         BitBlt(MemDC,
            +dest.x + xoffset,
            -dest.y + yoffset + LL - NewPixelHeight,
            NewPixelWidth,
            NewPixelHeight, DCHandle, 0, 0, SRCCOPY);
         }
      
      /* return resources */
      
      if (EnablePalette)
         {
         SelectPalette(MemDC, OldPalette, FALSE);
         SelectPalette(DCHandle, OldPalette2, FALSE);
         }
      
      SelectObject(MemDC, OldBitmap);
      SelectObject(DCHandle, OldBitmap2);
      DeleteDC(MemDC);
      DeleteDC(DCHandle);
      DeleteObject(NewBitmapHandle);
      }

   dwPixelWidth  = ReadBitmapInfo->bmiHeader.biWidth;
   dwPixelHeight = ReadBitmapInfo->bmiHeader.biHeight;

   return TRUE;
   }

BOOL TMyWindow::LoadBitmapFile(LPSTR Name, DWORD &dwPixelWidth, DWORD &dwPixelHeight)
   {

   /* Test if the passed file is a Windows 3.0 DIB bitmap and if so read it */

   int TheFile;
   long TestWin30Bitmap;
   char ErrorMsg[50]= "";
   BOOL retval;

   /* open then check if open */

   TheFile = _lopen(Name, OF_READ);
   if (TheFile != -1)
      {

      /* check if valid bitmap */

      _llseek(TheFile, 14, 0);
      _lread(TheFile, (LPSTR) & TestWin30Bitmap, sizeof(TestWin30Bitmap));
      if (TestWin30Bitmap == 40)
         {
         // Load hour-glass cursor.
         hCursor =::SetCursor(hCursorWait);

         /* if loaded ok then invalidate to display */

         if (OpenDIB(TheFile, dwPixelWidth, dwPixelHeight))
            {
            ::InvalidateRect(HWindow, NULL, TRUE);
            }

         /* else error */

         else
            {
            strcpy(ErrorMsg, "Unable to create Windows 3.0 bitmap");
            }

         // Reload arrow cursor.
         ::SetCursor(hCursor);
         }

      /* not a bitmap */

      else
         {
         strcpy(ErrorMsg, "Not a Windows 3.0 bitmap");
         }
      _lclose(TheFile);
      }

   /* else file not there */

   else
      {
      strcpy(ErrorMsg, "Cannot open bitmap file");
      }

   /* if no message the we are ok else display error message */

   if (ErrorMsg[0] == '\0')
      {
      retval = TRUE;
      }
   else
      {
      MessageBox(ErrorMsg, "Error", MB_OK);
      err_logo(STOP_ERROR, NIL);
      retval = FALSE;
      }

   return retval;
   }

/* __ahIncr, ordinal 114, is a 'magic' function. Defining this
function causes Windows to patch the value into the passed
reference.  This makes it a type of global variable. To use
the value of AHIncr, use FP_OFF(AHIncr). */

extern "C"
   {
   void FAR PASCAL __ahIncr();
   }

/* Copys the bitmap bit data from the file into memory. Since
copying cannot cross a segment (64K) boundary, we are forced
to do segment arithmetic to compute the next segment.  Created
a LongType type to simplify the process. */

void TMyWindow::GetBitmapData(int TheFile, HANDLE BitsHandle, long BitsByteSize)
   {
   long Count;
   long Start;
   long ToAddr;
   long Bits;

   Bits = (long) GlobalLock(BitsHandle);

   Start = 0L;

   Count = BitsByteSize;

   while (Count > 0)
      {
#if !defined(__WIN32__)
      ToAddr = MAKELONG(LOWORD(Start), HIWORD(Bits) + (HIWORD(Start) * FP_OFF(__ahIncr)));
#else
      ToAddr = Bits + Start;
#endif
      if (Count > 0x4000) Count = 0x4000;
      _lread(TheFile, (LPSTR) ToAddr, (WORD) Count);
      Start = Start + Count;
      Count = BitsByteSize - Start;
      }

   GlobalUnlock(BitsHandle);
   }

/* Copys the bitmap bit data from the memory into file. Since
copying cannot cross a segment (64K) boundary, we are forced
to do segment arithmetic to compute the next segment.  Created
a LongType type to simplify the process. */

void TMyWindow::PutBitmapData(int TheFile, HANDLE BitsHandle, long BitsByteSize)
   {
   long Count;
   long Start;
   long ToAddr;
   long Bits;

   Bits = (long) GlobalLock(BitsHandle);

   Start = 0L;

   Count = BitsByteSize;

   while (Count > 0)
      {
#if !defined(__WIN32__)
      ToAddr = MAKELONG(LOWORD(Start), HIWORD(Bits) + (HIWORD(Start) * FP_OFF(__ahIncr)));
#else
      ToAddr = Bits + Start;
#endif
      if (Count > 0x4000) Count = 0x4000;
      _lwrite(TheFile, (LPSTR) ToAddr, (WORD) Count);
      Start = Start + Count;
      Count = BitsByteSize - Start;
      }

   GlobalUnlock(BitsHandle);
   }

void TMyWindow::EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl)
   {
   
   // if control is null then it's the Window Scroller
   
   if (NULL == hWndCtl)
      {
      TFrameWindow::EvHScroll(scrollCode, thumbPos, hWndCtl);
      }

   // else bypass the window and send message straight to control
   
   else
      {
#if !defined(__WIN32__)
      ::SendMessage(hWndCtl, WM_HSCROLL, scrollCode, MAKELONG(thumbPos, hWndCtl));
#else
      ::SendMessage(hWndCtl, WM_HSCROLL, MAKELONG(scrollCode, thumbPos), (LPARAM) hWndCtl);
#endif
      }
   }

void TMyWindow::EvVScroll(uint scrollCode, uint thumbPos, HWND hWndCtl)
   {

   // if control is null then it's the Window Scroller
   
   if (NULL == hWndCtl) 
      {
      TFrameWindow::EvVScroll(scrollCode, thumbPos, hWndCtl);
      }

   // else bypass the window and send message straight to control
   
   else
      {
#if !defined(__WIN32__)
      ::SendMessage(hWndCtl, WM_VSCROLL, scrollCode, MAKELONG(thumbPos, hWndCtl));
#else
      ::SendMessage(hWndCtl, WM_VSCROLL, MAKELONG(scrollCode, thumbPos), (LPARAM) hWndCtl);
#endif
      }
   }

void TMyWindow::EvSize(UINT arg1, TSize &arg2)
   {
   TRect RRR;
   int Xr;
   int Yr;
   NUMBER XRatio, YRatio;

   /* pass message down */

   TFrameWindow::EvSize(arg1, arg2);

//   if (arg2.cx > 500)
//      {
//      PostMessage(WM_SIZE, arg1, MAKELONG(500, arg2.cy));
//      }

   /*
      Adjust scroller range so that thumb at each extreme corresponds
      to edge of extreme image.
    */

   GetClientRect(RRR);

   Xr = (BitMapWidth * the_zoom) - RRR.right;
   Yr = (BitMapHeight * the_zoom) - RRR.bottom;

   if (Xr < 0) Xr = 0;
   if (Yr < 0) Yr = 0;

   if (Scroller->XRange <= 0)
      {
      XRatio = 0.5;
      }
   else
      {
      XRatio = (NUMBER) Scroller->XPos / (NUMBER) Scroller->XRange;
      }

   if (Scroller->YRange <= 0)
      {
      YRatio = 0.5;
      }
   else
      {
      YRatio = (NUMBER) Scroller->YPos / (NUMBER) Scroller->YRange;
      }

   Scroller->SetRange(Xr, Yr);
   Scroller->ScrollTo(XRatio * Xr, YRatio * Yr);

   //   Scroller->SetPageSize();

   }

void TMyWindow::CMBitmapNew()
   {
   HBRUSH TempBrush;
   HDC TempDC;

   /* basically all that is done here is to reset things */

   TempBrush = CreateBrushIndirect(&ScreenBrush);
   TempDC = GetDC(MainHWindow);

   //memory

   MemDC = CreateCompatibleDC(TempDC);
   SelectObject(MemDC, MemoryBitMap);

   FillRect(MemDC, &FullRect, TempBrush);

   SetBkColor(MemDC, scolor);
   SetBkMode(MemDC, TRANSPARENT);

   DeleteDC(MemDC);

   ReleaseDC(MainHWindow, TempDC);
   DeleteObject(TempBrush);

   //screen

   ::InvalidateRect(HWindow, NULL, TRUE);

   IsNewBitmap = TRUE;
   }

void TMyWindow::CMBitmapOpen()
   {
   DWORD dwPixelWidth = 1;
   DWORD dwPixelHeight = 1;

   TOpenSaveDialog::TData FileData;
   FileData.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
   FileData.SetFilter("Bitmap Files (*.BMP)|*.BMP|GIF Files (*.GIF)|*.GIF|All Files (*.*)|*.*|");
   strcpy(FileData.FileName, "*.BMP");
   FileData.DefExt = "bmp";

   /* if user found a file then try to load it  */

   if (TFileOpenDialog(this, FileData).Execute() == IDOK)
      {
      char ext[_MAX_EXT];
      
      IsNewBitmap = FALSE;
      strcpy(BitmapName, FileData.FileName);
      
      _splitpath( BitmapName, NULL, NULL, NULL, ext );
      if (stricmp(ext, ".GIF") == 0)
         gifload_helper(BitmapName, dwPixelWidth, dwPixelHeight);
      else
         LoadBitmapFile(BitmapName, dwPixelWidth, dwPixelHeight);
      }
   }

void TMyWindow::CMBitmapSave()
   {
   /* if new file then switch to save file as, else save */

   if (IsNewBitmap) SaveBitmapAs();
   else SaveBitmap();
   }

void TMyWindow::SaveBitmapAs()
   {
   TOpenSaveDialog::TData FileData;

   /* if new then nulify File name */

   if (IsNewBitmap) strcpy(BitmapName, "");

   /* Get file name from user and then save the file */

   FileData.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER;
   FileData.SetFilter("Bitmap Files (*.BMP)|*.BMP|GIF Files (*.GIF)|*.GIF|All Files (*.*)|*.*|");
   strcpy(FileData.FileName, BitmapName);
   FileData.DefExt = "bmp";

   if (TFileSaveDialog(this, FileData).Execute() == IDOK)
      {
      IsNewBitmap = FALSE;
      strcpy(BitmapName, FileData.FileName);
      SaveBitmap();
      }
   }

void TMyWindow::CMBitmapSaveAs()
   {
   SaveBitmapAs();
   }

void TMyWindow::SaveBitmap()
   {
   char ext[_MAX_EXT];
   _splitpath( BitmapName, NULL, NULL, NULL, ext );
   if (stricmp(ext, ".GIF") == 0)
      gifsave_helper(BitmapName, -1, 0, -1, -1, 8);
   else
      DumpBitmapFile(BitmapName, 32);
   }

void TMyWindow::CMFileNew()
   {
   NODE *arg;

   // if doing new and dirty giver user a chance to abort the new

   if (IsDirty)
      {
      if (MainWindowx->CommandWindow->MessageBox("Executing a new will erase all definitions.\n\nContinue with New?", "You have not saved to disk", MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL) return;
      }

   // else start with a clean plate

   IsNewFile = TRUE;
   IsDirty = FALSE;

   arg = lcontents();

   lerase(arg);
   }

void TMyWindow::CMFileOpen()
   {
   int i;

   if (IsDirty)
      {
      if (MainWindowx->CommandWindow->MessageBox("The file being loaded may over write your changes.\n\nContinue with Load?", "You have not saved to disk", MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL) return;
      }

   /* if user found a file the try to load it  */

   TOpenSaveDialog::TData FileData;
   FileData.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
   FileData.SetFilter("Logo Files (*.LGO)|*.LGO|All Files (*.*)|*.*|");
   strcpy(FileData.FileName, "*.LGO");
   FileData.DefExt = "lgo";

   /* if user found a file then try to load it  */

   if (TFileOpenDialog(this, FileData).Execute() == IDOK)
      {
      IsNewFile = FALSE;
      halt_flag++;
      if (halt_flag < 1) halt_flag = 1;
      strcpy(FileName, FileData.FileName);
      fileload(FileName);

      // use dummy loop so that original code with "Breaks" does the right thing

      for (i = 0; i < 1; i++)
         {
         if (stopping_flag == THROWING)
            {
            if (compare_node(throw_node, Error, TRUE) == 0)
               {
               err_print();
               }
            else if (compare_node(throw_node, System, TRUE) == 0)
               break;
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

      halt_flag--;
      if (halt_flag < 0) halt_flag = 0;
      }
   }

void TMyWindow::CMFileSave()
   {
   /* if new file the switch to save file as, else save */

   if (IsNewFile) SaveFileAs();
   else SaveFile();
   }

void TMyWindow::SaveFileAs()
   {
   TOpenSaveDialog::TData FileData;

   /* if new the nulify File name */

   if (IsNewFile) strcpy(FileName, "");

   /* Get file name from user and then save the file */

   FileData.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER;
   FileData.SetFilter("Logo Files (*.LGO)|*.LGO|All Files (*.*)|*.*|");
   strcpy(FileData.FileName, FileName);
   FileData.DefExt = "lgo";

   if (TFileSaveDialog(this, FileData).Execute() == IDOK)
      {
      IsNewFile = FALSE;
      strcpy(FileName, FileData.FileName);
      SaveFile();
      }
   }

void TMyWindow::CMFileSaveAs()
   {
   SaveFileAs();
   }

void TMyWindow::SaveFile()
   {
   filesave(FileName);

   // check for error

   for (int i = 0; i < 1; i++)
      {
      if (stopping_flag == THROWING)
         {
         if (compare_node(throw_node, Error, TRUE) == 0)
            {
            err_print();
            }
         else if (compare_node(throw_node, System, TRUE) == 0) break;
         else if (compare_node(throw_node, Toplevel, TRUE) != 0)
            {
            err_logo(NO_CATCH_TAG, throw_node);
            err_print();
            }
         stopping_flag = RUN;
         }
      if (stopping_flag == STOP || stopping_flag == OUTPUT)
         {
         print_node(stdout, make_static_strnode("You must be in a procedure to use OUTPUT or STOP.\n"));
         stopping_flag = RUN;
         }
      }

   }

void TMyWindow::CMBitmapPrinterArea()
   {
   BOOL bAok;
   
	/* copy real to dynamic */
	
	TPrinterAreaXLow   = PrinterAreaXLow;
	TPrinterAreaXHigh  = PrinterAreaXHigh;
	TPrinterAreaYLow   = PrinterAreaYLow;
	TPrinterAreaYHigh  = PrinterAreaYHigh;
	TPrinterAreaPixels = PrinterAreaPixels;
	TPrinterCustomFlag = PrinterCustomFlag;
	
	do
		{
		bAok = TRUE;
		
		/* if user does not cancel then copy dynamic to real */
		
		if (TMyPrinterAreaWindow(this, "PrinterArea").Execute() == IDOK)
			{
			char szWinLocStr[WININISIZ];
			
			if ((TPrinterAreaXLow >= TPrinterAreaXHigh) || (TPrinterAreaYLow >= TPrinterAreaYHigh))
				{
				MainWindowx->CommandWindow->MessageBox("Bad arguments", "Active Area");
				bAok = FALSE;
				}
			else
				{
				PrinterAreaXLow   = TPrinterAreaXLow;
				PrinterAreaXHigh  = TPrinterAreaXHigh;
				PrinterAreaYLow   = TPrinterAreaYLow;
				PrinterAreaYHigh  = TPrinterAreaYHigh;
				if (TPrinterAreaPixels < 1) TPrinterAreaPixels = 1;
				PrinterAreaPixels = TPrinterAreaPixels;
				PrinterCustomFlag = TPrinterCustomFlag;
				
				sprintf(szWinLocStr, "%d", PrinterAreaXLow);
				WritePrivateProfileString("Printer", "XLow", szWinLocStr, "LOGO.INI");
				sprintf(szWinLocStr, "%d", PrinterAreaXHigh);
				WritePrivateProfileString("Printer", "XHigh", szWinLocStr, "LOGO.INI");
				sprintf(szWinLocStr, "%d", PrinterAreaYLow);
				WritePrivateProfileString("Printer", "YLow", szWinLocStr, "LOGO.INI");
				sprintf(szWinLocStr, "%d", PrinterAreaYHigh);
				WritePrivateProfileString("Printer", "YHigh", szWinLocStr, "LOGO.INI");
				sprintf(szWinLocStr, "%d", PrinterAreaPixels);
				WritePrivateProfileString("Printer", "Pixels", szWinLocStr, "LOGO.INI");
				}
			}
		} while (!bAok);
   }

void TMyWindow::CMFileEdit()
   {
   NODE *arg;

   // create dialog for which procedure to edit

   TMyFileEditWindow FileEditWindow(this, "DIALOGEDIT");

   FileEditWindow.FileEditAll = 0;

   // now do it

   if (FileEditWindow.Execute() == IDOK)
      {

      // if user clicked ALL get all procedures

      if (FileEditWindow.FileEditAll == 1)
         {
         arg = lprocedures();
         }

      // else find what user selected

      else
         {
         arg = cons(make_strnode(SelectedText, NULL, strlen(SelectedText), STRING, strnzcpy), NIL);
         }

      // if something edit it

      if (arg != NIL) ledit(arg);

      }
   }

void TMyWindow::CMFileErase()
   {
   NODE *arg;

   // create dialog for which procedure to erase

   TMyFileEditWindow FileEditWindow(this, "DIALOGERASE");

   FileEditWindow.FileEditAll = 0;

   // now do it

   if (FileEditWindow.Execute() == IDOK)
      {

      // if user clicked ALL get all procedures

      if (FileEditWindow.FileEditAll == 1)
         {
         arg = lprocedures();

         IsNewFile = TRUE;
         IsDirty = FALSE;
         }

      // else find what user selected

      else
         {
         arg = cons(make_strnode(SelectedText, NULL, strlen(SelectedText), STRING, strnzcpy), NIL);
         }

      // if something erase it

      if (arg != NIL) lerase(arg);

      }
   }

void TMyWindow::MyPopupEdit(char *FileName, NODE *args)
   {
   char *cp;
   char szWinLocStr[WININISIZ];
   char dfWinLocStr[WININISIZ];

   int x;
   int y;
   int w;
   int h;

   /* if called with NULL filename then prompt user */

#if !defined(__WIN32__)
   HGLOBAL hEditHeap = GlobalAlloc(GMEM_ZEROINIT, 0xFFFF);
   EditWindow = new TMyFileWindow(this, "Editor", FileName, args, new TModule("", (HMODULE) hEditHeap, ""), hEditHeap);
#else
   EditWindow = new TMyFileWindow(this, "Editor", FileName, args, 0, 0);
#endif

   /* Do win.ini stuff. Build default coords */

   x = (int) (MaxWidth * 0.25);
   y = (int) (MaxHeight * 0.25);
   w = (int) (MaxWidth * 0.75);
   h = (int) (MaxHeight * ScreenSz * 0.75);

	/* convert */
	
	sprintf(dfWinLocStr, "%d,%d,%d,%d", x, y, w, h);
	
	// Get last location and size of Edit Window from WIN.INI file.
	
	GetPrivateProfileString("LOGO", "Editor", dfWinLocStr, szWinLocStr, sizeof(szWinLocStr), "LOGO.INI");
	
	// Decode location and size of window from profile string.
	
	cp = szWinLocStr;
	x = (int) strtol(cp, &cp, 10);
	cp++;
	y = (int) strtol(cp, &cp, 10);
	cp++;
	w = (int) strtol(cp, &cp, 10);
	cp++;
	h = (int) strtol(cp, &cp, 10);
	
   checkwindow(&x, &y, &w, &h);

   /* now set them */

   EditWindow->Attr.Style = WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
#if defined(__WIN32__)
//   EditWindow->Attr.ExStyle |= WS_EX_DLGMODALFRAME;
#endif

   /*
      EditWindow->Attr.X = x;
      EditWindow->Attr.Y = y;
      EditWindow->Attr.W = w;
      EditWindow->Attr.H = h;
    */

   /* flag for how to handle focus */

   if (args != NULL) JustDidEdit = 1;

   EditWindow->Editor->Attr.Style |= ES_NOHIDESEL;

   /* let user edit */

   EditWindow->Create();

//#if defined(__WIN32__)
//   ::SetWindowLong( EditWindow->HWindow, GWL_EXSTYLE , ::GetWindowLong(EditWindow->HWindow, GWL_EXSTYLE) | WS_EX_DLGMODALFRAME); 
//#endif

   /* force a resize to fix RichEdit ScrollBar no appearing auto matically */

   EditWindow->SetWindowPos(0, x, y, w + 1, h, SWP_NOZORDER);
   EditWindow->SetWindowPos(0, x, y, w, h, SWP_NOZORDER);

   /* Undo the "-" RichEdit did on the main Window */

   SetCaption("MSWLogo Screen");

   if (args != NULL)
      {

      /* retitle without filename */

      EditWindow->SetWindowText("Editor");

      // if an error occured "force" a change so that we still in
      // "dirty" state

      if (error_happen)
         {
         error_happen = 0;
         EditWindow->Editor->Insert(" ");
         EditWindow->Editor->DeleteSubText(0, 1);
         int iLine = EditWindow->Editor->GetLineFromPos(LinesLoadedOnEdit);
         EditWindow->Editor->Scroll(0, iLine);
         EditWindow->Editor->SetSelection(LinesLoadedOnEdit, LinesLoadedOnEdit);
         }

      /* keep global pointer for hacking around */

      EdtHWindow = EditWindow->HWindow;
      }
   }

int TMyWindow::MyPopupInput(char *str, char *pmt)
   {
   // get user input

   if (TInputDialog(this, pmt, "Input:", str, MAX_BUFFER_SIZE).Execute() == IDOK)
      {
      return (1);
      }
   else
      {
      return (0);
      }
   }

void TMyWindow::SetupWindow()
   {
   TRect RR;

   TFrameWindow::SetupWindow();

   MainHWindow = HWindow;

   /* adjust scrollers */

   GetClientRect(RR);
   Scroller->SetRange(BitMapWidth - RR.right, BitMapHeight - RR.bottom);
   Scroller->ScrollTo((BitMapWidth / 2) - (0.5 * RR.right), (BitMapHeight / 2) - (0.5 * RR.bottom));

   /* it's show time for our little friend */

   draw_turtle(1);
   term_init();

#define ABOUT_VER "About_65"

   int iAbout = GetPrivateProfileInt("LOGO", ABOUT_VER, 1, "LOGO.INI");

   if (iAbout <= 10)
      {
      CMHelpAbout();
      if (iAbout <= 2) CMHelpAboutMS();
      char szWinLocStr[WININISIZ];
      sprintf(szWinLocStr, "%d", iAbout + 1);
      WritePrivateProfileString("LOGO", ABOUT_VER, szWinLocStr, "LOGO.INI");
      }

   /* pop up commander */

   MyPopupCommander();
   }

void TMyWindow::MyPopupCommander()
   {

   /* init commander window */

   char *cp;
   char szWinLocStr[WININISIZ];
   char dfWinLocStr[WININISIZ];
   int x, y, w, h;
   LOGFONT lf;
   //   BOOL bSetFont;

   _fmemset(&lf, 0, sizeof(LOGFONT));

   //   GetPrivateProfileString("CommanderFont", "FaceName", "UseDefault",(LPSTR) & lf.lfFaceName[0], LF_FACESIZE, "Logo.ini");
   //   if (strcmp(lf.lfFaceName, "UseDefault") == 0) bSetFont = FALSE; else bSetFont = TRUE;

   GetPrivateProfileString("CommanderFont", "FaceName", "Courier", (LPSTR) & lf.lfFaceName[0], LF_FACESIZE, "Logo.ini");
   lf.lfHeight = GetPrivateProfileInt("CommanderFont", "Height", -13, "Logo.ini");
   lf.lfWeight = GetPrivateProfileInt("CommanderFont", "Weight", 400, "Logo.ini");
   lf.lfItalic = GetPrivateProfileInt("CommanderFont", "Italic", 0, "Logo.ini");
   lf.lfCharSet = GetPrivateProfileInt("CommanderFont", "CharSet", 0, "Logo.ini");
   lf.lfOutPrecision = GetPrivateProfileInt("CommanderFont", "OutPrecision", 1, "Logo.ini");
   lf.lfClipPrecision = GetPrivateProfileInt("CommanderFont", "ClipPrecision", 2, "Logo.ini");
   lf.lfQuality = GetPrivateProfileInt("CommanderFont", "Quality", 1, "Logo.ini");
   lf.lfPitchAndFamily = GetPrivateProfileInt("CommanderFont", "PitchAndFamily", 49, "Logo.ini");

   hCmdFont = CreateFontIndirect(&lf);

   /* create resourced window */

   CommandWindow = new TMyCommandWindow(this, "DIALOGCOMMAND");
   CommandWindow->Create();

   CmdHWindow = CommandWindow->HWindow;

   EditboxWindow = new TMyEditboxWindow(CommandWindow, ID_EDITINPUT, 0);
   EditboxWindow->Create();
   EditboxWindow->SetWindowFont(hCmdFont, TRUE);

#if !defined(__WIN32__)
   hListboxHeap = GlobalAlloc(GMEM_ZEROINIT, 0xFFFF);
   pm = new TModule("", (HMODULE) hListboxHeap, "");
#else
   pm = 0;
#endif

   ListboxWindow = new TMyListboxWindow(CommandWindow, ID_LISTBOX, 0, pm);
   ListboxWindow->Create();
   ListboxWindow->SetWindowFont(hCmdFont, TRUE);

   //   TButton *TraceBtnWindow;
   //
   //   TraceBtnWindow = new TButton(CommandWindow, ID_TRACE, 0);
   //   TraceBtnWindow->Create();
   //   if (bSetFont) TraceBtnWindow->SetWindowFont(hFont, TRUE);
   //   delete ExecBtnWindow;

   /* get handle to each child window for resize purposes */

   ListHWindow = CommandWindow->GetDlgItem(ID_LISTBOX);
   EditHWindow = CommandWindow->GetDlgItem(ID_EDITINPUT);
   ExecuteHWindow = CommandWindow->GetDlgItem(ID_EXECUTE);
   EdallHWindow = CommandWindow->GetDlgItem(ID_EDALL);
   HaltHWindow = CommandWindow->GetDlgItem(ID_HALT);
   TraceHWindow = CommandWindow->GetDlgItem(ID_TRACE);
   PauseHWindow = CommandWindow->GetDlgItem(ID_PAUSE);
   StatusHWindow = CommandWindow->GetDlgItem(ID_STATUS);
   YieldHWindow = CommandWindow->GetDlgItem(ID_YIELD);
   ResetHWindow = CommandWindow->GetDlgItem(ID_RESET);

   /* build default coords */

   x = MainWindowx->Attr.X;
   y = MainWindowx->Attr.Y + MainWindowx->Attr.H;
   w = MainWindowx->Attr.W;
   h = MaxHeight - MainWindowx->Attr.H;

   if (!bFixed)
		{
		/* convert */
		
		sprintf(dfWinLocStr, "%d,%d,%d,%d", x, y, w, h);
		szWinLocStr[0] = '\0';
		
		// Get last location and size of command window from WIN.INI file.
		
		GetPrivateProfileString("LOGO", "Commander", dfWinLocStr, szWinLocStr, sizeof(szWinLocStr), "LOGO.INI");
		
		// Decode location and size of window from profile string.
		
		cp = szWinLocStr;
		x = (int) strtol(cp, &cp, 10);
		cp++;
		y = (int) strtol(cp, &cp, 10);
		cp++;
		w = (int) strtol(cp, &cp, 10);
		cp++;
		h = (int) strtol(cp, &cp, 10);
		}
	
   checkwindow(&x, &y, &w, &h);

#if defined(__WIN32__)
// ::SetWindowLong( CommandWindow->HWindow, GWL_EXSTYLE , ::GetWindowLong(CommandWindow->HWindow, GWL_EXSTYLE) | WS_EX_DLGMODALFRAME); 
#endif

   /* set it */

   CommandWindow->SetWindowPos(NULL, x, y, w, h, 0);
   CommandWindow->ShowWindow(SW_SHOW);

   Command_OK = 1;
   }

void TMyWindow::MyPopupStatusKill()
   {
   TRect wrect;
   int w, h;
   char szWinLocStr[WININISIZ];

   status_flag = 0;

   // Get location and size of our window on the screen so we can
   // come back up in the same spot next time we are invoked.

   StatusWindow->GetWindowRect(wrect);
   w = wrect.right - wrect.left;
   h = wrect.bottom - wrect.top;

   // Make a string with our window location and size.
   sprintf(szWinLocStr, "%d,%d,%d,%d", wrect.left, wrect.top, w, h);

   // Save in WIN.INI file.
   WritePrivateProfileString(
      "LOGO",
      "Status",
      szWinLocStr,
      "LOGO.INI");

   /* now kill it */

   delete StatusWindow;
   MainWindowx->CommandWindow->
      SendDlgItemMsg(ID_STATUS, WM_SETTEXT, 0, (DWORD) "Status");
   }

void TMyWindow::MyPopupStatus()
   {

   /* popup status window */

   char *cp;
   char szWinLocStr[WININISIZ];
   char dfWinLocStr[WININISIZ];
   int x, y, w, h;

   /* flag so that updates are sent */

   status_flag = 1;
   uCurrentTime = GetCurrentTime();

   /* pop it up */

   StatusWindow = new TMyStatusWindow(this, "DIALOGSTATUS");

   /* update button */

   MainWindowx->CommandWindow->
      SendDlgItemMsg(ID_STATUS, WM_SETTEXT, 0, (DWORD) "NoStatus");

   StatusWindow->Create();

   /* save a handle */

   StatHWindow = StatusWindow->HWindow;

   /* build default coords */

   x = (int) (0);
   y = (int) (0);
   w = (int) (0);
   h = (int) (0);

   /* convert */

   sprintf(dfWinLocStr, "%d,%d,%d,%d", x, y, w, h);

   // Get last location and size of status window from WIN.INI file.

   GetPrivateProfileString("LOGO", "Status", dfWinLocStr, szWinLocStr, sizeof(szWinLocStr), "LOGO.INI");

   // Decode location and size of window from profile string.

   cp = szWinLocStr;
   x = (int) strtol(cp, &cp, 10);
   cp++;
   y = (int) strtol(cp, &cp, 10);
   cp++;
   w = (int) strtol(cp, &cp, 10);
   cp++;
   h = (int) strtol(cp, &cp, 10);

   checkwindow(&x, &y, &w, &h);

   /* now set position */

   StatusWindow->SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE);
   StatusWindow->ShowWindow(SW_SHOW);

   /* update all fields */

   update_status_turtleposition();
   update_status_pencolor();
   update_status_floodcolor();
   update_status_screencolor();
   update_status_paletteuse();
   update_status_penwidth();
   update_status_turtleheading();
   update_status_turtleroll();
   update_status_turtlepitch();
   update_status_penstyle();
   update_status_pencontact();
   update_status_turtlevisability();
   update_status_turtlepitch();
   update_status_turtleroll();
   update_status_turtlewhich();
   update_status_evals();
   update_status_memory();
   update_status_vectors();
   }

void TMyWindow::CMControlExecute()
   {
   HWND EditH;
   HWND TempH;

   EditH = FindWindow(NULL, "Editor");
   TempH = GetActiveWindow();

   // if Main is active find alternate

   if (TempH == MainHWindow)
      {

      // if commander up then focud to input box

      if (!CommandWindow->IsIconic())
         {
         ::SetFocus(EditHWindow);
         }

      // else if a available editor go there

      else if (EditH != NULL)
         {
         if (!::IsIconic(EditH))
            {
            ::SetFocus(EditH);
            }
         }
      }

   // else if active is commander find alternate

   else if (TempH == CommandWindow->HWindow)
      {

      // if a available editor maybe go there

      if (EditH != NULL)
         {

         // if really available then go there

         if (!::IsIconic(EditH))
            {
            ::SetFocus(EditH);
            }

         // else go to main

         else
            {
            SetFocus();
            }
         }

      // else go to main

      else
         {
         SetFocus();
         }
      }

   // else go to main

   else
      {
      SetFocus();
      }
   }

void TMyWindow::CMZoomIn()
   {
   zoom_helper(the_zoom * 2.0);
   }

void TMyWindow::CMZoomOut()
   {
   zoom_helper(the_zoom * 0.5);
   }

void TMyWindow::CMZoomNormal()
   {
   zoom_helper(1.0);
   }

void TMyWindow::CMSetFont()
   {
   static CHOOSEFONT CF;

   // clear the struct

   memset(&CF, 0, sizeof(CF));

   // fill it with the right stuff

   CF.lStructSize = sizeof(CF);
   CF.hwndOwner = MainHWindow;
   CF.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
   CF.nFontType = SCREEN_FONTTYPE;
   CF.lpLogFont = &FontRec;
   CF.hInstance = 0;

   // FontRec is not trashed in case of cancel

   ChooseFont(&CF);

   // show changes

   //   if (status_flag)
   //      {
   //      update_status_fontwieght();
   //      update_status_fontsize();
   //      update_status_fontname();
   //      }

   }

void TMyWindow::CMSetCommanderFont()
   {
   static CHOOSEFONT CF;

   LOGFONT lf;

   _fmemset(&lf, 0, sizeof(LOGFONT));

   GetPrivateProfileString("CommanderFont", "FaceName", "Courier", (LPSTR) & lf.lfFaceName[0], LF_FACESIZE, "Logo.ini");
   lf.lfHeight = GetPrivateProfileInt("CommanderFont", "Height", -13, "Logo.ini");
   lf.lfWeight = GetPrivateProfileInt("CommanderFont", "Weight", 400, "Logo.ini");
   lf.lfItalic = GetPrivateProfileInt("CommanderFont", "Italic", 0, "Logo.ini");
   lf.lfCharSet = GetPrivateProfileInt("CommanderFont", "CharSet", 0, "Logo.ini");
   lf.lfOutPrecision = GetPrivateProfileInt("CommanderFont", "OutPrecision", 1, "Logo.ini");
   lf.lfClipPrecision = GetPrivateProfileInt("CommanderFont", "ClipPrecision", 2, "Logo.ini");
   lf.lfQuality = GetPrivateProfileInt("CommanderFont", "Quality", 1, "Logo.ini");
   lf.lfPitchAndFamily = GetPrivateProfileInt("CommanderFont", "PitchAndFamily", 49, "Logo.ini");

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

      WritePrivateProfileString("CommanderFont", "FaceName", (LPSTR) & lf.lfFaceName[0], "Logo.ini");
      wsprintf(tmp, "%d", lf.lfHeight); WritePrivateProfileString("CommanderFont", "Height", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfWeight); WritePrivateProfileString("CommanderFont", "Weight", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfItalic); WritePrivateProfileString("CommanderFont", "Italic", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfCharSet); WritePrivateProfileString("CommanderFont", "CharSet", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfOutPrecision); WritePrivateProfileString("CommanderFont", "OutPrecision", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfClipPrecision); WritePrivateProfileString("CommanderFont", "ClipPrecision", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfQuality); WritePrivateProfileString("CommanderFont", "Quality", (LPSTR) tmp, "Logo.ini");
      wsprintf(tmp, "%d", lf.lfPitchAndFamily); WritePrivateProfileString("CommanderFont", "PitchAndFamily", (LPSTR) tmp, "Logo.ini");

      hFont = CreateFontIndirect(&lf);
      MainWindowx->ListboxWindow->SetWindowFont(hFont, TRUE);
      MainWindowx->EditboxWindow->SetWindowFont(hFont, TRUE);

		if (hCmdFont) DeleteObject(hCmdFont);
		hCmdFont = hFont;
      }

   }

void TMyWindow::CMSetPenSize()
   {
   TColor TheSize(pen_width, pen_width, pen_width);

   // if OK then make change

   if (TSizeDialog(this, TheSize, "Pen Size").Execute() == IDOK)
      {
      set_pen_width(TheSize.Red());
      set_pen_height(TheSize.Red());
      save_size();
      }

   }

void TMyWindow::CMSetPenColor()
   {
   TColor TheColor(pcolor);

   // if OK then make change

   if (TColorDialog(this, TheColor, "Pen Color").Execute() == IDOK)
      {
      thepencolor(TheColor.Red(), TheColor.Green(), TheColor.Blue());
      save_color_pen();
      }

   }

void TMyWindow::CMSetFloodColor()
   {
   TColor TheColor(fcolor);

   // if OK then make changes

   if (TColorDialog(this, TheColor, "Flood Color").Execute() == IDOK)
      {
      thefloodcolor(TheColor.Red(), TheColor.Green(), TheColor.Blue());
      save_color_flood();
      }

   }

void TMyWindow::CMSetScreenColor()
   {
   TColor TheColor(scolor);

   // if OK then make changes

   if (TColorDialog(this, TheColor, "Screen Color").Execute() == IDOK)
      {
      thescreencolor(TheColor.Red(), TheColor.Green(), TheColor.Blue());
      save_color_screen();
      }

   }

void TMyWindow::CMHelp()
   {
#if defined(__WIN32__)
   WinHelp(szHelpFileName, HELP_FINDER, 0L);
#else
   WinHelp(szHelpFileName, HELP_INDEX, 0L);
#endif

   }

void TMyWindow::CMHelpMCI()
   {
   WinHelp(MCIHelpFileName, HELP_INDEX, 0L);
   }

void TMyWindow::CMHelpHelp()
   {
#if defined(__WIN32__)
   WinHelp("WINHELP.HLP", HELP_FINDER, 0L);
#else
   WinHelp("WINHELP.HLP", HELP_INDEX, 0L);
#endif
   }

void TMyWindow::CMHelpDemo()
   {
   do_execution("demo");
   }

void TMyWindow::CMHelpTutorial()
   {
   do_help("Where to Start");
//   do_execution("tutor");
   }

void TMyWindow::CMHelpExamples()
   {
   char szFileName[EXE_NAME_MAX_SIZE + 1];

   strcpy(szFileName, "NOTEPAD ");
   MakeHelpPathName(&szFileName[8], "EXAMPLES\\README.TXT");
   WinExec(szFileName, SW_SHOWNORMAL);
   }

void TMyWindow::CMHelpReleaseNotes()
   {
   char szFileName[EXE_NAME_MAX_SIZE + 1];

   strcpy(szFileName, "NOTEPAD ");
   MakeHelpPathName(&szFileName[8], "MSWLOGO.TXT");
   WinExec(szFileName, SW_SHOWNORMAL);
   }

void TMyWindow::CMHelpAbout()
   {
   TDialog(this, "AboutBox").Execute();
	}

void TMyWindow::CMHelpAboutMS()
   {
   TDialog(this, "AboutMSBox").Execute();
   }

// Execute File:Print command

void TMyWindow::CMBitmapPrint()
   {
   if (Printer)
      {
      TRulerOut Printout("Logo Picture");
#if defined(__WIN32__)
      Printout.SetBanding(false);
#else
      Printout.SetBanding(true);
#endif
      Printer->Print(this, Printout, true);
      }
   }


// Execute File:Printer-setup command

void TMyWindow::CMBitmapPrinterSetup()
   {
   if (Printer) Printer->Setup(this);
   }

void TMyWindow::EvDestroy()
   {
   TRect wrect;
   char szWinLocStr[WININISIZ];

   int w, h;

   // don't save sizes if iconed

   if (!IsIconic())
      {
      // Get location and size of our window on the screen so we can
      // come back up in the same spot next time we are invoked.

      GetWindowRect(wrect);
      w = wrect.right - wrect.left;
      h = wrect.bottom - wrect.top;

      // Make a string with our window location and size.
      sprintf(szWinLocStr, "%d,%d,%d,%d", wrect.left, wrect.top, w, h);

      // Save in WIN.INI file.
      WritePrivateProfileString(
         "LOGO",
         "Screen",
         szWinLocStr,
         "LOGO.INI");
      }

   TWindow::EvDestroy();
   }

void TMyWindow::EvLButtonDown(UINT, TPoint &point)
   {
   callthing *callevent;

   SetCapture();

   // if user turned on mouse the queue up event

   if (mouse_on == 1)
      {
      callevent = new callthing;

      callevent->func = mouse_lbuttondown;
      callevent->arg1 = point.x;
      callevent->arg2 = point.y;
      callevent->kind = 1;

      calllists.insert(callevent, 1);
      checkqueue();
      }
   }

void TMyWindow::EvLButtonUp(UINT, TPoint &point)
   {
   callthing *callevent;

   ReleaseCapture();

   // if user turned on mouse the queue up event

   if (mouse_on == 1)
      {
      callevent = new callthing;

      callevent->func = mouse_lbuttonup;
      callevent->arg1 = point.x;
      callevent->arg2 = point.y;
      callevent->kind = 1;

      calllists.insert(callevent, 1);
      checkqueue();
      }

   }

void TMyWindow::EvRButtonDown(UINT, TPoint &point)
   {
   callthing *callevent;

   SetCapture();

   // if user turned on mouse the queue up event

   if (mouse_on == 1)
      {
      callevent = new callthing;

      callevent->func = mouse_rbuttondown;
      callevent->arg1 = point.x;
      callevent->arg2 = point.y;
      callevent->kind = 1;

      calllists.insert(callevent, 1);
      checkqueue();
      }

   }

void TMyWindow::EvRButtonUp(UINT, TPoint &point)
   {
   callthing *callevent;

   ReleaseCapture();

   // if user turned on mouse the queue up event

   if (mouse_on == 1)
      {
      callevent = new callthing;

      callevent->func = mouse_rbuttonup;
      callevent->arg1 = point.x;
      callevent->arg2 = point.y;
      callevent->kind = 1;

      calllists.insert(callevent, 1);
      checkqueue();
      }

   }

void TMyWindow::EvMouseMove(UINT, TPoint &point)
   {
   callthing *callevent;

   // if user turned on mouse the queue up event

   if (mouse_on == 1)
      {
      callevent = new callthing;

      callevent->func = mouse_mousemove;
      callevent->arg1 = point.x;
      callevent->arg2 = point.y;
      callevent->kind = 1;

      calllists.insert(callevent, 1);
      checkqueue();
      }
   }

LRESULT TMyWindow::WMCheckQueue(WPARAM, LPARAM)
   {
   TMessage msg = __GetTMessage();

   checkqueue();
   return (LRESULT) msg.Result;
   }

LRESULT TMyWindow::OnNetworkConnectSendAck(WPARAM /* wParam */, LPARAM lParam)
   {
   TMessage msg = __GetTMessage();
   char Buffer[MAX_PACKET_SIZE];
   int status;
   callthing *callevent;

   if (WSAGETASYNCERROR(lParam) != 0)
      {
      MessageBox(WSAGetLastErrorString(WSAGETASYNCERROR(lParam)), "WSAAsyncGetHostByNameCallBack()");
      //    err_logo(STOP_ERROR,NIL);
      return 0L;
      }

   // queue message if send is enabled

   if (network_send_on == 1)
      {

      // update flags based on event type

      switch (WSAGETSELECTEVENT(lParam))
         {
          case FD_READ:
              {
                 memset(Buffer, 0, MAX_PACKET_SIZE);

                 // get a copy first for examination

                 if ((status = lprecv(sendSock, Buffer, MAX_PACKET_SIZE - 1, MSG_PEEK)) == SOCKET_ERROR)
                    {
                    // int iErrorCode;

                    // if block wait til we get called again

                    if ((/* iErrorCode = */ lpWSAGetLastError()) == WSAEWOULDBLOCK) return 0L;

                    MessageBox(WSAGetLastErrorString(0), "recv(sendsock)");
                    //             err_logo(STOP_ERROR,NIL);
                    return 0L;
                    }

                 // if something is there (better be) then process it

                 if (status != 0)
                    {
                    char *pBuffer;
                    int i;

                    // last byte is not end of packet then try to find one

                    if (Buffer[status - 1] |= 0)
                       {

                       // find last end of packet

                       for (i = status - 1; i >= 0; i--) if (Buffer[i] == 0) break;

                       // if not found

                       if (i < 0)
                          {
                          // if not full wait for more
                          if (status < MAX_PACKET_SIZE - 1) return 0L;

                          // read the whole thing anyway
                          i = MAX_PACKET_SIZE - 2;
                          }

                       // read for real up to a last packet boundary

                       memset(Buffer, 0, MAX_PACKET_SIZE);
                       status = lprecv(sendSock, Buffer, i + 1, 0);
                       }
                    else
                       {

                       // read the whole thng for real

                       memset(Buffer, 0, MAX_PACKET_SIZE);
                       status = lprecv(sendSock, Buffer, MAX_PACKET_SIZE - 1, 0);
                       }

                    i = 0;

                    // now queue up a seperate message for each packet

                    while (1)
                       {
                       pBuffer = (char *) malloc(strlen(&Buffer[i]) + 1);
                       strcpy(pBuffer, &Buffer[i]);

                       callevent = new callthing;

                       callevent->func = network_send_receive;
                       callevent->arg1 = LOWORD(pBuffer);
                       callevent->arg2 = HIWORD(pBuffer);
                       callevent->kind = 6;

                       calllists.insert(callevent, callevent->kind);

                       PostMessage(WM_CHECKQUEUE, 0, 0);
                       //                checkqueue();

                       i += strlen(&Buffer[i]) + 1;
                       if (i >= status) break;
                       }
                    }

                 return 0L;
              }

          case FD_WRITE:

              // allow another frame to go out.

              bSendBusy = FALSE;
              break;

          case FD_CONNECT:

              // flag it's ok to start firing

              bSendConnected = TRUE;
              break;

          case FD_CLOSE:

              // done

              bSendConnected = FALSE;
              break;
         }

      // we don't distinguish between all event types

      callevent = new callthing;

      callevent->func = network_send_send;
      callevent->arg1 = 0;
      callevent->arg2 = 0;
      callevent->kind = 4;

      calllists.insert(callevent, 4);

      PostMessage(WM_CHECKQUEUE, 0, 0);
      //    checkqueue();
      }

   return 0L;
   }

LRESULT TMyWindow::OnNetworkConnectSendFinish(WPARAM /* wParam */, LPARAM lParam)
   {
   TMessage msg = __GetTMessage();

   // should these not be automatic?
   SOCKADDR_IN send_dest_sin;

   if (WSAGETASYNCERROR(lParam) != 0)
      {
      MessageBox(WSAGetLastErrorString(WSAGETASYNCERROR(lParam)), "WSAAsyncGetHostByNameCallBack()");
      //    err_logo(STOP_ERROR,NIL);
      return 0L;
      }

   // always start clean

   memset(&send_dest_sin, 0, sizeof(SOCKADDR_IN));

   // what else is there

   send_dest_sin.sin_family = AF_INET;

   if (phes)
		{
		memcpy((char FAR *) &(send_dest_sin.sin_addr), phes->h_addr, phes->h_length);
		}
	else
		{
      MessageBox("Unexpected Error, Network may be shutdown", "Network Error");
      //    err_logo(STOP_ERROR,NIL);
		return 0L;
		}

   // set ports

   send_dest_sin.sin_port = lphtons(sendPort);/* Convert to network ordering  */

   // watch for connect

   if (lpWSAAsyncSelect(sendSock, MainHWindow, WM_NETWORK_CONNECTSENDACK, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR)
      {
      MessageBox(WSAGetLastErrorString(0), "WSAAsyncSelect(sendSock) FD_CONNECT");
      //    err_logo(STOP_ERROR,NIL);
      }

   // lets try now

   if (lpconnect(sendSock, (PSOCKADDR) & send_dest_sin, sizeof(send_dest_sin)) == SOCKET_ERROR)
      {
      // int iErrorCode;

      if ((/* iErrorCode = */ lpWSAGetLastError()) != WSAEWOULDBLOCK)
         {
         MessageBox(WSAGetLastErrorString(0), "connect(sendsock)");
         //       err_logo(STOP_ERROR,NIL);
         return 0L;
         }
      }

   // fake an FD_CONNECT for UDP, these would of been called by winsock if TCP

#ifdef UDP
   OnNetworkConnectSendAck(0, MAKELONG(FD_CONNECT, FD_CONNECT));
#endif

   // fire event that connection is made

   if (network_send_on == 1)
      {
      callthing *callevent = new callthing;

      callevent->func = network_send_send;
      callevent->arg1 = 0;
      callevent->arg2 = 0;
      callevent->kind = 4;

      calllists.insert(callevent, 4);

      PostMessage(WM_CHECKQUEUE, 0, 0);
      //    checkqueue();
      }

   return 0L;
   }

LRESULT TMyWindow::OnNetworkListenReceiveAck(WPARAM /* wParam */, LPARAM lParam)
   {
   SOCKADDR_IN acc_sin;              /* Accept socket address - internet style*/
   int acc_sin_len;                    /* Accept socket address length        */
   int status;
   char Buffer[MAX_PACKET_SIZE];
   callthing *callevent;

   TMessage msg = __GetTMessage();

   if (WSAGETASYNCERROR(lParam) != 0)
      {
      MessageBox(WSAGetLastErrorString(WSAGETASYNCERROR(lParam)), "WSAAsyncGetHostByNameCallBack()");
      //    err_logo(STOP_ERROR,NIL);
      return 0L;
      }

   // if receive is on process it

   if (network_receive_on == 1)
      {

      // based on event do the right thing

      switch (WSAGETSELECTEVENT(lParam))
         {
          case FD_READ:
              {
                 memset(Buffer, 0, MAX_PACKET_SIZE);

                 // get a copy first for examination

                 if ((status = lprecv(receiveSock, Buffer, MAX_PACKET_SIZE - 1, MSG_PEEK)) == SOCKET_ERROR)
                    {
                    // int iErrorCode;

                    // if block wait til we get called again

                    if ((/* iErrorCode = */ lpWSAGetLastError()) == WSAEWOULDBLOCK) return 0L;

                    MessageBox(WSAGetLastErrorString(0), "recv(receivesock)");
                    //             err_logo(STOP_ERROR,NIL);
                    return 0L;
                    }

                 // if something is there (better be) then process it

                 if (status != 0)
                    {
                    char *pBuffer;
                    int i;

                    // last byte is not end of packet then try to find one

                    if (Buffer[status - 1] |= 0)
                       {
                       for (i = status - 1; i >= 0; i--) if (Buffer[i] == 0) break;

                       // if not found

                       if (i < 0)
                          {
                          // if not full wait for more
                          if (status < MAX_PACKET_SIZE - 1) return 0L;

                          // read the whole thing anyway
                          i = MAX_PACKET_SIZE - 2;
                          }

                       // read for real up to a last packet boundary

                       memset(Buffer, 0, MAX_PACKET_SIZE);
                       status = lprecv(receiveSock, Buffer, i + 1, 0);
                       }
                    else
                       {

                       // read the whole thng for real

                       memset(Buffer, 0, MAX_PACKET_SIZE);
                       status = lprecv(receiveSock, Buffer, MAX_PACKET_SIZE - 1, 0);
                       }

                    i = 0;

                    // now queue up a seperate message for each packet

                    while (1)
                       {
                       pBuffer = (char *) malloc(strlen(&Buffer[i]) + 1);
                       strcpy(pBuffer, &Buffer[i]);

                       callevent = new callthing;

                       callevent->func = network_receive_receive;
                       callevent->arg1 = LOWORD(pBuffer);
                       callevent->arg2 = HIWORD(pBuffer);
                       callevent->kind = 5;

                       calllists.insert(callevent, callevent->kind);

                       PostMessage(WM_CHECKQUEUE, 0, 0);
                       //                checkqueue();

                       i += strlen(&Buffer[i]) + 1;
                       if (i >= status) break;
                       }
                    }

                 return 0L;
              }

          case FD_ACCEPT:
              bReceiveConnected = TRUE;

              // disabled for UDP

#ifndef USE_UDP
              acc_sin_len = sizeof(acc_sin);

              if ((receiveSock = lpaccept(receiveSock, (struct sockaddr FAR *) &acc_sin, (int FAR *) &acc_sin_len)) == INVALID_SOCKET)
                 {
                 MessageBox(WSAGetLastErrorString(0), "accept(receivesock)");
                 //          err_logo(STOP_ERROR,NIL);
                 }
#endif
              break;

          case FD_CLOSE:

              // done

              bReceiveConnected = FALSE;
              break;

          case FD_WRITE:

              // allow another frame to go out.

              bReceiveBusy = FALSE;
              break;

          default:
              MessageBox("Unexpected Message", "Status");
              //       err_logo(STOP_ERROR,NIL);
              break;

         }

      // all other events just queue the event

      callevent = new callthing;

      callevent->func = network_receive_send;
      callevent->arg1 = 0;
      callevent->arg2 = 0;
      callevent->kind = 4;

      calllists.insert(callevent, callevent->kind);

      PostMessage(WM_CHECKQUEUE, 0, 0);
      //                checkqueue();

      }

   return 0L;
   }

LRESULT TMyWindow::OnNetworkListenReceiveFinish(WPARAM /* wParam */, LPARAM lParam)
   {
   TMessage msg = __GetTMessage();

   SOCKADDR_IN receive_local_sin;      /* Local socket - internet style       */

   if (WSAGETASYNCERROR(lParam) != 0)
      {
      MessageBox(WSAGetLastErrorString(WSAGETASYNCERROR(lParam)), "WSAAsyncGetHostByNameCallBack()");
      //    err_logo(STOP_ERROR,NIL);
      return 0L;
      }

   // always start clean

   memset(&receive_local_sin, 0, sizeof(SOCKADDR_IN));

   // what else is there

   receive_local_sin.sin_family = AF_INET;

   memcpy((char FAR *) &(receive_local_sin.sin_addr), pher->h_addr, pher->h_length);

   // set ports

   receive_local_sin.sin_port = lphtons(receivePort);/* Convert to network ordering*/

   //   Associate an address with a socket. (bind)

   if (lpbind(receiveSock, (struct sockaddr FAR *) &receive_local_sin, sizeof(receive_local_sin)) == SOCKET_ERROR)
      {
      MessageBox(WSAGetLastErrorString(0), "bind(receivesock)");
      //    err_logo(STOP_ERROR,NIL);
      return 0L;
      }

   // listen for connect

#ifndef USE_UDP
   if (lplisten(receiveSock, MAX_PENDING_CONNECTS) == SOCKET_ERROR)
      {
      MessageBox(WSAGetLastErrorString(0), "listen(receivesock)");
      //    err_logo(STOP_ERROR,NIL);
      return 0L;
      }
#endif

   // watch for when connect happens

   if (lpWSAAsyncSelect(receiveSock, MainHWindow, WM_NETWORK_LISTENRECEIVEACK, FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
      {
      MessageBox(WSAGetLastErrorString(0), "WSAAsyncSelect(receivesock) FD_ACCEPT");
      //    err_logo(STOP_ERROR,NIL);
      }

   // fake an FD_ACCEPT for UDP, this automatically happens on TCP

#ifdef USE_UDP
   OnNetworkListenReceiveAck(0, MAKELONG(FD_ACCEPT, FD_ACCEPT));
#endif

   // queue this event

   if (network_receive_on == 1)
      {
      callthing *callevent = new callthing;

      callevent->func = network_receive_send;
      callevent->arg1 = 0;
      callevent->arg2 = 0;
      callevent->kind = 4;

      calllists.insert(callevent, 4);

      PostMessage(WM_CHECKQUEUE, 0, 0);
      //    checkqueue();
      }

   return 0L;
   }

LRESULT TMyWindow::MMMCINotify(WPARAM, LPARAM)
   {
   TMessage msg = __GetTMessage();

   // if user fired up a callback mci event the queue it up here

   callthing *callevent;

   callevent = new callthing;

   callevent->func = mci_callback;
   callevent->kind = 4;

   calllists.insert(callevent, 4);

   PostMessage(WM_CHECKQUEUE, 0, 0);

   return (LRESULT) msg.Result;
   }

void TMyWindow::EvTimer(UINT)
   {
   TMessage msg = __GetTMessage();

   // if user fired up a callback mci event the queue it up here

   callthing *callevent;

   callevent = new callthing;

   // the ID can only be 1-31 and select appropriate callback code

   callevent->func = timer_callback[msg.WParam];
   if (msg.WParam > 16)
      {
      callevent->kind = 3;
      calllists.insert(callevent, 3);
      }
   else
      {
      callevent->kind = 4;
      calllists.insert(callevent, 4);
      }

   PostMessage(WM_CHECKQUEUE, 0, 0);
   }

void checkwindow(int *x, int *y, int *w, int *h)
   {
   int MinX;
   int MinY;

#if !defined(__WIN32__)
   MinX = 0;
   MinY = 0;
#else
   RECT MaxRect;
   SystemParametersInfo(SPI_GETWORKAREA, 0, &MaxRect, 0);
   MinX = MaxRect.left;
   MinY = MaxRect.top;
#endif

   // sanity check window coordinates

   if (*x < MinX) *x = MinX;
   if (*y < MinY) *y = MinY;
   if (*w > MaxWidth) *w = MaxWidth;
   if (*h > MaxHeight) *h = MaxHeight;
   if ((*x + *w) > (MaxWidth + MinX)) *x = *x - (*x + *w - (MaxWidth + MinX));
   if ((*y + *h) > (MaxHeight + MinY)) *y = *y - (*y + *h - (MaxHeight + MinY));
   }

DEFINE_RESPONSE_TABLE1(TMyWindow, TFrameWindow)
EV_WM_KEYDOWN,
EV_WM_DESTROY,
EV_WM_KEYUP,
EV_WM_CHAR,
EV_WM_LBUTTONDOWN,
EV_WM_LBUTTONUP,
EV_WM_RBUTTONDOWN,
EV_WM_RBUTTONUP,
EV_WM_MOUSEMOVE,
EV_WM_TIMER,
EV_WM_SIZE,
EV_WM_HSCROLL,
EV_WM_VSCROLL,
EV_COMMAND(CM_FILENEW, CMFileNew),
EV_COMMAND(CM_FILEOPEN, CMFileOpen),
EV_COMMAND(CM_FILESAVE, CMFileSave),
EV_COMMAND(CM_FILESAVEAS, CMFileSaveAs),
EV_COMMAND(CM_FILEEDIT, CMFileEdit),
EV_COMMAND(CM_FILEERASE, CMFileErase),
EV_COMMAND(CM_EXIT, CMExit),
EV_COMMAND(CM_BITMAPNEW, CMBitmapNew),
EV_COMMAND(CM_BITMAPOPEN, CMBitmapOpen),
EV_COMMAND(CM_BITMAPSAVE, CMBitmapSave),
EV_COMMAND(CM_BITMAPSAVEAS, CMBitmapSaveAs),
EV_COMMAND(CM_BITMAPPRINT, CMBitmapPrint),
EV_COMMAND(CM_BITMAPPRINTERSETUP, CMBitmapPrinterSetup),
EV_COMMAND(CM_BITMAPPRINTERAREA, CMBitmapPrinterArea),
EV_COMMAND(CM_HELP, CMHelp),
EV_COMMAND(CM_HELPTUTORIAL, CMHelpTutorial),
EV_COMMAND(CM_HELPDEMO, CMHelpDemo),
EV_COMMAND(CM_HELPEXAMPLES, CMHelpExamples),
EV_COMMAND(CM_HELPRELEASENOTES, CMHelpReleaseNotes),
EV_COMMAND(CM_HELPMCI, CMHelpMCI),
EV_COMMAND(CM_HELPHELP, CMHelpHelp),
EV_COMMAND(CM_HELPABOUT, CMHelpAbout),
EV_COMMAND(CM_HELPABOUTMS, CMHelpAboutMS),
EV_COMMAND(CM_CONTROLEXECUTE, CMControlExecute),
EV_COMMAND(CM_SETFONT, CMSetFont),
EV_COMMAND(CM_SETCOMMANDERFONT, CMSetCommanderFont),
EV_COMMAND(CM_SETPENCOLOR, CMSetPenColor),
EV_COMMAND(CM_SETPENSIZE, CMSetPenSize),
EV_COMMAND(CM_SETFLOODCOLOR, CMSetFloodColor),
EV_COMMAND(CM_SETSCREENCOLOR, CMSetScreenColor),
EV_COMMAND(CM_ZOOMIN, CMZoomIn),
EV_COMMAND(CM_ZOOMOUT, CMZoomOut),
EV_COMMAND(CM_ZOOMNORMAL, CMZoomNormal),
EV_MESSAGE(WM_CHECKQUEUE, WMCheckQueue),
EV_MESSAGE(MM_MCINOTIFY, MMMCINotify),
EV_MESSAGE(WM_NETWORK_CONNECTSENDACK, OnNetworkConnectSendAck),
EV_MESSAGE(WM_NETWORK_CONNECTSENDFINISH, OnNetworkConnectSendFinish),
EV_MESSAGE(WM_NETWORK_LISTENRECEIVEACK, OnNetworkListenReceiveAck),
EV_MESSAGE(WM_NETWORK_LISTENRECEIVEFINISH, OnNetworkListenReceiveFinish),
END_RESPONSE_TABLE;



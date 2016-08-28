/*
*
*	Copyright (C) 1995 by the Regents of the University of California
*	Copyright (C) 1995 by George Mills
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

class TDMLClApp : public TApplication
   {
   public:
   TDMLClApp() : TApplication(),CallBackProc((FARPROC)CallBack)
      {
      InstId = 0;
      }

   DWORD  InstId;
   static HDDEDATA FAR PASCAL _export CallBack(WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD);

   TProcInstance CallBackProc;
   }
;

   DWORD   InstId()
      {
      return ((TDMLClApp*)GetApplication())->InstId;
      }

   char    Data[128];
   HCONV   HConv;
   BOOL    Loop;
   HSZ     Service;
   HSZ     Topic;
   HSZ     Item;

   }
;

TDMLClWnd::TDMLClWnd(TWindow* parent, const char* title) : TFrameWindow(parent, title), TWindow(parent, title)
   {
   Data[0] = 0;
   HConv = 0;
   Loop = 0;
   }

TDMLClWnd::~TDMLClWnd()
   {
   // This clean up is required for those resources that were allocated during
   // the DDEML conversation.
   //
   if (HConv) DdeDisconnect(HConv);     // Let the other party know we are leaving

   if (InstId())
      {
      DdeFreeStringHandle(InstId(), Service);
      DdeFreeStringHandle(InstId(), Topic);
      DdeFreeStringHandle(InstId(), Item);
      }
   }

Init
   {
   Service = 0;
   Topic = 0;
   Item = 0;

   Service = DdeCreateStringHandle(InstId(), "TDMLSR_Server", CP_WINANSI);
   Topic = DdeCreateStringHandle(InstId(), "Borland", CP_WINANSI);
   Item = DdeCreateStringHandle(InstId(), "Products", CP_WINANSI);

   if (!Service || !Topic || !Item)
      {
      MessageBox("Creation of strings failed.", Title, MB_ICONSTOP);
      }
   }

//
// The following 4 functions are used to communicate with DDE Server(s).
//

void TDMLClWnd::CmConnect()
   {
   HConv = DdeConnect(InstId(), Service, Topic, 0);
   if (!HConv) MessageBox("Can't start conversation.\nTry running DDESVR (the server).", Title, MB_ICONSTOP);
   }

//
// Request a data item. ReceiveData will be called asynchronously by the
// callback
//
void TDMLClWnd::CmRequest()
   {
   DdeClientTransaction(0, 0, HConv, Item, CF_TEXT, XTYP_REQUEST, TIMEOUT_ASYNC, 0);
   }

//
// Poke a string over to the server
//
void TDMLClWnd::CmPoke()
   {
   char buff[42] = "";
   if (TInputDialog(this, Title, "Poke string: ", buff, sizeof buff).Execute() == IDOK)
   DdeClientTransaction((unsigned char*)buff, strlen(buff)+1, HConv, Item, CF_TEXT, XTYP_POKE, 1000, 0);
   }

//
// Start or stop a continuous advise loop.
//
void TDMLClWnd::CmAdvise(WPARAM id)
   {
   TMenu menu(GetMenu());
   if (menu.GetMenuState(id, MF_BYCOMMAND) == MF_UNCHECKED)
      {
      DWORD temp;
      if (DdeClientTransaction(0, 0, HConv, Item, CF_TEXT, XTYP_ADVSTART | XTYPF_ACKREQ, 1000, &temp))
         {
         menu.CheckMenuItem(id, MF_BYCOMMAND | MF_CHECKED);
         Loop = TRUE;
         }
      }
   else
      {
      DWORD temp;
      if (DdeClientTransaction(0, 0, HConv, Item, CF_TEXT, XTYP_ADVSTOP, 1000, &temp))
         {
         menu.CheckMenuItem(id, MF_BYCOMMAND | MF_UNCHECKED);
         Loop = FALSE;
         }
      }
   DrawMenuBar();
   }

//
// This function is called when the callback function is notified of
// available data.
//
void TDMLClWnd::ReceivedData(HDDEDATA hData)
   {
   }

//
// This call back function is the heart of interaction between this program
// and DDEML.  Because Windows doesn't pass C++ 'this' pointers to call
// back functions, a static 'this' pointer was used.  If you wanted to
// create a Client that would allow for more than one conversation, using a
// List of conversations and their associated 'this' pointers would be one
// possible method to try.  The XTYP_ constants are described in detail in
// the online help.
//
HDDEDATA FAR PASCAL _export TDMLClApp::CallBack(WORD type, WORD, HCONV hConv, HSZ, HSZ, HDDEDATA hData, DWORD, DWORD)
   {
   switch (type)
      {
      case XTYP_ADVDATA:
      if (hConv == This->HConv)
        {
        if (hData)
          {
          DdeGetData(hData, (unsigned char*)Data, sizeof Data, 0);
          Invalidate(TRUE);
          }
        }
      return (HDDEDATA)DDE_FACK;

      case XTYP_XACT_COMPLETE:
      if (hConv == This->HConv)
        {
        if (hData)
          {
          DdeGetData(hData, (unsigned char*)Data, sizeof Data, 0);
          Invalidate(TRUE);
          }
        }
      break;

      case XTYP_DISCONNECT:
      This->MessageBox("Disconnected.", This->Title, MB_ICONINFORMATION);
      This->HConv = 0;
      This->Loop = 0;
      break;

      case XTYP_ERROR:
      This->MessageBox("A critical DDE error has occured.", This->Title, MB_ICONINFORMATION);
      }
   return 0;
   }

//
// The code below sets up the DDEML call back function that is used by the
// DDE Management Library to carry out data transfers between
// applications.
//
if (DdeInitialize(&InstId, (PFNCALLBACK)(FARPROC)CallBackProc, APPCMD_CLIENTONLY, 0) != DMLERR_NO_ERROR)
   {
   ::MessageBox(0,"Initialization failed.", "DDEML Client", MB_ICONSTOP|MB_TASKMODAL);
   }

if (InstId)
      {
      DdeUninitialize(InstId);
      }

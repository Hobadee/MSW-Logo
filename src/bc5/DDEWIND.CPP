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

typedef struct _wComClnt
   {
   DWORD idInst;                       /*  Instance of app for DDEML          */
   FARPROC lpDdeProc;                  /*  DDE callback function              */
   HSZ hszService;
   HSZ hszTopic;
   HSZ *hszItem;
   int numItems;
   HCONV hConv;                        /*Handle of established conversation   */
   void (*m_ServerClosedCB)();
   void (*m_ErrorMsgCB) (char *);
   UINT timeOut;
   struct _wComClnt *next;
   }
wComClnt;

static UINT wFmt = CF_TEXT;            /*  Clipboard format                   */
static wComClnt *TheCl;                /* one global instance                 */

/* just to show error msgs */
void WINAPI MsgErr(const char FAR *s, const char FAR *tit)
   {
   MessageBox(NULL, s, tit, MB_OK);
   }

static void HandleComError(char *s)
   {
   MsgErr(s, "Error Comunicating with DDE server!");
   }

int DoPutDDE(HINSTANCE hInst, const char *service, const char *topic, const char *item, char *argin)
   {
   hwCOMCLNT clnt;
   int done = 0;
   char *itarr[1];

   itarr[0] = (char *) item;

   clnt = NewClnt(service, topic, hInst);
   if (clnt)
      {
      ClntRegisterItems(clnt, itarr, 1);
      if (ClntConnect(clnt))
         {
         ClntRegisterCBs(clnt, 0, HandleComError);
         }
      else
         {
         DestroyClnt(clnt);
         return 0;
         }
      }
   else return 0;
   done = ClntPoke(clnt, itarr[0], argin, lstrlen(argin) + 1);
   DestroyClnt(clnt);
   return done;
   }

int DoExecDDE(HINSTANCE hInst, const char *service, const char *topic, char *argin)
   {
   hwCOMCLNT clnt;
   int done = 0;

   clnt = NewClnt(service, topic, hInst);
   if (clnt)
      {
      if (ClntConnect(clnt))
         {
         ClntRegisterCBs(clnt, 0, HandleComError);
         }
      else
         {
         DestroyClnt(clnt);
         return 0;
         }
      }
   else return 0;
   done = ClntExecute(clnt, argin, lstrlen(argin) + 1);
   DestroyClnt(clnt);
   return done;
   }


int DoGetDDE(HINSTANCE hInst, const char *service, const char *topic, const char *item, char *argout, int maxch)
   {
   hwCOMCLNT clnt;
   int mx, done = 0;
   char *itarr[1];
   char *data;

   itarr[0] = (char *) item;

   clnt = NewClnt(service, topic, hInst);
   if (clnt)
      {
      ClntRegisterItems(clnt, itarr, 1);
      if (ClntConnect(clnt))
         {
         ClntRegisterCBs(clnt, 0, HandleComError);
         }
      else
         {
         DestroyClnt(clnt);
         return 0;
         }
      }
   else return 0;

   data = ClntRequest(clnt, itarr[0], (UINT *) &done);
   if (data)
      {
      mx = strlen(data) + 1;
      mx = min(mx, maxch);
      memcpy(argout, data, mx);
      argout[mx] = 0;
      FreeRequest(data);
      }
   else argout[0] = 0;
   DestroyClnt(clnt);
   return done;
   }

static char *HandleDdeError(UINT DdeError)
   {
   switch (DdeError)
      {
       case DMLERR_DLL_NOT_INITIALIZED:
           return ("DLL não foi  inicializada.");

       case DMLERR_INVALIDPARAMETER:
           return ("invalid parameter.");
           break;

       case DMLERR_NO_CONV_ESTABLISHED:
           return ("no established conversation.");

       case DMLERR_NO_ERROR:
           break;
      }
   return ("OK.");
   }

HDDEDATA EXPENTRY _export DDECallback(UINT wType, UINT wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2)
   {
   switch (wType)
      {
       case XTYP_DISCONNECT:
           TheCl->hConv = NULL;
           if (TheCl->m_ServerClosedCB) (*TheCl->m_ServerClosedCB)();
           if (TheCl->m_ErrorMsgCB) (*TheCl->m_ErrorMsgCB) ("The server forced a disconnect.");
           return ((HDDEDATA) NULL);

       case XTYP_ERROR:
           break;

       case XTYP_XACT_COMPLETE:
           // compare transaction identifier, indicate transaction complete
           break;
      }
   return ((HDDEDATA) NULL);
   }

/***************************************************************************/
UINT ClntTimeOut(hwCOMCLNT cnc, UINT to)
   {
   wComClnt *nc = (wComClnt *) cnc;
   UINT oldt = nc->timeOut;
   if (to > 10) nc->timeOut = to;
   return oldt;
   }

hwCOMCLNT NewClnt(const char *serv, const char *topic, HINSTANCE hInst)
   {
   wComClnt *nc = (wComClnt *) calloc(1, sizeof(wComClnt));
   /** not needed after calloc ******
      nc->idInst = 0L;
      nc->hConv = NULL;
      nc->hszService = 0;
      nc->hszTopic = 0;
      nc->lpDdeProc = 0;
      nc->numItems = 0;
    ***********************************/
   nc->timeOut = 10000;                /* default timeout 10 secs             */
   nc->lpDdeProc = MakeProcInstance((FARPROC) DDECallback, hInst);
   if (DdeInitialize((LPDWORD) & nc->idInst, (PFNCALLBACK) nc->lpDdeProc, APPCMD_CLIENTONLY, 0L))
      {
      /* "Client DDE initialization failure."  */
      free(nc);
      return NULL;
      }

   nc->hszService = DdeCreateStringHandle(nc->idInst, (LPCSTR) serv, CP_WINANSI);
   nc->hszTopic = DdeCreateStringHandle(nc->idInst, (LPCSTR) topic, CP_WINANSI);

   TheCl = nc;
   return (hwCOMCLNT) nc;
   }

int ClntRegisterItems(hwCOMCLNT cnc, char **items, int ni)
   {
   int i, nn, off;
   void *nv;

   wComClnt *nc = (wComClnt *) cnc;
   if (nc == NULL) return 0;

   if (nc->numItems == 0)
      {
      nn = ni; off = 0;
      nc->hszItem = (HSZ *) malloc(sizeof(HSZ) * ni);
      }
   else
      {
      nn = nc->numItems + ni;
      off = nc->numItems;
      nv = realloc((void *) nc->hszItem, nn);
      if (nv == 0) return 0;
      else nc->hszItem = (HSZ *) nv;
      }

   for (i = 0; i < ni; i++)
      {
      nc->hszItem[i + off] = DdeCreateStringHandle(nc->idInst,
         (LPCSTR) items[i], CP_WINANSI);
      }
   nc->numItems = nn;
   return ni;
   }

void ClntRegisterCBs(hwCOMCLNT cnc, void (*ServerClosed)(), void (*ErrorMsg) (char *))
   {
   wComClnt *nc = (wComClnt *) cnc;
   nc->m_ServerClosedCB = ServerClosed;
   nc->m_ErrorMsgCB = ErrorMsg;
   }

int ClntConnect(hwCOMCLNT cnc)
   {
   wComClnt *nc = (wComClnt *) cnc;
   if (nc->hConv == NULL)
      {
      nc->hConv = DdeConnect(nc->idInst, nc->hszService, nc->hszTopic, (PCONVCONTEXT) NULL);
      if (nc->hConv == NULL)
         {
         if (nc->m_ErrorMsgCB) (*nc->m_ErrorMsgCB) (HandleDdeError(DdeGetLastError(nc->idInst)));
         }
      else return 1;
      }
   else
      {
      if (nc->m_ErrorMsgCB) (*nc->m_ErrorMsgCB) ("allready connected to DDE server.");
      }
   return 0;
   }

int ClntDisconnect(hwCOMCLNT cnc)
   {
   wComClnt *nc = (wComClnt *) cnc;
   if (nc->hConv != NULL)
      {
      DdeDisconnect(nc->hConv);
      nc->hConv = NULL;
      return 1;
      }
   else return 0;
   }

void DestroyClnt(hwCOMCLNT cnc)
   {
   int i;
   wComClnt *nc = (wComClnt *) cnc;
   if (nc == NULL) return;

   if (nc->hConv != NULL)
      {
      DdeDisconnect(nc->hConv);
      nc->hConv = NULL;
      }

   if (nc->idInst != 0)
      {
      DdeFreeStringHandle(nc->idInst, nc->hszService);
      DdeFreeStringHandle(nc->idInst, nc->hszTopic);
      for (i = 0; i < nc->numItems; i++) DdeFreeStringHandle(nc->idInst, nc->hszItem[i]);
      if (nc->numItems) free((void *) nc->hszItem);
      DdeUninitialize(nc->idInst);
      nc->idInst = 0;
      }

   if (nc->lpDdeProc)
      {
      FreeProcInstance(nc->lpDdeProc);
      nc->lpDdeProc = 0;
      }

   free(nc);
   }

int ClntPoke(hwCOMCLNT cnc, char *item, const char *msg, int msglen)
   {
   int i;
   HDDEDATA hData;
   char auxitem[64];
   DWORD dwResult = 33;
   wComClnt *nc = (wComClnt *) cnc;

   if (nc->hConv != NULL)
      {
      for (i = 0; i < nc->numItems; i++)
         {
         DdeQueryString(nc->idInst, nc->hszItem[i], (LPSTR) & auxitem,
            sizeof(auxitem), CP_WINANSI);
         if (!lstrcmpi(item, auxitem))
            break;
         }
      if (i >= nc->numItems) return 0;

      hData = DdeCreateDataHandle(nc->idInst, (void FAR *) msg, (DWORD) msglen, 0L, nc->hszItem[i], wFmt, 0);
      if (hData != NULL) hData = DdeClientTransaction((LPBYTE) hData, (DWORD) - 1, nc->hConv, nc->hszItem[i], wFmt, XTYP_POKE, nc->timeOut, &dwResult);
      else if (nc->m_ErrorMsgCB) (*nc->m_ErrorMsgCB) ("failed creation of 'data handle'.");
      }
   else if (nc->m_ErrorMsgCB) (*nc->m_ErrorMsgCB) ("no connection to a DDE server");
   return (dwResult == DDE_FACK);
   }


int ClntExecute(hwCOMCLNT cnc, const char *msg, int msglen)
   {
   HDDEDATA hData;
   DWORD dwResult = 33;
   wComClnt *nc = (wComClnt *) cnc;

   if (nc->hConv != NULL)
      {
      hData = DdeCreateDataHandle(nc->idInst, (void FAR *) msg, (DWORD) msglen, 0L,
         NULL, wFmt, 0);
      if (hData != NULL) hData = DdeClientTransaction((LPBYTE) hData, (DWORD) - 1, nc->hConv, NULL, wFmt, XTYP_EXECUTE, nc->timeOut, &dwResult);
      else if (nc->m_ErrorMsgCB) (*nc->m_ErrorMsgCB) ("failed creation of  'data handle'.");
      }
   else if (nc->m_ErrorMsgCB) (*nc->m_ErrorMsgCB) ("no connection to a DDE server");
   return (dwResult == DDE_FACK);
   }


char *ClntRequest(hwCOMCLNT cnc, char *item, UINT *nlen)
   {
   char *data;
   HDDEDATA hData;
   int i;
   char auxitem[64];
   DWORD dwResult = 33;
   wComClnt *nc = (wComClnt *) cnc;

   if (nc->hConv != NULL)
      {

      for (i = 0; i < nc->numItems; i++)
         {
         DdeQueryString(nc->idInst, nc->hszItem[i], (LPSTR) & auxitem,
            sizeof(auxitem), CP_WINANSI);
         if (!lstrcmpi(item, auxitem)) break;
         }
      if (i >= nc->numItems)
         {
         *nlen = 0;
         return NULL;
         }

      hData = DdeClientTransaction(NULL, 0, nc->hConv,
         nc->hszItem[i], wFmt, XTYP_REQUEST, nc->timeOut, &dwResult);

      if (dwResult == DDE_FNOTPROCESSED) return NULL;
      else
         {
         *nlen = (int) DdeGetData(hData, (LPBYTE) NULL, 0L, 0L);
         data = AllocRequest(*nlen);
         DdeGetData(hData, (LPBYTE) data, (DWORD) * nlen, 0L);
         DdeFreeDataHandle(hData);
         return data;
         }
      }
   return NULL;
   }

int ClntConnected(hwCOMCLNT nc)
   {
   return (((wComClnt *) nc)->hConv != 0);
   }

char *AllocRequest(int cb)
   {
   return (char *) GlobalAllocPtr(GHND, (DWORD) cb);
   }

void FreeRequest(char *data)
   {
   if (data) GlobalFreePtr(data);
   }



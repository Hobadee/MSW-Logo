
#include "allwind.h"

#if !defined(__WIN32__)
int ComId;
#else
HANDLE ComId;
//extern "C" int PASCAL vcpp_inpb (unsigned short port);
//extern "C" unsigned short PASCAL vcpp_inpw (unsigned short port);
//extern "C" unsigned long PASCAL vcpp_inpd (unsigned short port);
//extern "C" void PASCAL vcpp_outpb (unsigned short port, int databyte);
//extern "C" void PASCAL vcpp_outpw (unsigned short port, unsigned short dataword);
//extern "C" void PASCAL vcpp_outpd (unsigned short port, unsigned long dataword);
#endif

BOOL ComIsOpen = FALSE;

NODE *lmouseon(NODE *args)
   {
   char lbuttondown[MAX_BUFFER_SIZE];
   char lbuttonup[MAX_BUFFER_SIZE];
   char rbuttondown[MAX_BUFFER_SIZE];
   char rbuttonup[MAX_BUFFER_SIZE];
   char mousemove[MAX_BUFFER_SIZE];

   // get args

   cnv_strnode_string(lbuttondown, args);
   cnv_strnode_string(lbuttonup, args = cdr(args));
   cnv_strnode_string(rbuttondown, args = cdr(args));
   cnv_strnode_string(rbuttonup, args = cdr(args));
   cnv_strnode_string(mousemove, args = cdr(args));

   // most of mouse code is in DEFWNDPROC when this flag is on

   mouse_on = 1;

   if (mouse_lbuttondown == NULL)
      {
      mouse_lbuttondown = (char *) malloc(MAX_BUFFER_SIZE);
      mouse_lbuttonup = (char *) malloc(MAX_BUFFER_SIZE);
      mouse_rbuttondown = (char *) malloc(MAX_BUFFER_SIZE);
      mouse_rbuttonup = (char *) malloc(MAX_BUFFER_SIZE);
      mouse_mousemove = (char *) malloc(MAX_BUFFER_SIZE);
      }

   // these really should be NODEs but not yet

   strcpy(mouse_lbuttondown, lbuttondown);
   strcpy(mouse_lbuttonup, lbuttonup);
   strcpy(mouse_rbuttondown, rbuttondown);
   strcpy(mouse_rbuttonup, rbuttonup);
   strcpy(mouse_mousemove, mousemove);

   return (UNBOUND);
   }

NODE *lmouseoff(void)
   {

   // tell handler not to do anything with messages for mouse

   mouse_on = 0;

   return (UNBOUND);
   }

NODE *lkeyboardon(NODE *args)
   {
   char keyboarddown[MAX_BUFFER_SIZE];
   char keyboardup[MAX_BUFFER_SIZE];

   if (keyboard_keyup == NULL)
      {
      keyboard_keyup = (char *) malloc(MAX_BUFFER_SIZE);
      keyboard_keydown = (char *) malloc(MAX_BUFFER_SIZE);
      }

   // get args

   if (cdr(args) == NIL)
      {
      cnv_strnode_string(keyboardup, args);

      // most keyboard processing is done in DEFWNDPROC

      keyboard_on = 1;

      strcpy(keyboard_keyup, keyboardup);
      }
   else
      {

      cnv_strnode_string(keyboarddown, args);
      cnv_strnode_string(keyboardup, cdr(args));

      // most keyboard processing is done in DEFWNDPROC

      keyboard_on = 2;

      strcpy(keyboard_keydown, keyboarddown);
      strcpy(keyboard_keyup, keyboardup);
      }

   return (UNBOUND);
   }

NODE *lkeyboardoff(void)
   {

   // tell handler not to do anything with messages for keyboard

   keyboard_on = 0;

   return (UNBOUND);
   }

NODE *lmousepos()
   {

   // return current mouse position

   //   return(
   //   cons(make_intnode( mouse_posx+MainWindowx->Scroller->XPos-xoffset*the_zoom),
   //   cons(make_intnode(-(mouse_posy+MainWindowx->Scroller->YPos-yoffset*the_zoom)), NIL))
   //   );
   return (
      cons(make_intnode((mouse_posx + MainWindowx->Scroller->XPos) / the_zoom - xoffset),
         cons(make_intnode(-((mouse_posy + MainWindowx->Scroller->YPos) / the_zoom - yoffset)), NIL))
   );
   }

NODE *lkeyboardvalue()
   {

   // return current keyboard value

   return (make_intnode(keyboard_value));
   }

int min(int a, int b)
   {
   if (a < b) return (a); else return (b);
   }

NODE *lportclose(void)
   {

   // if port closed output error else close it

   if (!ComIsOpen)
      {
      MainWindowx->CommandWindow->MessageBox("Could not CLOSE port", "Error");
      err_logo(STOP_ERROR, NIL);
      }
   else
      {
      ComIsOpen = FALSE;
#if !defined(__WIN32__)
      CloseComm(ComId);
#else
      CloseHandle(ComId);
#endif
      }

   return (UNBOUND);
   }

NODE *lportopen(NODE *args)
   {

   char comport[MAX_BUFFER_SIZE];

   cnv_strnode_string(comport, args);

   // if port open output error else open it

   if (ComIsOpen)
      {
      MainWindowx->CommandWindow->MessageBox("PORT already open", "Error");
      err_logo(STOP_ERROR, NIL);
      }
   else
      {

#if !defined(__WIN32__)
      ComId = OpenComm(comport, 1024, 1024);
#else
      ComId = CreateFile(comport, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

      SetupComm(ComId, 4096, 4096);

      SetCommMask(ComId, 0);

      PurgeComm(ComId, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

      COMMTIMEOUTS CommTimeOuts;
      CommTimeOuts.ReadIntervalTimeout = MAXDWORD;
      CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
      CommTimeOuts.ReadTotalTimeoutConstant = 0;
      CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
      CommTimeOuts.WriteTotalTimeoutConstant = 0;
      SetCommTimeouts(ComId, &CommTimeOuts);

      DCB dcb;
      
      GetCommState(ComId, &dcb); // If we don't do this it won't work in Win95 !!!
      SetCommState(ComId, &dcb);
#endif

      if (ComId < 0)
         {
         MainWindowx->CommandWindow->MessageBox("Could not open PORT", "Error");
         err_logo(STOP_ERROR, NIL);
         }
      else
         {
         ComIsOpen = TRUE;
         }
      }

   return (UNBOUND);
   }

#if !defined(__WIN32__)
NODE *lportflush(NODE *args)
#else
NODE *lportflush(NODE */* args */)
#endif
   {

   int err;
#if !defined(__WIN32__)
   int que;

   que = getint(pos_int_arg(args));
#endif

   if (!ComIsOpen)
      {
      MainWindowx->CommandWindow->MessageBox("PORT not open", "Error");
      err_logo(STOP_ERROR, NIL);
      }
   else
      {

#if !defined(__WIN32__)
      err = !FlushComm(ComId, que);
#else
      err = FlushFileBuffers(ComId);
#endif

      if (err == 0)
         {
         MainWindowx->CommandWindow->MessageBox("Could not flush PORT", "Error");
         err_logo(STOP_ERROR, NIL);
         }

      }

   return (UNBOUND);
   }

NODE *lportmode(NODE *args)
   {

   char commode[MAX_BUFFER_SIZE];

   DCB dcb;

   int err;

   cnv_strnode_string(commode, args);

   // if closed output error else set mode

   if (!ComIsOpen)
      {
      MainWindowx->CommandWindow->MessageBox("PORT not open", "Error");
      err_logo(STOP_ERROR, NIL);
      }
   else
      {

      // build dcb, if no error continue

      memset(&dcb, 0, sizeof(DCB));

#if !defined(__WIN32__)
      err = !BuildCommDCB(commode, &dcb);
#else
      dcb.DCBlength = sizeof(DCB);
      err = BuildCommDCB(commode, &dcb);
#endif

      if (err == 0)
         {
         MainWindowx->CommandWindow->MessageBox("Could not build dcb on PORT", "Error");
         err_logo(STOP_ERROR, NIL);
         }
      else
         {
         // now set Id in dcb and set the state

#if !defined(__WIN32__)
         dcb.Id = ComId;
         err = !SetCommState(&dcb);
#else
         DCB dcbold;
         
         GetCommState(ComId, &dcbold);
         
         dcbold.BaudRate = dcb.BaudRate;
         dcbold.ByteSize = dcb.ByteSize;
         dcbold.Parity   = dcb.Parity;
         dcbold.StopBits = dcb.StopBits;

         err = SetCommState(ComId, &dcbold);
#endif

         if (err == 0)
            {
            MainWindowx->CommandWindow->MessageBox("Could not set PORT", "Error");
            err_logo(STOP_ERROR, NIL);
            }
         }
      }

   return (UNBOUND);
   }

NODE *lportwritearray(NODE *args)
   {

   char txbuffer[MAX_BUFFER_SIZE];

   int i;
#if !defined(__WIN32__)
   int count;
#else
   DWORD count;
   DWORD actual;
   DWORD Error;
#endif

   int status;

   NODE *obj;
   NODE *val;
   NODE *item;

   val = pos_int_arg(args);
   obj = cadr(args);

   while ((obj == NIL || obj == Null_Word) && NOT_THROWING)
      {
      setcar(cdr(args), err_logo(BAD_DATA, obj));
      obj = cadr(args);
      }

   if (NOT_THROWING)
      {
      if (nodetype(obj) == ARRAY)
         {

         // if closed the error, else continue

         if (!ComIsOpen)
            {
            MainWindowx->CommandWindow->MessageBox("PORT not open", "Error");
            err_logo(STOP_ERROR, NIL);
            }
         else
            {

            // get min of max array and the array

            count = min(min(getint(val), getarrdim(obj)), sizeof(txbuffer));

            // fill buffer with elements of the array

            for (i = 0; i < count; i++)
               {
               item = litem(cons(make_intnode(i + getarrorg(obj)), cons(obj, NIL)));
               txbuffer[i] = getint(cnv_node_to_numnode(item));
               }

            // now write buffer

#if !defined(__WIN32__)
            GetCommError(ComId, NULL);
            status = !WriteComm(ComId, txbuffer, count);
#else
            ClearCommError(ComId, &Error, NULL);
            status = WriteFile(ComId, txbuffer, count, &actual, NULL);
#endif

            // if problem GetComError will Put up Message box

            if (status == 0)
               {
#if !defined(__WIN32__)
               GetCommError(ComId, NULL);
#else
               ClearCommError(ComId, &Error, NULL);
#endif
               }

            // return byte count sent

            return (make_intnode(status));
            }
         }
      else
         {
         MainWindowx->CommandWindow->MessageBox("First arg must be an array", "Error");
         err_logo(STOP_ERROR, NIL);
         }
      }

   return (make_intnode(0));
   }

NODE *lportreadarray(NODE *args)
   {

   char rxbuffer[MAX_BUFFER_SIZE];

   int count;
   int i;
#if defined(__WIN32__)
   DWORD Error;
   DWORD actual;
#endif

   NODE *val;
   NODE *obj;

   COMSTAT Stat;

   val = pos_int_arg(args);
   obj = cadr(args);

   while ((obj == NIL || obj == Null_Word) && NOT_THROWING)
      {
      setcar(cdr(args), err_logo(BAD_DATA, obj));
      obj = cadr(args);
      }

   if (NOT_THROWING)
      {
      if (nodetype(obj) == ARRAY)
         {

         // if closed the error, else continue

         if (!ComIsOpen)
            {
            MainWindowx->CommandWindow->MessageBox("PORT not open", "Error");
            err_logo(STOP_ERROR, NIL);
            }
         else
            {

#if !defined(__WIN32__)
            // get status on port (returns pending bytes)
            GetCommError(ComId, &Stat);

            // if something pending continue
            if (!Stat.cbInQue) return (make_intnode(0));

            // don't overflow buffer
            count = min(min(min(getarrdim(obj), getint(val)), Stat.cbInQue), sizeof(rxbuffer));

            // do the read
            ReadComm(ComId, rxbuffer, count);
#else
            // don't overflow buffer
            count = min(min(getarrdim(obj), getint(val)), sizeof(rxbuffer));

            // Clear any errors
            ClearCommError(ComId, &Error, &Stat);

            // do the read (let if fail if nothing is there, it should not hang)
            ReadFile(ComId, rxbuffer, count, &actual, (LPDWORD) NULL);

            // if nothing there then return 0 count
            if (actual == 0) return (make_intnode(0));

            count = actual;
#endif
            // now fill in the array
            for (i = 0; i < count; i++)
               {
               lsetitem(
                  cons(make_intnode(i + getarrorg(obj)),
                     cons(obj,
                        cons(make_intnode(rxbuffer[i]),
                           NIL))));
               }

            // return actual transfered
            return (make_intnode(count));
            }
         }
      }

   return (make_intnode(0));

   }

NODE *lportwritechar(NODE *args)
   {

   char txchar[1];

#if !defined(__WIN32__)
   int status;
#else
   DWORD Error;
   DWORD status;
#endif

   // get arg

   txchar[0] = getint(pos_int_arg(args));

   // if not open output error, else continue

   if (!ComIsOpen)
      {
      MainWindowx->CommandWindow->MessageBox("PORT not open", "Error");
      err_logo(STOP_ERROR, NIL);
      }
   else
      {

      // write the 1 byte

#if !defined(__WIN32__)
      status = WriteComm(ComId, txchar, 1);
#else
      if (!WriteFile(ComId, txchar, 1, &status, NULL)) status = -1;
#endif

      // if problem GetComError will Put up Message box

      if (status != 1)
         {
#if !defined(__WIN32__)
         GetCommError(ComId, NULL);
#else
         ClearCommError(ComId, &Error, NULL);
#endif
         }


      // return byte count sent

      return (make_intnode(status));
      }

   return (make_intnode(0));
   }

NODE *lportreadchar(void)
   {

#if defined(__WIN32__)
   DWORD Error;
   DWORD actual;
#endif

   char rxchar[64];

   COMSTAT Stat;
   memset(&Stat, 0, sizeof(Stat));

   // if closed output error, else continue

   if (!ComIsOpen)
      {
      MainWindowx->CommandWindow->MessageBox("PORT not open", "Error");
      err_logo(STOP_ERROR, NIL);
      }
   else
      {

#if !defined(__WIN32__)
      // get status on port (returns pending bytes)
      GetCommError(ComId, &Stat);

      // if a byte then read it and return it
      if (Stat.cbInQue)
         {
         ReadComm(ComId, rxchar, min(1, Stat.cbInQue));
         return (make_intnode(rxchar[0]));
         }
      else
         {
         return (make_intnode(-1));
         }
#else
      // Clear Comm Error in case last operation failed otherwise we won't get anything
      ClearCommError(ComId, &Error, NULL);

      // ReadFile will return immediately even if there is nothing to read
      ReadFile(ComId, rxchar, 1, &actual, NULL);

      // if something was read then return it else return -1
      if (actual == 1)
         {
         return (make_intnode(rxchar[0]));
         }
      else
         {
         return (make_intnode(-1));
         }
#endif
      }

   return (make_intnode(-1));
   }

NODE *loutportb(NODE *args)
   {
   int portid;
   int value;

   // get args

   portid = getint(pos_int_arg(args));
   value = getint(pos_int_arg(cdr(args)));

#if !defined(__WIN32__)
   outportb(portid, value);
#else
   Myoutportb(portid, value);
#endif

   return (UNBOUND);
   }

NODE *linportb(NODE *args)
   {

   int value = 0;

   int portid;

   portid = getint(pos_int_arg(args));

#if !defined(__WIN32__)
   value = inportb(portid);
#else
   value = Myinportb(portid);
#endif

   return (make_intnode(value));
   }

NODE *loutport(NODE *args)
   {

   int value;
   int portid;

   // get args

   portid = getint(pos_int_arg(args));
   value = getint(pos_int_arg(cdr(args)));

#if !defined(__WIN32__)
   outport(portid, value);
#else
   Myoutport(portid, value);
#endif

   return (UNBOUND);
   }

NODE *linport(NODE *args)
   {
   int value;
   int portid;

   portid = getint(pos_int_arg(args));

#if !defined(__WIN32__)
   value = inport(portid);
#else
   value = Myinport(portid);
#endif

   return (make_intnode(value));
   }

NODE *lingameport(NODE *args)
   {
   int value;
   int portid;
   int mask;

   mask = getint(pos_int_arg(args));

   if (cdr(args) == NIL)
		portid = 0x201;
	else
		portid = getint(pos_int_arg(cdr(args)));

#if !defined(__WIN32__)
   value = 0;
#else
   value = Myingameport(portid, mask);
#endif

   return (make_intnode(value));
   }

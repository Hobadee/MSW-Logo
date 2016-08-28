/* -*- C++ -*-     
 *
 *  FILE:  dllfunc.h
 *
 *  DESCRIPTION: call of DLL funcs and DDE commands 
 *	
 * 	 	This code is free to use for non-comercial purposes
 *
 *  AUTHOR: Telmo A. Carmo
 *
 */

#ifndef dllfunc_H
#define dllfunc_H

#ifdef __cplusplus
extern "C" {
#endif

BOOL CallDllf(const char* fn,char* res,int nargs,char** args);
int FDllNargs(const char* sfn);

				
int DoPutDDE(HINSTANCE hInst,const char* service,const char* topic,
			 const char* item,char* argin);
int DoExecDDE(HINSTANCE hInst,
				const char* service,const char* topic,char* cmd);
int DoGetDDE(HINSTANCE hInst,
				const char* serv,const char* top,const char* item,
				char* argout,int maxch);
				
#ifdef __cplusplus
}
#endif

#endif /* dllfunc_H */


/*
	wipc.h, 	TAC, Fri Jul 23 16:03:13 1993

 	This code is free to use for non-comercial purposes
*/

#ifndef wipc_h
#define wipc_h

#ifdef __cplusplus
extern "C" {
#endif

typedef void* hwCOMCLNT;

hwCOMCLNT 	NewClnt(const char* serv, const char* topic, HINSTANCE hInst);
void 		DestroyClnt(hwCOMCLNT nc);

int 	ClntConnect(hwCOMCLNT);
int 	ClntRegisterItems(hwCOMCLNT clnt,char** items, int ni);
void 	ClntRegisterCBs(hwCOMCLNT clnt,
						void (*ServerClosed)(), void (*ErrorMsg)(char*));

int 	ClntDisconnect(hwCOMCLNT nc);
	
int 	ClntPoke(hwCOMCLNT, char* item,const char* msg, int msglen);
int 	ClntExecute(hwCOMCLNT, const char* cmd, int cmdlen);
char* 	ClntRequest(hwCOMCLNT,char* item, UINT* msglenP);
int  	ClntConnected(hwCOMCLNT nc);
UINT	ClntTimeOut(hwCOMCLNT nc,UINT timeout);

char* 	AllocRequest(int size);
void 	FreeRequest(char* data);

#ifdef __cplusplus
}
#endif

#endif

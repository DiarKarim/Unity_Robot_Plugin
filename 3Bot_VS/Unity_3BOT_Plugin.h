/******************************************************************************/
/*                                                                            */
/* MODULE  : Unity 3BOT Plugin	                                              */
/*                                                                            */
/* PURPOSE : To make 3BOT independent from the Oculus library by using Unity  */
/*                                                                            */
/* DATE    : 14/April/2019                                                    */
/*                                                                            */
/* V1.0  14/April/2019 - Initial development                                  */
/*                                                                            */
/* AUTHOR: Diar Karim                                                         */
/* CONTACT: diarkarim@gmail.com                                               */
/* WEBSITE: diarkarim.com                                                     */
/* SUPERVISORS: Chris Miall and Sang-Hoon Yeo                                 */
/*                                                                            */
/*                                                                            */
/******************************************************************************/


#include <motor.h>

#include <thread> 

#include <iostream>
#include <string.h>
#include <cstring>
#include <stdio.h>

#include <sstream>
#include<winsock2.h>

#include <locale>         
#include <codecvt>        

using namespace std;

class UDP_Sender // Client 
{
public:
	struct sockaddr_in ssi_other;
	int ss, sslen = sizeof(ssi_other);
	char sbuf;
	char smessage;
	char smessageX;
	char smessageY;
	char smessageZ;
	WSADATA swsa;

	UDP_Sender();

	void sendUDPmessage(string);
};

class UDP_Receiver // Server
{
public:
	struct sockaddr_in ssi_other;
	int ss, sslen = sizeof(ssi_other);
	char sbuf;
	char smessage;
	char smessageX;
	char smessageY;
	char smessageZ;
	WSADATA swsa;

	UDP_Receiver();

	void receiveUDPmessage(string);
};
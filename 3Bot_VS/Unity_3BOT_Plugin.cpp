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

#include "Unity_3BOT_Plugin.h"

#pragma comment(lib,"ws2_32.lib") // Winsock Library 
#define BUFLEN 4
#define PORT 8888

// Client stuff
//#define SERVER "127.0.0.1"  //ip address of udp server
#define SERVER "169.254.254.10"  //ip address of udp server

#define BUFLENS 16 //Max length of buffer
#define PORTS 8899   //The port on which to listen for incoming data

using namespace std;

// Client stuff start
struct sockaddr_in ssi_other;
int ss, sslen = sizeof(ssi_other);
char sbuf[BUFLENS];
char smessage[BUFLENS];
char smessageX[BUFLENS];
char smessageY[BUFLENS];
char smessageZ[BUFLENS];
WSADATA swsa;

	UDP_Sender::UDP_Sender()
	{

		//Initialise winsock
		printf("\nInitialising Winsock...");
		if (WSAStartup(MAKEWORD(2, 2), &swsa) != 0)
		{
			printf("Failed. Error Code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		printf("Initialised.\n");

		//create socket
		if ((ss = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		{
			printf("socket() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//setup address structure
		memset((char *)&ssi_other, 0, sizeof(ssi_other));
		ssi_other.sin_family = AF_INET;
		ssi_other.sin_port = htons(PORTS);
		ssi_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

	}

	void UDP_Sender::sendUDPmessage(string msg)
	{
		// Prep typed message for sending via UDP port
		char bufsend[1024];
		string tmp = msg;
		char sendMsg[sizeof bufsend];
		strncpy(sendMsg, tmp.c_str(), sizeof(sendMsg));
		memcpy(&bufsend, sendMsg, sizeof bufsend);

		printf("Sent: %s \n \r", bufsend);

		// Send typed message via UDP port
		//int sentz = sendto(ss, bufsend, sizeof bufsend, 0, (struct sockaddr*) &ssi_other, sslen);
		if (sendto(ss, bufsend, sizeof bufsend, 0, (struct sockaddr*) &ssi_other, sslen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}
//};
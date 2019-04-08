//  (C) 2001-2015 Force Dimension
//  All Rights Reserved.
//
//  Version 3.6.0

#include <sstream>

// UDP libs start
#include <locale>         // std::wstring_convert
#include <codecvt>        // std::codecvt_utf8

#include<future>

#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") // Winsock Library 
#define BUFLEN 4
#define PORT 8888

// Client stuff
#define SERVER "127.0.0.1"  //ip address of udp server
#define BUFLENS 16 //Max length of buffer
#define PORTS 8899   //The port on which to listen for incoming data

// UDP libs end

#include <thread> 
#include <stdio.h>
#include "dhdc.h"

#include <iostream>

//using namespace std; 

#define REFRESH_INTERVAL  0.1   // sec

inline void
MatTranspose (const double a[3][3],
              double       m[3][3])
{
  m[0][0] = a[0][0];  m[0][1] = a[1][0];  m[0][2] = a[2][0];
  m[1][0] = a[0][1];  m[1][1] = a[1][1];  m[1][2] = a[2][1];
  m[2][0] = a[0][2];  m[2][1] = a[1][2];  m[2][2] = a[2][2];
}






// UDP Receive Function Declaration
//float receiveUDPmessage(SOCKET, WSADATA, int, struct sockaddr_in&);
void receiveUDPmessage(SOCKET, WSADATA, int, struct sockaddr_in&, float*);
//void receiveUDPmessage();
//void sendUDPmessage(SOCKET, WSADATA, int, struct sockaddr_in&);

void sendUDPmessage(SOCKET, int, struct sockaddr_in&, double*);

void changeDouble()
{
	double dl = 0.1;
	double* fl = &dl;

	while (true)
	{
		*fl += 0.01;
		printf("%f: \n", *fl); // << std::endl;
	}
}


std::string Convert(float);


int main (int  argc, char **argv)
{

	// Client stuff start

	struct sockaddr_in ssi_other;
	int ss, sslen = sizeof(ssi_other);
	char sbuf[BUFLENS];
	char smessage[BUFLENS];
	char smessageX[BUFLENS];
	char smessageY[BUFLENS];
	char smessageZ[BUFLENS];
	WSADATA swsa;

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

	// Client stuff end




	// UDP Stuff start
	// ***********************************************************************
	// ***********************************************************************
	// ***********************************************************************
	float aPos;
	float* ptr = &aPos;

	int slen;
	char buf[BUFLEN];

	SOCKET s;
	WSADATA wsa;
	struct sockaddr_in server, si_other;


	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	slen = sizeof(si_other);

	//receiveUDPmessage(s,wsa,slen,recv_len);

	// ***********************************************************************
	// ***********************************************************************
	// ***********************************************************************
	// UDP Stuff end 

  const double K = 40;

  double px, py, pz;
  double fx, fy, fz;
  double j0, j1, j2;
  double g0, g1, g2;
  double q0, q1, q2;
  double J[3][3];
  double Jt[3][3];
  double freq   = 0.0;
  double t1,t0  = dhdGetTime ();
  bool   spring = false;
  int    done   = 0;
  int    sat;
  char xX[BUFLENS];


  double* ptrX = &px;
  double* ptrY = &py;
  double* ptrZ = &pz;

  int conter = 0;  // condition counter if 1 = transmit x, 2 = transmit y and 3 = transmit z

  // message
  int major, minor, release, revision;
  dhdGetSDKVersion (&major, &minor, &release, &revision);
  printf ("Force Dimension - Jacobian Usage Example %d.%d.%d.%d\n", major, minor, release, revision);
  printf ("(C) 2001-2015 Force Dimension\n");
  printf ("All Rights Reserved.\n\n");


  // required to get the Jacobian matrix
  dhdEnableExpertMode ();

  // open the first available device
  if (dhdOpen () < 0) {
    printf ("error: cannot open device (%s)\n", dhdErrorGetLastStr());
    dhdSleep (2.0);
    return -1;
  }

  // identify device
  printf ("%s device detected\n\n", dhdGetSystemName());

  // emulate button on supported devices
  dhdEmulateButton (DHD_ON);

  // display instructions
  printf ("press BUTTON to enable virtual spring\n");
  printf ("         'q' to quit\n\n");

  // enable force
  dhdEnableForce (DHD_ON);
  
  // start threads
  std::thread udp1(receiveUDPmessage, s, wsa, slen, si_other, ptr);
  udp1.detach();

  ////void sendUDPmessage(SOCKET ss, int sslen, struct sockaddr_in& ssi_other, char smessages[], double* poS)
  //std::thread udpSendx(sendUDPmessage,ss, sslen, ssi_other, ptrX);
  //udpSendx.detach();




  // Client message
  //printf("Enter message : ");
  //gets_s(smessage);
  //smessage = '546899';
  //memcpy(&smessage, smessage, sizeof smessage);

  // loop while the button is not pushed
  while (!done) {

    // retrieve joint angles
    if (dhdGetPosition (&px, &py, &pz) < DHD_NO_ERROR) {
      printf ("error: cannot get joint angles (%s)\n", dhdErrorGetLastStr());
      done = 1;
    }


	
	//printf("%f %f %f \r", *ptrX, *ptrY, *ptrZ);

	Sleep(1);
	if (conter == 0)
	{
		char smessageZ[16];
		int ret1 = snprintf(smessageZ, sizeof smessageZ, "%d", 100);
		if (sendto(ss, smessageZ, strlen(smessageZ), 0, (struct sockaddr *) &ssi_other, sslen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		//printf("%d \n", 100);
	}
	else if (conter == 1) 
	{
		char smessageZ[16];
		int ret1 = snprintf(smessageZ, sizeof smessageZ, "%f", *ptrX);
		if (sendto(ss, smessageZ, strlen(smessageZ), 0, (struct sockaddr *) &ssi_other, sslen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//printf("X: %f \r", *ptrX);
	}
	else if (conter == 2)  //(conter >= 120 && conter <= 240)
	{
		char smessageZ[16];
		int ret1 = snprintf(smessageZ, sizeof smessageZ, "%f", *ptrY);
		if (sendto(ss, smessageZ, strlen(smessageZ), 0, (struct sockaddr *) &ssi_other, sslen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		//printf("_______________Y: %f \r", *ptrY);
	}
	else if (conter == 3) //(conter >= 240 && conter <= 360)
	{
		char smessageZ[16];
		int ret1 = snprintf(smessageZ, sizeof smessageZ, "%f", *ptrZ);
		if (sendto(ss, smessageZ, strlen(smessageZ), 0, (struct sockaddr *) &ssi_other, sslen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		//printf("_________________________________Z: %f \r", *ptrZ);
		conter = 0;
	}
	//else if (conter >= 360)
	//{
		//conter = 0; 
	//}

	conter++;


	//// Client message start
	//int ret1 = snprintf(smessageX, sizeof smessageX, "%f", px);
	//if (ret1 < 0) {
	//	return EXIT_FAILURE;
	//}
	//if (ret1 > sizeof smessageX) {
	//	/* Result was truncated - resize the buffer and retry*/;
	//}

	//if (sendto(ss, smessageX, strlen(smessageX), 0, (struct sockaddr *) &ssi_other, sslen) == SOCKET_ERROR)
	//{
	//	printf("sendto() failed with error code : %d", WSAGetLastError());
	//	exit(EXIT_FAILURE);
	//}
	//// Client message end



    // compute force to apply
    if (spring) {
      fx = - K * px + aPos;
      fy = - K * py + aPos;
      fz = - K * pz + aPos;
    }
    else fx = fy = fz = 0.0;
	//printf("%0.06f %0.02f \r", aPos, freq);


    // retrieve joint angles
    if (dhdGetDeltaJointAngles (&j0, &j1, &j2) < DHD_NO_ERROR) {
      printf ("error: cannot get joint angles (%s)\n", dhdErrorGetLastStr());
      done = 1;
    }

    // compute jacobian
    if (dhdDeltaJointAnglesToJacobian (j0, j1, j2, J) < DHD_NO_ERROR) {
      printf ("error: cannot compute jacobian (%s)\n", dhdErrorGetLastStr());
      done = 1;
    }

    // compute joint torques required for gravity compensation
    if (dhdDeltaGravityJointTorques (j0, j1, j2, &g0, &g1, &g2) < DHD_NO_ERROR) {
      printf ("error: cannot compute gravity compensation joint torques (%s)\n", dhdErrorGetLastStr());
      done = 1;
    }

    // compute joint torques Q = ((J)T) * F
    MatTranspose (J, Jt);
    q0 = Jt[0][0]*fx + Jt[0][1]*fy + Jt[0][2]*fz;
    q1 = Jt[1][0]*fx + Jt[1][1]*fy + Jt[1][2]*fz;
    q2 = Jt[2][0]*fx + Jt[2][1]*fy + Jt[2][2]*fz;

    // combine gravity compensation and requested force
    q0 += g0;
    q1 += g1;
    q2 += g2;

    // apply joint torques
    if ((sat = dhdSetDeltaJointTorques (q0, q1, q2)) < DHD_NO_ERROR) {
      printf ("error: cannot set joint torques (%s)\n", dhdErrorGetLastStr());
      done = 1;
    }

    // display refresh rate and position at 10Hz
    t1 = dhdGetTime ();
    if ((t1-t0) > REFRESH_INTERVAL) {

      // retrieve information to display
      freq = dhdGetComFreq ();
      t0   = t1;

      // write down position
      if (dhdGetPosition (&px, &py, &pz) < 0) {
        printf ("error: cannot read position (%s)\n", dhdErrorGetLastStr());
        done = 1;
      }
      if (sat == DHD_MOTOR_SATURATED) printf ("[*] ");
      else                            printf ("[-] \n");
     // printf ("(%+0.03f, %+0.03f, %+0.03f) [Nm]  |  freq = %0.02f [kHz]       \r", px, py, pz, freq);

      // test for exit condition
      if (dhdGetButtonMask()) spring = true;
      else                    spring = false;
      if (dhdKbHit()) {
        switch (dhdKbGet()) {
        case 'q': done = 1; break;
        }
      }
    }
  }

  // close the connection
  dhdClose ();

  // Client close start 
  closesocket(s);
  WSACleanup();
  // Client close end

  // happily exit
  printf ("\ndone.\n");
  return 0;
}


/*
*************************************************
Diar UDP transmission function definition section
*************************************************
*/

//float receiveUDPmessage(SOCKET sz, WSADATA wsaz, int slenz, struct sockaddr_in& si_otherz)
void receiveUDPmessage(SOCKET sz, WSADATA wsaz, int slenz, struct sockaddr_in& si_otherz, float* val)
{
	// UDP init start
	float afz;
	char bufz[4];
	int recv_len;
	//struct sockaddr_in si_other; 

	while (true)
	{
		//printf("Waiting for data...");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(bufz, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(sz, bufz, BUFLEN, 0, (struct sockaddr *) &si_otherz, &slenz)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		memcpy(&afz, bufz, sizeof afz);
		//printf("AF: %f \n", afz);

		*val = afz;
		//printf("%f: \n", *val); // << std::endl;

		//************************** Send *****************************

		//char sendMsg[4] = { "78" };
		//char bufsend[4];

		//memcpy(&bufsend, sendMsg, sizeof bufsend);
		//printf("%s: \r", bufsend);

		////now reply the client with the same data
		//int sentz = sendto(sz, bufsend, recv_len, 0, (struct sockaddr*) &si_otherz, slenz);
		//if (sendto(sz, bufsend, recv_len, 0, (struct sockaddr*) &si_otherz, slenz) == SOCKET_ERROR)
		//{
		//	printf("sendto() failed with error code : %d", WSAGetLastError());
		//	exit(EXIT_FAILURE);
		//}

	}

	//return afz;
}

std::string Convert(float number) {
	std::ostringstream buff;
	buff << number;
	return buff.str();
}


void sendUDPmessage(SOCKET ss,  int sslen, struct sockaddr_in& ssi_other, double* poS)
{
	char smessageZ[16];
	int ret1 = snprintf(smessageZ, sizeof smessageZ, "%f", &poS);
	if (sendto(ss, smessageZ, strlen(smessageZ), 0, (struct sockaddr *) &ssi_other, sslen) == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
}


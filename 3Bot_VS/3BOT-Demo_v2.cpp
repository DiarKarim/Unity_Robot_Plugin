/******************************************************************************/
/*                                                                            */
/* MODULE  : 3BOT-OculusMotor.cpp                                             */
/*                                                                            */
/* PURPOSE : 3BOT and Oculus Rift HMD demo using OCULUS module in MOTOR.LIB   */
/*                                                                            */
/* DATE    : 05/Dec/2014                                                      */
/*                                                                            */
/* CHANGES                                                                    */
/*                                                                            */
/* V1.0  JNI 05/Dec/2014 - Initial development.                               */
/*                                                                            */
/* V1.1  JNI 12/Jan/2015 - Conversion to use new OCULUS module in MOTOR.LIB   */
/*                                                                            */
/* V1.2  JNI 21/Jan/2015 - OCULUS module uses traditional coordinate system.  */
/*                                                                            */
/******************************************************************************/

//#define  OS_WIN32
//#define  OVR_OS_WIN3

#define MODULE_NAME "3BOT-OculusMotor"

/******************************************************************************/
//#include<conio.h>
#include <motor.h>

#include <stdio.h>

// UDP libs
#include <sstream>

#include <locale>         // std::wstring_convert
#include <codecvt>        // std::codecvt_utf8

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
#include <iostream>
#include <string.h>
#include <cstring>

#define REFRESH_INTERVAL  0.1   // sec

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#else

#endif

using namespace std;

class UDP_Sender
{
public:
	// Client stuff start
	struct sockaddr_in ssi_other;
	int ss, sslen = sizeof(ssi_other);
	char sbuf[BUFLENS];
	char smessage[BUFLENS];
	char smessageX[BUFLENS];
	char smessageY[BUFLENS];
	char smessageZ[BUFLENS];
	WSADATA swsa;

	UDP_Sender()
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

		// Client stuff end

		// UDP Stuff start
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
		bind(s, (struct sockaddr *)&server, sizeof(server));
		//if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
		//{
		//	printf("Bind failed with error code : %d", WSAGetLastError());
		//	exit(EXIT_FAILURE);
		//}
		puts("Bind done");

		slen = sizeof(si_other);

		//receiveUDPmessage(s,wsa,slen,recv_len);

		//// start threads
		//std::thread udp1(receiveUDPmessage, s, wsa, slen, si_other, ptr);
		//udp1.detach();

		
	}

	void sendUDPmessage(string msg) // (char msg)
	{
		char bufsend[1024];
		//char sendMsg[4] = { '1' };
		//char sendMsg[sizeof bufsend] = { msg };
		
		string tmp = msg; 
		char sendMsg[sizeof bufsend];
		//strcpy(sendMsg, msg.c_str());
		strncpy(sendMsg, tmp.c_str(), sizeof(sendMsg));

		memcpy(&bufsend, sendMsg, sizeof bufsend);
		printf("Sent: %s \n \r", bufsend);

		int sentz = sendto(ss, bufsend, sizeof bufsend, 0, (struct sockaddr*) &ssi_other, sslen);
		if (sendto(ss, bufsend, sizeof bufsend, 0, (struct sockaddr*) &ssi_other, sslen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}
}; 

//void receiveUDPmessage(SOCKET, WSADATA, int, struct sockaddr_in&, float*);
//void sendUDPmessage(SOCKET, int, struct sockaddr_in&, double*);
//std::string Convert(float);

/******************************************************************************/



TIMER_Interval  GraphicsDrawLatencyTimer("GraphicsDrawLatency");
TIMER_Frequency GraphicsDrawFrequencyTimer("GraphicsDrawFrequency");
TIMER_Frequency GraphicsIdleFrequencyTimer("GraphicsIdleFrequency");

/******************************************************************************/

STRING RobotName="";
int    RobotID=ROBOT_INVALID;
matrix RobotPosition(3,1);             // cm
matrix RobotPositionRaw(3,1);          // cm
matrix RobotPositionOffset(3,1);       // cm
matrix RobotVelocity(3,1);             // cm/sec
matrix RobotForces(3,1);               // N
double RobotForceMax=15.0;             // N
double RobotSpringConstant=-10.0;      // N/cm

TIMER_Frequency RobotLoopFrequencyTimer("RobotLoopFrequency");
TIMER_Interval  RobotLoopLatencyTimer("RobotLoopLatency");

// SPHERES ******************************************************
matrix SphereStartPos(3, 1); // {{ 0, 0 }, { 1, 2 }, { 2, 4 }};;
double SphereStartRadius = 3.5; // cm
int    SphereStartColor = YELLOW;

matrix SphereTargetPos(3,1);
matrix STP1(3, 1); // {{ 0, 0 }, { 1, 2 }, { 2, 4 }};;
matrix STP2(3, 1);
matrix STP3(3, 1);
matrix STP4(3, 1);

double SphereTargetRadius = 3.0; // cm
int    SphereTargetColor = RED;

matrix SphereDiversionPos(3, 1); // {{ 0, 0 }, { 1, 2 }, { 2, 4 }};;
double SphereDiversionRadius = 1.0; // cm
int    SphereDiversionColor = BLUE;

BOOL   SphereInsideFlag=FALSE;

double CursorRadius = 2.0; // cm
int    CursorColor = WHITE;
double angles[3] = { 0.1, 0.1, 0.1 };


// UDP Variables start  
struct sockaddr_in ssi_other;
int ss, sslen = sizeof(ssi_other);
char sbuf[BUFLENS];
char smessage[BUFLENS];
char smessageX[BUFLENS];
char smessageY[BUFLENS];
char smessageZ[BUFLENS];
WSADATA swsa;

double* ptrX;
double* ptrY; // = &RobotPosition(2, 1);
double* ptrZ; // = &RobotPosition(3, 1);

// UDP Variables end

//char fileName[10] = {};
/******************************************************************************/

void ProgramExit( void );
void ControlVisual(void);

/******************************************************************************/

bool Target1 = false;
bool Target2 = false;
bool Target3 = false;
bool Target4 = false;
bool DrawStartSphere = false;
bool DrawDiversionSphere = false; 

UDP_Sender udpSndr;
char textPos = 'x';

void GraphicsTimingResults( void )
{
    printf("----------------------------------------------------------------\n");

    GraphicsDrawFrequencyTimer.Results();
    GraphicsDrawLatencyTimer.Results();
    GraphicsIdleFrequencyTimer.Results();

    GRAPHICS_TimingResults();
    GRAPHICS_AdaptiveDisplayResults();

    printf("----------------------------------------------------------------\n");
}

/******************************************************************************/

void GraphicsIdle( void )
{
    GraphicsIdleFrequencyTimer.Loop();
}

/******************************************************************************/

void GraphicsDraw( void )
{
    GraphicsDrawFrequencyTimer.Loop();
    GraphicsDrawLatencyTimer.Before();

    // Fixed object.
    //GRAPHICS_WireSphere(&SpherePosition,SphereRadius,SphereColor);
	//GRAPHICS_Cube(&SpherePosition, 4.0); 
	//GRAPHICS_Cuboid(&SpherePosition,300.0, 50.0, 5.0);
	//GRAPHICS_Cuboid()
	//GRAPHICS_Ring(&SpherePosition, 10.0, 3.5); 
	
	ControlVisual();

    // Robot cursor.
    GRAPHICS_Sphere(&RobotPosition,CursorRadius,CursorColor);

    GraphicsDrawLatencyTimer.After();
}

/******************************************************************************/

void ControlVisual()
{
	if (DrawStartSphere)
	{
		GRAPHICS_WireSphere(&SphereStartPos, SphereStartRadius, SphereStartColor);
	}
	if (DrawDiversionSphere)
	{
		GRAPHICS_WireSphere(&SphereDiversionPos, SphereDiversionRadius, SphereDiversionColor);
	}

	if (Target1)
	{
		SphereTargetPos = STP1; 

		SphereDiversionPos(1, 1) = SphereTargetPos(1, 1) + 5.0;
		SphereDiversionPos(2, 1) = SphereTargetPos(2, 1) / 6.0;
		SphereDiversionPos(3, 1) = SphereTargetPos(3, 1) / 4.0;

		GRAPHICS_WireSphere(&STP1, SphereTargetRadius, SphereTargetColor);
	}
	if (Target2)
	{
		SphereTargetPos = STP2;

		SphereDiversionPos(1, 1) = SphereTargetPos(1, 1) + 5.0;
		SphereDiversionPos(2, 1) = SphereTargetPos(2, 1) / 6.0;
		SphereDiversionPos(3, 1) = SphereTargetPos(3, 1) / 4.0;

		GRAPHICS_WireSphere(&STP2, SphereTargetRadius, SphereTargetColor);
	}
	if (Target3)
	{
		SphereTargetPos = STP3;

		SphereDiversionPos(1, 1) = SphereTargetPos(1, 1) + 5.0;
		SphereDiversionPos(2, 1) = SphereTargetPos(2, 1) / 6.0;
		SphereDiversionPos(3, 1) = SphereTargetPos(3, 1) / 4.0;

		GRAPHICS_WireSphere(&STP3, SphereTargetRadius, SphereTargetColor);
	}
	if (Target4)
	{
		SphereTargetPos = STP4;

		SphereDiversionPos(1, 1) = SphereTargetPos(1, 1) + 5.0;
		SphereDiversionPos(2, 1) = SphereTargetPos(2, 1) / 6.0;
		SphereDiversionPos(3, 1) = SphereTargetPos(3, 1) / 4.0;

		GRAPHICS_WireSphere(&STP4, SphereTargetRadius, SphereTargetColor);
	}

}


void GraphicsKeyboard( BYTE key, int x, int y )
{

	// This function provides keyboard control over system
	if (key)
	{
		printf("Key pressed: %c \n", key);
	}

	// Switch on/off start sphere 
	if (key == 's')
	{
		if (DrawStartSphere == false)
		{
			DrawStartSphere = true;
		}
		else
		{
			DrawStartSphere = false; 
		}
	}

	// Switch on/off diversion sphere 
	if (key == 'd')
	{
		if (DrawDiversionSphere == false)
		{
			DrawDiversionSphere = true;
		}
		else
		{
			DrawDiversionSphere = false;
		}
	}

	// Switch targets
	if (key == '1')
	{
		if (Target1 == false)
		{
			Target1 = true;
		}
		else
		{
			Target1 = false;
		}
	}
	if (key == '2')
	{
		if (Target2 == false)
		{
			Target2 = true;
		}
		else
		{
			Target2 = false;
		}
	}
	if (key == '3')
	{
		if (Target3 == false)
		{
			Target3 = true;
		}
		else
		{
			Target3 = false;
		}
	}
	if (key == '4')
	{
		if (Target4 == false)
		{
			Target4 = true;
		}
		else
		{
			Target4 = false;
		}
	}

    if( key == ESC )
    {
        ProgramExit();
    }
}

/******************************************************************************/

void GraphicsStart( void )
{
    // Start the graphics system.
    if( !GRAPHICS_GraphicsStart() )
    {
        printf("Cannot start graphics system.\n");
        return;
    }

    // Register call-back functions and pass control to graphics system.
    GRAPHICS_MainLoop(GraphicsKeyboard,GraphicsDraw,GraphicsIdle);
}

/******************************************************************************/

//void RobotForcesFunction( matrix &P, matrix &V, matrix &F )
//{
//int i;
//static matrix D(3,1);
//static double d,e;
//
//    RobotLoopFrequencyTimer.Loop();
//    RobotLoopLatencyTimer.Before();
//
//    RobotPositionRaw = P;
//    RobotPosition = P;
//    RobotVelocity = V;
//    RobotForces.zeros();
//
//    // Calculate robot position in graphics co-orindate system.
//	D = RobotPosition - SphereStartPos;
//    d = norm(D); // Magnitude
//	e = d - SphereStartRadius; // Scalar encroachment into surface of sphere.
//
//    if( ROBOT_JustActivated(RobotID) )
//    {
//        SphereInsideFlag = (e < 0);
//    }
//
//    if( SphereInsideFlag ) // If the cursor is inside the target ...
//    {
//        e = d - (SphereStartRadius-CursorRadius);
//    }
//    else // ... or if the cursor is outside the target 
//    {
//		e = d - (SphereStartRadius + CursorRadius);
//    }
//
//    if( (SphereInsideFlag && (e > 0.0)) || (!SphereInsideFlag && (e < 0.0)) )
//    {
//        RobotForces = RobotSpringConstant * e * (D/d); // PID controller but only position based i.e. Position/P controller 
//    }
//
//    RobotForces.clampnorm(RobotForceMax);
//
//    F = RobotForces; 
//
//    RobotLoopLatencyTimer.After();
//
//
//
//	//matrix JT = ROBOT_JT[RobotID];
//	//matrix ap(3, 1);
//	//matrix aa(3, 1);
//	//matrix av(3, 1);
//	//ROBOT_Angles(ap, av, aa);
//
//	//printf("-----------\n\n\n\n");
//	//for (int i = 1; i <= 3; i++){
//	//	printf("%lf %lf %lf\n", JT(i, 1), JT(i, 1), JT(i, 1));
//	//}
//	//printf("-----------\n\n\n\n");
//	
//}


void RobotForcesFunction2(matrix &P, matrix &V, matrix &F)
{
	int i;
	static matrix D(3, 1);
	static double d, e;
	float VelocityThreshold = 75.0; 

	RobotLoopFrequencyTimer.Loop();
	RobotLoopLatencyTimer.Before();

	RobotPositionRaw = P;
	RobotPosition = P;
	RobotVelocity = V;
	RobotForces.zeros();

//	printf("Robot velocity: %f \n", norm(RobotVelocity));

	if (norm(RobotVelocity) > VelocityThreshold)
	{
		D = RobotPosition - SphereDiversionPos;
		RobotForces = RobotSpringConstant * D;
		//RobotForces(1,1) = RobotSpringConstant * D(1,1); 
		//RobotForces(3, 1) = RobotSpringConstant * D(3, 1);
	}

	float ptrX = RobotPosition(1, 1);
	float ptrY = RobotPosition(2, 1);
	float ptrZ = RobotPosition(3, 1);
	//matrix RobotPointer = *RobotPosition;

	// UDP Message send start *********************************************************

	char buffer[1024];
	ostringstream buffX, buffY, buffZ;
	buffX << ptrX; 	
	buffY << ptrY; 	
	buffZ << ptrZ;
	
	string robotPositionMesg = buffX.str() + "," + buffY.str() + "," + buffZ.str();
	udpSndr.sendUDPmessage(robotPositionMesg);

	//int ret = snprintf(buffer, sizeof buffer, "%f", ptrX);
	//int ret2 = snprintf(buffer, sizeof buffer, "%f", ptrY);
	//int ret3 = snprintf(buffer, sizeof buffer, "%f", ptrZ);

	//if (ret < 0) {
	//	printf("err: Nothing in buffer!!! \n");
	//}
	//if (ret >= sizeof buffer) {
	//	printf("Result was truncated - resize the buffer and retry \n");
	//}
	//
	////printf("Robo pos string: %s	\n", buffer);

	////printf("Robo pos: %f	\n", ptrX);
	//udpSndr.sendUDPmessage(buffer);


	// UDP Message send end *********************************************************

	RobotForces.clampnorm(RobotForceMax);
	F = RobotForces;

	RobotLoopLatencyTimer.After();

	//matrix JT = ROBOT_JT[RobotID];
	//matrix ap(3, 1);
	//matrix aa(3, 1);
	//matrix av(3, 1);
	//ROBOT_Angles(ap, av, aa);

	//printf("-----------\n\n\n\n");
	//for (int i = 1; i <= 3; i++){
	//	printf("%lf %lf %lf\n", JT(i, 1), JT(i, 1), JT(i, 1));
	//}
	//printf("-----------\n\n\n\n");

}

/******************************************************************************/

BOOL RobotStart( void )
{
BOOL ok=FALSE;

    if( (RobotID=ROBOT_Open(RobotName)) == ROBOT_INVALID )
    {
        printf("Cannot open: \n",RobotName);
    }
    else
    if( !ROBOT_Start(RobotID,RobotForcesFunction2) )
    {
        printf("Cannot start: %s\n",RobotName);
    }
    else
    {
        ok = TRUE;
        printf("Robot started: %s\n",RobotName);
    }

    return(ok);
}

/******************************************************************************/

void RobotStop( void )
{
    ROBOT_Stop(RobotID);
    ROBOT_Close(RobotID);
}

/******************************************************************************/

void RobotTimingResults( void )
{
    RobotLoopFrequencyTimer.Results();
    RobotLoopLatencyTimer.Results();
}

/******************************************************************************/

void ProgramExit( void )
{
    // Stop and close the 3BOT and the Oculus HMD.
    RobotStop();
    GRAPHICS_Stop();

    // Timing results.
    RobotTimingResults();
    GraphicsTimingResults();

    // Exit program.
    exit(0);
}

/******************************************************************************/

void Usage( void )
{
    printf("Usage:\n\n");
    printf("%s /R:RobotName\n",MODULE_NAME);

    exit(0);
}

/******************************************************************************/

BOOL Parameters( int argc, char *argv[] )
{
BOOL ok;
char *data;
int i;

    for( ok=TRUE,i=1; ((i < argc) && ok); i++ )
    {
		//char test = CMDARG_code(argv[i], &data);
		//printf("%d:%s:%c", i, argv[i], test);
		//getchar();
        switch( CMDARG_code(argv[i],&data) )
        {
			
            case 'R' :
               ok = CMDARG_data(RobotName,data,STRLEN);
               break;

            case '?' :
               Usage();
               break;

            default :
               //ok = FALSE;
               break;
        }

        if( !ok )
        {
            printf("Invalid argument: %s\n",argv[i]);
        }
    }

    if( ok && STR_null(RobotName) )
    {
        printf("Robot name not specified.\n");
        ok = FALSE;
    }

    if( !ok )
    {
        Usage();
    }

    return(ok);
}

/******************************************************************************/
using namespace std;

void main(int argc, char *argv[])
{

	char key;
	int asciiValue; 

	while (1)
	{
		key = _getch(); 
		asciiValue = key; 

		if (asciiValue == 27)
		{
			break; 
		}

		cout << "Key pressed: -> " << key << "\"ascii value: " << asciiValue << endl; 

		//if (asciiValue == 104) // H key pressed
		//{
		//	udpSndr.sendUDPmessage('1');
		//}
		//if (asciiValue == 106) // H key pressed
		//{
		//	udpSndr.sendUDPmessage('2');
		//}
	}







	if (!Parameters(argc, argv))
	{
		exit(0);
	}

	// Position of sphere in simulation...
	//SphereStartPos.zeros();
	//printf("%d", SphereStartPos);

	SphereStartPos(1, 1) = 0.0; // Positive = Right		(X)
	SphereStartPos(2, 1) -= 10.0; // Positive = Forward	(Y)
	SphereStartPos(3, 1) = 0.0; // Positive = Up		(Z)

	// Target Positions 
	STP1(1, 1) = 0.0;
	STP1(2, 1) = 15.0;
	STP1(3, 1) = 15.0;

	STP2(1, 1) = 15.0;
	STP2(2, 1) = 15.0;
	STP2(3, 1) = 0.0;

	STP3(1, 1) = 0.0;
	STP3(2, 1) = 15.0;
	STP3(3, 1) -= 15.0;

	STP4(1, 1) -= 15.0;
	STP4(2, 1) = 15.0;
	STP4(3, 1) = 0.0;

	//printf ("%d",SpherePosition);

	if (!RobotStart())
	{
		exit(0);
	}

	// Start graphics.
	GraphicsStart();

}













//void sendUDPmessage(SOCKET ss, int sslen, struct sockaddr_in& ssi_other, double* poS)
//{
//	char smessageZ[16];
//	int ret1 = snprintf(smessageZ, sizeof smessageZ, "%f", &poS);
//	if (sendto(ss, smessageZ, strlen(smessageZ), 0, (struct sockaddr *) &ssi_other, sslen) == SOCKET_ERROR)
//	{
//		printf("sendto() failed with error code : %d", WSAGetLastError());
//		exit(EXIT_FAILURE);
//	}
//}
//
////float receiveUDPmessage(SOCKET sz, WSADATA wsaz, int slenz, struct sockaddr_in& si_otherz)
//void receiveUDPmessage(SOCKET sz, WSADATA wsaz, int slenz, struct sockaddr_in& si_otherz, float* val)
//{
//	// UDP init start
//	float afz;
//	char bufz[4];
//	int recv_len;
//	//struct sockaddr_in si_other; 
//
//	while (true)
//	{
//		//printf("Waiting for data...");
//		fflush(stdout);
//
//		//clear the buffer by filling null, it might have previously received data
//		memset(bufz, '\0', BUFLEN);
//
//		//try to receive some data, this is a blocking call
//		if ((recv_len = recvfrom(sz, bufz, BUFLEN, 0, (struct sockaddr *) &si_otherz, &slenz)) == SOCKET_ERROR)
//		{
//			printf("recvfrom() failed with error code : %d", WSAGetLastError());
//			exit(EXIT_FAILURE);
//		}
//
//		memcpy(&afz, bufz, sizeof afz);
//		//printf("AF: %f \n", afz);
//
//		*val = afz;
//		//printf("%f: \n", *val); // << std::endl;
//
//		//************************** Send *****************************
//
//		//char sendMsg[4] = { "78" };
//		//char bufsend[4];
//
//		//memcpy(&bufsend, sendMsg, sizeof bufsend);
//		//printf("%s: \r", bufsend);
//
//		////now reply the client with the same data
//		//int sentz = sendto(sz, bufsend, recv_len, 0, (struct sockaddr*) &si_otherz, slenz);
//		//if (sendto(sz, bufsend, recv_len, 0, (struct sockaddr*) &si_otherz, slenz) == SOCKET_ERROR)
//		//{
//		//	printf("sendto() failed with error code : %d", WSAGetLastError());
//		//	exit(EXIT_FAILURE);
//		//}
//
//	}
//
//	//return afz;
//}
//

/******************************************************************************/


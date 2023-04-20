/*
 
    -----------------------
    UDP-Receiver
    Author: Diar Karim
    email: diarkarim@gmail.com
    Date: 14th Aug 2018
    -----------------------
This script receives position data from the 3Bot robotic manipulandum and passes it to a gameobject's positions. 

Next, add collision detection and sent it back to the robot for force control e.g. for creating a force channel to guide user's hands during target reaching movements.
 
*/
using UnityEngine;
using System.Collections;
 
using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class UDPReceive_v1 : MonoBehaviour
{
   
	// receiving Thread
	Thread receiveThread;
 
	// udpclient object
	UdpClient client;

	public float heightOffset; 
	public int cnter = 0;
	public float eR = 0;
	public static float eX;
	public static float eY;
	public static float eZ;
	public GameObject player;

	// public
	// public string IP = "127.0.0.1"; default local
	public int port;
	// define > init
 
	// infos
	public string lastReceivedUDPPacket = "";
	public string allReceivedUDPPackets = "";
	// clean up this from time to time!
   
   
	// start from shell
	private static void Main ()
	{
		UDPReceive_v1 receiveObj = new UDPReceive_v1 ();
		receiveObj.init ();
 
		string text = "";
		do {
			text = Console.ReadLine ();
		} while(!text.Equals ("exit"));
	}
	// start from unity3d
	public void Start ()
	{
		init ();
	}

	void Update ()
	{
		Vector3 roboPos = new Vector3 (eY, eZ+heightOffset, -eX);
		Quaternion roboOrient = Quaternion.identity;
		player.transform.SetPositionAndRotation (roboPos, roboOrient);
	}

	//// OnGUI
	//void OnGUI()
	//{
	//    Rect rectObj=new Rect(40,10,200,400);
	//        GUIStyle style = new GUIStyle();
	//            style.alignment = TextAnchor.UpperLeft;
	//    GUI.Box(rectObj,"# UDPReceive\n127.0.0.1 "+port+" #\n"
	//                + "shell> nc -u 127.0.0.1 : "+port+" \n"
	//                + "\nLast Packet: \n"+ lastReceivedUDPPacket
	//                + "\n\nAll Messages: \n"+allReceivedUDPPackets
	//            ,style);
	//}
       
	// init
	private void init ()
	{
		// Endpunkt definieren, von dem die Nachrichten gesendet werden.
		print ("UDPSend.init()");
       
		// define port
		port = 8899;
 
		// status
		print ("Sending to 127.0.0.1 : " + port);
		print ("Test-Sending to this Port: nc -u 127.0.0.1  " + port + "");
 
   
		// ----------------------------
		// Abhören
		// ----------------------------
		// Lokalen Endpunkt definieren (wo Nachrichten empfangen werden).
		// Einen neuen Thread für den Empfang eingehender Nachrichten erstellen.
		receiveThread = new Thread (
			new ThreadStart (ReceiveData));
		receiveThread.IsBackground = true;
		receiveThread.Start ();
 
	}
 
    
	// receive thread
	private  void ReceiveData ()
	{
 
		client = new UdpClient (port);
		while (true) {
 
			try {
				// Bytes empfangen.

				IPEndPoint anyIPR = new IPEndPoint (IPAddress.Any, 0);
				byte[] dataR = client.Receive (ref anyIPR);  
				string textr = Encoding.UTF8.GetString (dataR);

				string[] recMsg = textr.Split(',');

				eX = float.Parse (recMsg[0]);
				eY = float.Parse (recMsg[1]);
				eZ = float.Parse (recMsg[2]);

				Debug.Log("Received udp message: " + eX.ToString()); 

				// cnter++;

			} catch (Exception err) {
				//print (err.ToString ());
			}
		}
	}
   
	//// getLatestUDPPacket
	//// cleans up the rest
	//public string getLatestUDPPacket()
	//{
	//    allReceivedUDPPackets="";
	//    return lastReceivedUDPPacket;
	//}
}

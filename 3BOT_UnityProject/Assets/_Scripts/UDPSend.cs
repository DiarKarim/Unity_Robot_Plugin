using UnityEngine;
using System.Collections;

using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class UDPSend : MonoBehaviour
{
    private static int localPort;

    // prefs
    private string IP;  // define in init
    public int port;  // define in init

    // "connection" things
    IPEndPoint remoteEndPoint;
    UdpClient client;

    // gui
    string strMessage = "0.1";


    // call it from shell (as program)
    private static void Main()
    {
        UDPSend sendObj = new UDPSend();
        sendObj.init();

        // testing via console
        // sendObj.inputFromConsole();

        // as server sending endless
        sendObj.sendEndless(" endless infos \n");

    }
    // start from unity3d
    public void Start()
    {
        init();
    }

	void Update()
	{
		sendString(strMessage + "\n");

	}

    // OnGUI
    void OnGUI()
    {
        Rect rectObj = new Rect(40, 380, 200, 400);
        GUIStyle style = new GUIStyle();
        style.alignment = TextAnchor.UpperLeft;
        GUI.Box(rectObj, "# UDPSend-Data\n127.0.0.1 " + port + " #\n"
                    + "shell> nc -lu 127.0.0.1  " + port + " \n"
                , style);

        // ------------------------
        // send it
        // ------------------------
		strMessage = GUI.TextField(new Rect(160, 360, 140, 20), strMessage);
        if (GUI.Button(new Rect(310, 360, 40, 20), "send"))
        {
            sendString(strMessage + "\n");
        }
    }

    // init
    public void init()
    {
        // Endpunkt definieren, von dem die Nachrichten gesendet werden.
        print("UDPSend.init()");

        // define
        IP = "127.0.0.1";
        port = 8888;

        // ----------------------------
        // Senden
        // ----------------------------
        remoteEndPoint = new IPEndPoint(IPAddress.Parse(IP), port);
        client = new UdpClient();

        // status
        print("Sending to " + IP + " : " + port);
        print("Testing: nc -lu " + IP + " : " + port);

    }

    //// inputFromConsole
    //private void inputFromConsole()
    //{
    //    try
    //    {
    //        string text;
    //        do
    //        {
    //            text = Console.ReadLine();

    //            // Den Text zum Remote-Client senden.
    //            if (text != "")
    //            {

    //                // Daten mit der UTF8-Kodierung in das Binärformat kodieren.
    //                byte[] data = Encoding.UTF8.GetBytes(text);

    //                // Den Text zum Remote-Client senden.
    //                client.Send(data, data.Length, remoteEndPoint);
    //            }
    //        } while (text != "");
    //    }
    //    catch (Exception err)
    //    {
    //        print(err.ToString());
    //    }

    //}

    // sendData
    private void sendString(string message)
    {
        try
        {
            float mess = float.Parse(message);

            // Daten mit der UTF8-Kodierung in das Binärformat kodieren.
            byte[] data = BitConverter.GetBytes(mess);

            // Den message zum Remote-Client senden.
            client.Send(data, data.Length, remoteEndPoint);
            //Debug.Log("Sending length: " + data.Length);
//            Debug.Log("Tx : " + message);

        }
        catch (Exception err)
        {
            print(err.ToString());
        }
    }


    // endless test
    private void sendEndless(string testStr)
    {
        do
        {
            sendString(testStr);


        }
        while (true);

    }

}
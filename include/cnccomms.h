#ifndef Comms_H
#define Comms_H
#if defined(_WIN32) | defined(_WIN64)
    #include <winsock2.h>
    #include "winpthreads.h"
#endif

#include <time.h>
#include "cncstatebuf.pb.h"
#include "cncplugin.h"
#include "PassiveSocket.h"

using namespace std;

namespace CncRemote {
#define CONN_TIMEOUT (2)

#define MAX_PACKET_SIZE 2048

#define MAX_AXES 6

#define pktESCAPE 0xFF

#define DEFAULT_COMMS_PORT 5080

class Server;

struct Packet
{
    uint16_t cmd;
    string data;
};

enum COMERROR{
    errOK, //no error
    errSOCKET, //Failed to create socket
    errBIND, //Failed to bind socket (server only)
    errCONNECT, //Lost connection
    errNOSOCKET, //No valid socket
    errFAILED, //Failed to send data
    errNODATA, //no data was received. This is not an error. Simply no data was available to be processed.
    errTHREAD, //Failed to create thread
    errRUNNING, //Thread is already running
    errNOTHREAD, //Unable to create thread
};


class Comms// : public StateBuf
{
    public:
        Comms(CActiveSocket *socket, Server * server = NULL);
        Comms();
        virtual ~Comms();

        void SetSocket(CActiveSocket *socket){m_socket = socket;}
        void Connect(const CncString& address, const uint32_t port); //Connect to a server. Note the connection will only be attempted on the next Poll()
       	bool IsConnected(){return m_isConnected;} //Is the server connected and running?
  	 	virtual void OnConnection(const bool state){}; //Override this if you want to be notfied of connection/disconnection. state=true if we have just connected


        COMERROR Poll(); //Call on a regular basis
        void SetTimeout(float seconds); //Set timeout for data send/receive. If set to 0 send/receive are non-blocking

        void Close(); //Close the connection

        enum CMDTYPE { //it is safe to add to this list but entries must not be deleted or rearranged
            cmdNULL,
            cmdPING,	//none
			cmdSTATE,	//StateBuf
            cmdDRIVESON, //boolean
            cmdJOGVEL,	//Axes. Jog at given velocities
			cmdJOGSTEP,  //Axes. Jog the given amount
            cmdMDI,		//string
            cmdFRO,		//float
            cmdFILE,	//string
			cmdCLOSEFILE, //none
            cmdSTART,	//none
            cmdSTOP,	//none
            cmdPAUSE,	//none
            cmdBLOCKDEL,	//boolean
            cmdSINGLESTEP,	//boolean
            cmdOPTSTOP,		//boolean
			cmdSENDFILE,	//string
			cmdREQFILE,
			cmdFLOOD,		//boolean
			cmdMIST,		//boolean
			cmdSPINDLE, //integer: one of spinOFF,spinFWD,spinREV
			cmdHOME,	//BoolAxes. All true = home all. otherwise ONE axis only.


			cmdMAX, //this should always be the last entry. Note your code should always be able to handle command numbers past this.
        };


    protected:

        static void * Entry(void * t);
        void * Entry();
		bool SendCommand(const uint16_t cmd);
		bool SendCommand(const uint16_t cmd, const bool state);
		bool SendCommand(const uint16_t cmd, const double value);
		bool SendCommand(const uint16_t cmd, const string value);
        bool SendCommand(const uint16_t command, const CmdBuf& data);
        void CheckConn();
		int RecvString(string& data);
		bool SendPacket(const Packet &packet);
//		bool RecvPacket(Packet &packet);
        size_t CobsEncode(const uint8_t *ptr, size_t length, uint8_t *dst);
        void CobsDecode(const uint8_t *ptr, size_t length);
        void OnData();

        virtual void HandlePacket(const Packet & pkt) = 0;
        virtual void OnPacketError(){};

    private:
        CActiveSocket *m_socket;
        Server * m_server;
        uint32_t m_port;
        CncString m_address;

    private:
        void Connected(const bool state);
        Packet m_packet;
        bool m_isConnected;
        float m_timeout;
        string m_rxData;

};

} //namespace CncRemote


#endif // Comms_H

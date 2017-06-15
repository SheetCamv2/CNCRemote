#ifndef Comms_H
#define Comms_H
#if defined(_WIN32) | defined(_WIN64)
    #include <winsock2.h>
#endif

#include <time.h>
#include "cncstatebuf.pb.h"
#include "cncplugin.h"
#include "zmq.h"

using namespace std;

namespace CncRemote {
#define CONN_TIMEOUT (2)


#define MAX_AXES 6

#define DEFAULT_COMMS_PORT 5080

class Comms : public StateBuf
{
    public:
		enum COMERROR{errOK, //no error
			errSOCKET, //Failed to create socket
			errBIND, //Failed to bind socket (server only)
			errCONNECT, //Failed to create connection (client only)
			errNOSOCKET, //No valid socket
			errFAILED, //Failed to send data
		};

        Comms();
        virtual ~Comms();

    protected:
		struct Packet
		{
			uint16_t cmd;
			string data;
		};

		bool SendCommand(const uint16_t cmd);
		bool SendCommand(const uint16_t cmd, const bool state);
		bool SendCommand(const uint16_t cmd, const double value);
		bool SendCommand(const uint16_t cmd, const string value);
        bool SendCommand(const uint16_t command, const CmdBuf& data);
        void CheckConn();
		int RecvString(string& data);
		bool SendPacket(const Packet &packet);
		bool RecvPacket(Packet &packet);

        virtual void HandlePacket(const Packet & pkt) = 0;

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

        void *m_socket;
		void *m_context;
		time_t m_timeout;
    private:
};

} //namespace CncRemote


#endif // Comms_H

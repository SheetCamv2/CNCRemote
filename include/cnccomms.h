#ifndef CNCCOMMS_H
#define CNCCOMMS_H

#include <cncstatebuf.pb.h>
#include "zmq.hpp"

using namespace std;

class CommsException : public std::exception
{
public:
    std::string m_msg;
    CommsException(const std::string msg){m_msg = msg;}
    CommsException() throw(){};
    virtual const char *what () const throw ()
    {
        return m_msg.c_str();
    }
    std::string GetMessage(){return m_msg;}
};

#define MAX_AXES 6

union AXES
{
    double array[MAX_AXES];
    struct{
        double x;
        double y;
        double z;
        double a;
        double b;
        double c;
    };
};

union BOOLAXES
{
    bool array[MAX_AXES];
    struct{
        bool x;
        bool y;
        bool z;
        bool a;
        bool b;
        bool c;
    };
};


class CncComms : public CncStateBuf
{
    public:
        CncComms(const string ipAddress, const string statePort, const string cmdPort, const bool server = false);
        virtual ~CncComms();

        void Send(); //Used in server mode. Send status to connected clients. Call after setting any status values.
        bool Poll(); //Call at least as often as you need to send/receive data. Returns true if any data was received.

    protected:
        void SendCommand(const CncCmdBuf& cmd); //Clients only
        void CheckConn();

        virtual void ExecuteCommand(const CncCmdBuf& cmd){return;} //Servers need to override this

        enum CMDTYPE { //it is safe to add to this list but entries must not be deleted or rearranged
            cmdNULL,
            cmdDRIVESON,
            cmdJOGVEL,
            cmdMDI,
            cmdFRO,
            cmdFILE,
            cmdSTART,
            cmdSTOP,
            cmdPAUSE,
            cmdBLOCKDEL,
            cmdSINGLESTEP,
            cmdOPTSTOP,
        };


        bool m_isServer;
        zmq::context_t *m_context;
        zmq::socket_t *m_stateSocket;
        zmq::socket_t *m_cmdSocket;
    private:
};

class CncClient : public CncComms
{
public:
    CncClient(const string ipAddress, const string statePort, const string cmdPort);
    //Avoid adding functions that are specific to a certain control software.
    //If you add functions you must also update CncComms::CMDTYPE
    void DrivesOn(const bool state); //Turn drives/control on
    void JogVel(const AXES velocities); //jog axes at the given rates. Use axis values of 0 to stop jogging.
    void Mdi(const string line); //execute MDI command
    void SetFRO(const double percent); //set feed rate override (1 = 100%)
    void LoadFile(const string file); //load file
    void CycleStart(); //cycle start
    void Stop(); //Stop execution
    void Pause(const bool state); //pause/resume motion
    void BlockDelete(const bool state); //Turn block delete on/off
    void SingleStep(const bool state); //Turn single stepping on/off
    void OptionalStop(const bool state); //Turn optional stop stepping on/off

};


#endif // CNCCOMMS_H

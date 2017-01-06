#ifndef CNCSERVER_H_INCLUDED
#ifndef CNCSERVER_H_INCLUDED

#include "cnccomms.h"

namespace CncRemote {


class Server : public Comms
{
public:

    Server();
    virtual ~Server();
    COMERROR SendState(const string& id); //Send status to connected clients. Call after setting any status values. Should be called at least twice a second even if no new data is available.
    CncString GenerateTcpAddress(const int port = DEFAULT_COMMS_PORT); //Generates a TCP address for Connect(). useLocal overrides the given IP address
    COMERROR Connect(const CncString address);
    void Disconnect();
    bool Poll(); //Call at least as often as you need to send/receive data. Returns true if any data was received.
    virtual void UpdateState(){} //Called just before state is sent out

protected:
/*    struct Identity
    {
        string name;
        time_t timeout;
        bool needState;
    };

    vector<Identity> m_ids;*/
    string m_curId;

private:
};


}//namespace CncRemote

#endif

#endif //#ifndef CNCSERVER_H_INCLUDED


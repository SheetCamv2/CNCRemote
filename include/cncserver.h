#ifndef CNCSERVER_H_INCLUDED
#ifndef CNCSERVER_H_INCLUDED

#include "cnccomms.h"
#include "cncstatebuf.pb.h"

namespace CncRemote {

class Server;

class Connection : public Comms
{
public:

    Connection(CActiveSocket * socket, Server * server);
    virtual ~Connection();
//    COMERROR SendState(const string& id); //Send status to connected clients. Call after setting any status values. Should be called at least twice a second even if no new data is available.
    void Close();
    COMERROR Poll(); //Call at least as often as you need to send/receive data. Only needed if this connection isn't running in it's own thread
    virtual void UpdateState(); //Called just before state is sent out. Default code just grabs state from the server and sends it out. Override this if you need to add your own state data.
    inline bool IsClosing(){return m_closing;};
    COMERROR Run(); //Start this connection in a new thread. Returns immediately after the thread has been created.

protected:

    string m_curId;
    CActiveSocket * m_socket;
    Server * m_server;

private:
    static void * Entry(void * param);
    void * Entry();

    bool m_closing;
    pthread_t m_thread;
};

//simple class to handle locking. Mutex remains locked for the lifetime of this object.
class MutexLocker
{
public:
    MutexLocker(pthread_mutex_t * mutex)
    {
        m_mutex = mutex;
        pthread_mutex_lock(mutex);
    }

    ~MutexLocker()
    {
        pthread_mutex_unlock(m_mutex);
    }

private:
    pthread_mutex_t * m_mutex;
};

class Server
{
public:
    Server();
    void SetTimeout(const float seconds);
    COMERROR Bind(const uint32_t port = DEFAULT_COMMS_PORT);
    COMERROR Poll();
    void RemoveConn(Connection * conn);
    virtual Connection * CreateConnection(CActiveSocket * client, Server * server)= 0;
    Packet GetState();
    void UpdateState();
    MutexLocker GetLock() {return (MutexLocker(&m_syncLock));} //Sync your thread to the main thread for as long as the MutexLocker object exists. Basically locks your thread to the server's Poll() loop

protected:
    void SetTimeout();

    CPassiveSocket * m_socket;
    float m_timeout;
    bool m_listening;
    vector<Connection *> m_conns;
    StateBuf m_state;
    Packet m_packet;
    Packet m_statePacket;
    pthread_mutex_t m_stateLock;
    pthread_mutex_t m_syncLock;
private:

};

}//namespace CncRemote

#endif

#endif //#ifndef CNCSERVER_H_INCLUDED


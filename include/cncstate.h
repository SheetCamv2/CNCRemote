#ifndef CNCSTATE_H
#define CNCSTATE_H

#include <cncstatebuf.pb.h>


class CncState : public CncStateBuf
{
    public:
        CncState();
        virtual ~CncState();
    protected:
    private:
};

#endif // CNCSTATE_H

// rrsignal.h
//
// Author: Kendall Auel
//
// The class "RRsignal" represents a red/green signal light for a
// railroad traffic signaling simulation. When placed on an end of
// a track segment, it will prevent trains from passing that end
// point until the signal becomes green.
//
// The signal is green when the track segments beyond this endpoint
// are free from any oncoming trains on all continuation segments
// (i.e. up to the next junction). Also the signal is green if the
// one next track segment is free from any outgoing trains.
//
// The signal is red if the next segment is occupied, if there is
// an oncoming train anywhere up to the next junction, and if entering
// a junction from a fork but the switch is not connected to this fork.
// The signal will always be red on a terminator segment.

#ifndef _CS_RRSIGNAL_H_
#define _CS_RRSIGNAL_H_

#include "common.h"

namespace rrsim {

class RRsignal
{
public:
    RRsignal(Edge* trackSeg, eEnd trackEnd);
    ~RRsignal();

    void updateSignal();
    bool signalIsRed() { return m_isRed; }

    static void updateAllSignals();

private:
    bool        m_isRed;
    EdgeEnd     m_edge;
};

} // namespace rrsim

#endif // _CS_RRSIGNAL_H_

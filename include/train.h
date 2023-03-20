// train.h
//
// Author: Kendall Auel
//
// This class represents a train that can travel along a network
// of tracks. It can run with or without a pre-planned route.
// If a route does exist then the train will request the junction
// switch positions that are specified in its route.
//
// The train always obeys any signals that it encounters while
// traveling on the network. In particular, it will pause its
// travel whenever it encounters a red signal, and resume after
// the signal changes to green.
//
// If the train encounters a terminator at the end of a track
// segment, it will stop permanently. The train has a specific
// direction of travel, and will not reverse direction.

#ifndef _CS_TRAIN_H_
#define _CS_TRAIN_H_

#include "common.h"
#include <string>

namespace rrsim {

class Train
{
public:
    Train();
    ~Train();

    void placeOnTrack(const std::string& trackName, eEnd direction);

    // Returns false when the train has reached a terminator.
    bool stepSimulation();

    EdgeEnd getPosition() { return m_edge; }

    void show();

private:
    std::string m_name;
    EdgeEnd     m_edge;
};

} // namespace rrsim

#endif // _CS_TRAIN_H_

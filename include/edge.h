// edge.h
//
// Author: Kendall Auel
//
// The "Edge" class represents a railroad track segment between
// two nodes. The weight of the edge corresponds to the length
// of track.
//
// NOTE: currently, all edges are assumed to have weight = 1.
//
// A train can travel along the edge (track segment) in either
// direction, toward node A or toward node B. When the train
// crosses to the next segment the apparent direction can then
// flip, say, if it was traveling toward A and then entered
// the next track at node A, traveling toward node B.
//
// Either (or both) ends of the track segment can have a signal
// device placed on it. This controls whether or not the train
// is allowed to proceed through that node to the next track
// segment.
//
// The Edge object coordinates with other Edge objects when
// determining the state of the signal, so as to avoid collisions
// between trains on the track network.

#ifndef _CS_EDGE_H_
#define _CS_EDGE_H_

#include "common.h"
#include "system.h"
#include <string>
#include <map>

namespace rrsim {

class Node;
class RRsignal;
class Train;

// Class Edge
//     This class represents a segment of railroad track.
//     It acts as the edge between nodes in the graph of
//     of the track network.

class Edge : public std::enable_shared_from_this<Edge>
{
public:
    Edge(const std::string& name);
    ~Edge();

    RRsignal* getSignal(eEnd myEnd);
    void placeSignalLight(eEnd myEnd);

    TrainPtr getTrain() { return m_train; }
    void setTrain(TrainPtr train) { m_train = train; }

    NodeSlot getNode(eEnd getEnd);
    NodeSlot getAdjacent(eEnd getEnd);
    void assignNodeSlot(NodeSlot node, eEnd nodeEnd);

    const std::string& name() { return m_name; }

    void show(eEnd showEnd = eNumEnds);

    std::string serialize();
    void deserialize(const std::string& serialStr);

private:
    std::string     m_name;
    double          m_weight;
    NodeSlot        m_ends[eNumEnds];
    RRsignal*       m_signals[eNumEnds];
    TrainPtr        m_train;
};

} // namespace rrsim

#endif // _CS_EDGE_H_

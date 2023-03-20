// node.h
//
// Author: Kendall Auel
//
// The class "Node" represents a point where track segments are
// connected, in an undirected graph that is the track network.
// This is used for simulating a railroad traffic signaling system.
//
// A node can have at most three track segments connected to it:
// - One track if this end of the segment is a terminator.
// - Two tracks if the segments are a continuation of each other.
// - Three tracks if a switchable junction occurs at this node.
//
// A junction Node keeps the switch state to determine which side
// currently contributes to the track graph.
//
// When searching the graph, the junction behaves as two separate
// continuation Node objects, each of which are adjacent to the common
// Node. This prevents the search from linking the left and right
// connected nodes, since the train cannot travel on that path.

#ifndef _CS_NODE_H_
#define _CS_NODE_H_

#include "common.h"
#include <string>
#include <map>

namespace rrsim {

// Class Node
//     This class acts as the adjacency list and state manager for
//     nodes in a graph that represents a network of railroad tracks.

class Node
{
public:
    Node();
    ~Node();

    eNodeType getNodeType();

    // Make this node a terminator by setting the EdgeEnd into the first
    // slot. This assumes that all slots are currently empty.
    void makeTerminator(const EdgeEnd& track);

    // Make this node a continuation by setting the EdgeEnd into the second
    // slot. This assumes the node is currently a terminator.
    void makeContinuation(const EdgeEnd& track);

    // Make this node a junction by setting the EdgeEnd into the third slot.
    // The first slot holds the common track, the second slot holds the left
    // fork of the junction, and the third slot holds the right fork. This
    // assumes the node is currently a continuation.
    void makeJunction(const EdgeEnd& track);

    // Return the EdgeEnd attached at the given slot.
    EdgeEnd getEdgeEnd(eSlot slot) { return m_slots[slot]; }

    // Return the next EdgeEnd encountered by a train that is traveling
    // through the given node slot. If this is a terminator, the EdgeEnd
    // will be empty. If this is a junction fork that is not currently
    // switched, the EdgeEnd will also be empty.
    EdgeEnd getNext(eSlot slot);

    const std::string& name() { return m_name; }

    eJSwitch getSwitchPos() { return m_switchState; }

    void show();

    // This static method returns a node name of the pattern "nodeNNN"
    // that is not currently in the global node map.
    static std::string getUniqueNodeName();

private:
    std::string     m_name;
    EdgeEnd         m_slots[3];
    eJSwitch        m_switchState;
};

// NodeMap
//     All nodes are kept in this globally accessible container. This is the
//     adjacency list for the graph of the railroad track network. Searching
//     the graph is useful for finding the optimal route for a train.
//
using NodeMap = std::map<std::string, Node*>;
using NodePair = std::pair<std::string, Node*>;

extern NodeMap g_nodeMap;

} // namespace rrsim

#endif // _CS_NODE_H_

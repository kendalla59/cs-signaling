// common.h
//
// Author: Kendall Auel
//
// This file contains the definitions of data elements that are
// common across the various modules. This includes enum declarations,
// and structures used to manage shared data.

#ifndef _CS_COMMON_H_
#define _CS_COMMON_H_

namespace rrsim {

// Nodes can be terminator, continuation, or junction types.
//
enum eNodeType {
    eEmpty,         // No edges have been connected.
    eTerminator,    // Only one connected edge.
    eContinuation,  // Two connected edges.
    eJunction,      // Three connected edges.
};

// A junction switch can be in one of three possible states.
//
enum eJSwitch {
    eSwitchNone,    // Unknown or possibly in motion.
    eSwitchLeft,    // Switch from common to left track.
    eSwitchRight,   // Switch from common to right track.
};

// Each node has three slots to which an edge can be
// attached. For a junction, slot 1 is the common edge,
// slot 2 is the left edge, and slot 3 is the right edge.
// This enum also acts as the index into the array of
// Edge/End structures kept by the Node.
//
enum eSlot {
    eSlot1 = 0,
    eSlot2 = 1,
    eSlot3 = 2,

    eNumSlots = 3
};

// Each edge has two ends, each connected to a node.
// This enum also acts as the index into the array of
// Node/Slot structures kept by the Edge.
//
enum eEnd {
    eEndA = 0,
    eEndB = 1,

    eNumEnds = 2
};

class Node;
class Edge;

struct NodeSlot {
    Node*   nsNode;
    eSlot   nsSlot;
};

struct EdgeEnd {
    Edge*   eeEdge;
    eEnd    eeEnd;
};

} // namespace rrsim

#endif // _CS_COMMON_H_

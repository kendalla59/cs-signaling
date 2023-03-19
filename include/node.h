// node.h
//
// Author: Kendall Auel
// The class Node represents a connection between two track segments,
// used for simulating a railroad traffic signaling system.
//
// There are three Node types:
// - Adjacent to only one other Node, a terminator on the track graph.
// - Adjacent to two other Node objects, a continuation of the track.
// - Adjacent to two three Node objects, one half of a left/right junction.
//
// A junction consists of a left and a right Node, and keeps the switch
// state to determine which side contributes to the current track graph.
//

#ifndef _CS_NODE_H_
#define _CS_NODE_H_

#include <string>

class Node
{
public:
    Node();
    virtual ~Node();

private:
    std::string m_name;

};

#endif // _CS_NODE_H_

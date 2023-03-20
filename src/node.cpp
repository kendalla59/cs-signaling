// node.cpp
//
// Author: Kendall Auel
//
// Implementation of the Node class.

#include "node.h"
#include "edge.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace rrsim {

// This is the adjacency mapping for the entire graph of track segments.
NodeMap g_nodeMap;

Node::Node()
{
    m_name = getUniqueNodeName();
    for (int ix = 0; ix < eNumSlots; ix++) {
        m_slots[ix].eeEdge = nullptr;
        m_slots[ix].eeEnd = eNumEnds;
    }
    m_switchState = eSwitchNone;

    g_nodeMap.insert(NodePair(m_name, this));
}

Node::~Node()
{
    auto it = g_nodeMap.find(m_name);
    if (it != g_nodeMap.end()) {
        g_nodeMap.erase(it);
    }
}

eNodeType Node::getNodeType()
{
    if (m_slots[eSlot3].eeEdge) { return eJunction; }
    if (m_slots[eSlot2].eeEdge) { return eContinuation; }
    if (m_slots[eSlot1].eeEdge) { return eTerminator; }
    return eEmpty;
}

void Node::makeTerminator(const EdgeEnd& track)
{
    if (getNodeType() != eEmpty) {
        throw std::runtime_error(
                "Attempt to makeTerminator, but node is not empty");
    }
    m_slots[eSlot1] = track;
}

void Node::makeContinuation(const EdgeEnd& track)
{
    if (getNodeType() != eTerminator) {
        throw std::runtime_error(
                "Attempt to makeContinuation, but node is not a terminator");
    }
    m_slots[eSlot2] = track;
}

void Node::makeJunction(const EdgeEnd& track)
{
    if (getNodeType() != eContinuation) {
        throw std::runtime_error(
                "Attempt to makeJunction, but node is not a continuation");
    }
    m_slots[eSlot3] = track;
    m_switchState = eSwitchLeft;
}

EdgeEnd Node::getNext(eSlot slot)
{
    // Initialize the return value to be empty.
    EdgeEnd rval = { nullptr, eNumEnds };

    switch (getNodeType()) {
    default:
    case eEmpty:
        throw std::runtime_error("Unexpected result in Node::getNext");

    case eTerminator:
        // Nowhere to go from here (rval stays empty).
        break;

    case eContinuation:
        // Continue to the opposite edge.
        if      (slot == eSlot1) { rval = m_slots[eSlot2]; }
        else if (slot == eSlot2) { rval = m_slots[eSlot1]; }
        // Otherwise return empty. TODO throw an exception?
        break;

    case eJunction:
        // The switch state determines the next edge.
        if (m_switchState == eSwitchLeft) {
            if      (slot == eSlot1) { rval = m_slots[eSlot2]; }
            else if (slot == eSlot2) { rval = m_slots[eSlot1]; }
            // Otherwise blocked on the right fork (return empty).
        }
        else if (m_switchState == eSwitchRight) {
            if      (slot == eSlot1) { rval = m_slots[eSlot3]; }
            else if (slot == eSlot3) { rval = m_slots[eSlot1]; }
            // Otherwise blocked on the left fork (return empty).
        }
        // Otherwise not switched, so nobody is going anywhere.
        break;
    }
    return rval;
}

void Node::show()
{
    std::stringstream nstr;
    Edge* eptr;
    nstr << std::setw(12) << std::right << m_name << ':';

    for (int ix = 0; ix < eNumSlots; ix++) {
        eptr = m_slots[ix].eeEdge;
        if (eptr) {
            Node* next = eptr->getAdjacent(m_slots[ix].eeEnd).nsNode;
            if (next) {
                if (ix > 0) { nstr << ','; }
                nstr << std::setw(10) << next->name();
            }
        }
        else break;
    }

    // This is true if and only if the node is a junction.
    if (eptr) {
        nstr << std::setw(9) << std::right << "(switch";
        switch (m_switchState) {
        case eSwitchNone:   nstr << ": none)";   break;
        case eSwitchLeft:   nstr << ": left)";   break;
        case eSwitchRight:  nstr << ": right)";  break;
        default:            nstr << ": --?--)";  break;
        }
    }
    std::cout << nstr.str() << std::endl;
}

// Static method to determine the next unused name for a node.
std::string Node::getUniqueNodeName()
{
    int ix = 1;
    std::string name = "node001";
    while (g_nodeMap.find(name) != g_nodeMap.end()) {
        ix++;
        std::stringstream ns;
        ns << "node" << std::setw(3) << std::setfill('0') << ix;
        name = ns.str();
    }
    return name;
}

} // namespace rrsim

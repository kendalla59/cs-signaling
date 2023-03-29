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

Node::Node(const std::string& name) : m_name(name), m_switchState(eSwitchNone)
{
    // Initialize edge ends as invalid.
    for (int ix = 0; ix < eNumSlots; ix++) {
        m_slots[ix].eeEnd = eNumEnds;
    }
}

Node::~Node()
{
}

eNodeType Node::getNodeType()
{
    if (m_slots[eSlot3].eeEdge.lock()) { return eJunction; }
    if (m_slots[eSlot2].eeEdge.lock()) { return eContinuation; }
    if (m_slots[eSlot1].eeEdge.lock()) { return eTerminator; }
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

void Node::setEdgeEnd(const EdgeEnd& track, eSlot slot)
{
    if ((slot != eSlot1) && (slot != eSlot2) && (slot != eSlot3)) {
        throw std::runtime_error("Invalid slot for setEdgeEnd");
    }
    m_slots[slot] = track;
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
    EdgePtr eptr;
    nstr << std::setw(12) << std::right << m_name << ':';

    for (int ix = 0; ix < eNumSlots; ix++) {
        eptr = m_slots[ix].eeEdge.lock();
        if (eptr) {
            NodePtr next = eptr->getAdjacent(m_slots[ix].eeEnd).nsNode;
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

} // namespace rrsim

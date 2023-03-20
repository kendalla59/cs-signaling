// edge.cpp
//
// Author: Kendall Auel
//
// Implementation of the Edge class.

#include "edge.h"
#include "node.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace rrsim {

// This is the list of all edges which exist in the railroad track network.
EdgeMap g_edgeMap;

Edge::Edge()
{
    m_name = getUniqueEdgeName();
    m_weight = 1.0; // TODO

    Node* nodeA = new Node;
    Node* nodeB = new Node;

    EdgeEnd edgeA = { this, eEndA };
    nodeA->makeTerminator(edgeA);
    EdgeEnd edgeB = { this, eEndB };
    nodeB->makeTerminator(edgeB);

    m_ends[eEndA].nsNode = nodeA;
    m_ends[eEndA].nsSlot = eSlot1;

    m_ends[eEndB].nsNode = nodeB;
    m_ends[eEndB].nsSlot = eSlot1;

    m_signals[0] = nullptr; // TODO
    m_signals[1] = nullptr; // TODO

    g_edgeMap.insert(EdgePair(m_name, this));
}

Edge::~Edge()
{
    for (int ix = 0; ix < eNumEnds; ix++) {
        // TODO: remove edge from node
        if (m_signals[ix]) {
            // TODO: delete m_signals[ix];
            m_signals[ix] = nullptr;
        }
    }
}

void Edge::connectEdge(eEnd myEnd, Edge* other, eEnd toEnd)
{
    // If the other track is null, there is nothing more to do.
    if (!other) { return; }

    // Throw an exception if a track end is being connected to itself.
    if (this == other) {
        throw std::runtime_error("Cannot connect track segment to itself.");
    }

    NodeSlot cnctNode = getNode(myEnd);
    NodeSlot rmovNode = other->getNode(toEnd);
    NodeSlot replNode;

    // Throw an exception if the end of the other track is
    // not a terminator -- i.e., it must be unconnected.
    if (rmovNode.nsNode->getNodeType() != eTerminator) {
        throw std::runtime_error("Cannot connect if end of other is occupied");
    }

    EdgeEnd thisEdge = { this, myEnd };
    EdgeEnd thatEdge = { other, toEnd };
    EdgeEnd contEdge;

    // Connect to the other track as implied by this track's connection.
    switch (cnctNode.nsNode->getNodeType()) {
    case eTerminator:
        // This connection results in a continuation of this track to the other.
        cnctNode.nsNode->makeContinuation(thatEdge);

        // Replace the other edge's node slot entry.
        replNode = { cnctNode.nsNode, eSlot2 };
        other->m_ends[toEnd] = replNode;

        // Now we can delete the node that we just replaced.
        delete rmovNode.nsNode;
        break;

    case eContinuation:
        // Make sure the currently connected track is not also the track
        // we are trying using to form a junction.
        contEdge = cnctNode.nsNode->getNext(cnctNode.nsSlot);
        if (contEdge.eeEdge == other) {
            throw std::runtime_error("Attempt to connect same edge as junction");
        }

        // This connection results in a junction from this track to the
        // currently connected track (left) or to the new track (right).
        cnctNode.nsNode->makeJunction(thatEdge);

        // Replace the other edge's node slot entry.
        replNode = { cnctNode.nsNode, eSlot3 };
        other->m_ends[toEnd] = replNode;

        // Now we can delete the node that we just replaced.
        delete rmovNode.nsNode;
        break;

    // Throw exceptions if connection criteria are violated.
    case eJunction:
        // We cannot connect any more tracks to this end.
        throw std::runtime_error("Attempt to connect to a junction");

    default:
        throw std::runtime_error("Unexpected node type in connectEdge");
    }
}

NodeSlot Edge::getNode(eEnd getEnd)
{
    if (getEnd == eEndA) {
        return m_ends[eEndA];
    }
    else if (getEnd == eEndB) {
        return m_ends[eEndB];
    }
    throw std::runtime_error("Bad end value in getNode");
}

Node* Edge::getAdjacent(eEnd getEnd)
{
    if (getEnd == eEndA) {
        return m_ends[eEndB].nsNode;
    }
    else if (getEnd == eEndB) {
        return m_ends[eEndA].nsNode;
    }
    throw std::runtime_error("Bad end value in getAdjacent");
}

void Edge::show(eEnd showEnd)
{
    eJSwitch sw;
    std::string msg;
    if ((showEnd == eEndA) || (showEnd == eNumEnds)) {
        NodeSlot node = m_ends[eEndA];
        EdgeEnd edge;
        if (node.nsNode == nullptr) {
            throw std::runtime_error("Edge has null end node");
        }
        switch (node.nsNode->getNodeType()) {
        case eEmpty: // TODO: exception?
        case eTerminator:
            msg += "<term-> ||== ";
            break;
        case eContinuation:
            edge = node.nsNode->getNext(node.nsSlot);
            if (edge.eeEdge) { msg += edge.eeEdge->name() + " <==> "; }
            // TODO: else: exception?
            break;

        case eJunction:
            edge = node.nsNode->getNext(node.nsSlot);
            if (edge.eeEdge) { msg += edge.eeEdge->name(); }
            else { msg += "<block>"; }
            sw = node.nsNode->getSwitchPos();
            if (node.nsSlot == eSlot1) {
                if      (sw == eSwitchNone)  { msg += " XX"; }
                else if (sw == eSwitchLeft)  { msg += " //"; }
                else                         { msg += " \\\\"; }
                msg += "=> ";
            }
            else {
                msg += " <=";
                if      (sw == eSwitchNone)  { msg += "XX "; }
                else if (sw == eSwitchLeft)  { msg += "// "; }
                else                         { msg += "\\\\ "; }
            }
            break;
        }
    }

    msg += m_name;

    if ((showEnd == eEndB) || (showEnd == eNumEnds)) {
        NodeSlot node = m_ends[eEndB];
        EdgeEnd edge;
        if (node.nsNode == nullptr) {
            throw std::runtime_error("Edge has null end node");
        }
        switch (node.nsNode->getNodeType()) {
        case eEmpty: // TODO: exception?
        case eTerminator:
            msg += " ==|| <-term>";
            break;
        case eContinuation:
            edge = node.nsNode->getNext(node.nsSlot);
            if (edge.eeEdge) { msg += " <==> " + edge.eeEdge->name(); }
            // TODO: else: exception?
            break;

        case eJunction:
            sw = node.nsNode->getSwitchPos();
            if (node.nsSlot == eSlot1) {
                msg += " <=";
                if      (sw == eSwitchNone)  { msg += "XX "; }
                else if (sw == eSwitchLeft)  { msg += "// "; }
                else                         { msg += "\\\\ "; }
            }
            else {
                if      (sw == eSwitchNone)  { msg += " XX"; }
                else if (sw == eSwitchLeft)  { msg += " //"; }
                else                         { msg += " \\\\"; }
                msg += "=> ";
            }
            edge = node.nsNode->getNext(node.nsSlot);
            if (edge.eeEdge) { msg += edge.eeEdge->name(); }
            else { msg += "<block>"; }
            break;
        }
    }
    std::cout << msg << std::endl;
}

// Static method to determine the next unused name for an edge.
std::string Edge::getUniqueEdgeName()
{
    int ix = g_edgeMap.size() + 1;
    std::stringstream ns;
    ns << "tseg" << std::setw(3) << std::setfill('0') << ix;
    return ns.str();
}

} // namespace rrsim

// edge.cpp
//
// Author: Kendall Auel
//
// Implementation of the Edge class.

#include "edge.h"
#include "node.h"
#include "rrsignal.h"
#include "train.h"
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

    m_signals[0] = nullptr;
    m_signals[1] = nullptr;

    m_train = nullptr;

    g_edgeMap.insert(EdgePair(m_name, this));
}

Edge::Edge(const std::string& serialStr) {
    int slot;
    std::string name;
    std::string token;
    EdgeEnd edge = { this, eEndA };
    Node* nptr;
    size_t pos1 = 7;
    size_t pos2 = serialStr.find(',', pos1);
    name = serialStr.substr(pos1, pos2-pos1);
    m_name = name;
    std::cout << "Name: " << m_name;
    pos1 = pos2 + 1;
    pos2 = serialStr.find(',', pos1);
    token = serialStr.substr(pos1, pos2-pos1);
    m_weight = std::stod(token);
    std::cout << " weight: " << m_weight;

    // Node at the A side.
    pos1 = pos2 + 1;
    pos2 = serialStr.find(',', pos1);
    name = serialStr.substr(pos1, pos2-pos1);
    pos1 = pos2 + 1;
    pos2 = serialStr.find(',', pos1);
    token = serialStr.substr(pos1, pos2-pos1);
    slot = std::stoi(token);
    std::cout << " endA: " << name << "-" << slot;
    auto itA = g_nodeMap.find(name);
    if (itA == g_nodeMap.end()) { nptr = new Node(name); }
    else                        { nptr = itA->second; }
    edge.eeEnd = eEndA;
    nptr->setEdgeEnd(edge, (eSlot)slot);
    m_ends[eEndA].nsNode = nptr;
    m_ends[eEndA].nsSlot = (eSlot)slot;

    // Node at the B side.
    pos1 = pos2 + 1;
    pos2 = serialStr.find(',', pos1);
    name = serialStr.substr(pos1, pos2-pos1);
    pos1 = pos2 + 1;
    pos2 = serialStr.find(',', pos1);
    token = serialStr.substr(pos1, pos2-pos1);
    slot = std::stoi(token);
    std::cout << " endB: " << name << "-" << slot;
    auto itB = g_nodeMap.find(name);
    if (itB == g_nodeMap.end()) { nptr = new Node(name); }
    else                        { nptr = itB->second; }
    edge.eeEnd = eEndB;
    nptr->setEdgeEnd(edge, (eSlot)slot);
    m_ends[eEndB].nsNode = nptr;
    m_ends[eEndB].nsSlot = (eSlot)slot;

    // Signal lights.
    pos1 = pos2 + 1;
    pos2 = serialStr.find(',', pos1);
    token = serialStr.substr(pos1, pos2-pos1);
    if (token == "sigA:Y") {
        std::cout << " sigA";
        placeSignalLight(eEndA);
    }
    token = serialStr.substr(pos2 + 1);
    if (token == "sigB:Y") {
        std::cout << " sigB";
        placeSignalLight(eEndB);
    }
    std::cout << std::endl;

    m_train = nullptr;

    g_edgeMap.insert(EdgePair(m_name, this));
}

Edge::~Edge()
{
    for (int ix = 0; ix < eNumEnds; ix++) {
        if (m_signals[ix]) {
            delete m_signals[ix];
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

RRsignal* Edge::getSignal(eEnd myEnd)
{
    if ((myEnd != eEndA) && (myEnd != eEndB)) {
        throw std::runtime_error("Invalid enum passed to getSignal");
    }
    return m_signals[myEnd];
}

void Edge::placeSignalLight(eEnd myEnd)
{
    if ((myEnd != eEndA) && (myEnd != eEndB)) {
        throw std::runtime_error("Invalid enum passed to placeSignalLight");
    }
    if (m_signals[myEnd]) {
        throw std::runtime_error("Signal has already been placed here");
    }
    m_signals[myEnd] = new RRsignal(this, myEnd);
}

NodeSlot Edge::getNode(eEnd getEnd)
{
    if ((getEnd != eEndA) && (getEnd != eEndB)) {
        throw std::runtime_error("Invalid enum passed to getNode");
    }
    return m_ends[getEnd];
}

NodeSlot Edge::getAdjacent(eEnd getEnd)
{
    if ((getEnd != eEndA) && (getEnd != eEndB)) {
        throw std::runtime_error("Invalid enum passed to getAdjacent");
    }
    return m_ends[(getEnd == eEndA) ? eEndB : eEndA];
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
        if (m_signals[eEndA]) {
            msg += (m_signals[eEndA]->signalIsRed() ? "R " : "G ");
        }
        else {
            msg += "_ ";
        }
    }

    msg += m_name;

    if ((showEnd == eEndB) || (showEnd == eNumEnds)) {
        if (m_signals[eEndB]) {
            msg += (m_signals[eEndB]->signalIsRed() ? " R" : " G");
        }
        else {
            msg += " _";
        }
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
    if (m_train) {
        if (m_train->getPosition().eeEnd == eEndA) {
            msg += "  /[o==o]-[o==o]";
        }
        else {
            msg += "  [o==o]-[o==o]\\";
        }
    }
    std::cout << msg << std::endl;
}

std::string Edge::serialize()
{
    std::stringstream ss;
    ss << "track: " << m_name << ',' << m_weight << ','
       << m_ends[0].nsNode->name() << ',' << m_ends[0].nsSlot << ','
       << m_ends[1].nsNode->name() << ',' << m_ends[1].nsSlot << ','
       << "sigA:" << (m_signals[0] ? "Y" : "N") << ','
       << "sigB:" << (m_signals[1] ? "Y" : "N") << std::endl;
    return ss.str();
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

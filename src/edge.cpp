// edge.cpp
//
// Author: Kendall Auel
//
// Implementation of the Edge class.

#include "edge.h"
#include "node.h"
#include "rrsignal.h"
#include "train.h"
#include "system.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace rrsim {

Edge::Edge(const std::string& name) : m_name(name), m_weight(1.0)
{
    // TODO: Weighted edges

    // Initialize node slots as invalid.
    m_ends[eEndA].nsSlot = eNumSlots;
    m_ends[eEndB].nsSlot = eNumSlots;

    m_signals[0] = nullptr;
    m_signals[1] = nullptr;
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
    EdgePtr eptr = shared_from_this();
    m_signals[myEnd] = new RRsignal(eptr, myEnd);
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

void Edge::assignNodeSlot(NodeSlot node, eEnd nodeEnd)
{
    if ((nodeEnd != eEndA) && (nodeEnd != eEndB)) {
        throw std::runtime_error("Invalid enum passed to assignNodeSlot");
    }
    m_ends[nodeEnd] = node;
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

void Edge::deserialize(const std::string& serialStr)
{
    int slot;
    std::string name;
    std::string token;
    NodePtr nptr;

    EdgePtr eptr = shared_from_this();
    EdgeEnd edge = { eptr, eEndA };
    size_t pos1 = 7;
    size_t pos2 = serialStr.find(',', pos1);
    name = serialStr.substr(pos1, pos2-pos1);
    if (name != m_name) {
        throw std::runtime_error("deserialize " + m_name + " != " + name);
    }
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
    nptr = sys().getNode(name);
    if (!nptr) { nptr = sys().createNode(name); }
    edge.eeEnd = eEndA;
    nptr->setEdgeEnd(edge, (eSlot)slot);
    if (slot == eSlot3) { nptr->setSwitchPos(eSwitchLeft); }
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
    nptr = sys().getNode(name);
    if (!nptr) { nptr = sys().createNode(name); }
    edge.eeEnd = eEndB;
    nptr->setEdgeEnd(edge, (eSlot)slot);
    if (slot == eSlot3) { nptr->setSwitchPos(eSwitchLeft); }
    m_ends[eEndB].nsNode = nptr;
    m_ends[eEndB].nsSlot = (eSlot)slot;

    // Signal lights.
    m_signals[eEndA] = nullptr;
    m_signals[eEndB] = nullptr;
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
}

} // namespace rrsim

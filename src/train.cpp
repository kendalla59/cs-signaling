// train.cpp
//
// Author: Kendall Auel
//
// Implementation of the Train class.

#include "train.h"
#include "edge.h"
#include "node.h"
#include "rrsignal.h"
#include <iostream>

namespace rrsim {


Train::Train()
{
    m_name = "train1";
    m_edge.eeEdge = nullptr;
    m_edge.eeEnd = eNumEnds;
    m_destination = nullptr;
}

Train::~Train()
{
}

void Train::placeOnTrack(Edge* start, Edge* end)
{
    if (start->getTrain()) {
        throw std::runtime_error("A train is already on segment: " + start->name());
    }
    start->setTrain(this);

    m_edge.eeEdge = start;
    m_edge.eeEnd = eEndA;
    m_destination = end;
}

bool Train::stepSimulation()
{
    // Nothing to do if we are not on a track segment.
    if (m_edge.eeEdge == nullptr) { return false; }

    // Nothing to do if we are at the destination.
    if (m_edge.eeEdge == m_destination) { return false; }

    EdgeEnd next;
    eJSwitch jsw;

    // Do not advance the train if the signal is red.
    bool advance = true;
    RRsignal * light = m_edge.eeEdge->getSignal(m_edge.eeEnd);
    if (light && light->signalIsRed()) { advance = false; }

    NodeSlot node = m_edge.eeEdge->getNode(m_edge.eeEnd);
    switch (node.nsNode->getNodeType()) {
    default:
    case eEmpty: // TODO: throw exception?
    case eTerminator: return false;

    case eContinuation:
        if (advance) {
            next = node.nsNode->getEdgeEnd(
                    (node.nsSlot == eSlot1) ? eSlot2 : eSlot1);
            if (next.eeEdge) {
                m_edge.eeEdge->setTrain(nullptr);
                if (next.eeEdge->getTrain()) {
                    m_edge.eeEdge = nullptr;
                    throw std::runtime_error("Train collision detected!");
                }
                m_edge.eeEdge = next.eeEdge;
                m_edge.eeEnd = (next.eeEnd == eEndA) ? eEndB : eEndA;
                m_edge.eeEdge->setTrain(this);
            }
        }
        break;

    case eJunction:
        jsw = node.nsNode->getSwitchPos();
        if (jsw == eSwitchNone) {
            node.nsNode->setSwitchPos(eSwitchLeft); // TODO: use route.
        }
        else {
            if (node.nsSlot == eSlot1) {
                if (advance) {
                    next = node.nsNode->getEdgeEnd(
                            (jsw == eSwitchLeft) ? eSlot2 : eSlot3);
                    if (next.eeEdge) {
                        m_edge.eeEdge->setTrain(nullptr);
                        if (next.eeEdge->getTrain()) {
                            m_edge.eeEdge = nullptr;
                            throw std::runtime_error("Train collision detected!");
                        }
                        m_edge.eeEdge = next.eeEdge;
                        m_edge.eeEnd = (next.eeEnd == eEndA) ? eEndB : eEndA;
                        m_edge.eeEdge->setTrain(this);

                        // Toggle the junction switch. TODO: implement BFS routing.
                        node.nsNode->setSwitchPos(
                                (jsw == eSwitchLeft) ? eSwitchRight : eSwitchLeft);
                    }
                }
            }
            else if (node.nsSlot == eSlot2) {
                if (jsw == eSwitchRight) {
                    node.nsNode->setSwitchPos(eSwitchLeft);
                }
                else if (advance) {
                    next = node.nsNode->getEdgeEnd(eSlot1);
                    if (next.eeEdge) {
                        m_edge.eeEdge->setTrain(nullptr);
                        if (next.eeEdge->getTrain()) {
                            m_edge.eeEdge = nullptr;
                            throw std::runtime_error("Train collision detected!");
                        }
                        m_edge.eeEdge = next.eeEdge;
                        m_edge.eeEnd = (next.eeEnd == eEndA) ? eEndB : eEndA;
                        m_edge.eeEdge->setTrain(this);
                    }
                }
            }
            else if (node.nsSlot == eSlot3) {
                if (jsw == eSwitchLeft) {
                    node.nsNode->setSwitchPos(eSwitchRight);
                }
                else if (advance) {
                    next = node.nsNode->getEdgeEnd(eSlot1);
                    if (next.eeEdge) {
                        m_edge.eeEdge->setTrain(nullptr);
                        if (next.eeEdge->getTrain()) {
                            m_edge.eeEdge = nullptr;
                            throw std::runtime_error("Train collision detected!");
                        }
                        m_edge.eeEdge = next.eeEdge;
                        m_edge.eeEnd = (next.eeEnd == eEndA) ? eEndB : eEndA;
                        m_edge.eeEdge->setTrain(this);
                    }
                }
            }
        }
        break;
    }
    return true;
}

void Train::show()
{
    std::cout << "Train: " << m_name << std::endl;
    if (m_edge.eeEdge) {
        std::cout << "  Location: track segment \""
                  << m_edge.eeEdge->name() << "\"" << std::endl;
        std::cout << "  Direction: toward segment end "
                  << ((m_edge.eeEnd == eEndA) ? "A" : "B") << std::endl;
    }
}

} // namespace rrsim

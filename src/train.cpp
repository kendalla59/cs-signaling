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
}

Train::~Train()
{
}

void Train::placeOnTrack(const std::string& trackName, eEnd direction)
{
    if ((direction != eEndA) && (direction != eEndB)) {
        throw std::runtime_error("Failed placeOnTrack, invalid direction");
    }
    auto iter = g_edgeMap.find(trackName);
    if (iter == g_edgeMap.end()) {
        throw std::runtime_error("No such track segment: " + trackName);
    }
    if (iter->second->getTrain()) {
        throw std::runtime_error("A train is already on segment: " + trackName);
    }
    iter->second->setTrain(this);

    m_edge.eeEdge = iter->second;
    m_edge.eeEnd = direction;
}

bool Train::stepSimulation()
{
    // Nothing to do if we are not on a track segment.
    if (m_edge.eeEdge == nullptr) { return false; }

    EdgeEnd next;
    eJSwitch jsw;

    // Do not advance the train if the signal is red.
    RRsignal * light = m_edge.eeEdge->getSignal(m_edge.eeEnd);
    if (light && light->signalIsRed()) { return true; }

    NodeSlot node = m_edge.eeEdge->getNode(m_edge.eeEnd);
    switch (node.nsNode->getNodeType()) {
    default:
    case eEmpty: // TODO: throw exception?
    case eTerminator: return false;

    case eContinuation:
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
        break;

    case eJunction:
        jsw = node.nsNode->getSwitchPos();
        if (jsw != eSwitchNone) {
            if (node.nsSlot == eSlot1) {
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
                }
            }
            else if ((node.nsSlot == eSlot2) && (jsw == eSwitchLeft)) {
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
            else if ((node.nsSlot == eSlot3) && (jsw == eSwitchRight)) {
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
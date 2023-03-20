// rrsignal.cpp
//
// Author: Kendall Auel
//
// Implementation of the RRsignal class.

#include "rrsignal.h"
#include "edge.h"
#include "node.h"
#include "train.h"

namespace rrsim {

RRsignal::RRsignal(Edge* trackSeg, eEnd trackEnd) : m_isRed(true)
{
    m_edge.eeEdge = trackSeg;
    m_edge.eeEnd = trackEnd;
}

RRsignal::~RRsignal()
{
}

void RRsignal::updateSignal()
{
    m_isRed = true; // Assume the signal is red.

    // Nothing to do if we aren't placed anywhere.
    if (m_edge.eeEdge == nullptr) { return; }

    NodeSlot node = m_edge.eeEdge->getNode(m_edge.eeEnd);
    EdgeEnd edge = node.nsNode->getNext(node.nsSlot);

    // There is no next track segment, nothing more to do.
    if (edge.eeEdge == nullptr) { return; }

    // The next segment has a train, nothing more to do.
    if (edge.eeEdge->getTrain()) { return; }
    node = m_edge.eeEdge->getNode((m_edge.eeEnd == eEndA) ? eEndB : eEndA);

    // Now assume we have a green light, unless we find an oncoming train.
    m_isRed = false;

    while (node.nsNode->getNodeType() != eJunction) {
        edge = node.nsNode->getNext(node.nsSlot);
        if (edge.eeEdge == nullptr) { return; }
        Train* train = edge.eeEdge->getTrain();
        if (train && (train->getPosition().eeEnd == edge.eeEnd)) {
            // The train is headed toward us.
            m_isRed = true;
            return;
        }
        node = m_edge.eeEdge->getNode((m_edge.eeEnd == eEndA) ? eEndB : eEndA);
    }
}

} // namespace rrsim
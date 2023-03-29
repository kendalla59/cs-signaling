// rrsignal.cpp
//
// Author: Kendall Auel
//
// Implementation of the RRsignal class.

#include "rrsignal.h"
#include "edge.h"
#include "node.h"
#include "train.h"
#include <set>

namespace rrsim {

RRsignal::RRsignal(EdgePtr trackSeg, eEnd trackEnd) : m_isRed(true)
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
    EdgePtr eptr = m_edge.eeEdge.lock();

    // Nothing to do if we aren't placed anywhere.
    if (!eptr) { return; }

    NodeSlot node = eptr->getNode(m_edge.eeEnd);
    EdgeEnd edge = node.nsNode->getNext(node.nsSlot);
    eptr = edge.eeEdge.lock();

    // There is no next track segment, nothing more to do.
    if (!eptr) { return; }

    // The next segment has a train, nothing more to do.
    if (eptr->getTrain()) { return; }
    node = eptr->getNode((edge.eeEnd == eEndA) ? eEndB : eEndA);

    // Avoid infinite loops.
    std::set<std::string> visitedEdges;
    visitedEdges.insert(eptr->name());

    // Now assume we have a green light, unless we find an oncoming train.
    m_isRed = false;

    while (node.nsNode->getNodeType() != eJunction) {
        edge = node.nsNode->getNext(node.nsSlot);
        eptr = edge.eeEdge.lock();
        if (!eptr) { return; }
        if (visitedEdges.find(eptr->name()) != visitedEdges.end()) {
            // The track formed a loop before a junction was seen.
            return;
        }
        visitedEdges.insert(eptr->name());

        TrainPtr train = eptr->getTrain();
        if (train && (train->getPosition().eeEnd == edge.eeEnd)) {
            // The train is headed toward us.
            m_isRed = true;
            return;
        }
        node = eptr->getNode((edge.eeEnd == eEndA) ? eEndB : eEndA);
    }
}

} // namespace rrsim

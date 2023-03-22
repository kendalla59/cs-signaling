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
    node = edge.eeEdge->getNode((edge.eeEnd == eEndA) ? eEndB : eEndA);

    // Avoid infinite loops.
    std::set<Edge*> visitedEdges;
    visitedEdges.insert(edge.eeEdge);

    // Now assume we have a green light, unless we find an oncoming train.
    m_isRed = false;

    while (node.nsNode->getNodeType() != eJunction) {
        edge = node.nsNode->getNext(node.nsSlot);
        if (edge.eeEdge == nullptr) { return; }
        if (visitedEdges.find(edge.eeEdge) != visitedEdges.end()) { return; }
        visitedEdges.insert(edge.eeEdge);

        Train* train = edge.eeEdge->getTrain();
        if (train && (train->getPosition().eeEnd == edge.eeEnd)) {
            // The train is headed toward us.
            m_isRed = true;
            return;
        }
        node = edge.eeEdge->getNode((edge.eeEnd == eEndA) ? eEndB : eEndA);
    }
}

void RRsignal::updateAllSignals()
{
    for (auto iter: g_edgeMap) {
        if (iter.second) {
            for (int ix = 0; ix < eNumEnds; ix++) {
                RRsignal* sig = iter.second->getSignal((eEnd)ix);
                if (sig) { sig->updateSignal(); }
            }
        }
    }
}

} // namespace rrsim

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
#include <queue>
#include <set>

namespace rrsim {


Train::Train(const std::string& name) : m_name(name)
{
    // Initialize edge end to an invalid value.
    m_edge.eeEnd = eNumEnds;
}

Train::~Train()
{
}

void Train::placeOnTrack(EdgePtr start, EdgePtr end)
{
    EdgePtr eptr = m_edge.eeEdge.lock();
    if (eptr) {
        // Remove the train from its current track segment.
        eptr->setTrain(nullptr);
        m_edge.eeEdge.reset();
        m_destination.reset();
    }
    while (!m_route.empty()) { m_route.pop(); }

    // Nothing else to do if we aren't going anywhere.
    if (!start || !end) { return; }

    if (start->getTrain()) {
        throw std::runtime_error(
                "A train is already on segment: " + start->name());
    }
    start->setTrain(shared_from_this());
    m_edge.eeEdge = start;
    m_edge.eeEnd = eEndB; // getOptimalRoute determines the final value.
    m_destination = end;

    getOptimalRoute();
}

bool Train::stepSimulation()
{
    EdgePtr eptr = m_edge.eeEdge.lock();

    // Nothing to do if we are not on a track segment.
    if (!eptr) { return false; }

    // Nothing to do if we are at the destination.
    if (eptr == m_destination.lock()) { return false; }

    EdgeEnd next;
    EdgePtr nexp;
    eJSwitch jsw;

    // Do not advance the train if the signal is red.
    bool advance = true;
    RRsignal * light = eptr->getSignal(m_edge.eeEnd);
    if (light && light->signalIsRed()) { advance = false; }

    NodeSlot node = eptr->getNode(m_edge.eeEnd);
    switch (node.nsNode->getNodeType()) {
    default:
    case eEmpty: // TODO: throw exception?
    case eTerminator: return false;

    case eContinuation:
        if (advance) {
            next = node.nsNode->getEdgeEnd(
                    (node.nsSlot == eSlot1) ? eSlot2 : eSlot1);
            nexp = next.eeEdge.lock();
            if (nexp) {
                eptr->setTrain(nullptr);
                if (nexp->getTrain()) {
                    m_edge.eeEdge.reset();
                    throw std::runtime_error("Train collision detected!");
                }
                m_edge.eeEdge = next.eeEdge;
                m_edge.eeEnd = (next.eeEnd == eEndA) ? eEndB : eEndA;
                nexp->setTrain(shared_from_this());
            }
        }
        break;

    case eJunction:
        jsw = node.nsNode->getSwitchPos();
        if (node.nsSlot == eSlot1) {
            if (m_route.empty()) { std::cout << "No route for " << eptr->name() << std::endl; }
            else { std::cout << "Route wants " << ((m_route.top() == eSwitchLeft) ? "left" : "right")
                             << ", switch is " << ((jsw == eSwitchLeft) ? "left" : "right") << std::endl; }
            if (!m_route.empty() && (m_route.top() != jsw)) {
                node.nsNode->setSwitchPos(m_route.top());
                std::cout << "Switch " << eptr->name() << "->"
                          << node.nsNode->name() << " set to "
                          << ((jsw == eSwitchRight) ? "right" : "left")
                          << std::endl;
            }
            else if (advance) {
                next = node.nsNode->getEdgeEnd(
                        (jsw == eSwitchLeft) ? eSlot2 : eSlot3);
                nexp = next.eeEdge.lock();
                if (nexp) {
                    eptr->setTrain(nullptr);
                    if (nexp->getTrain()) {
                        m_edge.eeEdge.reset();
                        throw std::runtime_error("Train collision detected!");
                    }
                    m_edge.eeEdge = next.eeEdge;
                    m_edge.eeEnd = (next.eeEnd == eEndA) ? eEndB : eEndA;
                    nexp->setTrain(shared_from_this());
                    if (!m_route.empty()) { m_route.pop(); }
                }
            }
        }
        else if (node.nsSlot == eSlot2) {
            next = node.nsNode->getEdgeEnd(eSlot1);
            nexp = next.eeEdge.lock();
            if (jsw != eSwitchLeft) {
                // Set the junction switch if no train is waiting.
                if (nexp && !nexp->getTrain()) {
                    node.nsNode->setSwitchPos(eSwitchLeft);
                    std::cout << "Switch " << eptr->name() << "->"
                              << node.nsNode->name() << " set to left"
                              << std::endl;
                }
            }
            else if (advance) {
                if (nexp) {
                    eptr->setTrain(nullptr);
                    if (nexp->getTrain()) {
                        m_edge.eeEdge.reset();
                        throw std::runtime_error("Train collision detected!");
                    }
                    m_edge.eeEdge = next.eeEdge;
                    m_edge.eeEnd = (next.eeEnd == eEndA) ? eEndB : eEndA;
                    nexp->setTrain(shared_from_this());
                }
            }
        }
        else if (node.nsSlot == eSlot3) {
            next = node.nsNode->getEdgeEnd(eSlot1);
            nexp = next.eeEdge.lock();
            if (jsw != eSwitchRight) {
                // Set the junction switch if no other train is waiting.
                if (nexp && !nexp->getTrain()) {
                    nexp = node.nsNode->getEdgeEnd(eSlot2).eeEdge.lock();
                    if (nexp && !nexp->getTrain()) {
                        node.nsNode->setSwitchPos(eSwitchRight);
                        std::cout << "Switch " << eptr->name() << "->"
                                  << node.nsNode->name() << " set to left"
                                  << std::endl;
                    }
                }
            }
            else if (advance) {
                if (nexp) {
                    eptr->setTrain(nullptr);
                    if (nexp->getTrain()) {
                        m_edge.eeEdge.reset();
                        throw std::runtime_error("Train collision detected!");
                    }
                    m_edge.eeEdge = next.eeEdge;
                    m_edge.eeEnd = (next.eeEnd == eEndA) ? eEndB : eEndA;
                    nexp->setTrain(shared_from_this());
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
    EdgePtr eptr = m_edge.eeEdge.lock();
    if (eptr) {
        std::cout << "  Location: track segment \""
                  << eptr->name() << "\"" << std::endl;
        std::cout << "  Direction: toward segment end "
                  << ((m_edge.eeEnd == eEndA) ? "A" : "B") << std::endl;
    }
}

// -----------------------------------------------------------------------------
// Optimal Route Generation (Breadth-first search of track network)
// -----------------------------------------------------------------------------

struct QNode
{
    QNode*      parent;
    NodeSlot    node;
    QNode(QNode* p, NodeSlot n) : parent(p), node(n) {}
};

static std::string getNodeSlotID(NodeSlot node)
{
    if (node.nsNode) {
        return node.nsNode->name() + std::to_string(node.nsSlot);
    }
    return "nodeNULL";
}

// NOTE: The only control the train has on its route is the switch
//       position at each junction. The end result of the search is
//       then merely an ordered list of junction switch positions.
//
// The optimal route uses a breadth-first search of the track network
// nodes, looking for any node accessible to the end edge.
//
void Train::getOptimalRoute()
{
    EdgePtr start = m_edge.eeEdge.lock();
    EdgePtr end = m_destination.lock();
    if (!start || !end) {
        // Missing end(s), no route is possible.
        while (!m_route.empty()) { m_route.pop(); }
        return;
    }
    std::queue<QNode*> searchQueue;
    std::queue<QNode*> poppedQueue;
    std::set<std::string> visitedSet;
    NodeSlot node;
    EdgeEnd edge;
    EdgePtr eptr;

    // Initialize the BFS search queue with the end nodes of the start edge.
    searchQueue.push(new QNode(nullptr, start->getNode(eEndA)));
    visitedSet.insert(getNodeSlotID(searchQueue.back()->node));
    searchQueue.push(new QNode(nullptr, start->getNode(eEndB)));
    visitedSet.insert(getNodeSlotID(searchQueue.back()->node));

    // Now cycle through the front of the queue looking for the end edge.
    // If not found, push unvisited adjacent nodes onto the search queue.
    QNode* found = nullptr;
    while (!found) {
        if (searchQueue.empty()) {
            // ERROR: clear out the popped items and throw an exception.
            while (!poppedQueue.empty()) {
                delete poppedQueue.front();
                poppedQueue.pop();
            }
            throw std::runtime_error("getOptimalRoute failed to reach the end");
        }
        QNode* front = searchQueue.front();
        searchQueue.pop();
        poppedQueue.push(front);

        switch (front->node.nsSlot) {
        case eSlot1:
            // Both slot 2 and 3 are adjacent (if not null)
            edge = front->node.nsNode->getEdgeEnd(eSlot2);
            eptr = edge.eeEdge.lock();
            if (eptr) {
                if (eptr == end) { found = front; }
                else {
                    node = eptr->getAdjacent(edge.eeEnd);
                    std::string ID = getNodeSlotID(node);
                    if (visitedSet.find(ID) == visitedSet.end()) {
                        visitedSet.insert(ID);
                        searchQueue.push(new QNode(front, node));
                    }
                }
            }
            if (found) break;

            edge = front->node.nsNode->getEdgeEnd(eSlot3);
            eptr = edge.eeEdge.lock();
            if (eptr) {
                if (eptr == end) { found = front; }
                else {
                    node = eptr->getAdjacent(edge.eeEnd);
                    std::string ID = getNodeSlotID(node);
                    if (visitedSet.find(ID) == visitedSet.end()) {
                        visitedSet.insert(ID);
                        searchQueue.push(new QNode(front, node));
                    }
                }
            }
            break;

        case eSlot2:
        case eSlot3:
            // Only slot 1 is adjacent (either continuation, or junction fork).
            edge = front->node.nsNode->getEdgeEnd(eSlot1);
            eptr = edge.eeEdge.lock();
            if (eptr) {
                if (eptr == end) { found = front; }
                else {
                    node = eptr->getAdjacent(edge.eeEnd);
                    std::string ID = getNodeSlotID(node);
                    if (visitedSet.find(ID) == visitedSet.end()) {
                        visitedSet.insert(ID);
                        searchQueue.push(new QNode(front, node));
                    }
                }
            }
            break;

        default:
            // This indicates a bug, don't bother with garbage collection.
            throw std::runtime_error("Unexpected slot number getOptimalRoute");
        }
    }
    // We have a node that is connected to the end edge. Built the route
    // by pushing junction switch positions onto the route stack.

    // First, clear out anything on the route.
    while (!m_route.empty()) { m_route.pop(); }

    EdgePtr from = end;
    std::cout << "Route ends at edge: " << end->name() << std::endl;
    while (found) {
        node = found->node;
        if ((node.nsNode->getNodeType() == eJunction) &&
            (node.nsSlot == eSlot1)) {
            eptr = found->node.nsNode->getEdgeEnd(eSlot2).eeEdge.lock();
            if (eptr == from) {
                std::cout << "         -- via junction switch LEFT" << std::endl;
                m_route.push(eSwitchLeft);
            }
            else {
                std::cout << "         -- via junction switch RIGHT" << std::endl;
                m_route.push(eSwitchRight);
            }
        }
        edge = node.nsNode->getEdgeEnd(node.nsSlot);
        from = edge.eeEdge.lock();
        found = found->parent;
        if (found) { std::cout << "         from edge: " << from->name() << std::endl; }
        else       { std::cout << "Starting from edge: " << from->name() << std::endl; }
    }
    // Set the initial position and direction.
    m_edge = edge;

    // Free up the QNode elements.
    while (!poppedQueue.empty()) {
        delete poppedQueue.front();
        poppedQueue.pop();
    }
    while (!searchQueue.empty()) {
        delete searchQueue.front();
        searchQueue.pop();
    }
}

} // namespace rrsim

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
    if (m_edge.eeEdge) {
        // Remove the train from its current track segment.
        m_edge.eeEdge->setTrain(nullptr);
        m_edge.eeEdge = nullptr;
    }
    if (start->getTrain()) {
        throw std::runtime_error("A train is already on segment: " + start->name());
    }
    start->setTrain(this);
    m_edge.eeEdge = start;
    m_edge.eeEnd = eEndB; // getOptimalRoute determines the final value.
    m_destination = end;

    getOptimalRoute();
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
        if (node.nsSlot == eSlot1) {
            if (!m_route.empty() && (m_route.top() != jsw)) {
                node.nsNode->setSwitchPos(m_route.top());
            }
            else if (advance) {
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
                    m_route.pop();
                }
            }
        }
        else if (node.nsSlot == eSlot2) {
            if (jsw != eSwitchLeft) {
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
            if (jsw != eSwitchRight) {
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
    Edge* start = m_edge.eeEdge;
    Edge* end = m_destination;

    std::queue<QNode*> searchQueue;
    std::queue<QNode*> poppedQueue;
    std::set<std::string> visitedSet;
    NodeSlot node;
    EdgeEnd edge;

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
            if (edge.eeEdge) {
                if (edge.eeEdge == end) { found = front; }
                else {
                    node = edge.eeEdge->getAdjacent(edge.eeEnd);
                    std::string ID = getNodeSlotID(node);
                    if (visitedSet.find(ID) == visitedSet.end()) {
                        visitedSet.insert(ID);
                        searchQueue.push(new QNode(front, node));
                    }
                }
            }
            if (found) break;

            edge = front->node.nsNode->getEdgeEnd(eSlot3);
            if (edge.eeEdge) {
                if (edge.eeEdge == end) { found = front; }
                else {
                    node = edge.eeEdge->getAdjacent(edge.eeEnd);
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
            if (edge.eeEdge) {
                if (edge.eeEdge == end) { found = front; }
                else {
                    node = edge.eeEdge->getAdjacent(edge.eeEnd);
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

    Edge* from = end;
    std::cout << "Route ends at edge: " << end->name() << std::endl;
    while (found) {
        node = found->node;
        if ((node.nsNode->getNodeType() == eJunction) &&
            (node.nsSlot == eSlot1)) {
            if (found->node.nsNode->getEdgeEnd(eSlot2).eeEdge == from) {
                std::cout << "         -- via junction switch LEFT" << std::endl;
                m_route.push(eSwitchLeft);
            }
            else {
                std::cout << "         -- via junction switch RIGHT" << std::endl;
                m_route.push(eSwitchRight);
            }
        }
        edge = node.nsNode->getEdgeEnd(node.nsSlot);
        from = edge.eeEdge;
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

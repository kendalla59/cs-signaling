// system.cpp
//
// Author: Kendall Auel
//
// Implementation of the System singleton class.

#include "system.h"
#include "edge.h"
#include "node.h"
#include "train.h"
#include "rrsignal.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace rrsim {

const std::string System::emptyStr;

System& System::instance()
{
    static System S;
    return S;
}

System::System()
{
}

System::~System()
{
}

void System::resetTrackNetwork()
{
    // Clear out the existing network.
    std::cout << std::endl << "Removing " << m_edgeMap.size() << " edges...";
    m_edgeMap.clear();
    std::cout << std::endl << "Removing " << m_nodeMap.size() << " nodes...";
    m_nodeMap.clear();
    std::cout << std::endl << "Removing " << m_trainMap.size() << " trains...";
    m_trainMap.clear();
    std::cout << std::endl;
}

EdgePtr System::createEdge(const std::string& name)
{
    std::string edgeName = name;
    if (name.empty()) {
        edgeName = getUniqueEdgeName();
    }
    else {
        // Verify the given edge name is unique.
        auto iter = m_edgeMap.find(name);
        if (iter != m_edgeMap.end()) {
            throw std::runtime_error("createEdge already exists: " + name);
        }
    }
    EdgePtr rval = std::make_shared<Edge>(edgeName);
    m_edgeMap.insert(EdgeItem(rval->name(), rval));

    // Place terminator nodes at each end of the edge.
    NodePtr nptrA = createNode();
    nptrA->makeTerminator(EdgeEnd(rval, eEndA));
    rval->assignNodeSlot(NodeSlot(nptrA, eSlot1), eEndA);

    NodePtr nptrB = createNode();
    nptrB->makeTerminator(EdgeEnd(rval, eEndB));
    rval->assignNodeSlot(NodeSlot(nptrB, eSlot1), eEndB);

    return rval;
}

EdgePtr System::getEdge(const std::string& name)
{
    auto iter = m_edgeMap.find(name);
    if (iter == m_edgeMap.end()) { return nullptr; }
    return iter->second;
}

NodePtr System::createNode(const std::string& name)
{
    std::string nodeName = name;
    if (name.empty()) {
        nodeName = getUniqueNodeName();
    }
    else {
        // Verify the given node name is unique.
        auto iter = m_nodeMap.find(name);
        if (iter != m_nodeMap.end()) {
            throw std::runtime_error("createNode already exists: " + name);
        }
    }
    NodePtr rval = std::make_shared<Node>(nodeName);
    m_nodeMap.insert(NodeItem(rval->name(), rval));
    return rval;
}

NodePtr System::getNode(const std::string& name)
{
    auto iter = m_nodeMap.find(name);
    if (iter == m_nodeMap.end()) { return nullptr; }
    return iter->second;
}

TrainPtr System::createTrain(const std::string& name)
{
    auto iter = m_trainMap.find(name);
    if (iter != m_trainMap.end()) {
        throw std::runtime_error("createTrain already exists: " + name);
    }
    std::string trainName = name;
    if (name.empty()) {
        trainName = getUniqueTrainName();
    }
    TrainPtr rval = std::make_shared<Train>(trainName);
    m_trainMap.insert(TrainItem(rval->name(), rval));
    return rval;
}

TrainPtr System::getTrain(const std::string& name)
{
    auto iter = m_trainMap.find(name);
    if (iter == m_trainMap.end()) { return nullptr; }
    return iter->second;
}

int System::connectSegments(const EdgeEnd& s1, const EdgeEnd& s2)
{
    // If either track is null, there is nothing more to do.
    EdgePtr ept1 = s1.eeEdge.lock();
    EdgePtr ept2 = s2.eeEdge.lock();
    if (!ept1 || !ept2) { return EINVAL; }

    NodeSlot cnctNode = ept1->getNode(s1.eeEnd);
    NodeSlot rmovNode = ept2->getNode(s2.eeEnd);
    NodeSlot replNode(nullptr, eNumSlots);

    // Return error if the end of the other track is
    // not a terminator -- i.e., it must be unconnected.
    if (rmovNode.nsNode->getNodeType() != eTerminator) {
        std::cout << "ERROR: Cannot connect if end of other is occupied"
                  << std::endl;
        return EBUSY;
    }

    // Connect to the other track as implied by this track's connection.
    switch (cnctNode.nsNode->getNodeType()) {
    case eTerminator:
        // This connection results in a continuation of this track to the other.
        cnctNode.nsNode->makeContinuation(s2);

        // Replace the other edge's node slot entry.
        replNode = { cnctNode.nsNode, eSlot2 };
        ept2->assignNodeSlot(replNode, s2.eeEnd);
        break;

    case eContinuation:
        // This connection results in a junction from this track to the
        // currently connected track (left) or to the new track (right).
        cnctNode.nsNode->makeJunction(s2, cnctNode.nsSlot);

        // Replace the other edge's node slot entry.
        replNode = { cnctNode.nsNode, eSlot3 };
        ept2->assignNodeSlot(replNode, s2.eeEnd);
        break;

    // Throw exceptions if connection criteria are violated.
    case eJunction:
        // We cannot connect any more tracks to this end.
        throw std::runtime_error("Attempt to connect to a junction");

    default:
        throw std::runtime_error("Unexpected node type in connectEdge");
    }
    return 0;
}

int System::stepSimulation()
{
    try {
        for (auto iter: m_trainMap) {
            TrainPtr tptr = iter.second;
            bool chk = tptr->stepSimulation();
            updateAllSignals();
            tptr->show();
            if (!chk) {
                std::cout << ">>> The Simulation Is Complete : "
                          << tptr->name() << " <<<" << std::endl;
            }
        }
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }
    return 0;
}

int System::showEdges()
{
    try {
        for (auto iter: m_edgeMap) {
            iter.second->show();
        }
        std::cout << std::endl
                  << "TOTAL: " << m_edgeMap.size() << " track segments"
                  << std::endl;
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }
    return 0;
}

int System::showNodes()
{
    try {
        for (auto iter: m_nodeMap) {
            iter.second->show();
        }
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }
    return 0;
}

void System::addSignalsToAllJunctions()
{
    for (auto iter: m_edgeMap) {
        EdgePtr eptr = iter.second;
        if (eptr) {
            for (int ix = 0; ix < eNumEnds; ix++) {
                eEnd ex = (eEnd)ix;
                if (!eptr->getSignal(ex)) {
                    NodeSlot node = eptr->getNode(ex);
                    if (node.nsNode &&
                            (node.nsNode->getNodeType() == eJunction)) {

                        eptr->placeSignalLight(ex);
                        std::cout << "Added signal to " << eptr->name()
                                  << ((ex == eEndA) ? "[A]" : "[B]")
                                  << std::endl;
                    }
                }
            }
        }
    }
}

void System::updateAllSignals()
{
    for (auto iter: m_edgeMap) {
        EdgePtr eptr = iter.second;
        if (eptr) {
            for (int ix = 0; ix < eNumEnds; ix++) {
                RRsignal* sig = eptr->getSignal((eEnd)ix);
                if (sig) { sig->updateSignal(); }
            }
        }
    }
}

NodeVec System::getAllJunctions()
{
    NodeVec rval;
    for (auto iter: m_nodeMap) {
        NodePtr nptr = iter.second;
        if (nptr && (nptr->getNodeType() == eJunction)) {
            rval.push_back(nptr);
        }
    }
    return rval;
}

int System::serialize(std::ofstream& ofstr)
{
    try {
        for (auto iter: m_edgeMap) {
            EdgePtr edge = iter.second;
            if (edge) {
                ofstr << edge->serialize();
            }
        }
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EINVAL;
    }
    return 0;
}

int System::deserialize(std::ifstream& ifstr)
{
    // Clear out the existing network.
    resetTrackNetwork();

    // Load the previously saved network.
    std::string segment;
    try {
        while (!ifstr.eof()) {
            std::getline(ifstr, segment);
            if (segment.empty()) continue;
            size_t pos1 = 7;
            if (segment.substr(0, pos1) != "track: ") {
                throw std::runtime_error("Serialized string preamble missing");
            }
            size_t pos2 = segment.find(',', pos1);
            std::string name = segment.substr(pos1, pos2-pos1);
            EdgePtr eptr = std::make_shared<Edge>(name);
            m_edgeMap.insert(EdgeItem(eptr->name(), eptr));
            eptr->deserialize(segment);
        }
        updateAllSignals();
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        std::cout << "segment: \"" << segment << "\"" << std::endl;
        return EFAULT;
    }
    return 0;
}

std::string System::getUniqueEdgeName()
{
    int ix = 1;
    std::string name = "tseg001";
    while (m_edgeMap.find(name) != m_edgeMap.end()) {
        ix++;
        std::stringstream ns;
        ns << "tseg" << std::setw(3) << std::setfill('0') << ix;
        name = ns.str();
    }
    return name;
}
std::string System::getUniqueNodeName()
{
    int ix = 1;
    std::string name = "node001";
    while (m_nodeMap.find(name) != m_nodeMap.end()) {
        ix++;
        std::stringstream ns;
        ns << "node" << std::setw(3) << std::setfill('0') << ix;
        name = ns.str();
    }
    return name;
}
std::string System::getUniqueTrainName()
{
    int ix = 1;
    std::string name = "train1";
    while (m_trainMap.find(name) != m_trainMap.end()) {
        ix++;
        std::stringstream ns;
        ns << "train" << ix;
        name = ns.str();
    }
    return name;
}

} // namespace rrsim

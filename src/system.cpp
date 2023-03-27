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
    for (auto iter: m_edgeMap) {
        if (iter.second) { iter.second.reset(); }
    }
    m_edgeMap.clear();

    for (auto iter: m_nodeMap) {
        if (iter.second) { iter.second.reset(); }
    }
    m_nodeMap.clear();

    for (auto iter: m_trainMap) {
        if (iter.second) { iter.second.reset(); }
    }
    m_trainMap.clear();
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
    if (!s1.eeEdge || !s2.eeEdge) { return EINVAL; }

    NodeSlot cnctNode = s1.eeEdge->getNode(s1.eeEnd);
    NodeSlot rmovNode = s2.eeEdge->getNode(s2.eeEnd);
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
        s2.eeEdge->assignNodeSlot(replNode, s2.eeEnd);
        break;

    case eContinuation:
        // This connection results in a junction from this track to the
        // currently connected track (left) or to the new track (right).
        cnctNode.nsNode->makeJunction(s2);

        // Replace the other edge's node slot entry.
        replNode = { cnctNode.nsNode, eSlot3 };
        s2.eeEdge->assignNodeSlot(replNode, s2.eeEnd);
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

/*
int System::cmdConnectSegments(const EdgeEnd& s1, const EdgeEnd& s2)
{
    try {
        iter1->second->connectEdge(end1, iter2->second, end2);
        iter1->second->show(end1);
    }
    catch(std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EBUSY;
    }
    return 0;
}

static int cmdPlaceSignal()
{
    std::string resp1 = enterName();
    if (resp1.empty()) { return 0; }
    auto iter1 = g_edgeMap.find(resp1);
    if (iter1 == g_edgeMap.end()) {
        resp1 = nameFromNumber(resp1);
        iter1 = g_edgeMap.find(resp1);
        if (iter1 == g_edgeMap.end()) {
            std::cout << "No such segment \"" << resp1 << "\"" << std::endl;
            return EINVAL;
        }
    }
    rrsim::eEnd end1 = enterAorB();

    try {
        iter1->second->placeSignalLight(end1);
        rrsim::RRsignal::updateAllSignals();
        iter1->second->show(end1);
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }

    return 0;
}

static int cmdToggleSwitch()
{
    std::vector<Node*> junctions;
    for (auto it: g_nodeMap) {
        if (it.second->getNodeType() == rrsim::eJunction) {
            junctions.push_back(it.second);
        }
    }
    if (junctions.empty()) {
        std::cout << ">>> There are no junctions in the track network <<<"
                  << std::endl;
        return 0;
    }
    int jnum = 0;
    for (auto it: junctions) {
        std::cout << ++jnum << ": " << it->name() << std::endl;
    }
    std::cout << "Enter junction (1.." << jnum << "): ";
    std::string numstr;
    std::getline(std::cin, numstr);
    if (numstr.empty()) {
        std::cout << "No entry, quitting function..." << std::endl;
        return 0;
    }
    int val = std::atoi(numstr.c_str());
    if ((val < 1) || (val > jnum)) {
        std::cout << "No such junction" << std::endl;
        return EINVAL;
    }
    val--; // Make the index zero based.
    junctions[val]->toggleSwitchPos();
    std::cout << junctions[val]->name() << ": junction switch is ";
    rrsim::eJSwitch jsw = junctions[val]->getSwitchPos();
    std::cout << ((jsw == rrsim::eSwitchLeft) ? "LEFT" : "RIGHT" ) << std::endl;
    return 0;

}

static int cmdListSegments()
{
    try {
        for (auto it: g_edgeMap) {
            it.second->show();
        }
        std::cout << std::endl;
        std::cout << "TOTAL: " << g_edgeMap.size() << " track segments" << std::endl;
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }
    return 0;
}

static int cmdShowConnections()
{
    try {
        for (auto it: g_nodeMap) {
            it.second->show();
        }
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }
    return 0;
}

static int cmdPlaceTrain()
{
    std::cout << "Starting - ";
    std::string resp1 = enterName();
    if (resp1.empty()) { return 0; }
    auto iter1 = g_edgeMap.find(resp1);
    if (iter1 == g_edgeMap.end()) {
        resp1 = nameFromNumber(resp1);
        iter1 = g_edgeMap.find(resp1);
        if (iter1 == g_edgeMap.end()) {
            std::cout << "No such segment \"" << resp1 << "\"" << std::endl;
            return EINVAL;
        }
    }
    std::cout << "Ending - ";
    std::string resp2 = enterName();
    if (resp2.empty()) { return 0; }
    auto iter2 = g_edgeMap.find(resp2);
    if (iter2 == g_edgeMap.end()) {
        resp2 = nameFromNumber(resp2);
        iter2 = g_edgeMap.find(resp2);
        if (iter2 == g_edgeMap.end()) {
            std::cout << "No such segment \"" << resp2 << "\"" << std::endl;
            return EINVAL;
        }
    }

    try {
        rrsim::EdgeEnd edge = g_train.getPosition();
        if (edge.eeEdge) { edge.eeEdge->setTrain(nullptr); }
        g_train.placeOnTrack(iter1->second, iter2->second);
        rrsim::RRsignal::updateAllSignals();
        g_train.show();
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }

    return 0;
}

static int cmdStepSimulation()
{
    try {
        bool chk = g_train.stepSimulation();
        rrsim::RRsignal::updateAllSignals();
        g_train.show();
        if (!chk) {
            std::cout << ">>> The Simulation Is Complete <<<" << std::endl;
        }
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }

    return 0;
}

static int cmdSaveNetwork()
{
    std::string path;
    std::cout << "Enter file path: ";
    std::getline(std::cin, path);
    if (path.empty()) {
        std::cout << "No response, quitting..." << std::endl;
        return 0;
    }
    std::ifstream ifstr(path);
    if (ifstr.good()) {
        std::cout << path << " exists. Replace contents? [y/n] ";
        std::string resp;
        std::getline(std::cin, resp);
        if ((resp != "Y") && (resp != "y")) {
            std::cout << "NOT replacing contents. quitting..." << std::endl;
            return 0;
        }
    }
    ifstr.close();
    std::ofstream ofstr(path, std::ofstream::trunc);
    if (!ofstr.good()) {
        std::cout << "Unable to open file " << path << ", quitting..." << std::endl;
        return 0;
    }
    for (auto iter: g_edgeMap) {
        Edge* edge = iter.second;
        if (edge) {
            ofstr << edge->serialize();
        }
    }
    ofstr.close();
    return 0;
}

static int cmdLoadNetwork()
{
    if (!g_edgeMap.empty()) {
        std::cout << "WARNING: This will delete the existing network" << std::endl;
        std::cout << "         Press ENTER key at the prompt to quit" << std::endl;
    }
    std::string path;
    std::cout << "Enter file path: ";
    std::getline(std::cin, path);
    if (path.empty()) {
        std::cout << "No response, quitting..." << std::endl;
        return 0;
    }
    std::ifstream ifstr(path);
    if (!ifstr.good()) {
        std::cout << path << " not found, quitting..." << std::endl;
        return ENOENT;
    }
    // Clear out the existing network.
    resetTrackNetwork();

    // Load the previously saved network.
    std::string segment;
    try {
        while (!ifstr.eof()) {
            std::getline(ifstr, segment);
            if (segment.empty()) continue;
            if (segment.substr(0, 7) != "track: ") {
                throw std::runtime_error("Serialized string preamble missing");
            }
            new Edge(segment);
        }
        rrsim::RRsignal::updateAllSignals();
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        std::cout << "segment: \"" << segment << "\"" << std::endl;
        return EFAULT;
    }
    return 0;
}
*/

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

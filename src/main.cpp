// main.cpp
//
// Author: Kendall Auel
// Description:
//     Entry point for the railroad signaling case study implementation.
//

#include "node.h"
#include "edge.h"
#include "config.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>

using rrsim::Node;
using rrsim::Edge;
using rrsim::g_nodeMap;
using rrsim::g_edgeMap;


static std::string enterName()
{
    std::string resp;
    std::cout << "Enter track segment name: ";
    std::getline(std::cin, resp);
    if (resp.empty()) {
        std::cout << "No name entered, quitting..." << std::endl;
    }
    return resp;
}

static std::string nameFromNumber(const std::string& num)
{
    int val = std::atoi(num.c_str());
    std::stringstream ss;
    ss << "tseg" << std::setw(3) << std::setfill('0') << val;
    return ss.str();
}

static rrsim::eEnd enterAorB()
{
    rrsim::eEnd rval = rrsim::eNumEnds;
    std::string resp;
    while (rval == rrsim::eNumEnds) {
        std::cout << "Enter track end (A or B): ";
        std::getline(std::cin, resp);
        if      ((resp == "a") || (resp == "A")) { rval = rrsim::eEndA; }
        else if ((resp == "b") || (resp == "B")) { rval = rrsim::eEndB; }
        else if (!resp.empty()) {
            std::cout << "Expected one of [ABab], got: " << resp << std::endl;
        }
    }
    return rval;
}
static int cmdSmokeTest()
{
    return 0;
}

static int cmdAddSegment()
{
    Edge* pEdge = new Edge();
    g_edgeMap.insert(rrsim::EdgePair(pEdge->name(), pEdge));
    std::cout << "Added new track segment \"" << pEdge->name() << "\"" << std::endl;
    return 0;
}

static int cmdConnectSegments()
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
    rrsim::eEnd end2 = enterAorB();

    try {
        iter1->second->connectEdge(end1, iter2->second, end2);
    }
    catch(std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EBUSY;
    }
    iter1->second->show(end1);
    return 0;
}

static int cmdListSegments()
{
    for (auto it: g_edgeMap) {
        it.second->show();
    }
    std::cout << std::endl;
    std::cout << "TOTAL: " << g_edgeMap.size() << " track segments" << std::endl;
    return 0;
}

static int cmdShowConnections()
{
    for (auto it: g_nodeMap) {
        it.second->show();
    }
    return 0;
}

int runCommand()
{
    int rc;

    std::cout << std::endl <<
            "Train Signaling System Simulator"              << std::endl <<
            "1. Add a track segment"                        << std::endl <<
            "2. Connect track segments"                     << std::endl <<
            "3. List track segments"                        << std::endl <<
            "4. Show track connections"                     << std::endl <<
            "5. Run smoke test"                             << std::endl <<
            "Q/quit/exit"                                   << std::endl;

    std::string resp;
    while (resp.empty()) {
        std::cout << "=> ";
        std::getline(std::cin, resp);
    }
    if (resp == "Q" || resp == "q" || resp == "quit" || resp == "exit") {
        return 1;
    }
    int cmd;
    try { cmd = std::stoi(resp); } catch (...) { cmd = 0; }

    switch (cmd) {
    case 1:
        std::cout << "---------------- Add Track Segment -----------------" << std::endl;
        rc = cmdAddSegment();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 2:
        std::cout << "-------------- Connect Track Segment ---------------" << std::endl;
        rc = cmdConnectSegments();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 3:
        std::cout << "--------------- List Track Segments ----------------" << std::endl;
        rc = cmdListSegments();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 4:
        std::cout << "----------------- Show Connections -----------------" << std::endl;
        rc = cmdShowConnections();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 5:
        std::cout << "------------------ Run Smoke Test ------------------" << std::endl;
        rc = cmdSmokeTest();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    default:
        std::cout << "Invalid entry: \"" << resp << "\"" << std::endl;
        rc = EINVAL;
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    }
    if (rc) std::cout << "(Error code " << rc << " was returned)" << std::endl;
    return 0;
}

// -----------------------------------------------------------------------------
// main -- Entry point
// -----------------------------------------------------------------------------

int main(int argc, char **argv) {
    std::cout << "Case Study Implementation -- Railroad Signaling System" << std::endl;
    std::cout << "Version " << cs_signaling_VERSION_MAJOR << "." << cs_signaling_VERSION_MINOR << std::endl;

    while (runCommand() == 0) {}
    return 0;
}

// system.h
//
// Author: Kendall Auel
//
// The class "System" represents the entire railroad system
// including the track network and all trains running on the
// tracks.
//
// This is instantiated as a singleton object.

#ifndef _CS_SYSTEM_H_
#define _CS_SYSTEM_H_

#include "common.h"
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <fstream>

namespace rrsim {

using EdgeMap   = std::map<std::string, EdgePtr>;
using NodeMap   = std::map<std::string, NodePtr>;
using TrainMap  = std::map<std::string, TrainPtr>;

using EdgeItem  = EdgeMap::value_type;
using NodeItem  = NodeMap::value_type;
using TrainItem = TrainMap::value_type;

using NodeVec   = std::vector<NodePtr>;

class System
{
public:
    static System& instance();
    static void destroy();

    static const std::string emptyStr;

    void        resetTrackNetwork();
    int         edgeCount() { return m_edgeMap.size(); }

    EdgePtr     createEdge(const std::string& name = emptyStr);
    EdgePtr     getEdge(const std::string& name);
    void        removeEdge(const std::string& name);

    NodePtr     createNode(const std::string& name = emptyStr);
    NodePtr     getNode(const std::string& name);
    void        removeNode(const std::string& name);

    TrainPtr    createTrain(const std::string& name = emptyStr);
    TrainPtr    getTrain(const std::string& name);
    void        removeTrain(const std::string& name);

    int         connectSegments(const EdgeEnd& s1, const EdgeEnd& s2);
    int         stepSimulation();
    int         runSimulation();
    int         showEdges();
    int         showNodes();

    void        addSignalsToAllJunctions();
    void        updateAllSignals();
    NodeVec     getAllJunctions();
    int         serialize(std::ofstream& ofstr);
    int         deserialize(std::ifstream& ifstr);

    // Disallow copying the System singleton.
    System(System const&)           = delete;
    void operator=(System const&)   = delete;

private:
    System();
    ~System();

    std::string getUniqueEdgeName();
    std::string getUniqueNodeName();
    std::string getUniqueTrainName();

    EdgeMap     m_edgeMap;
    NodeMap     m_nodeMap;
    TrainMap    m_trainMap;
};

} // namespace rrsim

// Handy shortcut to the singleton from outside our namespace.
inline rrsim::System& sys() { return rrsim::System::instance(); }

#endif // _CS_SYSTEM_H_

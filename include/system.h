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

#include "edge.h"
#include "node.h"
#include "train.h"
#include <string>
#include <map>
#include <memory>

namespace rrsim {

using EdgePtr   = std::shared_ptr<Edge>;
using TrainPtr  = std::shared_ptr<Train>;

using EdgeMap   = std::map<std::string, EdgePtr>;
using TrainMap  = std::map<std::string, TrainPtr>;

using EdgeItem  = EdgeMap::value_type;
using TrainItem = TrainMap::value_type;

class System
{
public:
    static System& instance();
    static void destroy();

    void resetTrackNetwork();

    EdgePtr     createEdge();
    EdgePtr     getEdge(const std::string& name);

    TrainPtr    createTrain();
    TrainPtr    getTrain(const std::string& name);

    int         connectSegments(const EdgeEnd& s1, const EdgeEnd& s2);
    int cmdPlaceSignal();
    int cmdToggleSwitch();

    // Disallow copying the System singleton.
    System(System const&)           = delete;
    void operator=(System const&)   = delete;

private:
    System();
    ~System();

    EdgeMap     m_edgeMap;
    TrainMap    m_trainMap;
};

} // namespace rrsim

// Handy shortcut to the singleton from outside our namespace.
inline rrsim::System& sys() { return rrsim::System::instance(); }

#endif // _CS_SYSTEM_H_

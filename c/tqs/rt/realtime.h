#ifndef _TQS_REALTIME_H
#define _TQS_REALTIME_H

#include <string>
#include <functional>

#include "stralet.h"

namespace tquant { namespace stralet { namespace realtime {

    using namespace std;
    using namespace tquant::stralet;

    struct RealTimeConfig {
        string tqapi_addr;
        string output_dir;
    };

    void run(const RealTimeConfig & cfg, function<Stralet*()> creator);
    void run(const char* cfg, function<Stralet*()> creator);


} } }


#endif

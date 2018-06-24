#ifndef _TQS_REALTIME_H
#define _TQS_REALTIME_H

#include <string>
#include <functional>

#include "stralet.h"

namespace tquant { namespace stralet { namespace realtime {

    using namespace std;
    using namespace tquant::stralet;

    struct RealTimeConfig {
        string data_api_addr;
        string trade_api_addr;
        string output_dir;
    };

    _TQAPI_EXPORT void run(const RealTimeConfig & cfg, function<Stralet*()> creator);
    _TQAPI_EXPORT void run(const char* cfg, function<Stralet*()> creator);


} } }


#endif

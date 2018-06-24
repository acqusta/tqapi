#ifndef _TQS_BACKTEST_H
#define _TQS_BACKTEST_H

#include <string>
#include <vector>

#include "stralet.h"
#include <functional>

namespace tquant { namespace stralet { namespace backtest {

    using namespace std;

    using namespace tquant::stralet;

    struct Holding {
        string  code;
        string  side;
        int64_t size;
        double  cost_price;
    };

    struct AccountConfig {
        string           account_id;
        double           init_balance;
        vector<Holding>  init_holdings;


        AccountConfig(const string& a_account_id, double a_init_balance)
            : account_id(a_account_id)
            , init_balance(a_init_balance)
        {}

        AccountConfig(const string& a_account_id, double a_init_balance, const vector<Holding>& a_init_holdings)
        : account_id(a_account_id)
            , init_balance(a_init_balance)
            , init_holdings(a_init_holdings)
        {}
    };

    struct BackTestConfig {
        string dapi_addr;
        string data_level; // tk, 1m, 1d
        int    begin_date;
        int    end_date;
        vector<AccountConfig> accounts;
        string result_dir;

        string properties;

        BackTestConfig() 
            : begin_date(0)
            , end_date(0)
        {
        }
    };

    //typedef Stralet* (*create_stralet

    _TQAPI_EXPORT void run(const BackTestConfig & cfg, function<Stralet*()> creator);
    _TQAPI_EXPORT void run(const char* cfg, function<Stralet*()> creator);

} } }

#endif

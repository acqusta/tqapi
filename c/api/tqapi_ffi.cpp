#include <iostream>
#include "tquant_api.h"
#include "tqapi_ffi.h"
#include "myutils/stringutils.h"

using namespace std;

struct DataApi {
    tquant::api::DataApi* instance;
};

extern "C" {
    DataApi* tqapi_create_data_api(const char* addr)
    {
        auto inst = tquant::api::create_data_api(addr);
        auto dapi = new DataApi();
        dapi->instance = inst;
        return dapi;
    }

    void tqapi_free_data_api(DataApi* dapi)
    {
        if (dapi) {
            delete dapi->instance;
            delete dapi;
        }
    }

    struct GetTickResultData {
        tquant::api::CallResult<const tquant::api::MarketQuoteArray> result;
    };

    GetTickResult* tqapi_dapi_get_ticks(DataApi* dapi, const char* code, int trade_date)
    {
        std::cout<<"get_ticks: " <<code <<"," << strlen(code) << "," << trade_date << endl;
        auto r = dapi->instance->tick(code, 0);//trade_date);
        auto gtr = new GetTickResult;
        gtr->data =  new GetTickResultData;
        gtr->data->result = r;

        if (gtr->data->result.value) {
            auto& tmp = gtr->data->result.value;
            gtr->array = (MarketQuote*)tmp->_data;
            gtr->array_length = tmp->_size;
            gtr->element_size = tmp->_type_size;
            gtr->msg = nullptr;
        } else {
            gtr->msg = gtr->data->result.msg.c_str();
            gtr->array = nullptr;
            gtr->array_length = 0;
            gtr->element_size = 0;
        }
        return gtr;
    }

    void tqapi_dapi_free_get_ticks_result(DataApi* dapi, GetTickResult* result)
    {
        if (result) {
            delete result->data;
            delete result;
        }
    }

    struct SubscribeResultData {
        string codes;
        string msg;
    };

    SubscribeResult* tqapi_dapi_subscribe(DataApi* dapi, const char*codes)
    {
        vector<string> ss;
        split(codes, ",", &ss);
        for (auto& s : ss)
            std::cout << "sub: " << s << endl;
        auto r = dapi->instance->subscribe(ss);

        SubscribeResult* result = new SubscribeResult();
        result->_data = new SubscribeResultData();
        if (r.value) {
            stringstream ss;
            for(auto s : *(r.value))
                ss << s << ",";
            result->_data->codes = ss.str();
            result->_data->codes.resize(result->_data->codes.size()-1);
            result->codes = result->_data->codes.c_str();
            result->msg   = nullptr;
        }
        else {
            result->_data->msg = r.msg;
            result->msg = nullptr;
            result->codes = nullptr;
        }
        return result;
    }

    void tqapi_dapi_free_subscribe_result(DataApi* dapi, SubscribeResult* result)
    {
        if (result) {
            delete result->_data;
            delete result;
        }
    }

    struct UnSubscribeResultData {
        string codes;
        string msg;
    };

    UnSubscribeResult* tqapi_dapi_unsubscribe(DataApi* dapi, const char*codes)
    {
        vector<string> ss;
        split(codes, ",", &ss);
        auto r = dapi->instance->subscribe(ss);

        auto result = new UnSubscribeResult();
        result->_data = new UnSubscribeResultData();
        if (r.value) {
            stringstream ss;
            for(auto s : *(r.value))
                ss << s << ",";
            result->_data->codes = ss.str();
            result->_data->codes.resize(result->_data->codes.size()-1);
            result->codes = result->_data->codes.c_str();
            result->msg   = nullptr;
        }
        else {
            result->_data->msg = r.msg;
            result->msg = nullptr;
            result->codes = nullptr;
        }
        return result;
    }

    void tqapi_dapi_free_unsubscribe_result(DataApi* dapi, UnSubscribeResult* result)
    {
        if (result) {
            delete result->_data;
            delete result;
        }
    }

}

#include <iostream>
#include "tquant_api.h"
#include "tqapi_ffi.h"
#include "myutils/stringutils.h"

using namespace std;

static_assert(sizeof(MarketQuote) == sizeof(tquant::api::RawMarketQuote), "Wrong MarketQuote size");
static_assert(sizeof(Bar)         == sizeof(tquant::api::RawBar),         "Wrong Bar size");
static_assert(sizeof(DailyBar)    == sizeof(tquant::api::RawDailyBar),    "Wrong DailyBar size");


struct DataApi : public tquant::api::DataApi_Callback {

    tquant::api::DataApi* instance;
    DataApiCallback* cb;
    bool is_owner;

    virtual void on_market_quote(shared_ptr<const tquant::api::MarketQuote> quote) override {
        if (cb)
            cb->on_quote(cb->obj, (const MarketQuote*)quote.get());
    }

    virtual void on_bar(const string& cycle, shared_ptr<const tquant::api::Bar> bar) override {
        if (cb)
            cb->on_bar(cb->obj, cycle.c_str(), (const Bar*)bar.get());
    }
};

extern "C" {

    DataApi* tqapi_create_data_api(const char* addr)
    {
        auto inst = tquant::api::create_data_api(addr);
        if (inst == nullptr) {
            return nullptr;
        }
        auto dapi = new DataApi();
        dapi->instance = inst;
        dapi->cb = nullptr;
        dapi->is_owner = true;
        inst->set_callback(dapi);
        return dapi;
    }

    void tqapi_free_data_api(DataApi* dapi)
    {
        if (dapi && dapi->is_owner) {
            dapi->instance->set_callback(nullptr);
            delete dapi->instance;
            delete dapi;
        }
    }

    DataApi* tqapi_dapi_from(tquant::api::DataApi* inst)
    {
        auto dapi = new DataApi();
        dapi->instance = inst;
        dapi->cb = nullptr;
        dapi->is_owner = true;
        inst->set_callback(dapi);
        return dapi;
    }

    DataApiCallback*   tqapi_dapi_set_callback(DataApi* dapi, DataApiCallback* callback)
    {
        auto old = dapi->cb;
        dapi->cb = callback;
        return old;
    }

    struct GetTickResultData {
        tquant::api::CallResult<const tquant::api::MarketQuoteArray> result;
    };

    GetTickResult* tqapi_dapi_get_ticks(DataApi* dapi, const char* code, int trade_date, int number)
    {
        auto r = dapi->instance->tick(code, trade_date, number);

        //dapi->cb->on_quote(nullptr, dapi->cb->user_data);
        // for (int i = 0; i < r.value->size(); i++) {
        //     dapi->cb->on_quote((const MarketQuote*)&r.value->at(i), dapi->cb->user_data);
        // }
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

    struct GetBarResultData {
        tquant::api::CallResult<const tquant::api::BarArray> result;
    };

    GetBarResult* tqapi_dapi_get_bars(DataApi* dapi, const char* code, const char* cycle, int trade_date, int align, int number)
    {
        auto r = dapi->instance->bar(code, cycle, trade_date, align, number);
        auto gbr = new GetBarResult;
        gbr->data =  new GetBarResultData;
        gbr->data->result = r;

        if (gbr->data->result.value) {
            auto& tmp = gbr->data->result.value;
            gbr->array = (Bar*)tmp->_data;
            gbr->array_length = tmp->_size;
            gbr->element_size = tmp->_type_size;
            gbr->msg = nullptr;
        } else {
            gbr->msg = gbr->data->result.msg.c_str();
            gbr->array = nullptr;
            gbr->array_length = 0;
            gbr->element_size = 0;
        }
        return gbr;
    }

    void tqapi_dapi_free_get_bars_result(DataApi* dapi, GetBarResult* result)
    {
        if (result) {
            delete result->data;
            delete result;
        }
    }

    struct GetDailyBarResultData {
        tquant::api::CallResult<const tquant::api::DailyBarArray> result;
    };

    GetDailyBarResult* tqapi_dapi_get_dailybars(DataApi* dapi, const char* code, const char* price_type, int align, int number)
    {
        auto r = dapi->instance->daily_bar(code, price_type, align, number);
        auto gbr  = new GetDailyBarResult;
        gbr->data = new GetDailyBarResultData;
        gbr->data->result = r;

        if (gbr->data->result.value) {
            auto& tmp = gbr->data->result.value;
            gbr->array = (DailyBar*)tmp->_data;
            gbr->array_length = tmp->_size;
            gbr->element_size = tmp->_type_size;
            gbr->msg = nullptr;
        }
        else {
            gbr->msg = gbr->data->result.msg.c_str();
            gbr->array = nullptr;
            gbr->array_length = 0;
            gbr->element_size = 0;
        }
        return gbr;
    }

    void tqapi_dapi_free_get_dailybars_result(DataApi* dapi, GetDailyBarResult* result)
    {
        if (result) {
            delete result->data;
            delete result;
        }
    }

    struct GetQuoteResultData {
        tquant::api::CallResult<const tquant::api::MarketQuote> result;
    };

    GetQuoteResult* tqapi_dapi_get_quote(DataApi* dapi, const char* code)
    {
        auto r = dapi->instance->quote(code);
        auto gbr  = new GetQuoteResult;
        gbr->_data = new GetQuoteResultData;
        gbr->_data->result = r;

        if (gbr->_data->result.value) {
            gbr->quote = (const MarketQuote*)gbr->_data->result.value.get();
            gbr->msg = nullptr;
        }
        else {
            gbr->msg = gbr->_data->result.msg.c_str();
            gbr->quote = nullptr;
        }
        return gbr;
    }

    void tqapi_dapi_free_get_quote_result(DataApi* dapi, GetQuoteResult* result)
    {
        if (result) {
            delete result->_data;
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
        // for (auto& s : ss)
        //     std::cout << "sub: " << s << endl;
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

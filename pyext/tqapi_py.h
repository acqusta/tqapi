#ifndef _TQAPI_API_H
#define _TQAPI_API_H

// Remove automatically imports python27_d.lib on Windows
// Reference to pkconfig.h
#ifdef _WIN32
#  ifdef _DEBUG
#    undef _DEBUG
#    include <Python.h>
#    define _DEBUG
#  else
#    include <Python.h>
#  endif
#  define API_EXPORT 
#else
#  include <Python.h>
#  define API_EXPORT __attribute__ ((visibility("default")))
#endif

#include <unordered_map>
#include <mutex>
#include "myutils/stringutils.h"
#include "myutils/loop/MsgRunLoop.h"
#include "tquant_api.h"


using namespace tquant::api;

class PyObjectHolder {
public:
    PyObject* obj;

    PyObjectHolder(PyObject* a_obj = Py_None) {
        obj = a_obj;
        Py_XINCREF(obj);
    }

    PyObjectHolder(const PyObjectHolder& rhd) {
        Py_XDECREF(obj);
        this->obj = rhd.obj;
        Py_XINCREF(this->obj);

    }

    void operator =(PyObject* new_obj) {
        Py_XDECREF(obj);
        this->obj = new_obj;
        Py_XINCREF(this->obj);
    }

    ~PyObjectHolder() {
        Py_XDECREF(obj);
    }
};

void call_callback(PyObject* callback, const char* evt, PyObject* data);

static inline void dict_set_item(PyObject* obj, const char* key, PyObject* value)
{
    PyDict_SetItemString(obj, key, value);
    Py_XDECREF(value);
}

static inline void dict_set_item(PyObject* obj, const char* key, const char* str)
{
    PyObject * v = PyString_FromString(str);
    PyDict_SetItemString(obj, key, v);
    Py_XDECREF(v);
}

static inline void dict_set_item(PyObject* obj, const char* key, const string& str)
{
    dict_set_item(obj, key, str.c_str());
}

static inline void dict_set_item(PyObject* obj, const char* key, int32_t value)
{
    PyObject * v = PyLong_FromLong(value);
    PyDict_SetItemString(obj, key, v);
    Py_XDECREF(v);
}

static inline void dict_set_item(PyObject* obj, const char* key, int64_t value)
{
    PyObject * v = PyLong_FromLongLong(value);
    PyDict_SetItemString(obj, key, v);
    Py_XDECREF(v);
}

static inline void dict_set_item(PyObject* obj, const char* key, double value)
{
    PyObject * v = PyFloat_FromDouble(value);
    PyDict_SetItemString(obj, key, v);
    Py_XDECREF(v);
}

static inline void dict_set_item(PyObject* obj, const char* key, bool value)
{
    PyObject * v = PyBool_FromLong(value);
    PyDict_SetItemString(obj, key, v);
    Py_XDECREF(v);
}

class DataApiWrap : public DataApi_Callback, public loop::MsgLoopRun {
public:
    DataApiWrap(DataApi* dapi)
        : m_dapi(dapi)
    {
        m_dapi->set_callback(this);
    }

    virtual ~DataApiWrap() {

    }

    // DataApi_Callback
    virtual void on_market_quote (shared_ptr<const MarketQuote> quote) override;
    virtual void on_bar          (const string& cycle, shared_ptr<const Bar> bar) override;

    PyObjectHolder  m_dapi_cb;
    DataApi* m_dapi;
};

class TradeApiWrap : public TradeApi_Callback, public loop::MsgLoopRun
{
    friend DataApiWrap;
public:
    TradeApiWrap(TradeApi* tapi)
        : m_tapi(tapi)
    {
        m_tapi->set_callback(this);
    }

    ~TradeApiWrap() {
        //for (auto e : m_dapi_map)
        //    delete e.second;
    }

    // TradeApi_Callback
    virtual void on_order_status   (shared_ptr<Order> order) override;
    virtual void on_order_trade    (shared_ptr<Trade> trade) override;
    virtual void on_account_status (shared_ptr<AccountInfo> account) override;

    //TradeApi* trade_api() { return m_tapi; }

    //DataApiWrap* data_api(const char* source) {
    //    unique_lock<mutex> lock(m_mtx);
    //    string str = trim(source ? source : "");
    //    auto it = m_dapi_map.find(str);
    //    if (it != m_dapi_map.end())
    //        return it->second;

    //    auto dapi = m_api->data_api(source);
    //    if (!dapi)
    //        return nullptr;

    //    auto dapi_wrap = new DataApiWrap(this, dapi);
    //    m_dapi_map[str] = dapi_wrap;
    //    return dapi_wrap;
    //}

    TradeApi*       m_tapi;
    PyObjectHolder  m_tapi_cb;
    mutex m_mtx;
};

PyObject* convert_quote        (const MarketQuote* q);
PyObject* convert_bar          (const Bar* b);
PyObject* convert_order        (const Order* order);
PyObject* convert_trade        (const Trade* trade);
PyObject* convert_position     (const Position* position);
PyObject* convert_account_info (const AccountInfo* account);

PyObject* _wrap_tapi_create                 (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_destroy                (PyObject* self, PyObject *args, PyObject* kwargs);

PyObject* _wrap_tapi_place_order            (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_cancel_order           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_orders           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_trades           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_positions        (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_balance          (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_set_callback           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query                  (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_account_status   (PyObject* self, PyObject *args, PyObject* kwargs);

PyObject* _wrap_dapi_create                 (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_destroy                (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_set_callback           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_subscribe              (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_unsubscribe            (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_quote                  (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_bar                    (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_tick                   (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_dailybar               (PyObject* self, PyObject *args, PyObject* kwargs);

PyObject* _wrap_tqs_sc_trading_day          (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_cur_time             (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_post_event           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_set_timer            (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_kill_timer           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_dapi_get             (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_dapi_put             (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_tapi_get             (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_tapi_put             (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_log                  (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_get_properties       (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_get_property         (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_sc_mode                 (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_bt_run                  (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqs_rt_run                  (PyObject* self, PyObject *args, PyObject* kwargs);

#endif

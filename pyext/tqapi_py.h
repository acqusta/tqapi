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

class TQuantApiWrap;

class DataApiWrap : public DataApi_Callback {
public:
    DataApiWrap(TQuantApiWrap* tqapi, DataApi* dapi)
        : m_tqapi(tqapi)
        , m_dapi(dapi)
    {
        m_dapi->set_callback(this);
    }

    // DataApi_Callback
    virtual void on_market_quote (shared_ptr<MarketQuote> quote) override;
    virtual void on_bar          (const char* cycle, shared_ptr<Bar> bar) override;

    PyObjectHolder  m_dapi_cb;
    TQuantApiWrap*  m_tqapi;
    DataApi* m_dapi;
};

class TQuantApiWrap :
        public TradeApi_Callback,
        public loop::MsgLoopRun
{
    friend DataApiWrap;
public:
    TQuantApiWrap(TQuantApi* api)
        : m_api(api)
    {
        m_api->trade_api()->set_callback(this);
    }

    ~TQuantApiWrap() {
        for (auto e : m_dapi_map)
            delete e.second;
    }

    // TradeApi_Callback
    virtual void on_order_status   (shared_ptr<Order> order) override;
    virtual void on_order_trade    (shared_ptr<Trade> trade) override;
    virtual void on_account_status (shared_ptr<AccountInfo> account) override;

    TradeApi* trade_api() { return m_api->trade_api(); }

    DataApiWrap* data_api(const char* source) {
        unique_lock<mutex> lock(m_mtx);
        string str = trim(source ? source : "");
        auto it = m_dapi_map.find(str);
        if (it != m_dapi_map.end())
            return it->second;

        auto dapi = m_api->data_api(source);
        if (!dapi)
            return nullptr;

        auto dapi_wrap = new DataApiWrap(this, dapi);
        m_dapi_map[str] = dapi_wrap;
        return dapi_wrap;
    }

    TQuantApi*      m_api;
    PyObjectHolder  m_tapi_cb;
    unordered_map<string, DataApiWrap*> m_dapi_map;
    mutex m_mtx;
};


PyObject* _wrap_tqapi_create            (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqapi_destroy           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqapi_get_data_api      (PyObject* self, PyObject *args, PyObject* kwargs);
//PyObject* _wrap_tqapi_get_trade_api     (PyObject* self, PyObject *args, PyObject* kwargs);

PyObject* _wrap_tapi_place_order            (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_cancel_order           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_orders           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_trades           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_positions        (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_balance          (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_set_callback           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query                  (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_account_status   (PyObject* self, PyObject *args, PyObject* kwargs);

PyObject* _wrap_dapi_set_callback       (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_subscribe          (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_unsubscribe        (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_quote              (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_bar                (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_tick               (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_dailybar           (PyObject* self, PyObject *args, PyObject* kwargs);

#endif

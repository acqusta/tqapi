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

#include "loop/MsgRunLoop.h"
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

class TQuantApiWrap :
        public DataApi_Callback,
        public TradeApi_Callback,
        public loop::MsgLoopRun
{
public:
    TQuantApiWrap(TQuantApi* api)
        : m_api(api)
    {
        m_api->data_api()->set_callback(this);
        m_api->trade_api()->set_callback(this);
    }

    ~TQuantApiWrap() {

    }

    // DataApi_Callback
    virtual void onMarketQuote(shared_ptr<MarketQuote> quote) override;
    virtual void onBar(const char* cycle, shared_ptr<Bar> bar) override;

    // TradeApi_Callback
    virtual void onOrderStatus(shared_ptr<Order> order) override;
    virtual void onOrderTrade(shared_ptr<Trade> trade) override;
    virtual void onAccountStatus(shared_ptr<AccountInfo> account) override;

    DataApi*  data_api()  { return m_api->data_api(); }
    TradeApi* trade_api() { return m_api->trade_api(); }

    TQuantApi*      m_api;
    PyObjectHolder  m_dapi_cb;
    PyObjectHolder  m_tapi_cb;
};

PyObject* _wrap_tqapi_create            (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqapi_destroy           (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqapi_get_data_api      (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tqapi_get_trade_api     (PyObject* self, PyObject *args, PyObject* kwargs);

PyObject* _wrap_tapi_place_order        (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_cancel_order       (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_orders       (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_trades       (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_positions    (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query_balance      (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_set_callback       (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_query              (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_tapi_get_account_status (PyObject* self, PyObject *args, PyObject* kwargs);

PyObject* _wrap_dapi_set_callback       (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_subscribe          (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_unsubscribe        (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_quote              (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_bar                (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_tick               (PyObject* self, PyObject *args, PyObject* kwargs);
PyObject* _wrap_dapi_dailybar           (PyObject* self, PyObject *args, PyObject* kwargs);

#endif

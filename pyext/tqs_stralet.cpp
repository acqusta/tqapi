#include "tqapi_py.h"
#ifdef _WIN32
#  undef ERROR
#endif

#include "stralet.h"
#include "bt/backtest.h"
#include "rt/realtime.h"
#include "myutils/stringutils.h"

using namespace tquant::api;
using namespace tquant::stralet;

class StraletWrap : public Stralet {
public:
    StraletWrap(PyObject* cb)
        : m_callback(cb)
    {
    }

	virtual void on_init			() override;
	virtual void on_fini			() override;
	virtual void on_quote			(shared_ptr<const MarketQuote> quote) override;
	virtual void on_bar				(const string& cycle, shared_ptr<const Bar> bar) override;
	virtual void on_order			(shared_ptr<const Order> order) override;
	virtual void on_trade			(shared_ptr<const Trade> trade) override;
	virtual void on_timer			(int64_t id, void* data) override;
	virtual void on_event			(const string& name, void* data) override;
	virtual void on_account_status	(shared_ptr<const AccountInfo> account) override;

    PyObjectHolder  m_callback;
};

PyObject* _wrap_tqs_sc_trading_day(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;

    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);
    return PyLong_FromLong(sc->trading_day());
}

PyObject* _wrap_tqs_sc_cur_time(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;

    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    DateTime dt = reinterpret_cast<StraletContext*>(h)->cur_time();

    return Py_BuildValue("ii", dt.date, dt.time);
}

PyObject* _wrap_tqs_sc_post_event(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* evt;
    int64_t data;

    if (!PyArg_ParseTuple(args, "LsL", &h, &evt, &data))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);
    sc->post_event(evt, (void*)data);

    Py_RETURN_NONE;
}

PyObject* _wrap_tqs_sc_set_timer(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    int64_t id;
    int64_t delay;
    int64_t data;

    if (!PyArg_ParseTuple(args, "LLLL", &h, &id, &delay, &data))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);
    sc->set_timer(id, delay, (void*)data);

    Py_RETURN_NONE;
}

PyObject* _wrap_tqs_sc_kill_timer(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    int64_t id;

    if (!PyArg_ParseTuple(args, "LL", &h, &id))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);
    sc->kill_timer(id);

    Py_RETURN_NONE;
}

PyObject* _wrap_tqs_sc_dapi_get(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;

    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);
    DataApi* dapi = sc->data_api();
    DataApiWrap* wrap = new DataApiWrap(dapi);

    return PyLong_FromLongLong((int64_t)(wrap));
}

PyObject* _wrap_tqs_sc_dapi_put(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;

    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");
    
    delete reinterpret_cast<DataApiWrap*>(h);
    Py_RETURN_NONE;
}

PyObject* _wrap_tqs_sc_tapi_get(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;

    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);
    TradeApi* tapi = sc->trade_api();
    TradeApiWrap* wrap = new TradeApiWrap(tapi);

    return PyLong_FromLongLong((int64_t)(wrap));
}

PyObject* _wrap_tqs_sc_tapi_put(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;

    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    delete reinterpret_cast<TradeApiWrap*>(h);
    Py_RETURN_NONE;
}

PyObject* _wrap_tqs_sc_log(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* msg;
    const char* severity;

    if (!PyArg_ParseTuple(args, "Lss", &h, &severity, &msg))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);

    if      (strcmp(severity, "INFO")   == 0)   sc->logger(INFO) << msg << endl;
    else if (strcmp(severity, "ERROR")  == 0)   sc->logger(ERROR) << msg << endl;
    else if (strcmp(severity, "WARNING")== 0)   sc->logger(WARNING) << msg << endl;
    else if (strcmp(severity, "FATAL")  == 0)   sc->logger(FATAL) << msg << endl;

    Py_RETURN_NONE;
}

PyObject* _wrap_tqs_sc_get_properties(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;

    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);
    string str = sc->get_properties();

    return Py_BuildValue("s", str.c_str());
}

PyObject* _wrap_tqs_sc_get_property(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* name;
    const char* def_value;

    if (!PyArg_ParseTuple(args, "Lss", &h, &name, &def_value))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);
    string str = sc->get_property(name, def_value);

    return Py_BuildValue("s", str.c_str());
}

PyObject* _wrap_tqs_sc_mode(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;

    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto sc = reinterpret_cast<StraletContext*>(h);
    string str = sc->mode();

    return Py_BuildValue("s", str.c_str());
}

Stralet* create_stralet_wrap(PyObject* cb)
{
    return new StraletWrap(cb);
}

PyObject* _wrap_tqs_bt_run(PyObject* self, PyObject *args, PyObject* kwargs)
{
    const char* cfg;
    PyObject* cb;
    if (!PyArg_ParseTuple(args, "sO", &cfg, &cb))
        return NULL;

    backtest::run(cfg, [cb]() { return create_stralet_wrap(cb); });

    Py_RETURN_NONE;
}

PyObject* _wrap_tqs_rt_run(PyObject* self, PyObject *args, PyObject* kwargs)
{
    const char* cfg;
    PyObject* cb;
    if (!PyArg_ParseTuple(args, "sO", &cfg, &cb))
        return NULL;

    realtime::run(cfg, [cb]() { return create_stralet_wrap(cb); });

    Py_RETURN_NONE;
}

static void call_callback(PyObject* callback, PyObject* arg)
{
    PyObject* result = PyObject_CallObject(callback, arg);

    if (PyErr_Occurred() != nullptr) {
        PyErr_Print();
        PyErr_Clear();
        throw runtime_error("Unhandled exception in Python code");
    }

    Py_XDECREF(result);
    Py_XDECREF(arg);
}

//void StraletWrap::on_event(shared_ptr<StraletEvent> evt)

void StraletWrap::on_init()
{
    auto gstate = PyGILState_Ensure();

    PyObject* arg = Py_BuildValue("iL", ON_INIT, m_ctx);
    call_callback(m_callback.obj, arg);

    PyGILState_Release(gstate);
}

void StraletWrap::on_fini()
{
    auto gstate = PyGILState_Ensure();

    PyObject* arg = Py_BuildValue("iL", ON_FINI, m_ctx);
    call_callback(m_callback.obj, arg);

    PyGILState_Release(gstate);
}

void StraletWrap::on_quote(shared_ptr<const MarketQuote> quote)
{
    auto gstate = PyGILState_Ensure();

    PyObject* arg = Py_BuildValue("iN", ON_QUOTE, convert_tick(quote.get()));
    call_callback(m_callback.obj, arg);

    PyGILState_Release(gstate);
}

void StraletWrap::on_bar(const string& cycle, shared_ptr<const Bar> bar)
{
    auto gstate = PyGILState_Ensure();

    PyObject* tmp = Py_BuildValue("sN", cycle.c_str(), convert_bar(bar.get()));
    PyObject* arg = Py_BuildValue("iN", ON_BAR, tmp);

    call_callback(m_callback.obj, arg);

    PyGILState_Release(gstate);
}

void StraletWrap::on_order(shared_ptr<const Order> order)
{
    auto gstate = PyGILState_Ensure();

    PyObject* arg = Py_BuildValue("iN", ON_ORDER, convert_order(order.get()));
    call_callback(m_callback.obj, arg);

    PyGILState_Release(gstate);
}

void StraletWrap::on_trade(shared_ptr<const Trade> trade)
{
    auto gstate = PyGILState_Ensure();

    PyObject* arg = Py_BuildValue("iN", ON_TRADE, convert_trade(trade.get()));
    call_callback(m_callback.obj, arg);

    PyGILState_Release(gstate);
}

void StraletWrap::on_timer(int64_t id, void* data)
{
    auto gstate = PyGILState_Ensure();

    PyObject* tmp = Py_BuildValue("LL", id, data);
    PyObject* arg = Py_BuildValue("iN", ON_TIMER, tmp);

    call_callback(m_callback.obj, arg);

    PyGILState_Release(gstate);
}

void StraletWrap::on_event(const string& name, void* data)
{
    auto gstate = PyGILState_Ensure();

    PyObject* tmp = Py_BuildValue("sL", name.c_str(), data);
    PyObject* arg = Py_BuildValue("iN", ON_EVENT, tmp);
    call_callback(m_callback.obj, arg);

    PyGILState_Release(gstate);
}

void StraletWrap::on_account_status(shared_ptr<const AccountInfo> account)
{
    auto gstate = PyGILState_Ensure();

    PyObject* arg = Py_BuildValue("iN", ON_ACCOUNT_STATUS, convert_account_info(account.get()));
    call_callback(m_callback.obj, arg);

    PyGILState_Release(gstate);
}

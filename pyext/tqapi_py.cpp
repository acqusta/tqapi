#include "tqapi_py.h"
#include "tquant_api.h"

using namespace tquant::api;

void call_callback(PyObject* callback, const char* evt, PyObject* data)
{
    PyObject* arg = Py_BuildValue("sN", evt, data);

    PyObject* result = PyObject_CallObject(callback, arg);

    if (PyErr_Occurred() != nullptr) {
        PyErr_Print();
        PyErr_Clear();
    }

    Py_XDECREF(result);
    Py_XDECREF(arg);
}

static PyMethodDef Methods[] = {
    { (char *)"tqapi_create",            (PyCFunction)_wrap_tqapi_create,           METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tqapi_destroy",           (PyCFunction)_wrap_tqapi_destroy,          METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tqapi_get_data_api",      (PyCFunction)_wrap_tqapi_get_data_api,     METH_KEYWORDS | METH_VARARGS, NULL },
    //{ (char *)"tqapi_get_trade_api",     (PyCFunction)_wrap_tqapi_get_trade_api,    METH_KEYWORDS | METH_VARARGS, NULL },

    { (char *)"tapi_place_order",           (PyCFunction)_wrap_tapi_place_order,            METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tapi_cancel_order",          (PyCFunction)_wrap_tapi_cancel_order,           METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tapi_query_orders",          (PyCFunction)_wrap_tapi_query_orders,           METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tapi_query_trades",          (PyCFunction)_wrap_tapi_query_trades,           METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tapi_query_positions",       (PyCFunction)_wrap_tapi_query_positions,        METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tapi_query_balance",         (PyCFunction)_wrap_tapi_query_balance,          METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tapi_set_callback",          (PyCFunction)_wrap_tapi_set_callback,           METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tapi_query",                 (PyCFunction)_wrap_tapi_query,                  METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"tapi_query_account_status",  (PyCFunction)_wrap_tapi_query_account_status,   METH_KEYWORDS | METH_VARARGS, NULL },

    { (char *)"dapi_set_callback",          (PyCFunction)_wrap_dapi_set_callback,           METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"dapi_subscribe",             (PyCFunction)_wrap_dapi_subscribe,              METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"dapi_unsubscribe",           (PyCFunction)_wrap_dapi_unsubscribe,            METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"dapi_quote",                 (PyCFunction)_wrap_dapi_quote,                  METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"dapi_bar",                   (PyCFunction)_wrap_dapi_bar,                    METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"dapi_tick",                  (PyCFunction)_wrap_dapi_tick,                   METH_KEYWORDS | METH_VARARGS, NULL },
    { (char *)"dapi_dailybar",              (PyCFunction)_wrap_dapi_dailybar,               METH_KEYWORDS | METH_VARARGS, NULL },
    { NULL, NULL, 0, NULL }
};

PyObject* _wrap_tqapi_create(PyObject* self, PyObject *args, PyObject* kwargs)
{
    const char* addr;

    if (!PyArg_ParseTuple(args, "s", (char*)&addr))
        return NULL;

    auto api = TQuantApi::create(addr);
    if (!api)
        Py_RETURN_NONE;

    auto wrap = new TQuantApiWrap(api);

    return PyLong_FromLongLong((int64_t)(wrap));
}

PyObject* _wrap_tqapi_destroy(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (h) {
        auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
        delete wrap;
    }

    Py_RETURN_TRUE;
}

PyObject* _wrap_tqapi_get_data_api(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* source;
    if (!PyArg_ParseTuple(args, "Ls", &h, (char*)&source))
        return NULL;

    if (!h) Py_RETURN_NONE;
    
    try {
        auto wrap = reinterpret_cast<TQuantApiWrap*>(h);

        DataApiWrap* dapi_wrap = wrap->data_api(source);

        return PyLong_FromLongLong((int64_t)(dapi_wrap));
    }
    catch (const std::exception& e) {
        Py_RETURN_NONE;
    }
}


PyMODINIT_FUNC API_EXPORT init_tqapi(void)
{
    PyEval_InitThreads();

    Py_InitModule("_tqapi", Methods);
}


#ifdef BUILD_API_TEST

int main(int argc, char** argv)
{
    Py_Initialize();

    init_tqapi();

    Py_Main(argc, argv);

    return 0;
}

#endif

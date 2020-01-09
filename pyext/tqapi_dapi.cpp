#include <string>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include "myutils/stringutils.h"
#include "tqapi_py.h"
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

static inline PyObject* bars_to_dataframe(const BarArray* bars);
static inline PyObject* dailybars_to_dataframe(const DailyBarArray* bars);
static inline PyObject* ticks_to_dataframe(const MarketQuoteArray* ticks);
static inline PyObject* bars_to_array(const BarArray* bars);
static inline PyObject* dailybars_to_array(const DailyBarArray* bars);
static inline PyObject* ticks_to_array(const MarketQuoteArray* ticks);


PyObject* _wrap_dapi_create(PyObject* self, PyObject *args, PyObject* kwargs)
{
    const char* addr;

    if (!PyArg_ParseTuple(args, "s", (char*)&addr))
        return NULL;

    auto api = create_data_api(addr);
    if (!api)
        Py_RETURN_NONE;

    auto wrap = new DataApiWrap(api, true);

    return PyLong_FromLongLong((int64_t)(wrap));
}

PyObject* _wrap_dapi_destroy(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (h) {
        auto wrap = reinterpret_cast<DataApiWrap*>(h);
        delete wrap;
    }

    Py_RETURN_TRUE;
}


PyObject* _wrap_dapi_set_callback(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    PyObject* cb;
    if (!PyArg_ParseTuple(args, "LO", &h, &cb))
        return NULL;

    if (h)  {
        auto wrap = reinterpret_cast<DataApiWrap*>(h);
        wrap->m_dapi_cb = cb;
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

PyObject* _wrap_dapi_subscribe(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* codes;
    if (!PyArg_ParseTuple(args, "Ls", &h, &codes))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    vector<string> ss;
    split(codes, ",", &ss);

    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    auto r = wrap->m_dapi->subscribe(ss);

    if (r.value) {
        for (auto& c : ss) {
            auto r = wrap->m_dapi->quote(c);
            if (r.value)
                wrap->on_market_quote(r.value);
        }
        stringstream ss;
        for (auto& c : *r.value) ss << c << ",";
        string sub_codes = ss.str();
        if (sub_codes.size()) sub_codes.resize(sub_codes.size() - 1);
        return Py_BuildValue("sO", sub_codes.c_str(), Py_None);
    }
    else {
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
    }    
}

PyObject* _wrap_dapi_unsubscribe(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* codes;
    if (!PyArg_ParseTuple(args, "Ls", &h, &codes))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    vector<string> ss;
    split(codes, ",", &ss);

    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    auto r = wrap->m_dapi->unsubscribe(ss);

    if (r.value) {
        stringstream ss;
        for (auto& c : *r.value) ss << c << ",";
        string sub_codes = ss.str();
        if (sub_codes.size()) sub_codes.resize(sub_codes.size() - 1);
        return Py_BuildValue("sO", sub_codes.c_str(), Py_None);
    }
    else {
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
    }
}

PyObject* _wrap_dapi_quote(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* code;

    if (!PyArg_ParseTuple(args, "Ls", &h, &code))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    auto r = wrap->m_dapi->quote(code);

    if (r.value) {
        return Py_BuildValue("NO", convert_tick(r.value.get()), Py_None);
    }
    else {
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
    }
}

PyObject* _wrap_dapi_bar(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t      h = 0;
    const char*  code = nullptr;
    int32_t      trading_day = 0;
    PyObject*    align;
    const char*  cycle = nullptr;
    PyObject*    df_value;
    int32_t      number = 0;

    if (!PyArg_ParseTuple(args, "LssiOOi",
        &h, &code, &cycle, &trading_day, &align, &df_value, &number))
    {
        return NULL;
    }

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");
    if (!code || !strlen(code)) return Py_BuildValue("Os", Py_None, "empty code");
    if (!cycle || !strlen(cycle)) return Py_BuildValue("Os", Py_None, "empty cycle");

    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    auto r = wrap->m_dapi->bar(code, cycle, trading_day, PyObject_IsTrue(align)!=0, number);

    if (r.value) {
        if (PyObject_IsTrue(df_value))
            return Py_BuildValue("NO", bars_to_dataframe(r.value.get()), Py_None);
        else
            return Py_BuildValue("NO", bars_to_array(r.value.get()), Py_None);
    }
    else {
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
    }
}

PyObject* _wrap_dapi_dailybar(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h = 0;
    const char* code = nullptr;
    PyObject* align = nullptr;
    const char* price_adj = nullptr;
    PyObject*    df_value;
    int32_t number = 0;

    if (!PyArg_ParseTuple(args, "LssOOi",
        &h, &code, &price_adj, &align, &df_value, &number))
    {
        return NULL;
    }

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");
    if (!code || !strlen(code)) return Py_BuildValue("Os", Py_None, "empty code");
    if (!price_adj) return Py_BuildValue("Os", Py_None, "null price_adj");

    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    auto r = wrap->m_dapi->daily_bar(code, price_adj, PyObject_IsTrue(align)!=0, number);

    if (r.value) {
        if (PyObject_IsTrue(df_value))
            return Py_BuildValue("NO", dailybars_to_dataframe(r.value.get()), Py_None);
        else
            return Py_BuildValue("NO", dailybars_to_array(r.value.get()), Py_None);
    }
    else {
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
    }
}

PyObject* _wrap_dapi_tick(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h = 0;
    const char* code;
    int32_t trading_day = 0;
    PyObject*    df_value;
    int32_t number = 0;

    if (!PyArg_ParseTuple(args, "LsiOi",
        &h, &code, &trading_day, &df_value, &number))
    {
        return NULL;
    }

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");
    if (!code || strlen(code)==0) return Py_BuildValue("Os", Py_None, "empty code");

    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    auto r = wrap->m_dapi->tick(code, trading_day, number);

    if (r.value) {

        if (PyObject_IsTrue(df_value))
            return Py_BuildValue("NO", ticks_to_dataframe(r.value.get()), Py_None);
        else
            return Py_BuildValue("NO", ticks_to_array(r.value.get()), Py_None);
    }
    else {
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
    }
}

// DataApi_Callback
void DataApiWrap::on_market_quote(shared_ptr<const MarketQuote> quote)
{
    if (m_dapi_cb.obj == Py_None) return;

    msg_loop().post_task([this, quote]() {
        auto gstate = PyGILState_Ensure();

        if (m_dapi_cb.obj != Py_None) {
            PyObject* obj = convert_tick(quote.get());
            call_callback(this->m_dapi_cb.obj, "dapi.quote", obj);
            PyGILState_Release(gstate);
        }
    });
}

void DataApiWrap::on_bar(const string& cycle, shared_ptr<const Bar> bar)
{
    if (m_dapi_cb.obj != Py_None) return;

    msg_loop().post_task([this, cycle, bar]() {
        auto gstate = PyGILState_Ensure();
        if (m_dapi_cb.obj != Py_None) {
            PyObject *obj = Py_BuildValue("sN", cycle.c_str(), convert_bar(bar.get()));
            call_callback(this->m_dapi_cb.obj, "dapi.bar", obj);
            PyGILState_Release(gstate);
        }
    });
}

PyObject* convert_tick(const RawMarketQuote* q)
{
    PyObject* obj = PyDict_New();

    dict_set_item(obj, "code"           , q->code);
    dict_set_item(obj, "date"           , q->date);
    dict_set_item(obj, "time"           , q->time);
    dict_set_item(obj, "recv_time"      , q->recv_time);
    dict_set_item(obj, "trading_day"    , q->trading_day);
    dict_set_item(obj, "open"           , q->open);
    dict_set_item(obj, "high"           , q->high);
    dict_set_item(obj, "low"            , q->low);
    dict_set_item(obj, "close"          , q->close);
    dict_set_item(obj, "last"           , q->last);
    dict_set_item(obj, "high_limit"     , q->high_limit);
    dict_set_item(obj, "low_limit"      , q->low_limit);
    dict_set_item(obj, "pre_close"      , q->pre_close);
    dict_set_item(obj, "volume"         , q->volume);
    dict_set_item(obj, "turnover"       , q->turnover);
    dict_set_item(obj, "ask1"           , q->ask1);
    dict_set_item(obj, "ask2"           , q->ask2);
    dict_set_item(obj, "ask3"           , q->ask3);
    dict_set_item(obj, "ask4"           , q->ask4);
    dict_set_item(obj, "ask5"           , q->ask5);
    dict_set_item(obj, "bid1"           , q->bid1);
    dict_set_item(obj, "bid2"           , q->bid2);
    dict_set_item(obj, "bid3"           , q->bid3);
    dict_set_item(obj, "bid4"           , q->bid4);
    dict_set_item(obj, "bid5"           , q->bid5);
    dict_set_item(obj, "ask_vol1"       , q->ask_vol1);
    dict_set_item(obj, "ask_vol2"       , q->ask_vol2);
    dict_set_item(obj, "ask_vol3"       , q->ask_vol3);
    dict_set_item(obj, "ask_vol4"       , q->ask_vol4);
    dict_set_item(obj, "ask_vol5"       , q->ask_vol5);
    dict_set_item(obj, "bid_vol1"       , q->bid_vol1);
    dict_set_item(obj, "bid_vol2"       , q->bid_vol2);
    dict_set_item(obj, "bid_vol3"       , q->bid_vol3);
    dict_set_item(obj, "bid_vol4"       , q->bid_vol4);
    dict_set_item(obj, "bid_vol5"       , q->bid_vol5);
    dict_set_item(obj, "settle"         , q->settle);
    dict_set_item(obj, "pre_settle"     , q->pre_settle);
    dict_set_item(obj, "oi"             , q->oi);
    dict_set_item(obj, "pre_oi"         , q->pre_oi);

    return obj;
}


#define to_pyarray(_dict, _objs, _len, _field, _type, _np_type)         \
    PyObject* _field = PyArray_SimpleNew(1, _len, _np_type);            \
    _type* p_##_field = (_type*)PyArray_DATA((PyArrayObject*)_field);   \
    for (size_t i = 0; i < _objs->size(); i++)                          \
        *p_##_field++ = _objs->at(i)._field;                            \
    dict_set_item(_dict, # _field, _field);


PyObject* ticks_to_dataframe(const MarketQuoteArray* ticks)
{

    import_array1(nullptr);

    PyObject* dict = PyDict_New();
    npy_intp array_len[] = { (npy_intp)ticks->size(), 0 };

    to_pyarray(dict, ticks, array_len, date          ,  int32_t, NPY_INT    );
    to_pyarray(dict, ticks, array_len, time          ,  int32_t, NPY_INT    );
    to_pyarray(dict, ticks, array_len, recv_time     ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, trading_day   ,  int32_t, NPY_INT    );
    to_pyarray(dict, ticks, array_len, open          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, high          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, low           ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, close         ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, last          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, high_limit    ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, low_limit     ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, pre_close     ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, volume        ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, turnover      ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, ask1          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, ask2          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, ask3          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, ask4          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, ask5          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, bid1          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, bid2          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, bid3          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, bid4          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, bid5          ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, ask_vol1      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, ask_vol2      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, ask_vol3      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, ask_vol4      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, ask_vol5      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, bid_vol1      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, bid_vol2      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, bid_vol3      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, bid_vol4      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, bid_vol5      ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, settle        ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, pre_settle    ,  double,  NPY_DOUBLE );
    to_pyarray(dict, ticks, array_len, oi            ,  int64_t, NPY_INT64  );
    to_pyarray(dict, ticks, array_len, pre_oi        ,  int64_t, NPY_INT64  );

    // TODO: 1 don't return code, 2. only create one object for each code
    std::vector<PyObject*> code(ticks->size());
    for (size_t k = 0; k < ticks->size(); k++)
        code[k] = PyString_FromString(ticks->at(k).code);

    PyObject* py_code = PyArray_SimpleNew(1, array_len, NPY_OBJECT);
    if (code.size())
        memcpy(PyArray_DATA((PyArrayObject*)py_code), &code[0], sizeof(PyObject*)*code.size());
    dict_set_item(dict, "code", py_code);

    return dict;
}

PyObject* ticks_to_array(const MarketQuoteArray* ticks)
{
    PyObject* arr = PyList_New(ticks->size());

    for (size_t k = 0; k < ticks->size(); k++) {
        PyObject* obj = convert_tick(&ticks->at(k));
        PyList_SetItem(arr, k, obj);
    }

    return arr;
}

PyObject* convert_bar(const RawBar* b)
{
    PyObject* obj = PyDict_New();

    dict_set_item(obj, "code"           , b->code);
    dict_set_item(obj, "date"           , b->date);
    dict_set_item(obj, "time"           , b->time);
    dict_set_item(obj, "trading_day"    , b->trading_day);
    dict_set_item(obj, "open"           , b->open);
    dict_set_item(obj, "high"           , b->high);
    dict_set_item(obj, "low"            , b->low);
    dict_set_item(obj, "close"          , b->close);
    dict_set_item(obj, "volume"         , b->volume);
    dict_set_item(obj, "turnover"       , b->turnover);
    dict_set_item(obj, "oi"             , b->oi);

    return obj;
}

PyObject* bars_to_dataframe(const BarArray* bars)
{
    import_array1(nullptr);

    PyObject* dict = PyDict_New();
    npy_intp array_len[] = { (npy_intp)bars->size(), 0 };

    to_pyarray(dict, bars, array_len, date,         int32_t, NPY_INT);
    to_pyarray(dict, bars, array_len, time,         int32_t, NPY_INT);
    to_pyarray(dict, bars, array_len, trading_day,  int32_t, NPY_INT);
    to_pyarray(dict, bars, array_len, open,         double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, high,         double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, low,          double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, close,        double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, volume,       int64_t, NPY_INT64);
    to_pyarray(dict, bars, array_len, turnover,     double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, oi,           int64_t, NPY_INT64);

    // TODO: 1 don't return code, 2. only create one object for each code

    std::vector<PyObject*> code(bars->size());
    for (size_t k = 0; k < bars->size(); k++)
        code[k] = PyString_FromString(bars->at(k).code);

    PyObject* py_code = PyArray_SimpleNew(1, array_len, NPY_OBJECT);
    if (code.size())
        memcpy(PyArray_DATA((PyArrayObject*)py_code), &code[0], sizeof(PyObject*)*code.size());
    dict_set_item(dict, "code", py_code);

    return dict;
}

PyObject* bars_to_array(const BarArray* bars)
{
    PyObject* arr = PyList_New(bars->size());

    for (size_t k = 0; k < bars->size(); k++) {
        PyObject* obj = convert_bar(&bars->at(k));
        PyList_SetItem(arr, k, obj);
    }

    return arr;
}

static inline PyObject* convert_dailybar(RawDailyBar* b)
{
    PyObject* obj = PyDict_New();

    dict_set_item(obj, "code"           , b->code);
    dict_set_item(obj, "date"           , b->date);
    dict_set_item(obj, "open"           , b->open);
    dict_set_item(obj, "high"           , b->high);
    dict_set_item(obj, "low"            , b->low);
    dict_set_item(obj, "close"          , b->close);
    dict_set_item(obj, "volume"         , b->volume);
    dict_set_item(obj, "turnover"       , b->turnover);
    dict_set_item(obj, "oi"             , b->oi);
    dict_set_item(obj, "settle"         , b->settle);
    dict_set_item(obj, "pre_close"      , b->pre_close);
    dict_set_item(obj, "pre_settle"     , b->pre_settle);

    return obj;
}

PyObject* dailybars_to_dataframe(const DailyBarArray* bars)
{
    import_array1(nullptr);

    PyObject* dict = PyDict_New();
    npy_intp array_len[] = { (npy_intp)bars->size(), 0 };

    to_pyarray(dict, bars, array_len, date,         int32_t, NPY_INT);
    to_pyarray(dict, bars, array_len, open,         double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, high,         double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, low,          double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, close,        double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, volume,       int64_t, NPY_INT64);
    to_pyarray(dict, bars, array_len, turnover,     double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, oi,           int64_t, NPY_INT64);
    to_pyarray(dict, bars, array_len, settle,       double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, pre_close,    double,  NPY_DOUBLE);
    to_pyarray(dict, bars, array_len, pre_settle,   double,  NPY_DOUBLE);

    // TODO: 1 don't return code, 2. only create one object for each code
    std::vector<PyObject*> code(bars->size());
    for (size_t k = 0; k < bars->size(); k++)
        code[k] = PyString_FromString(bars->at(k).code);

    PyObject* py_code = PyArray_SimpleNew(1, array_len, NPY_OBJECT);
    if (code.size())
        memcpy(PyArray_DATA((PyArrayObject*)py_code), &code[0], sizeof(PyObject*)*code.size());
    dict_set_item(dict, "code", py_code);

    return dict;
}

PyObject* dailybars_to_array(const DailyBarArray* bars)
{
    PyObject* arr = PyList_New(bars->size());

    for (size_t k = 0; k < bars->size(); k++) {
        PyObject* obj = convert_dailybar(&bars->at(k));
        PyList_SetItem(arr, k, obj);
    }

    return arr;
}

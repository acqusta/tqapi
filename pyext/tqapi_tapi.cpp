#include "tqapi_py.h"

#include "myutils/stringutils.h"

PyObject* convert_balance(const Balance* bal)
{
    PyObject* obj = PyDict_New();
    
    dict_set_item (obj, "account_id"     , bal->account_id);
    dict_set_item (obj, "fund_account"   , bal->fund_account);
    dict_set_item (obj, "init_balance"   , bal->init_balance);
    dict_set_item (obj, "enable_balance" , bal->enable_balance);
    dict_set_item (obj, "margin"         , bal->margin);
    dict_set_item (obj, "float_pnl"      , bal->float_pnl);
    dict_set_item (obj, "close_pnl"      , bal->close_pnl);
    
    return obj;
}

PyObject* convert_order(const Order* order)
{
    PyObject* obj = PyDict_New();

    dict_set_item (obj, "account_id"     , order->account_id       );
    dict_set_item (obj, "code"           , order->code             );
    dict_set_item (obj, "name"           , order->name             );
    dict_set_item (obj, "entrust_no"     , order->entrust_no       );
    dict_set_item (obj, "entrust_action" , order->entrust_action   );
    dict_set_item (obj, "entrust_price"  , order->entrust_price    );
    dict_set_item (obj, "entrust_size"   , order->entrust_size     );
    dict_set_item (obj, "entrust_date"   , order->entrust_date     );
    dict_set_item (obj, "entrust_time"   , order->entrust_time     );
    dict_set_item (obj, "fill_price"     , order->fill_price       );
    dict_set_item (obj, "fill_size"      , order->fill_size        );
    dict_set_item (obj, "status"         , order->status           );
    dict_set_item (obj, "status_msg"     , order->status_msg       );
    dict_set_item (obj, "order_id"       , order->order_id         );

    return obj;
}

PyObject* convert_trade(const Trade* trade)
{
    PyObject* obj = PyDict_New();

    dict_set_item (obj, "account_id"      , trade->account_id     );
    dict_set_item (obj, "code"            , trade->code           );
    dict_set_item (obj, "name"            , trade->name           );
    dict_set_item (obj, "entrust_no"      , trade->entrust_no     );
    dict_set_item (obj, "entrust_action"  , trade->entrust_action );
    dict_set_item (obj, "fill_no"         , trade->fill_no        );
    dict_set_item (obj, "fill_size"       , trade->fill_size      );
    dict_set_item (obj, "fill_price"      , trade->fill_price     );
    dict_set_item (obj, "fill_date"       , trade->fill_date      );
    dict_set_item (obj, "fill_time"       , trade->fill_time      );

    return obj;
}

PyObject* convert_position(const Position* pos)
{
    PyObject* obj = PyDict_New();

    dict_set_item (obj, "account_id"   , pos->account_id  );
    dict_set_item (obj, "code"         , pos->code        );
    dict_set_item (obj, "name"         , pos->name        );
    dict_set_item (obj, "current_size" , pos->current_size);
    dict_set_item (obj, "enable_size"  , pos->enable_size );
    dict_set_item (obj, "init_size"    , pos->init_size   );
    dict_set_item (obj, "today_size"   , pos->today_size  );
    dict_set_item (obj, "frozen_size"  , pos->frozen_size );
    dict_set_item (obj, "side"         , pos->side        );
    dict_set_item (obj, "cost"         , pos->cost        );
    dict_set_item (obj, "cost_price"   , pos->cost_price  );
    dict_set_item (obj, "last_price"   , pos->last_price  );
    dict_set_item (obj, "float_pnl"    , pos->float_pnl   );
    dict_set_item (obj, "close_pnl"    , pos->close_pnl   );
    dict_set_item (obj, "margin"       , pos->margin      );
    dict_set_item (obj, "commission"   , pos->commission  );

    return obj;
}

PyObject* convert_account_info(const AccountInfo* account)
{
    PyObject* obj = PyDict_New();

    dict_set_item (obj, "account_id"   , account->account_id  );
    dict_set_item (obj, "broker"       , account->broker      );
    dict_set_item (obj, "account"      , account->account     );
    dict_set_item (obj, "status"       , account->status      );
    dict_set_item (obj, "msg"          , account->msg         );
    dict_set_item (obj, "account_type" , account->account_type);

    return obj;
}

PyObject* convert_orders(const vector<Order>* orders)
{
    PyObject* list = PyList_New(orders->size());
    for (size_t i = 0; i < orders->size(); i++)
        PyList_SetItem(list, i, convert_order(&orders->at(i)));
    return list;
}

PyObject* convert_trades(const vector<Trade>* trades)
{
    PyObject* list = PyList_New(trades->size());
    for (size_t i = 0; i < trades->size(); i++)
        PyList_SetItem(list, i, convert_trade(&trades->at(i)));
    return list;
}

PyObject* convert_positions(const vector<Position>* positions)
{
    PyObject* list = PyList_New(positions->size());
    for (size_t i = 0; i < positions->size(); i++)
        PyList_SetItem(list, i, convert_position(&positions->at(i)));
    return list;
}

PyObject* convert_account_status_list(const vector<AccountInfo>* accounts)
{
    PyObject* list = PyList_New(accounts->size());
    for (size_t i = 0; i < accounts->size(); i++)
        PyList_SetItem(list, i, convert_account_info(&accounts->at(i)));
    return list;
}

PyObject* _wrap_tapi_create(PyObject* self, PyObject *args, PyObject* kwargs)
{
    const char* addr;

    if (!PyArg_ParseTuple(args, "s", (char*)&addr))
        return NULL;

    auto api = create_trade_api(addr);
    if (!api)
        Py_RETURN_NONE;

    auto wrap = new TradeApiWrap(api, true);

    return PyLong_FromLongLong((int64_t)(wrap));
}

PyObject* _wrap_tapi_destroy(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (h) {
        auto wrap = reinterpret_cast<TradeApiWrap*>(h);
        delete wrap;
    }

    Py_RETURN_TRUE;
}

PyObject* _wrap_tapi_place_order(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* account_id;
    const char* code;
    double price;
    int64_t size;
    const char* action;
    const char* price_type;
    int64_t order_id;

    if (!PyArg_ParseTuple(args, "LssdLssL", &h, &account_id, &code, &price, &size, &action, &price_type, &order_id))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto wrap = reinterpret_cast<TradeApiWrap*>(h);
    auto r = wrap->m_tapi->place_order(account_id, code, price, size, action, price_type, (int32_t)order_id);

    if (r.value) {
        auto ordid = PyDict_New();
        dict_set_item(ordid, "entrust_no", r.value->entrust_no.c_str());
        dict_set_item(ordid, "order_id", r.value->order_id);
        return Py_BuildValue("NO", ordid, Py_None);
    }
    else {
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
    }
}
PyObject* _wrap_tapi_cancel_order(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* account_id;
    const char* code;
    int64_t order_id;
    const char* entrust_no;

    if (!PyArg_ParseTuple(args, "LsssL", &h, &account_id, &code, &entrust_no, &order_id))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto wrap = reinterpret_cast<TradeApiWrap*>(h);
    CallResult<bool> r("");
    if (order_id)
        r = wrap->m_tapi->cancel_order(account_id, code, (int32_t)order_id);
    else if (strlen(entrust_no))
        r = wrap->m_tapi->cancel_order(account_id, code, entrust_no);
    else
        r = CallResult<bool>("empty entrust_no and order_id");

    if (r.value)
        return Py_BuildValue("OO", PyBool_FromLong(*r.value), Py_None);
    else
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
}


PyObject* _wrap_tapi_query_orders(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* account_id;
    const char* codes;

    if (!PyArg_ParseTuple(args, "Lss", &h, &account_id, &codes))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto wrap = reinterpret_cast<TradeApiWrap*>(h);
    
    auto r = wrap->m_tapi->query_orders(account_id, codes);

    if (r.value)
        return Py_BuildValue("NO", convert_orders(r.value.get()), Py_None);
    else
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
}

PyObject* _wrap_tapi_query_trades(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* account_id;
    const char* codes;

    if (!PyArg_ParseTuple(args, "Lss", &h, &account_id, &codes))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto wrap = reinterpret_cast<TradeApiWrap*>(h);

    auto r = wrap->m_tapi->query_trades(account_id, codes);

    if (r.value)
        return Py_BuildValue("NO", convert_trades(r.value.get()), Py_None);
    else
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
}

PyObject* _wrap_tapi_query_positions(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* account_id;
    const char* codes;

    if (!PyArg_ParseTuple(args, "Lss", &h, &account_id, &codes))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto wrap = reinterpret_cast<TradeApiWrap*>(h);

    auto r = wrap->m_tapi->query_positions(account_id, codes);

    if (r.value)
        return Py_BuildValue("NO", convert_positions(r.value.get()), Py_None);
    else
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
}

PyObject* _wrap_tapi_query_balance(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* account_id;

    if (!PyArg_ParseTuple(args, "Ls", &h, &account_id))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto wrap = reinterpret_cast<TradeApiWrap*>(h);

    auto r = wrap->m_tapi->query_balance(account_id);

    if (r.value)
        return Py_BuildValue("NO", convert_balance(r.value.get()), Py_None);
    else
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
}

PyObject* _wrap_tapi_set_callback(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    PyObject* cb;
    if (!PyArg_ParseTuple(args, "LO", &h, &cb))
        return NULL;

    if (h) {
        auto wrap = reinterpret_cast<TradeApiWrap*>(h);
        wrap->m_tapi_cb = cb;
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

PyObject* _wrap_tapi_query(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;
    const char* account_id;
    const char* cmd;
    const char* params;

    if (!PyArg_ParseTuple(args, "Lsss", &h, &account_id, &cmd, &params))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto wrap = reinterpret_cast<TradeApiWrap*>(h);

    auto r = wrap->m_tapi->query(account_id, cmd, params);

    if (r.value)
        return Py_BuildValue("sO", r.value->c_str() , Py_None);
    else
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
}

PyObject* _wrap_tapi_query_account_status(PyObject* self, PyObject *args, PyObject* kwargs)
{
    int64_t h;

    if (!PyArg_ParseTuple(args, "L", &h))
        return NULL;

    if (!h) return Py_BuildValue("Os", Py_None, "null handle");

    auto wrap = reinterpret_cast<TradeApiWrap*>(h);

    auto r = wrap->m_tapi->query_account_status();

    if (r.value)
        return Py_BuildValue("NO", convert_account_status_list(r.value.get()), Py_None);
    else
        return Py_BuildValue("Os", Py_None, r.msg.c_str());
}


// TradeApi_Callback
void TradeApiWrap::on_order_status(shared_ptr<Order> order)
{
    if (m_tapi_cb.obj == Py_None) return;

    m_msg_loop.post_task([this, order]() {
        auto gstate = PyGILState_Ensure();
        PyObject* obj = convert_order(order.get());
        call_callback(this->m_tapi_cb.obj, "tapi.order_status_ind", obj);
        PyGILState_Release(gstate);
    });
}

void TradeApiWrap::on_order_trade(shared_ptr<Trade> trade)
{
    if (m_tapi_cb.obj == Py_None) return;

    m_msg_loop.post_task([this, trade]() {
        auto gstate = PyGILState_Ensure();
        PyObject* obj = convert_trade(trade.get());
        call_callback(this->m_tapi_cb.obj, "tapi.order_trade_ind", obj);
        PyGILState_Release(gstate);
    });
}

void TradeApiWrap::on_account_status(shared_ptr<AccountInfo> account)
{
    if (m_tapi_cb.obj == Py_None) return;

    m_msg_loop.post_task([this, account]() {
        auto gstate = PyGILState_Ensure();
        PyObject* obj = convert_account_info(account.get());
        call_callback(this->m_tapi_cb.obj, "tapi.account_status_ind", obj);
        PyGILState_Release(gstate);
    });

}

import _tqapi
import pandas as pd
import traceback
import threading
import json
import datetime as dt

def _to_date(row):
    date = int(row['date'])
    return pd.datetime(year=date // 10000, month=date // 100 % 100, day=date % 100)

def _to_datetime(row):
    date = int(row['date'])
    time = int(row['time']) // 1000
    return pd.datetime(year=date // 10000, month=date // 100 % 100, day=date % 100,
                       hour=time // 10000, minute=time // 100 % 100, second=time % 100)
def _add_index(df):
    if 'time' in df.columns:
        df.index = df.apply(_to_datetime, axis=1)
    elif 'date' in df.columns:
        df.index = df.apply(_to_date, axis=1)    
    return df

class TradeApi:
    
    def __init__(self, addr):
        if type(addr) is str:
            self._handle = _tqapi.tapi_create(addr)
            self._on_order_status   = None
            self._on_order_trade    = None
            self._on_account_status = None
            self._tqapi_created = True

        elif type(addr) is int or type(addr) is long:
            self._handle = addr
            self._on_order_status   = None
            self._on_order_trade    = None
            self._on_account_status = None
            self._tqapi_created = False

    def __del__(self):
        if self._handle and self._tqapi_created:
            _tqapi.tapi_destroy(self._handle)

    def _on_callback(self, method, data):
        if method == "tapi.order_status_ind":
            if self._on_order_status:
                self._on_order_status(data)

        elif method == "tapi.order_trade_ind":
            if self._on_order_trade:
                self._on_order_trade(data)
            
        elif method == "tapi.account_status_ind":
            if self._on_account_status:
                self._on_account_status(data)

    def set_on_order_status(self, callback):
        """Set on_order_status callback"""
        self._on_order_status = callback
   
    def set_on_order_trade(self, callback):
        """Set on_order_trade callback."""
        self._on_order_trade = callback

    def set_on_account_status(self, callback):
        """Set on_account_status callback"""
        self._on_account_status = callback

    def account_status(self):
        """Get trade account connection status"""
        return _tqapi.tapi_query_account_status(self._handle)

    def query_balance(self, account_id):
        """Get balance of one account."""
        return _tqapi.tapi_query_balance(self._handle, str(account_id))

    def query_trades(self, account_id, codes="", to_dataframe=True):
        """Get trades of one account."""
        if type(codes) is tuple or type(codes) is list:
            codes = ",".join(codes)

        v, msg = _tqapi.tapi_query_trades(self._handle, str(account_id), str(codes))
        if v is not None:
            return pd.DataFrame(v) if to_dataframe else v, msg
        else:
            return (None, msg)

    def query_orders(self, account_id, codes="", to_dataframe=True):
        """Get orders of one account."""
        if type(codes) is tuple or type(codes) is list:
            codes = ",".join(codes)

        v, msg = _tqapi.tapi_query_orders(self._handle, str(account_id), str(codes))
        if v is not None:
            return pd.DataFrame(v) if to_dataframe else v, msg
        else:
            return (None, msg)

    def query_positions(self, account_id, codes="", to_dataframe=True):
        """Get positions of one account."""
        if type(codes) is tuple or type(codes) is list:
            codes = ",".join(codes)

        v, msg = _tqapi.tapi_query_positions(self._handle, str(account_id), str(codes))
        if v is not None:
            return pd.DataFrame(v) if to_dataframe else v, msg
        else:
            return (None, msg)
    
    def place_order(self, account_id, code, price, size, action, price_type="", order_id=0):
        """Place an order and return entrust_no"""
        return _tqapi.tapi_place_order(self._handle, str(account_id), str(code),
                                       float(price), long(size), str(action),
                                       str(price_type), int(order_id))
            
    def cancel_order(self, account_id, code, entrust_no="", order_id=0):
        """Canel order"""
        return _tqapi.tapi_cancel_order(self._handle, account_id, code, entrust_no, order_id)

    def query(self, account_id, command, params=""):
        """common query"""
        return _tqapi.tapi_query(self._handle, str(account_id), str(command), str(params))

class DataApi:
    def __init__(self, addr):
        if type(addr) is str:
            self._handle = _tqapi.dapi_create(addr)
            self._on_quote = None
            self._on_bar = None
            self._tqapi_created = True
            _tqapi.dapi_set_callback(self._handle, self._on_callback)

        elif type(addr) is int or type(addr) is long:
            self._handle   = addr
            self._on_quote = None
            self._on_bar   = None
            self._tqapi_created = False

    def __del__(self):
        if self._handle and self._tqapi_created:
                _tqapi.dapi_destroy(self._handle)

    def _on_callback(self, method, data):
        try:
            if method == "dapi.quote" :
                if self._on_quote :
                    self._on_quote(data)
            elif method == "dapi.bar" :
                if self._on_bar:
                    self._on_bar(data["cycle"], data["bar"])
        except Exception as e:
            traceback.print_exc()

    def set_on_quote(self, func):
        """Set on_quote callback"""
        self._on_quote = func

    def set_on_bar(self, func):
        """Set on_bar callback"""
        self._on_bar = func

    def subscribe(self, codes) :
        if type(codes) is tuple or type(codes) is list:
            codes = ",".join(codes)

        return _tqapi.dapi_subscribe(self._handle, str(codes) )

    def unsubscribe(self, codes):
        if type(codes) is tuple or type(codes) is list:
            codes = ",".join(codes)            

        return _tqapi.dapi_unsubscribe(self._handle, str(codes))

    #def _my_callback(self, event, id, data):
    #    cb = self._callback_map.get(event)
    #    if cb:
    #        cb(id, data)
    #

    def quote(self, code):
        return _tqapi.dapi_quote(self._handle, str(code))

    def bar(self, code, cycle="1m", trading_day=0, align=True, index=False):
        v, msg = _tqapi.dapi_bar(self._handle, str(code), str(cycle), int(trading_day), bool(align))
        if v:
            df = pd.DataFrame(v)
            if df is not None and index:
                df = _add_index(df)
            return (df, msg)
        else:
            return (v, msg)

    def daily_bar(self, code, price_adj="", align=True, index=False):
        v, msg = _tqapi.dapi_dailybar(self._handle, str(code), str(price_adj), bool(align))
        if v:
            df = pd.DataFrame(v)
            if df is not None and index:
                df = _add_index(df)
            return (df, msg)
        else:
            return (v, msg)

    def tick(self, code, trading_day=0, index=False):
        v, msg = _tqapi.dapi_tick(self._handle, str(code), int(trading_day))
        if v:
            df = pd.DataFrame(v)
            if df is not None and index:
                df = _add_index(df)
            return (df, msg)
        else:
            return (v, msg)

def set_params(key, value):
    _tqapi.set_params(str(key), str(value))



class StraletEvent:
    ZERO_ID             = 0
    ON_INIT             = 1
    ON_FINI             = 2
    ON_QUOTE            = 3
    ON_BAR              = 4
    ON_TIMER            = 5
    ON_EVENT            = 6
    ON_ORDER            = 7
    ON_TRADE            = 8
    ON_ACCOUNT_STATUS   = 9

class StraletContext:
    def __init__(self, handle):
        self._handle    = handle
        self._data_api  = DataApi(_tqapi.tqs_sc_dapi_get(self._handle))
        self._trade_api = TradeApi(_tqapi.tqs_sc_tapi_get(self._handle))

    def __del__(self):
        _tqapi.tqs_sc_dapi_put(self._data_api._handle)
        _tqapi.tqs_sc_dapi_put(self._trade_api._handle)


    @property
    def trading_day(self):
        return _tqapi.tqs_sc_trading_day(self._handle)

    @property
    def mode(self):
        return _tqapi.tqs_sc_mode(self._handle)

    @property
    def cur_datetime(self):
        date, time = _tqapi.tqs_sc_cur_time(self._handle)
        y = date // 10000
        m = (date // 100) % 100
        d = date % 100
        MS = time % 1000
        time //= 1000
        H = time // 10000
        M = (time // 100) % 100
        S = time % 100

        return dt.datetime(y,m,d, H,M,S,MS)


    @property
    def cur_fin_time(self):
        return _tqapi.tqs_sc_cur_time(self._handle)

    def post_event(self, name, data):
        # FIXME:
        #  data should be number!
        return _tqapi.tqs_sc_post_event(self._handle, name, data)

    def set_timer(self, id, delay, data):
        """
        virtual void set_timer (int64_t id, int64_t delay, void* data) = 0;
        """
        return _tqapi.tqs_sc_set_timer(self._handle, id, delay, data)

    def kill_timer(self, id):
        return _tqapi.tqs_sc_kill_timer(self, id)

    @property
    def data_api(self):
        return self._data_api

    @property
    def trade_api(self):
        return self._trade_api

    def get_property(self, name, def_value=None):
        return _tqapi.tqs_sc_get_property(self, name, def_value)

    def log(self, severity, msg):
        return _tqapi.tqs_sc_log(self._handle, severity, msg)

class Stralet:

    class Logger:
        def __init__(self, ctx):
            self._ctx = ctx

        def _str_arg(self, arg): 
            return " ".join( [ str(a) for a in arg])

        def info (self, *arg):  self._ctx.log('INFO',    self._str_arg(arg))
        def error(self, *arg):  self._ctx.log('ERROR',   self._str_arg(arg))
        def warn (self, *arg):  self._ctx.log('WARNING', self._str_arg(arg))
        def info (self, *arg):  self._ctx.log('FATAL',   self._str_arg(arg))

    def __init__(self, ctx):
        self._ctx = ctx
        self._logger = Stralet.Logger(ctx)

    def __del__(self):
        pass

    @property
    def ctx(self):
        return self._ctx

    @property
    def data_api(self):
        return self._ctx.data_api

    @property
    def trade_api(self):
        return self._ctx.trade_api

    @property
    def logger(self):
        return self._logger

    def on_init(self):  pass
    def on_fini(self):  pass
    def on_quote(self, quote): pass
    def on_bar(self, cycle, bar): pass
    def on_timer(self, id, data): pass
    def on_event(self, name, data): pass
    def on_trade(self, trade): pass
    def on_order(self, order): pass
    def on_account_status(self, account_status): pass


class StraletWrap:
    def __init__(self, StraletClass):
        self._stralet_class = StraletClass
        self._strallet = None


    def _stralet_callback(self, evt, data):
        #evt, data = arg
        if evt == StraletEvent.ON_INIT:
            self._stralet = self._stralet_class(StraletContext(data))
            self._evt_map = {
                StraletEvent.ON_QUOTE:      self._stralet.on_quote,
                StraletEvent.ON_BAR:        self._stralet.on_bar,
                StraletEvent.ON_ORDER:      self._stralet.on_order,
                StraletEvent.ON_TRADE:      self._stralet.on_trade,
                StraletEvent.ON_TIMER:      self._stralet.on_timer,
                StraletEvent.ON_EVENT:      self._stralet.on_event,
                StraletEvent.ON_ACCOUNT_STATUS:    self._stralet.on_account_status
            }

            self._stralet.on_init()

        elif evt == StraletEvent.ON_FINI:
            self._stralet.on_fini()
            self._stralet = None
            self._evt_map = {}

        elif evt == StraletEvent.ON_BAR:
            self._stralet.on_bar(data[0], data[1])

        else:
            self._evt_map[evt](data)

def bt_run(cfg, StraletClass):
    if type(cfg) is not str:
        cfg = json.dumps(cfg)

    wrap = StraletWrap(StraletClass)
    _tqapi.tqs_bt_run(cfg, wrap._stralet_callback)

def rt_run(cfg, StraletClass):
    if type(cfg) is not str:
        cfg = json.dumps(cfg)

    wrap = StraletWrap(StraletClass)
    _tqapi.tqs_rt_run(cfg, wrap._stralet_callback)

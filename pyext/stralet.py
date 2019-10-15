from . import _tqapi
import datetime as dt
import json

from .tqapi import DataApi, TradeApi


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
        _tqapi.tqs_sc_tapi_put(self._trade_api._handle)


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
        tmp = _tqapi.tqs_sc_cur_time(self._handle)
        return {'date': tmp[0], 'time': tmp[1]}

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
        return _tqapi.tqs_sc_kill_timer(self._handle, id)

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

    def stop(self):
        _tqapi.tqs_sc_stop(self._handle)

class Stralet:

    class Logger:
        def __init__(self, ctx):
            self._ctx = ctx

        def _str_arg(self, arg): 
            return " ".join( [ str(a) for a in arg])

        def info (self, *arg):  self._ctx.log('INFO',    self._str_arg(arg))
        def error(self, *arg):  self._ctx.log('ERROR',   self._str_arg(arg))
        def warn (self, *arg):  self._ctx.log('WARNING', self._str_arg(arg))
        def fatal(self, *arg):  self._ctx.log('FATAL',   self._str_arg(arg))

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

        elif evt == StraletEvent.ON_TIMER:
            self._stralet.on_timer(data[0], data[1])
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

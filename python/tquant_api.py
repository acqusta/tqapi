import zmq
import time
import datetime
import pandas as pd
import random
import Queue
import json
import threading
from collections import namedtuple

def _to_obj(str) :
    return json.loads(str)

def _to_json(obj) :
    return json.dumps(obj)

def _to_namedtuple(class_name, data):
    try:
        if type(data) == list or type(data) == tuple:
            result = []
            for d in data:
                result.append( namedtuple(class_name, d.keys())(*d.values()) )
            return result

        elif type(data) == dict :
            result = namedtuple(class_name, data.keys())(*data.values())
            return result
        else:
            return data
    except Exception, e:
        print class_name, data, e
        return data

def _to_rowset(class_name, data):
    try:
        if type(data) == dict :
            rowset = []
            fields = data.keys()
            values = data.values()
            field_count = len(fields)
            for i in xrange(len(data[fields[0]])):
                v = []
                for k in xrange(field_count):
                    v.append( values[k][i])

                rowset.append ( namedtuple(class_name, fields)(*v))

            return rowset
        else:
            return data
    except Exception, e:
        print class_name, data, e
        return data

def _error_to_str(error):
    if error:
        if error.has_key('message'):
            return str(error['error']) + "," + error['message']
        else:
            return str(error['error']) + ","
    else:
        return ","

def _to_date(df):
    return pd.to_datetime(df['date'], format='%Y%m%d')#, unit='ms')

def _to_datetime(df):
    return pd.to_datetime(df['date'] * 1000000000 + df['time'], format='%Y%m%d%H%M%S%f')#, unit='ms')

def _to_dataframe(cloumset, index_func = None, index_column = None):
    df = pd.DataFrame(cloumset)
    if index_func:
        #df.index = df.apply(index_func, axis = 1)
        df.index = index_func(df)
    elif index_column:
        df.index = df[index_column]
        del df.index.name

    return df

def _extract_result(cr, data_format="obj", index_column=None, class_name="", to_rowset = False):
    err = _error_to_str(cr['error']) if cr.has_key('error') else None
    if cr.has_key('result'):
        if data_format == "pandas" :
            if index_column :
                return (_to_dataframe(cr['result'], None, index_column), err)
            if 'time' in cr['result']:
                return (_to_dataframe(cr['result'], _to_datetime), err)
            elif 'date' in cr['result']:
                return (_to_dataframe(cr['result'], _to_date), err)
            else:
                return (_to_dataframe(cr['result']), err)
        elif data_format == "obj" and class_name:
            if to_rowset:
                return (_to_rowset(class_name, cr['result']), err)
            else:
                return (_to_namedtuple(class_name, cr['result']), err)
        else:
            return (cr['result'], err)
    else:
        return (None, err)

class JsonRpcClient :
    
    def __init__(self) :
        self._ctx = None
        self._sock = None
        self._client_id = ""
        #self._cmd_queue = Queue.Queue()
        self._waiter_lock = threading.Lock()        
        self._waiter_map = {}

        self._should_close = False
        self._next_callid = 0
        self._send_lock = threading.Lock()
        self._callid_lock = threading.Lock()
        
        self._last_heartbeat_rsp_time = 0
        self._connected = False

        self.on_disconnected = None
        self.on_rpc_callback = None
        self._callback_queue = Queue.Queue()

        td = threading.Thread(target=self._recv_run)
        td.setDaemon(True)
        td.start()

        td = threading.Thread(target=self._callback_run)
        td.setDaemon(True)
        td.start()
        
    def __del__(self):
        #print "JsonRpcClient closed"
        self.close()


    def next_callid(self):
        self._callid_lock.acquire()
        self._next_callid += 1
        callid = self._next_callid
        self._callid_lock.release()
        return self._next_callid

    def _recv_run(self):
        heartbeat_time = 0
        while not self._should_close:
            try:
                if self._connected and time.time() - self._last_heartbeat_rsp_time > 6:
                    self._connected = False
                    if self.on_disconnected: self._async_call(self.on_disconnected)
                    # TODO:
                    #  Reconnect socket

                if not self._sock :
                    time.sleep(0.01)
                    continue

                if time.time() - heartbeat_time > 2.0 :
                    self._send_hearbeat()
                    heartbeat_time = time.time()

                #print time.ctime(), "begin recv"
                data = self._sock.recv()
                if data:
                    #print time.ctime(), "RECV", data
                    self._on_data_arrived(str(data))
                else:
                    #print time.ctime(), "RECV timeout"
                    pass

            except zmq.error.Again, e:
                #print "RECV timeout: ", e
                pass
            except Exception, e:
                print("_recv_run:", e)

        try:
            self._sock.close()
        finally:
            self._sock = None

        #print "recv_thread exits"

    def _callback_run(self):
        while not self._should_close:
            try:
                r = self._callback_queue.get(timeout = 1)
                if r :
                    r()
            except Queue.Empty, e:
                pass

            except Exception, e:
                print "_callback_run", type(e)

    def _async_call(self, func):
        self._callback_queue.put( func )

    def _send_request(self, json) :
        #print time.ctime(), "SEND", json
        try:
            self._send_lock.acquire()
            self._sock.send(json)
        finally:
            self._send_lock.release()
            
    def connect(self, addr) :
        client_id = str(random.randint(1000000, 100000000))

        self._client_id = client_id
            
        context = zmq.Context()  
        socket = context.socket(zmq.DEALER)
        socket.identity = str(client_id) + '$' + str(random.randint(1000000, 1000000000))
        socket.setsockopt(zmq.RCVTIMEO, 500)
        socket.setsockopt(zmq.SNDTIMEO, 500)
        socket.setsockopt(zmq.LINGER,  1000)
        socket.connect(addr)
        
        self._ctx = context
        self._sock = socket

    def close(self):
        self._should_close = True
                
    def _on_data_arrived(self, str):
        try:
            msg = _to_obj(str)

            if msg.has_key('method') and msg['method'] == '.sys.heartbeat':

                self._last_heartbeat_rsp_time = time.time()
                if not self._connected:
                    self._connected = True
                    if self._on_connected :
                        self._async_call(self._on_connected)
                        
                if self.on_rpc_callback :
                    self._async_call( lambda: self.on_rpc_callback(msg['method'], msg['result']) )

            elif msg.has_key('id') and msg['id'] :

                # Call result
                id = int(msg['id'])
                if self._waiter_lock.acquire():
                    if self._waiter_map.has_key(id):
                        q = self._waiter_map[id]
                        if q: q.put(msg)
                    self._waiter_lock.release()
            else:
                # Notification message
                if msg.has_key('method') and msg.has_key('result') and self.on_rpc_callback :
                    self._async_call( lambda: self.on_rpc_callback(msg['method'], msg['result']) )
                
        except Exception, e:
            print( "_on_data_arrived:", e)
            pass
    

    def _send_hearbeat(self):
        msg = { 'jsonrpc' : '2.0',
                'method'  : '.sys.heartbeat',
                'params'  : { 'time': time.time() },
                'id'      : str(self.next_callid()) }
        json_str = _to_json(msg)
        self._send_request(json_str)

    def call(self, method, params, timeout = 20) :
        #print "call", method, params, timeout
        callid = self.next_callid()
        if timeout:
            # TODO: Reuse this queue
            q = Queue.Queue()
            #print time.strftime('%X'), "set q begin"
            self._waiter_lock.acquire()
            self._waiter_map[callid] = q
            self._waiter_lock.release()
            #print time.strftime('%X'), "set q end"
        
        msg = { 'jsonrpc' : '2.0',
                'method'  : method,
                'params'  : params,
                'id'      : str(callid) }
        json_str = _to_json(msg)
        self._send_request(json_str)
        
        if timeout:
            ret = {}
            #print time.strftime('%X'), "q.get begin"
            try:
                r = q.get(timeout = timeout)
                q.task_done()
            except Queue.Empty, e:
                #print e
                r = None
            #print time.strftime('%X'), "q.get done"

            self._waiter_lock.acquire()
            self._waiter_map[callid] = None
            self._waiter_lock.release()
            if r:
                if r.has_key('result'):
                    ret['result'] = r['result']

                if r.has_key('error'):
                    ret['error'] = r['error']

            return ret if ret else { 'error': {'error': -1, 'message': "timeout"}}
        else:
            return { 'result': True }




class TQuantApi:

    def __init__(self, addr):
        self._remote = JsonRpcClient()
        self._remote.on_rpc_callback = self._on_rpc_callback
        self._remote.on_disconnected = self._on_disconnected
        self._remote._on_connected   = self._on_connected
        self._remote.connect(addr)

        self._quote_callback = None
        self._connected = False
        self._username = ""
        self._password = ""
        self._has_session = False
        self._dapi = DataApi(self)
        self._tapi = TradeApi(self)

    def __del__(self):
        #print "TQuantApi closed"
        self._remote.close()
    
    def close(self):
        self._remote.close()

    def _on_rpc_callback(self, method, data):
        if method.startswith("dapi."):
            self._dapi._on_rpc_callback(method, data)

    def _on_disconnected(self):
        #self._do_login(self._account_id, self._password)
        #print "_on_disconnected"
        self._connected = False
        self._has_session = False

    def _on_connected(self):
        #print "_on_connected"
        self._connected = True

    def _check_session(self):
        if not self._connected:
            return (False, "NO CONNECTION")

        if not self._has_session:
            return self._do_login()

        return (True, "")

    def _call(self, method, params):
        return self._remote.call(method, params)

    def data_api(self) :
        return self._dapi

    def trade_api(self) :
        return self._tapi


class DataApi:

    def __init__(self, tqapi):
        self._tqapi = tqapi
        self._data_format = "obj"
        self._on_quote = None

    def set_format(self, format) :
        self._data_format = format

    def _on_rpc_callback(self, method, data):
        if method == "dapi.quote" :
            if self._on_quote :
                self._on_quote(data)

    def tick(self, code, tradingday = 0) :
        params = {}
        if tradingday :
            params["tradingday"] = int(tradingday)
        params["code"] = code

        cr = self._tqapi._call("dapi.tst", params)
        return _extract_result(cr, self._data_format, class_name = "MaketQuote")#, to_rowset = True)

    def bar(self, code, cycle="1m", tradingday = 0) :
        params = { 
            "code"  : code,
            "cycle" : cycle,
            }

        if tradingday :
            params["tradingday"] = int(tradingday)

        cr = self._tqapi._call("dapi.tsi", params)
        return _extract_result(cr, self._data_format, class_name="Bar")#, to_rowset = True)


    def quote(self, code) :
        params = { "code" : code }

        cr = self._tqapi._call("dapi.tsq_quote", params)
        return _extract_result(cr,
                "obj" if self._data_format == "pandas" else self._data_format,
                class_name = "MaketQuote")

    def set_on_quote(self, func):
        self._on_quote = func

    def subscribe(self, codes):
        """
        return (result, message)
        if result is None, message contains error infomation
        """
        #codes = securities
        if type(codes) == tuple or \
           type(codes) == list or \
           type(codes) == set :
           codes = ','.join(codes)


        rpc_params = { 'codes' : codes }
        
        cr = self._tqapi._call("dapi.tsq_sub", rpc_params)
        return _extract_result(cr, "")

    def unsubscribe(self, codes):
        """
        return (result, message)
        if result is None, message contains error infomation
        """
        if type(codes) == tuple or \
           type(codes) == list or \
           type(codes) == set :
           codes = ','.join(codes)


        rpc_params = { 'codes' : codes }
        
        cr = self._tqapi._call("dapi.tsq_unsub", rpc_params)
        return _extract_result(cr, "")


    def close(self):
        self._tqapi.close()
            

class TradeApi:
    
    def __init__(self, tqapi):
        self._tqapi = tqapi
        self._data_format = "obj"
        self._on_quote = None

    def set_format(self, format) :
        self._data_format = format

    def _on_rpc_callback(self, method, data):
        if method == "tapi.order_status":
            order = data
            if self._ordstatus_callback:
                self._ordstatus_callback(order)

        #elif method == ".sys.heartbeat":
            #if self._has_session and ("session" not in data or data["session"] != self._username):
            #    print data
            #    print "no session in server, login again"
            #    self._do_login()

    def set_ordstatus_callback(self, callback):
        self._ordstatus_callback = callback


    def account_status(self):

        rpc_params = { }

        cr = self._tqapi._call("tapi.account_status", rpc_params)

        return _extract_result(cr, self._data_format, class_name="AccountStatus")

    def query_balance(self, account_id):

        rpc_params = { "account_id" : account_id }
    
        cr = self._tqapi._call("tapi.query_balance", rpc_params)

        return _extract_result(cr,
            "obj" if self._data_format == "pandas" else self._data_format,
            class_name = "Balance")

    def query_trades(self, account_id):

        rpc_params = { "account_id" : account_id }
    
        cr = self._tqapi._call("tapi.query_trades", rpc_params)

        return _extract_result(cr, self._data_format, class_name="Trade")

    def query_orders(self, account_id):

        rpc_params = { "account_id" : account_id }
    
        cr = self._tqapi._call("tapi.query_orders", rpc_params)

        return _extract_result(cr, self._data_format, class_name="Order")


    def query_positions(self, account_id):

        rpc_params = { "account_id" : account_id }
    
        cr = self._tqapi._call("tapi.query_positions", rpc_params)

        return _extract_result(cr, self._data_format, class_name="Position")

    def buy(self, account_id, code, price, size) :
        return self.place_order(account_id, code, price, size, "Buy")

    def short(self, account_id, code, price, size) :
        return self.place_order(account_id, code, price, size, "Short")

    def cover(self, account_id, code, price, size) :
        return self.place_order(account_id, code, price, size, "Cover")

    def sell(self, account_id, code, price, size) :
        return self.place_order(account_id, code, price, size, "Sell")

    def cover_today(self, account_id, code, price, size) :
        return self.place_order(account_id, code, price, size, "CoverToday")

    def sell_today(self, account_id, code, price, size) :
        return self.place_order(account_id, code, price, size, "SellToday")

    def place_order(self, account_id, code, price, size, action):
        """
        return (result, message)
        if result is None, message contains error infomation
        """
        rpc_params = { "code"        : code,
                       "price"       : price,
                       "size"        : size,
                       "action"      : action,
                       "account_id"  : account_id}

    
        cr = self._tqapi._call("tapi.place_order", rpc_params)
        return _extract_result(cr, "")
            

    def cancel_order(self, account_id, entrust_no, code):
        """
        return (result, message)
        if result is None, message contains error infomation
        """
        rpc_params = { "account_id" : account_id,
                       "entrust_no" : entrust_no,
                       "code"       : code }
    
        cr = self._tqapi._call("tapi.cancel_order", rpc_params)
        return _extract_result(cr, "")

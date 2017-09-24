import zmq
import time
import pandas as pd
import random
import Queue
import msgpack
import threading
from collections import namedtuple

def _to_obj(str) :
    return  msgpack.loads(str)

def _to_json(obj) :
    return msgpack.dumps(obj)

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
            return str(error['code']) + "," + error['message']
        else:
            return str(error['code']) + ","
    else:
        return ","

def _to_date(df):
    return pd.to_datetime(df['date'], format='%Y%m%d')#, unit='ms')

def _to_datetime(df):
    return pd.to_datetime(df['date'] * 1000000000 + df['time'], format='%Y%m%d%H%M%S%f')#, unit='ms')

def _to_dataframe(cloumset, index_func = None, index_column = None):
    df = pd.DataFrame(cloumset)
    if index_func:
        df.index = index_func(df)
        del df.index.name
    elif index_column:
        df.index = df[index_column]
        del df.index.name

    return df

def _extract_result2(cr, data_format="obj", index_column=None, class_name="",
                    to_rowset = False, index=True):

    err = _error_to_str(cr['error']) if cr.has_key('error') else None
    if cr.has_key('result'):
        if data_format == "pandas" :
            if index :
                if index_column :
                    return (_to_dataframe(cr['result'], None, index_column), err)
                if 'time' in cr['result']:
                    return (_to_dataframe(cr['result'], _to_datetime), err)
                elif 'date' in cr['result']:
                    return (_to_dataframe(cr['result'], _to_date), err)

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

def _extract_result(cr, data_format="obj", index_column=None, class_name="",
                    to_rowset = False,                    
                    error_mode="inline",
                    index = True):
    v, err_msg = _extract_result2(cr, data_format, index_column, class_name, to_rowset, index)
    if error_mode == "exception":
        if v is not None:
            return v
        else:
            raise RuntimeError(err_msg)
    else:
        return (v, err_msg)

class JsonRpcClient :
    
    def __init__(self) :
        self._waiter_lock = threading.Lock()        
        self._waiter_map = {}

        self._should_close = False
        self._next_callid = 0
        self._send_lock = threading.Lock()
        self._callid_lock = threading.Lock()
        
        self._last_heartbeat_rsp_time = 0
        self._connected = False

        self.on_connected = None
        self.on_disconnected = None
        self.on_rpc_callback = None
        self._callback_queue = Queue.Queue()
        self._call_wait_queue = Queue.Queue()

        self._ctx = zmq.Context()
        self._pull_sock = self._ctx.socket(zmq.PULL)
        self._pull_sock.bind("inproc://pull_sock")
        self._push_sock = self._ctx.socket(zmq.PUSH)
        self._push_sock.connect("inproc://pull_sock")

        self._heartbeat_interval = 1
        self._heartbeat_timeout = 3

        self._addr = None

        t = threading.Thread(target=self._recv_run)
        t.setDaemon(True)
        t.start()

        t = threading.Thread(target=self._callback_run)
        t.setDaemon(True)
        t.start()
        
    def __del__(self):
        self.close()


    def next_callid(self):
        self._callid_lock.acquire()
        self._next_callid += 1
        callid = self._next_callid
        self._callid_lock.release()
        return callid

    def _recv_run(self):

        heartbeat_time = 0

        poller = zmq.Poller()
        poller.register(self._pull_sock, zmq.POLLIN)

        remote_sock = None

        while not self._should_close:

            try:
                if self._connected and time.time() - self._last_heartbeat_rsp_time > self._heartbeat_timeout:
                    self._connected = False
                    if self.on_disconnected: self._async_call(self.on_disconnected)

                if remote_sock and time.time() - heartbeat_time > self._heartbeat_interval :
                    self._send_hearbeat()
                    heartbeat_time = time.time()

                socks = dict(poller.poll(500))
                if self._pull_sock in socks and socks[self._pull_sock] == zmq.POLLIN:
                    cmd = self._pull_sock.recv()
                    if cmd == "CONNECT":
                        # print time.ctime(), "CONNECT " + self._addr
                        if remote_sock:
                            poller.unregister(remote_sock)
                            remote_sock.close()
                            remote_sock = None

                        remote_sock = self._do_connect()

                        if remote_sock :
                            poller.register(remote_sock, zmq.POLLIN)

                    elif cmd.startswith("SEND:") and remote_sock :
                        #print time.ctime(), "SEND " + cmd[5:]
                        remote_sock.send(cmd[5:])

                if remote_sock and remote_sock in socks and socks[remote_sock] == zmq.POLLIN:
                    data = remote_sock.recv()
                    if data:
                            #if not data.find("heartbeat"):
                        #print time.ctime(), "RECV", data
                        self._on_data_arrived(str(data))

            except zmq.error.Again as e:
                #print "RECV timeout: ", e
                pass
            except Exception as e:
                print("_recv_run:", e)

    def _callback_run(self):
        while not self._should_close:
            try:
                r = self._callback_queue.get(timeout = 1)
                if r :
                    r()
            except Queue.Empty as e:
                pass

            except Exception as e:
                print "_callback_run", type(e), e

    def _async_call(self, func):
        self._callback_queue.put( func )

    def _send_request(self, json) :

        try:
            self._send_lock.acquire()
            self._push_sock.send("SEND:" + json)

        finally:
            self._send_lock.release()
            
    def connect(self, addr) :
        self._addr = addr
        self._push_sock.send("CONNECT")


    def _do_connect(self):
            
        client_id = str(random.randint(1000000, 100000000))

        socket = self._ctx.socket(zmq.DEALER)
        socket.identity = str(client_id) + '$' + str(random.randint(1000000, 1000000000))
        socket.setsockopt(zmq.RCVTIMEO, 500)
        socket.setsockopt(zmq.SNDTIMEO, 500)
        socket.setsockopt(zmq.LINGER, 0)
        socket.connect(self._addr)
        
        return socket

    def close(self):
        self._should_close = True
                
    def _on_data_arrived(self, str):
        try:
            msg = _to_obj(str)
            #print "RECV", msg

            if msg.has_key('method') and msg['method'] == '.sys.heartbeat':

                self._last_heartbeat_rsp_time = time.time()
                if not self._connected:
                    self._connected = True
                    if self.on_connected :
                        self._async_call(self.on_connected)
                        
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
                if msg.has_key('method') and msg.has_key('params') and self.on_rpc_callback :
                    self._async_call( lambda: self.on_rpc_callback(msg['method'], msg['params']) )
                
        except Exception, e:
            print( "_on_data_arrived:", e)
            pass
    

    def _send_hearbeat(self):
        msg = { #'jsonrpc' : '2.0',
                'method'  : '.sys.heartbeat',
                'params'  : { 'time': time.time() },
                'id'      : self.next_callid() }
        json_str = _to_json(msg)
        self._send_request(json_str)

    def _alloc_wait_queue(self):
        self._waiter_lock.acquire()
        if self._call_wait_queue:
            q = self._call_wait_queue
            self._call_wait_queue = None
        else:
            q = Queue.Queue()
        self._waiter_lock.release()
        return q

    def _free_wait_queue(self, q):
        self._waiter_lock.acquire()
        if not self._call_wait_queue :
            self._call_wait_queue  = q
        else:
            del q
        self._waiter_lock.release()

    def call(self, method, params, timeout = 6) :
        #print "call", method, params, timeout
        callid = self.next_callid()
        if timeout:

            q = self._alloc_wait_queue()

            self._waiter_lock.acquire()
            self._waiter_map[callid] = q
            self._waiter_lock.release()
        
        msg = { 'method'  : method,
                'params'  : params,
                'id'      : callid }
        json_str = _to_json(msg)
        self._send_request(json_str)
        
        if timeout:
            ret = {}
            try:
                r = q.get(timeout = timeout)
                q.task_done()
            except Queue.Empty as e:
                r = None

            self._waiter_lock.acquire()
            self._waiter_map[callid] = None
            self._waiter_lock.release()
            self._free_wait_queue(q)

            if r:
                if r.has_key('result'):
                    ret['result'] = r['result']

                if r.has_key('error'):
                    ret['error'] = r['error']

            return ret if ret else { 'error': {'code': -1, 'message': "timeout"}}
        else:
            return { 'result': True }




class TQuantApi:

    def __init__(self, addr):
        self._remote = JsonRpcClient()
        self._remote.on_rpc_callback = self._on_rpc_callback
        self._remote.on_disconnected = self._on_disconnected
        self._remote.on_connected    = self._on_connected
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
        elif method.startswith("tapi."):
            self._tapi._on_rpc_callback(method, data)
        elif method.startswith(".sys."):
            self._dapi._on_rpc_callback(method, data)
            self._tapi._on_rpc_callback(method, data)

    def _on_disconnected(self):
        #self._do_login(self._account_id, self._password)
        #print "_on_disconnected"
        self._connected = False
        self._has_session = False
        self._dapi._on_disconnected()
        self._tapi._on_disconnected()

    def _on_connected(self):
        #print "_on_connected"
        self._connected = True
        self._dapi._on_connected()
        self._tapi._on_connected()

    # def _check_session(self):
    #     if not self._connected:
    #         return (False, "no connection")

    #     if not self._has_session:
    #         return self._do_login()

    #     return (True, "")

    def _call(self, method, params, timeout = 6):
        return self._remote.call(method, params, timeout)

    def data_api(self) :
        return self._dapi

    def trade_api(self) :
        return self._tapi


class DataApi:

    def __init__(self, tqapi):
        self._tqapi = tqapi
        self._data_format = "pandas"
        self._on_quote = None
        self._on_bar = None
        self._error_mode = "inline"
        self._sub_codes = set()
        self._sub_hash = 0
        self._subscribing = False

    def set_error_mode(self, mode):
        """Set error mode.

        This api provides two modes of processing mode.
        inline:
            In this mode, call result is a tuple, (value, err_msg). No exception
            will be thrown.
            It is the default mode.

            example:            
                bar, err_msg = dapi.bar('000001.SH')

        exception:
            In this mode, call result is the value of successful call. If there
            is any error, an exception will be thrown.            

            example:
                try :
                    bar = dapi.bar('000001.SH')
                    ...
                except Exception as e:
                    print e
        """
        self._error_mode = mode

    def set_format(self, format) :
        """Set format of out data. 
        
        Available formats are:
            ""        raw format. Usually the type of data are map.
            "pandas"  convert table like data to DataFrame.
            "obj",    like raw format but convert map to an object
        """
        self._data_format = format

    def _on_rpc_callback(self, method, data):
        if method == "dapi.quote" :
            if self._on_quote :
                self._on_quote(data)
        elif method == "dapi.bar" :
            if self._on_bar:
                self._on_bar(data["cycle"], data["bar"])
        elif method == ".sys.heartbeat" :
            self._on_heartbeat(data)
        # else:
        #     print "unkown method", method

    def _on_heartbeat(self, data):
        sub_hash = 0
        if 'sub_hash' in data:
            sub_hash = data['sub_hash']
        
        if not self._sub_codes:
            return

        if self._sub_hash == sub_hash and sub_hash:
            return

        if not self._subscribing :
            self._subscribing = True
            t = threading.Thread(target=self._subscribe_again)
            t.setDaemon(True)
            t.start()

    def _subscribe_again(self):

        try :
            #print "subscribe again:", self._sub_codes, self._sub_hash, sub_hash, data
            rpc_params = { 'codes' : ','.join(self._sub_codes) }
            cr = self._tqapi._call("dapi.tsq_sub", rpc_params, timeout=2)
            r = _extract_result(cr, "", error_mode = "inline")
            if r[0]:
                self._sub_hash = r[0]['sub_hash']
        finally:
            self._subscribing = False

    def _on_connected(self):
        pass

    def _on_disconnected(self):
        pass

    def tick(self, code, trading_day = 0, df_index=True) :
        """Get ticks by code and trading_day. 
        
        trading_day - integer, format yyyyMMdd.
                      If trading_day is 0, means current trading day in the server.

        df_index    - Whether the dataframe has an index or not.

        """
        params = {}
        if trading_day :
            params["trading_day"] = int(trading_day)
        params["code"] = code

        cr = self._tqapi._call("dapi.tst", params)
        return _extract_result(cr, self._data_format, class_name = "MaketQuote",
                               error_mode = self._error_mode,
                               index = df_index )

    def bar(self, code, cycle="1m", trading_day = 0, price_adj="", df_index=True, align=False) :
        """ Get bar by code, cycle and trading_day
        
        code   - example "000001.SH"
        cycle  - "1m", one miniute bar
                 "1d", one day bar
    
        trading_day - exmaple 20170818
        price_adj   - daily price adjust mode, only available for stock.
                  "back"    : keep price of first day same and chang following days' price
                  "forward" : keep price of last day same and change before days' price 
        df_index    - Whether the dataframe has an index or not.
                  
        """
        params = { 
            "code"  : code,
            "cycle" : cycle,
            "align" : True if align else False
        }

        if trading_day :
            params["trading_day"] = int(trading_day)

        if price_adj and cycle=="1d":
            params["price_adj"] = price_adj

        cr = self._tqapi._call("dapi.tsi", params)
        return _extract_result(cr, self._data_format, class_name="Bar",
                               error_mode=self._error_mode,
                               index = df_index)


    def quote(self, code) :
        """Get latest market quote of the code."""

        params = { "code" : code }

        cr = self._tqapi._call("dapi.tsq_quote", params)
        return _extract_result(cr,
                "" if self._data_format == "pandas" else self._data_format,
                class_name = "MaketQuote",
                error_mode=self._error_mode)

    def set_on_quote(self, func):
        """Set on_quote callback.
        
        When client receives a pushed quote from server, it will notify client
        by calling this function.
        
        def on_quote(quote):
            print quote

        The argument, quote, of callback is same as function DataApi.quote(code).
        The processing of quote should be quick,  or the execution of api will be blocked!
        """
        self._on_quote = func

    def set_on_bar(self, func):
        """Set on_bar callback.
        
        When client receives the pushed bar from server, it will notify client
        by calling this function.
        
        def on_bar(bar)
            print bar

        Unlike on_quote, the argument, bar, is only one bar, not the whole bars!
        As on_quote, the processing of callback should be quick too.
        """
        self._on_bar = func

    def subscribe(self, codes):
        """Subscribe codes and return all subscribed codes.
        
        Subscribe code from server. Multiple call will subscribe codes togeher.

        codes : "code1,code2,...", example "000001.SH,399001.SZ"
        When codes is empty, return all subscribed codes.        
        """

        if type(codes) == tuple or \
           type(codes) == list or \
           type(codes) == set :
           codes = ','.join(codes)

        if codes:
            codes = codes.strip()
            for s in codes.split(','):
                if s :
                    self._sub_codes.add(s)

        rpc_params = { 'codes' : codes }

        cr = self._tqapi._call("dapi.tsq_sub", rpc_params)
        r = _extract_result(cr, "", error_mode=self._error_mode)

        if self._error_mode == "exception":
            if codes :
                self._sub_hash = r['sub_hash']
            r = r['sub_codes']
        else:
            if r[0]:
                if codes :
                    self._sub_hash = r[0]['sub_hash']
                r = (r[0]['sub_codes'], r[1])
        return r

    def unsubscribe(self, codes):
        """Unsubscribe cdoes and return all subscribed codes.

        codes : "code1,code2,...", example "000001.SH,399001.SZ"
        """
        if type(codes) == tuple or \
           type(codes) == list or \
           type(codes) == set :
           codes = ','.join(codes)

        if codes:
            for s in codes.split(','):
                self._sub_codes.remove(s)

        rpc_params = { 'codes' : codes }
        
        cr = self._tqapi._call("dapi.tsq_unsub", rpc_params)
        r = _extract_result(cr, "", error_mode=self._error_mode)

        if self._error_mode == "exception":
            self._sub_hash = r['sub_hash']
            r = r['sub_codes']
        else:
            if r[0]:
                self._sub_hash = r[0]['sub_hash']
                r = (r[0]['sub_codes'], r[1])
        return r

class TradeApi:
    
    def __init__(self, tqapi):
        self._tqapi             = tqapi
        self._data_format       = "pandas"
        self._on_order_status   = None
        self._on_order_trade    = None
        self._on_account_status = None
        self._error_mode        = "inline"

    def set_error_mode(self, mode):
        """Set error mode.

        This api provides two modes of processing mode.
        inline:
            In this mode, call result is a tuple, (value, err_msg). No exception
            will be thrown.
            It is the default mode.

            example:            
                bar, err_msg = dapi.bar('000001.SH')

        exception:
            In this mode, call result is the value of successful call. If there
            is any error, an exception will be thrown.            

            example:
                try :
                    bar = dapi.bar('000001.SH')
                    ...
                except Exception as e:
                    print e
        """
        self._error_mode = mode

    def set_format(self, format) :
        """Set format of out data. 
        
        Available formats are:
            ""        raw format. Usually the type of data are map.
            "pandas"  convert table like data to DataFrame.
            "obj",    like raw format but convert map to an object
        """
        
        self._data_format = format

    def _on_connected(self):
        pass

    def _on_disconnected(self):
        pass

    def _on_rpc_callback(self, method, data):
        if method == "tapi.order_status_ind":
            if self._on_order_status:
                self._on_order_status(data)

        elif method == "tapi.order_trade_ind":
            if self._on_order_trade:
                self._on_order_trade(data)
            
        elif method == "tapi.account_status_ind":
            if self._on_account_status:
                self._on_account_status(data)

        #elif method == ".sys.heartbeat":
            #if self._has_session and ("session" not in data or data["session"] != self._username):
            #    print data
            #    print "no session in server, login again"
            #    self._do_login()

    def set_on_order_status(self, callback):
        """Set on_order_status callback.
        
        The server will push status changed message when it receives notification
        from broker link.
        
        def on_order_status(order)
            print order

        Status:  New, Accepted, Filled, Rejected, Caancelled

        The order is same as the one queried through query_orders().
        """        
        self._on_order_status = callback
   
    def set_on_order_trade(self, callback):
        """Set on_order_trade callback."""

        self._on_order_trade = callback

    def set_on_account_status(self, callback):
        """Set on_account_status callback.
        
        The callback will be called when trade account is connected or disconnected.
        """
        self._on_account_status = callback

    def account_status(self):
        """Get trade account connection status"""
        rpc_params = { }

        cr = self._tqapi._call("tapi.account_status", rpc_params)

        return _extract_result(cr, self._data_format, class_name="AccountStatus",
                               error_mode=self._error_mode)

    def query_balance(self, account_id):
        """Get balance of one account.
        """
        rpc_params = { "account_id" : account_id }
    
        cr = self._tqapi._call("tapi.query_balance", rpc_params)

        return _extract_result(cr,
            "" if self._data_format == "pandas" else self._data_format,
            class_name = "Balance",
            error_mode=self._error_mode)

    def query_trades(self, account_id):
        """Get trades of one account."""

        rpc_params = { "account_id" : account_id }
    
        cr = self._tqapi._call("tapi.query_trades", rpc_params)

        return _extract_result(cr, self._data_format, class_name="Trade",
                               error_mode=self._error_mode)

    def query_orders(self, account_id):
        """Get orders of one account."""
        rpc_params = { "account_id" : account_id }
    
        cr = self._tqapi._call("tapi.query_orders", rpc_params)

        return _extract_result(cr, self._data_format, class_name="Order",
                               error_mode=self._error_mode)


    def query_positions(self, account_id):
        """Get positions of one account."""
        rpc_params = { "account_id" : account_id }
    
        cr = self._tqapi._call("tapi.query_positions", rpc_params)

        return _extract_result(cr, self._data_format, class_name="Position",
                               error_mode=self._error_mode)

    def place_order(self, account_id, code, price, size, action, order_id=None):
        """Place an order and return entrust_no

        Arguments:
            
        account_id - account id
        code       -
        price      - 
        size       - The size unit of stock or future.
        action     - "Buy"     buy   stock, open long future
                     "Sell"    sell  stock, close long future
                     "Short"   ---          open short future
                     "Cover"   ---          close short future

                     "SellToday", "SellYesterday"
                     "CoverToday", "CoverYesterday"

        order_id   - Integer, user defined id of an order. It can be used for canceling later.
                     Only avaialbe for CTP account. User should ensure the unique of order_id 

        Return:
            If the order is submited to broker successfully, this function will
            return entrust_no and order_id
              (
                {
                'entrust_no' : entrust_no,
                'order_id'   : order_id
                },
                err_msg
              )
            For CTP, once the order is inserted/submited, this function will 
            return an order_id and no entrust_no. If the order has been accepted,
            later order status_ind will have enturst_no.
        """
        rpc_params = { "code"        : code,
                       "price"       : price,
                       "size"        : size,
                       "action"      : action,
                       "account_id"  : account_id}

        if order_id is not None:
            rpc_params['order_id'] = order_id

        cr = self._tqapi._call("tapi.place_order", rpc_params)
        return _extract_result(cr, "", class_name="OrderInfo",
                               error_mode=self._error_mode)
            

    def cancel_order(self, account_id, code, entrust_no=None, order_id=None):
        """Canel order.
        
        An unfinised order can be cancelled by entrust_no or order_id. order_id
        is only available for CTP order.        
        """
        rpc_params = { "account_id" : account_id,
                       "code"       : code }

        if entrust_no:
            rpc_params["entrust_no"] = entrust_no,
        if order_id:
            rpc_params['order_id'] = order_id
                       
        cr = self._tqapi._call("tapi.cancel_order", rpc_params)
        return _extract_result(cr, "", error_mode=self._error_mode)

    def query(self, account_id, command, params=""):
        """common query.
        
        This query function provides speical queries for different accounts.
        
        For eaxample, use can query code table of CTP accounts through command, 'ctp_codetable'.
        """
        rpc_params = { "account_id" : account_id,
                       "command"    : command,
                       "params"     : params }

        cr = self._tqapi._call("tapi.common_query", rpc_params)
        return _extract_result(cr,             
            "" if self._data_format == "pandas" else self._data_format,
            class_name = "QueryResult",
            error_mode=self._error_mode)


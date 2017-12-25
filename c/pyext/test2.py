import tquant as tq
import time

#tqapi = tq.TQuantApi('tcp://127.0.0.1:10001')
tqapi = tq.TQuantApi('ipc://tqc_10001')
dapi = tqapi.data_api()
tapi = tqapi.trade_api()

dapi.bar('000001.SH', '1m')

def print_quote(q):
    print "quote:", q['code'], q['date'], q['time'], q['last'], q['volume'], q['ask1'], q['bid1']

def on_quote(q):
    print_quote(q)

dapi.set_on_quote(on_quote)

dapi.subscribe('rb1801.SHF,J1805.DCE,000001.SH')

while True:
    q, msg = dapi.quote('000001.SH')
    if q:
    	print "get_quote:"
    	print_quote(q)
    else:
    	print "quote error:", msg

    time.sleep(10)


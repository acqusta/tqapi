import sys

import tquant as tq
import time

#tqapi = tqapi.TQuantApi('tcp://127.0.0.1:10001')
tqapi = tq.TQuantApi('ipc://tqc_10001')
dapi = tqapi.data_api()
dapi.bar('000001.SH', '1m')


def on_quote(q):
    print q['code'], q['date'], q['time'], q['last'], q['volume']

dapi.set_on_quote(on_quote)
dapi.subscribe('rb1805.SHF,j1805.DCE,000001.SH')

while True:
    time.sleep(1)



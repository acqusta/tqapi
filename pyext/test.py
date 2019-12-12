import sys

import tquant as tq
import time

dapi = tq.DataApi('tcp://192.168.2.231:10002')
dapi.bar('000001.SH', '1m')


def on_quote(q):
    print (q['code'], q['date'], q['time'], q['last'], q['volume'])

dapi.set_on_quote(on_quote)
dapi.subscribe('*.SH,*.SZ')

while True:
    time.sleep(1)



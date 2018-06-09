import sys

import tquant as tq
import time

dapi = tq.DataApi('ipc://tqc_10001')
dapi.bar('000001.SH', '1m')


def on_quote(q):
    print q['code'], q['date'], q['time'], q['last'], q['volume']

dapi.set_on_quote(on_quote)
dapi.subscribe('rb1805.SHF,j1805.DCE,000001.SH')

while True:
    time.sleep(1)



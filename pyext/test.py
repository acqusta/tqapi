import sys
sys.path=['', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\site-packages\\pyro4-4.59-py2.7.egg','D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\site-packages\\selectors34-1.1-py2.7.egg', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\site-packages\\serpent-1.19-py2.7.egg', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\site-packages\\msgpack_python-0.4.8-py2.7-win32.egg', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\python27.zip', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\DLLs', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\plat-win', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\lib-tk', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\site-packages', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\site-packages\\win32', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\site-packages\\win32\\lib', 'D:\\WinPython-32bit-2.7.10.3\\python-2.7.10\\lib\\site-packages\\Pythonwin']

import tqapi
import time
tqapi = tqapi.TQuantApi('tcp://127.0.0.1:10001')
dapi = tqapi.data_api()
dapi.bar('000001.SH', '1m')


def on_quote(q):
    print q

dapi.set_on_quote(on_quote)
dapi.subscribe('rb1801.SHF,J1805.DCE,000001.SH')



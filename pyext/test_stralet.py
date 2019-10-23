import sys

import tqapi2 as tq

class MyStralet(tq.Stralet):

    def on_init(self):
        print "on_init", self.ctx.trading_day, self.ctx.mode
        self.ctx.data_api.subscribe('000001.SH,600000.SH')

    def on_fini(self):
        #print "on_fini"
        self.ctx.log("info", "--fini--")
        pass

    def on_quote(self, q):
        #print q['code'], q['time'], q['last']
        pass

    def on_bar(self, cycle, bar):
        #print bar['code'], bar['time'], bar['close']

        q = self.data_api.quote('600000.SH')[0]

        now = self.ctx.cur_time
        print now

        self.ctx.trade_api.place_order('sim', code='600000.SH', price=q['last'], size=100, action='Buy')
    
    def on_order(self, order):
        #print order
        pass

    def on_trade(sef, trade):
        #print trade
        pass

cfg =  {
    "dapi_addr"   : "tcp://127.0.0.1:10001",
    "begin_date"  : 20181008,
    "end_date"    : 20181031,
    "data_level"  : "tk"
}

tq.bt_run(cfg, MyStralet)
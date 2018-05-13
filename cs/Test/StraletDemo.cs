using System;
using System.Collections.Generic;
using TQuant.Stralet;
using TQuant.Api;

namespace Test
{
    class StraletDemo : Stralet
    {
        
        public override void OnInit(StraletContext sc) { }
        public override void OnFini() { }
        public override void OnQuote(MarketQuote quote) { }
        public override void OnBar(Bar bar) { }
        public override void OnTimer(Int32 id, Object data) { }
        public override void OnEvent(String evt, Object data) { }
        public override void OnOrderStatus(OrderStatus order) { }
        public override void OnOrderTrade(Trade trade) { }
        public override void OnAccountStatus(AccountInfo account) { }
    }
}

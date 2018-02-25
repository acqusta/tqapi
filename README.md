# tqapi

TQuant是[acqusta.com](http://www.acqusta.com)开发的量化交易工具。它通过互联网提供行情和交易服务，支持本地插件提供行情和交易服务，适合量化爱好者、私募搭建自己的交易系统。

tqapi是TQuant的的标准API，包含行情和交易接口。源码开放，支持多种语言。

使用说明参见API源码注释和 [Wiki](https://github.com/acqusta/tqapi/wiki)。
更多资讯，请关注微信号：tquant。

<img src="weichat_tquant.jpg" width="150" height="150">

## 特点

* 实时推送行情，支持sina等互联网Level 1行情、CTP期货行情
* 快速获取历史数据，包括tick, 分钟线，日线
* 本地文件历史数据库，特有压缩算法高效压缩和快速读取
* 交易接口统一，同时支持股票和期货
* 订单状态、成交回报实时通知
* 支持C++, Pyhon, Java, Scala, JavaScript等编程语言
* 支持 Windows, Linux, OSX三种平台，在Mac上也可以进行交易和研究
* API源码开放，用户策略安全可靠

## 行情接口

* **接口列表**

| 函数            | 功能                |
| ------------- | ----------------- |
| quote()       | 取最新行情             |
| bar()         | 取分钟K线，支持实时和历史K线 |
| daily_bar(）  | 取日线 |
| tick()        | 取tick数据，支持实时和历史数据 |
| subscribe()   | 订阅行情              |
| unsubscribe() | 取消订阅              |
| callback      | 回调通知 |

* 提供国内六大交易所的股票、指数、基金、商品期货、股指期货等行情。

## 交易接口

支持CTP交易，持国内部分券商交易。

**接口列表**

| 函数                | 功能                |
| ----------------- | ----------------- |
| query_balance()   | 取资金信息             |
| query_orders()    | 取当日订单列表           |
| query_trades()    | 取当日成交列表           |
| query_positions() | 取当日持仓             |
| place_order()     | 下单                |
| cancel_order()    | 取消订单              |
| query()           | 通用查询接口，可以查询代码表等信息 |
| callback          | 回调函数 |

## TQC 客户端
请从[www.acqusta.com](http://www.acqusta.com)下载tqc客户端。该程序负责用户登录、数据加速，并提供API服务。






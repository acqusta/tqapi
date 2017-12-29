# tqapi

tqapi是acqusta.com的量化交易平台TQuant的客户端API。它通过TQuant平台提供的互联网服务，提供行情和交易接口。适合量化爱好者、私募搭建自己的交易系统。

使用说明参见API源码注释和 [Wiki](https://github.com/tzxu/tqc-api/wiki)。
更多资讯，请关注微信号：tquant。

<img src="weichat_tquant.jpg" width="150" height="150">

## 特点

* 实时推送行情，支持Sina等互联网Level 1行情、CTP期货行情
* 快速获取历史数据，包括tick, 分钟线，日线
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
* 历史数据期限

| 数据类型 | 期限   |
| ---- | ---- |
| tick | 3个月  |
| 分钟线  | 1 年  |
| 日行情  | 全部   |

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
用户需要在自己电脑上运行客户端程序tqc。该程序负责用户登录、数据加速，并提供tqapi服务。tqc下载地址： [TQuant](http://www.acqusta.com/download/tqc)。

> 目前只发布了Windows版本。如有需要Linux和Max版本，可以通过微信联系。

**试用帐号: 用户名 demo 密码 123456。** 

试用帐号有以下限制：

1. 允许订阅的代码：000001.SH, 399001.SZ, 600000.SH, 000001.SZ, IF.CFE, rb.SHF
1. 允许交易的代码：600000.SH, 000001.SZ， IF.CFE, rb.SHF






#ifndef _TQUANT_API_H
#define _TQUANT_API_H

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <exception>
#include <cstring>
#include <unordered_set>

#ifdef _WIN32
#  ifdef _TQAPI_DLL
#    define _TQAPI_EXPORT __declspec(dllimport)
#  elif defined(_TQAPI_DLL_IMPL)
#    define _TQAPI_EXPORT __declspec(dllexport)
#  else
#    define _TQAPI_EXPORT
#  endif
#else
#  define _TQAPI_EXPORT
#endif

namespace tquant {
	namespace api {

		using namespace std;

		template<typename T>
		class TickDataHolder : public T {
			string _code;
		public:
			TickDataHolder() {
				std::memset(this, 0, sizeof(T));
			}

			TickDataHolder(const T& t, const string& a_code) : T(t), _code(a_code) {
				this->code = _code.c_str();
			}

			TickDataHolder(const T& t) : T(t), _code(t.code) {
				this->code = _code.c_str();
			}

			TickDataHolder(const TickDataHolder<T>& t) {
				*this = t;
				if (t.code) {
					this->_code = t.code;
					this->code = this->_code.c_str();
				}
			}

			void assign(const T& t, const char* code = nullptr) {
				*(T*)this = t;
				if (code) {
					_code = code;
					this->code = _code.c_str();
				}
			}

			void set_code(const string& a_code) {
				_code = a_code;
				this->code = _code.c_str();
			}
		};

		struct TickArray {
			TickArray(size_t type_size, size_t max_size)
				: _data(nullptr)
				, _type_size((int)type_size)
				, _size(0)
			{
				if (max_size)
					_data = new uint8_t[type_size* max_size];
			}

			TickArray()
				: _data(nullptr)
				, _type_size(0)
			{}

			~TickArray() {
				if (_data)
					delete[] _data;
			}

			const uint8_t*  data() const { return _data; }
			size_t          type_size() const { return _type_size; }
			size_t          size() const { return _size; }
			const string&   code() const { return _code; }

			void set_code(const string& code) { _code = code; }

			// Be careful!
			void set_size(int size) { _size = size; }

			void assign(const char* code, uint8_t* data, int type_size, int size) {
				if (this->_data) delete[] this->_data;
				this->_data = data;
				this->_type_size = type_size;
				this->_size = size;
				this->_code = code;

				uint8_t* p = this->_data;
				for (int i = 0; i < _size; i++) {
					*((const char**)p) = _code.c_str();
					p += _type_size;
				}
			}
		  //protected:
			uint8_t*    _data;
			int         _type_size;
			int         _size;
			string      _code;
		};

		template <class T>
		struct _TickArray : public TickArray {

			_TickArray(const string& code, size_t max_size)
				: TickArray(sizeof(T), max_size)
			{
				set_code(code);
			}

			T& operator[] (size_t i) const {
				if (i < this->_size)
					return *reinterpret_cast<T*>(_data + _type_size * i);
				else
					throw std::runtime_error("wrong index");
			}
			T&  at(size_t i) const {
				return *reinterpret_cast<T*>(_data + _type_size * i);
			}

			void push_back(const T& t) {
				auto t2 = reinterpret_cast<T*>(_data + _type_size * _size);
				*t2 = t;
				t2->code = _code.c_str();
				_size++;
			}
		};

#pragma pack(1)
		// keep same with tk_schema!
		struct RawMarketQuote {
			const char*     code;
#if defined(WIN32) && !defined(_WIN64)
			int32_t         _padding_1;
#endif
			int32_t         date;
			int32_t         time;
			int64_t         recv_time;
			int32_t         trading_day;
			double          open;
			double          high;
			double          low;
			double          close;
			double          last;
			double          high_limit;
			double          low_limit;
			double          pre_close;
			int64_t         volume;
			double          turnover;
			double          ask1;
			double          ask2;
			double          ask3;
			double          ask4;
			double          ask5;
			double          bid1;
			double          bid2;
			double          bid3;
			double          bid4;
			double          bid5;
			int64_t         ask_vol1;
			int64_t         ask_vol2;
			int64_t         ask_vol3;
			int64_t         ask_vol4;
			int64_t         ask_vol5;
			int64_t         bid_vol1;
			int64_t         bid_vol2;
			int64_t         bid_vol3;
			int64_t         bid_vol4;
			int64_t         bid_vol5;
			double          settle;
			double          pre_settle;
			int64_t         oi;
			int64_t         pre_oi;
		};

		typedef TickDataHolder<RawMarketQuote> MarketQuote;

		struct RawBar {
			const char*     code;
#if defined(WIN32) && !defined(_WIN64)
			int32_t         _padding_1;
#endif
			int32_t         date;
			int32_t         time;
			int32_t         trading_day;
			double          open;
			double          high;
			double          low;
			double          close;
			int64_t         volume;
			double          turnover;
			int64_t         oi;
		};

		typedef TickDataHolder<RawBar> Bar;

		struct RawDailyBar {
			const char*     code;
#if defined(WIN32) && !defined(_WIN64)
			int32_t         _padding_1;
#endif
			int32_t         date;
			double          open;
			double          high;
			double          low;
			double          close;
			int64_t         volume;
			double          turnover;
			int64_t         oi;
			double          settle;
			double          pre_close;
			double          pre_settle;
			double          af;
		};

		typedef TickDataHolder<RawDailyBar> DailyBar;

#pragma pack()

		typedef _TickArray<RawMarketQuote> MarketQuoteArray;
		typedef _TickArray<RawBar>         BarArray;
		typedef _TickArray<RawDailyBar>    DailyBarArray;


		/**
		*  数据查询接口
		*
		*  功能：
		*      查实时行情，当天的tick, 分钟线
		*      订阅和推送行情
		*/
		class DataApi_Callback {
		public:
			virtual ~DataApi_Callback() { }
			virtual void on_market_quote(shared_ptr<const MarketQuote> quote) = 0;
			virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) = 0;
		};

		template<typename T_VALUE>
		struct CallResult {
			shared_ptr<T_VALUE> value;
			string    msg;

			CallResult(shared_ptr<T_VALUE> a_value)
				: value(a_value)
			{
			}
			CallResult(const string& a_msg)
				: msg(a_msg)
			{
			}
		        CallResult() {
		        }
		};

		class DataApi {
		public:
			DataApi() {}

			virtual ~DataApi() {}

			/**
			* 取某交易日的某个代码的 ticks
			*
			* 当tradingday为0，表示当前交易日
			*
			* @param code
			* @param trading_day
			* @return
			*/
			virtual CallResult<const MarketQuoteArray> tick(const string& code, int trading_day) = 0;

			/**
			* 取某个代码的Bar
			*
			* 目前只支持分钟线
			*  当 cycle == "1m"时，返回trading_day的分钟线，trading_day=0表示当前交易日。
			*
			* @param code          证券代码
			* @param cycle         "1m""
			* @param trading_day   交易日
			* @param align         是否对齐
			* @return
			*/
			virtual CallResult<const BarArray> bar(const string& code, const string& cycle, int trading_day, bool align) = 0;

			/**
			* 取某个代码的日线
			*
			*
			* @param code          证券代码
			* @param price_adj     价格复权，取值
			*                        back -- 后复权
			*                        forward -- 前复权
			* @param align         是否对齐
			* @return
			*/
			virtual CallResult<const DailyBarArray> daily_bar(const string& code, const string& price_adj, bool align) = 0;

			/**
			* 取当前的行情快照
			*
			* @param code
			* @return
			*/
			virtual CallResult<const MarketQuote> quote(const string& code) = 0;

			/**
			* 订阅行情
			*
			* codes为新增的订阅列表，返回所有已经订阅的代码,包括新增的列表。如果codes为空，可以返回已订阅列表。
			*
			* @param codes
			* @return 所有已经订阅的代码
			*/
			virtual CallResult<const vector<string>> subscribe(const vector<string>& codes) = 0;

			/**
			* 取消订阅
			*
			* codes为需要取消的列表，返回所有还在订阅的代码。
			* 如果需要取消所有订阅，先通过 subscribe 得到所有列表，然后使用unscribe取消
			* @param codes
			* @return
			*/
			virtual CallResult<const vector<string>> unsubscribe(const vector<string>& codes) = 0;

			/**
			* 设置推送行情的回调函数
			*
			* 当订阅的代码列表中有新的行情，会通过该callback通知用户。
			*
			* @param callback
			*/
			virtual DataApi_Callback* set_callback(DataApi_Callback* callback) = 0;
		};


		// TradeApi

		struct AccountInfo {
			string account_id;       // 帐号编号
			string broker;           // 交易商名称，如招商证券
			string account;          // 交易帐号
			string status;           // 连接状态，取值 Disconnected, Connected, Connecting
			string msg;              // 状态信息，如登录失败原因
			string account_type;     // 帐号类型，如 stock, ctp
		};

		struct Balance {
			string account_id;       // 帐号编号
			string fund_account;     // 资金帐号
			double init_balance;     // 初始化资金
			double enable_balance;   // 可用资金
			double margin;           // 保证金
			double float_pnl;        // 浮动盈亏
			double close_pnl;        // 实现盈亏

			Balance() : init_balance(0.0), enable_balance(0.0), margin(0.0)
				, float_pnl(0.0), close_pnl(0.0)
			{}
		};

		//struct OrderStatus {
#define OS_New        "New"
#define OS_Accepted   "Accepted"
#define OS_Filled     "Filled"
#define OS_Rejected   "Rejected"
#define OS_Cancelled  "Cancelled"
	//}

	//class EntrustAction {
#define EA_Buy             "Buy"
#define EA_Short           "Short"
#define EA_Cover           "Cover"
#define EA_Sell            "Sell"
#define EA_CoverToday      "CoverToday"
#define EA_CoverYesterday  "CoverYesterday"
#define EA_SellToday       "SellToday"
#define EA_SellYesterday   "SellYesterday"
	//}

		struct Order {
			string  account_id;       // 帐号编号
			string  code;             // 证券代码
			string  name;             // 证券名称
			string  entrust_no;       // 委托编号
			string  entrust_action;   // 委托动作
			double  entrust_price;    // 委托价格
			int64_t entrust_size;     // 委托数量，单位：股
			int32_t entrust_date;     // 委托日期
			int32_t entrust_time;     // 委托时间
			double  fill_price;       // 成交价格
			int64_t fill_size;        // 成交数量
			string  status;           // 订单状态：取值: OrderStatus
			string  status_msg;       // 状态消息
			int32_t order_id;         // 自定义订单编号

			Order()
				: entrust_price(0.0), entrust_size(0), entrust_date(0), entrust_time(0)
				, fill_price(0.0), fill_size(0), order_id(0)
			{}
		};

		struct Trade {
			string  account_id;       // 帐号编号
			string  code;             // 证券代码
			string  name;             // 证券名称
			string  entrust_no;       // 委托编号
			string  entrust_action;   // 委托动作
			string  fill_no;          // 成交编号
			int64_t fill_size;        // 成交数量
			double  fill_price;       // 成交价格
			int32_t fill_date;        // 成交日期
			int32_t fill_time;        // 成交时间
			int32_t order_id;         // 自定义订单编号

			Trade() : fill_size(0), fill_price(0.0), fill_date(0), fill_time(0), order_id(0)
			{}
		};

		// Side {
#define SD_Long "Long"
#define SD_Short "Short"
	//}

		struct Position {
			string  account_id;       // 帐号编号
			string  code;             // 证券代码
			string  name;             // 证券名称
			int64_t current_size;     // 当前持仓
			int64_t enable_size;      // 可用（可交易）持仓
			int64_t init_size;        // 初始持仓
			int64_t today_size;       // 今日持仓
			int64_t frozen_size;      // 冻结持仓
			string  side;             // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
			double  cost;             // 成本
			double  cost_price;       // 成本价格
			double  last_price;       // 最新价格
			double  float_pnl;        // 持仓盈亏
			double  close_pnl;        // 平仓盈亏
			double  margin;           // 保证金
			double  commission;       // 手续费

			Position()
				: current_size(0), enable_size(0), init_size(0), today_size(0), frozen_size(0)
				, cost(0.0), cost_price(0.0), last_price(0.0), float_pnl(0.0), close_pnl(0.0)
				, margin(0.0), commission(0.0)
			{
			}
		};

		struct OrderID {
			string  entrust_no;       // 订单委托号
			int32_t order_id;         // 自定义编号
		};


		class TradeApi_Callback {
		public:
			virtual ~TradeApi_Callback() { }
			virtual void on_order_status(shared_ptr<Order> order) = 0;
			virtual void on_order_trade(shared_ptr<Trade> trade) = 0;
			virtual void on_account_status(shared_ptr<AccountInfo> account) = 0;
		};

		class TradeApi {
		public:
			TradeApi() { }

			virtual ~TradeApi() {}

			/**
			* 查询帐号连接状态。
			*
			* @return
			*/
			virtual CallResult<const vector<AccountInfo>> query_account_status() = 0;

			/**
			* 查询某个帐号的资金使用情况
			*
			* @param account_id
			* @return
			*/
			virtual CallResult<const Balance> query_balance(const string& account_id) = 0;

			/**
			* 查询某个帐号的当天的订单
			*
			* @param account_id
			* @return
			*/
			virtual CallResult<const vector<Order>> query_orders(const string& account_id, const unordered_set<string>* codes = nullptr) = 0;
			virtual CallResult<const vector<Order>> query_orders(const string& account_id, const string& codes) = 0;

			/**
			* 查询某个帐号的当天的成交
			*
			* @param account_id
			* @return
			*/
			virtual CallResult<const vector<Trade>> query_trades(const string& account_id, const unordered_set<string>* codes = nullptr) = 0;
			virtual CallResult<const vector<Trade>> query_trades(const string& account_id, const string& codes) = 0;

			/**
			* 查询某个帐号的当天的持仓
			*
			* @param account_id
			* @return
			*/
			virtual CallResult<const vector<Position>> query_positions(const string& account_id, const unordered_set<string>* codes = nullptr) = 0;
			virtual CallResult<const vector<Position>> query_positions(const string& account_id, const string& codes) = 0;

			/**
			* 下单
			*
			* 股票通道为同步下单模式，即必须下单成功必须返回委托号 entrust_no。
			*
			* CTP交易通道为异步下单模式，下单后立即返回自定义编号order_id。当交易所接受订单，生成委托号好，通过 Callback.on_order_status通知
			* 用户。用户可以通过order_id匹配。如果订单没有被接收，on_order_status回调函数中entrust_no为空，状态为Rejected。
			* 当参数order_id不为0，表示用户自己对订单编号，这时用户必须保证编号的唯一性。如果交易通道不支持order_id，该函数返回错误代码。
			*
			* @param account_id    帐号编号
			* @param code          证券代码
			* @param price         委托价格
			* @param size          委托数量
			* @param action        委托动作
			* @param price_type    价格类型，缺省为限价单(LimitPrice)
			* @param order_id      自定义订单编号，不为0表示有值
			* @return OrderID      订单ID
			*/
			virtual CallResult<const OrderID> place_order(const string& account_id, const string& code, double price, int64_t size, const string& action, const string& price_type, int order_id) = 0;

			/**
			* 根据订单号撤单
			*
			* security 不能为空
			*
			* @param account_id    帐号编号
			* @param code          证券代码
			* @param order_id      订单号
			* @return 是否成功
			*/
			virtual CallResult<bool> cancel_order(const string& account_id, const string& code, int order_id) = 0;

			/**
			* 根据委托号撤单
			*
			* security 不能为空
			*
			* @param account_id    帐号编号
			* @param code          证券代码
			* @param entrust_no    委托编号
			* @return 是否成功
			*/
			virtual CallResult<bool> cancel_order(const string& account_id, const string& code, const string& entrust_no) = 0;

			/**
			* 通用查询接口
			*
			* 用于查询交易通道特有的信息。如查询 CTP的代码表 command="ctp_codetable".
			* 返回字符串。
			*
			* @param account_id
			* @param command
			* @param params
			* @return
			*/
			virtual CallResult<string> query(const string& account_id, const string& command, const string& params) = 0;

			/**
			* 设置 TradeApi.Callback
			*
			* @param callback
			*/
			virtual TradeApi_Callback* set_callback(TradeApi_Callback* callback) = 0;
		};

		_TQAPI_EXPORT DataApi*  create_data_api(const string& addr);

		_TQAPI_EXPORT TradeApi* create_trade_api(const string&  addr);

		_TQAPI_EXPORT void set_params(const string& key, const string& value);

		typedef DataApi*  (*T_create_data_api)(const char* str_params);
		typedef TradeApi* (*T_create_trade_api)(const char* str_params);


		_TQAPI_EXPORT void register_trade_api_factory(T_create_trade_api factory);

	}
}

#endif

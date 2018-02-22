#ifndef _DATA_API_IMPL_H
#define _DATA_API_IMPL_H


namespace tquant { namespace api { namespace impl {

    using namespace ::tquant::api;
    using namespace ::mprpc;

    class DataApiImpl;
    class TradeApiImpl;

    static string builld_errmsg(int code, const string& msg)
    {
        stringstream ss;
        ss << code << "," << msg;
        return ss.str();
    }

    struct BinDataHead {
        uint32_t element_size;
        uint32_t element_count;
        char     data[];

        template<typename T> T* elements() const {
            return reinterpret_cast<T*>(data);
        }
    };

    struct SubInfo {
        unordered_set<string> codes;
        uint64_t              hash_code;

        SubInfo() : hash_code(0)
        {}
    };

    class DataApiImpl : public DataApi {
        mprpc::MpRpcClient*   m_client;
        DataApi_Callback*     m_callback;
        mutex                 m_mtx;
        string                m_source;
        unordered_map<string, SubInfo> m_sub_info_map;
    public:
        DataApiImpl(mprpc::MpRpcClient* client, const char* source) 
            : m_client(client)
            , m_callback(nullptr)
            , m_source(source)
        {}

        virtual ~DataApiImpl() override
        {}

        virtual CallResult<vector<MarketQuote>> tick(const char* code, int trading_day) override
        {
            mprpc::MsgPackPacker pk;
            pk.pack_map(4);
            pk.pack_map_item("code",        code);
            pk.pack_map_item("trading_day", trading_day);
            pk.pack_map_item("_format",     "bin");
            pk.pack_map_item("source",      m_source);

            auto rsp = m_client->call("dapi.tst", pk.sb.data, pk.sb.size);
            if (!is_bin(rsp->result))
                return CallResult<vector<MarketQuote>>(builld_errmsg(rsp->err_code, rsp->err_msg));
            
            const BinDataHead* bin_head = reinterpret_cast<const BinDataHead*>(rsp->result.via.bin.ptr);
            //uint32_t bin_len = rsp->result.via.bin.size;

            if (bin_head->element_size < sizeof(RawMarketQuote))
                return CallResult<vector<MarketQuote>>("-1,wrong data format");
            auto ticks = make_shared<vector<MarketQuote>>();
            const char* p = bin_head->data;
            for (uint32_t i = 0; i < bin_head->element_count; i++) {
                ticks->push_back(MarketQuote(*reinterpret_cast<const RawMarketQuote*>(p), code));
                p += bin_head->element_size;
            }
            return CallResult<vector<MarketQuote>>(ticks);
        }

        virtual CallResult<vector<Bar>> bar(const char* code, const char* cycle, int trading_day, bool align) override
        {
            MsgPackPacker pk;
            pk.pack_map(6);
            pk.pack_map_item("code",        code);
            pk.pack_map_item("cycle",       cycle);
            pk.pack_map_item("trading_day", trading_day);
            pk.pack_map_item("align",       align);
            pk.pack_map_item("_format",     "bin");
            pk.pack_map_item("source",      m_source);

            auto rsp = m_client->call("dapi.tsi", pk.sb.data, pk.sb.size);
            if (!is_bin(rsp->result))
                return CallResult<vector<Bar>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            const BinDataHead* bin_head = reinterpret_cast<const BinDataHead*>(rsp->result.via.bin.ptr);
            //uint32_t bin_len = rsp->result.via.bin.size;

            if (bin_head->element_size < sizeof(RawBar))
                return CallResult<vector<Bar>>("-1,wrong data format");
            auto bars = make_shared<vector<Bar>>();
            const char* p = bin_head->data;
            for (uint32_t i = 0; i < bin_head->element_count; i++) {
                bars->push_back(Bar(*reinterpret_cast<const RawBar*>(p), code));
                p += bin_head->element_size;
            }
            return CallResult<vector<Bar>>(bars);
        }

        virtual CallResult<vector<DailyBar>> daily_bar(const char* code, const char* price_adj, bool align) override
        {
            MsgPackPacker pk;
            pk.pack_map(6);
            pk.pack_map_item("code",        code);
            pk.pack_map_item("cycle",       "1d");
            pk.pack_map_item("price_adj",   price_adj);
            pk.pack_map_item("align",       align);
            pk.pack_map_item("_format",     "bin");
            pk.pack_map_item("source",       m_source);

            auto rsp = m_client->call("dapi.tsi", pk.sb.data, pk.sb.size);
            if (!is_bin(rsp->result))
                return CallResult<vector<DailyBar>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            const BinDataHead* bin_head = reinterpret_cast<const BinDataHead*>(rsp->result.via.bin.ptr);
            //uint32_t bin_len = rsp->result.via.bin.size;

            if (bin_head->element_size < sizeof(RawDailyBar))
                return CallResult<vector<DailyBar>>("-1,wrong data format");

            auto bars = make_shared<vector<DailyBar>>();
            const char* p = bin_head->data;
            for (uint32_t i = 0; i < bin_head->element_count; i++) {
                bars->push_back(DailyBar(*reinterpret_cast<const RawDailyBar*>(p), code));
                p += bin_head->element_size;
            }
            return CallResult<vector<DailyBar>>(bars);
        }

        virtual CallResult<MarketQuote> quote(const char* code) override
        {
            MsgPackPacker pk;
            pk.pack_map(3);
            pk.pack_map_item("code",    code);
            pk.pack_map_item("_format", "bin");
            pk.pack_map_item("source",  m_source);

            auto rsp = m_client->call("dapi.tsq_quote", pk.sb.data, pk.sb.size);
            if (!is_bin(rsp->result))
                return CallResult<MarketQuote>(builld_errmsg(rsp->err_code, rsp->err_msg));

            const char* p = (const char*)(rsp->result.via.bin.ptr);
            uint32_t bin_len = rsp->result.via.bin.size;

            size_t code_len = strlen(p);
            if ( bin_len < code_len + 1 + sizeof(RawMarketQuote))
                return CallResult<MarketQuote>("-1,wrong data format");

            auto quote = make_shared<MarketQuote>(*(const RawMarketQuote*)(p+code_len+1), code);
            return CallResult<MarketQuote>(quote);
        }

        CallResult<vector<string>> update_subscribe_result(msgpack_object& result)
        {
            unique_lock<mutex> lock(m_mtx);

            if (!is_map(result))
                return CallResult<vector<string>>("-1,wrong data format");

            string new_codes;
            string source;
            uint64_t sub_hash;
            mp_map_get(result, "sub_hash", (int64_t*)&sub_hash);
            mp_map_get(result, "sub_codes", &new_codes);
            mp_map_get(result, "source",    &source);

            vector<string> sub_codes;
            split(new_codes, ",", &sub_codes);

            if (source.size()) {
                auto si = &m_sub_info_map[source];
                si->codes.clear();
                for (auto& s : sub_codes) si->codes.insert(s);
                si->hash_code = sub_hash;
            }

            return CallResult<vector<string>>(make_shared <vector<string>>(sub_codes));
        }

        void subscribe_again()
        {
            unique_lock<mutex> lock(m_mtx);

            for (auto& e : m_sub_info_map) {
                auto si = &e.second;
                if (si->codes.empty()) continue;

                stringstream ss;
                for (auto it = si->codes.begin(); it != si->codes.end(); it++) {
                    if (it != si->codes.end())
                        ss << *it << ",";
                    else
                        ss << *it;
                }

                MsgPackPacker pk;
                pk.pack_map(3);
                pk.pack_map_item("codes",        ss.str());
                pk.pack_map_item("want_bin_fmt", true);
                pk.pack_map_item("source",       e.first);

                m_client->call("dapi.tsq_sub", pk.sb.data, pk.sb.size, 0);
            }
        }

        virtual CallResult<vector<string>> subscribe(const vector<string>& codes) override
        {
            stringstream ss;
            for (size_t i = 0; i < codes.size(); i++) {
                if (i != codes.size() - 1)
                    ss << codes[i] << ",";
                else
                    ss << codes[i];
            }

            MsgPackPacker pk;
            pk.pack_map(3);
            pk.pack_map_item ("codes",        ss.str());
            pk.pack_map_item ("want_bin_fmt", true);
            pk.pack_map_item("source",        m_source);

            auto rsp = m_client->call("dapi.tsq_sub", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<vector<string>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            return update_subscribe_result(rsp->result);
        }

        virtual CallResult<vector<string>> unsubscribe(const vector<string>& codes) override
        {
            MsgPackPacker pk_codes;
            pk_codes.pack_array(codes.size());
            for (auto & code : codes)
                pk_codes.pack_string(code);

            stringstream ss;
            for (size_t i = 0; i < codes.size(); i++) {
                if (i != codes.size() - 1)
                    ss << codes[i] << ",";
                else
                    ss << codes[i];
            }

            MsgPackPacker pk;
            pk.pack_map(3);
            pk.pack_map_item("codes",        ss.str());
            pk.pack_map_item("want_bin_fmt", true);
            pk.pack_map_item("source",       m_source);

            auto rsp = m_client->call("dapi.tsq_unsub", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<vector<string>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_map(rsp->result))
                return CallResult<vector<string>>("-1,wrong data format");

            return update_subscribe_result(rsp->result);
        }

        virtual void set_callback(DataApi_Callback* callback) override
        {
            m_callback = callback;
        }

        void on_notification(shared_ptr<mprpc::MpRpcMessage> rpcmsg)
        {
            if (rpcmsg->method == "dapi.quote") {
                if (m_callback && is_bin(rpcmsg->params)) {
                    //static int64_t tick_count;
                    //static int64_t csum_time;

                    //tick_count++;
                    //csum_time += duration_cast<microseconds>(system_clock::now() - rpcmsg->recv_time).count();
                    //if (tick_count % 10 == 0) {
                    //    std::cout << "callback delay: " << (csum_time / tick_count) << " microseconds" << endl;
                    //    tick_count = 0;
                    //    csum_time = 0;
                    //}

                    const char* code = rpcmsg->params.via.bin.ptr;
                    auto quote = make_shared<MarketQuote>(*(RawMarketQuote*)(code + strlen(code) + 1), code);
                    m_callback->on_market_quote(quote);
                }
            }
            else if (rpcmsg->method == "dapi.bar") {
                if (m_callback && is_bin(rpcmsg->params)) {
                    const char* cycle = rpcmsg->params.via.bin.ptr;
                    const char* code = cycle + strlen(cycle) + 1;
                    const RawBar* rbar = reinterpret_cast<const RawBar*>(code + strlen(code) + 1);

                    auto bar = make_shared<Bar>(*rbar, code);
                    m_callback->on_bar(cycle, bar);
                }
            }
            else if (rpcmsg->method == ".sys.heartbeat") {
                if (!is_map(rpcmsg->result)) return;
                msgpack_object sub_info;
                if (mp_map_get(rpcmsg->result, "sub_info", &sub_info)) {
                    unordered_map<string, uint64_t> new_sub_info;
                    if (is_arr(sub_info)) {
                        for (uint32_t i = 0; i < sub_info.via.array.size; i++) {
                            msgpack_object* tmp = sub_info.via.array.ptr + i;
                            if (!is_map(*tmp)) continue;
                            string source;
                            uint64_t sub_hash = 0;
                            mp_map_get(*tmp, "source",   &source);
                            mp_map_get(*tmp, "sub_hash", &sub_hash);
                            if (source.size())
                                new_sub_info[source] = sub_hash;
                        }
                    }

                    bool mismatch = false;
                    for (auto & e : m_sub_info_map) {
                        if (new_sub_info[e.first] != e.second.hash_code) {
                            mismatch = true;
                            break;
                        }
                    }
                    if (mismatch) subscribe_again();
                }
            }
            else if (rpcmsg->method == "dapi.tsq_sub") {
                update_subscribe_result(rpcmsg->result);
            }
        }
    };


} } }

#endif

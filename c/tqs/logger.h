#ifndef _tQUANT_LOGGER_H
#define _tQUANT_LOGGER_H

#include <ostream>
#include <memory>

using namespace std;

namespace tquant {
    namespace stralet {

        enum LogSeverity {
            INFO,
            WARNING,
            ERROR,
            FATAL
        };

        class LogStreamBuf : public std::streambuf {
        public:
            LogStreamBuf(LogSeverity leverl, int date, int time);

            virtual int_type overflow(int_type ch) {
                return ch;
            }

            ~LogStreamBuf();

            size_t pcount() const { return pptr() - pbase(); }
            char* pbase() const { return std::streambuf::pbase(); }

            char* buf;
            LogSeverity severity;
        };

        class LogStream : public std::ostream {
        public:
            LogStream(shared_ptr<LogStreamBuf> streambuf__)
                : std::ostream(NULL),
                streambuf_(streambuf__) {
                rdbuf(streambuf_.get());
            }

            ~LogStream() {

            }
            size_t pcount() const { return streambuf_->pcount(); }
            char* pbase() const { return streambuf_->pbase(); }
            char* str() const { return pbase(); }

            LogStream(LogStream& left)
                : std::ostream(NULL) {
                this->streambuf_ = left.streambuf_;
            }

            shared_ptr<LogStreamBuf> streambuf_;
        };
    }
}

#endif

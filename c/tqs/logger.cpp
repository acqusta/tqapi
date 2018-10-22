/* Most code are copied from glog */
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "logger.h"
#include "myutils/timeutils.h"

#ifdef _WIN32
# include <process.h>
# include "Windows.h"
# undef ERROR
#else
# include <unistd.h>
#endif

using namespace std;

namespace tquant {

    namespace stralet {

        const int buf_len = 1024 * 64;

        static const char*const LogSeverityNames[] = {
            "I", "W", "E", "F"
        };

        static void color_console_output(const char* buf, int len, LogSeverity severity);
        static void file_output(const char* buf, int len);

        LogStreamBuf::LogStreamBuf(LogSeverity a_severity, int date, int time)
            : severity(a_severity)
        {
            buf = new char[buf_len];

            int ms = time % 1000;
            time /= 1000;
            int h = time / 10000;
            int m = (time / 100) % 100;
            int s = time % 100;
            int n = sprintf(buf, "%s%04d %02d:%02d:%02d.%03d ",
                LogSeverityNames[severity],
                date % 10000,
                h, m, s, ms);

            setp(buf + n , buf + buf_len - 2);
        }

        LogStreamBuf::~LogStreamBuf()
        {
            size_t len = pcount() + pbase() - buf;
            if (buf[len - 1] != '\n') {
                buf[len++] = '\n';
            }
            buf[len] = '\0';

            color_console_output(buf, len, severity);
            file_output(buf, len);

            if (severity == LogSeverity::FATAL) {
#if defined(_DEBUG) && defined(_MSC_VER)
                // When debugging on windows, avoid the obnoxious dialog and make
                // it possible to continue past a LOG(FATAL) in the debugger
                __debugbreak();
#else
                abort();
#endif
            }
        }


        static ofstream out;
        static int cache_len = 0;

        void file_output(const char* buf, int len)
        {
            if (!out.is_open()) {
                int date, time;
                fin_datetime(&date, &time);

                char buf[100];
                sprintf(buf, "%8d-%8d.%d.log", date, time, getpid());                
                out.open(buf, ios::app);
            }

            if (out.is_open()) {
                out.write(buf, len);
                cache_len += len;
                if (cache_len > 1024 * 32) out.flush();
            }
        }

        enum GLogColor {
            COLOR_DEFAULT,
            COLOR_RED,
            COLOR_GREEN,
            COLOR_YELLOW
        };

        static GLogColor SeverityToColor(LogSeverity severity) {
            //assert(severity >= 0 && severity < NUM_SEVERITIES);
            GLogColor color = COLOR_DEFAULT;
            switch (severity) {
            case LogSeverity::INFO:
                color = COLOR_DEFAULT;
                break;
            case LogSeverity::WARNING:
                color = COLOR_YELLOW;
                break;
            case LogSeverity::ERROR:
            case LogSeverity::FATAL:
                color = COLOR_RED;
                break;
            default:
                // should never get here.
                assert(false);
            }
            return color;
        }
        static bool TerminalSupportsColor() {
            bool term_supports_color = false;
#ifdef WIN32
            // on Windows TERM variable is usually not set, but the console does
            // support colors.
            term_supports_color = true;
#else
            // On non-Windows platforms, we rely on the TERM variable.
            const char* const term = getenv("TERM");
            if (term != NULL && term[0] != '\0') {
                term_supports_color =
                    !strcmp(term, "xterm") ||
                    !strcmp(term, "xterm-color") ||
                    !strcmp(term, "xterm-256color") ||
                    !strcmp(term, "screen-256color") ||
                    !strcmp(term, "screen") ||
                    !strcmp(term, "linux") ||
                    !strcmp(term, "cygwin");
            }
#endif
            return term_supports_color;
        }

#ifdef WIN32
        // Returns the character attribute for the given color.
        WORD GetColorAttribute(GLogColor color) {
            switch (color) {
            case COLOR_RED:    return FOREGROUND_RED;
            case COLOR_GREEN:  return FOREGROUND_GREEN;
            case COLOR_YELLOW: return FOREGROUND_RED | FOREGROUND_GREEN;
            default:           return 0;
            }
        }

#else
        // Returns the ANSI color code for the given color.
        const char* GetAnsiColorCode(GLogColor color) {
            switch (color) {
            case COLOR_RED:     return "1";
            case COLOR_GREEN:   return "2";
            case COLOR_YELLOW:  return "3";
            case COLOR_DEFAULT:  return "";
            };
            return NULL; // stop warning about return type.
        }

#endif
        static bool terminal_supports_color = TerminalSupportsColor();
        void color_console_output(const char* message, int len, LogSeverity severity)
        {
            const GLogColor color = terminal_supports_color ?
                SeverityToColor(severity) : COLOR_DEFAULT;

            // Avoid using cerr from this module since we may get called during
            // exit code, and cerr may be partially or fully destroyed by then.
            if (COLOR_DEFAULT == color) {
                fwrite(message, len, 1, stderr);
                return;
            }
#ifdef WIN32
            const HANDLE stderr_handle = GetStdHandle(STD_ERROR_HANDLE);

            // Gets the current text color.
            CONSOLE_SCREEN_BUFFER_INFO buffer_info;
            GetConsoleScreenBufferInfo(stderr_handle, &buffer_info);
            const WORD old_color_attrs = buffer_info.wAttributes;

            // We need to flush the stream buffers into the console before each
            // SetConsoleTextAttribute call lest it affect the text that is already
            // printed but has not yet reached the console.
            fflush(stderr);
            SetConsoleTextAttribute(stderr_handle,
                GetColorAttribute(color) | FOREGROUND_INTENSITY);
            fwrite(message, len, 1, stderr);
            fflush(stderr);
            // Restores the text color.
            SetConsoleTextAttribute(stderr_handle, old_color_attrs);
#else
            fprintf(stderr, "\033[0;3%sm", GetAnsiColorCode(color));
            fwrite(message, len, 1, stderr);
            fprintf(stderr, "\033[m");  // Resets the terminal to default.
#endif
        }
    }
}

#ifndef MPV_LOG_LEVEL_H_
#define MPV_LOG_LEVEL_H_

/**
 * Numeric log levels. The lower the number, the more important the message is.
 * MPV_LOG_LEVEL_NONE is never used when receiving messages. The string in
 * the comment after the value is the name of the log level as used for the
 * mpv_request_log_messages() function.
 * Unused numeric values are unused, but reserved for future use.
 */
typedef enum mpv_log_level {
    MPV_LOG_LEVEL_NONE  = 0,    /// "no"    - disable absolutely all messages
    MPV_LOG_LEVEL_FATAL = 10,   /// "fatal" - critical/aborting errors
    MPV_LOG_LEVEL_ERROR = 20,   /// "error" - simple errors
    MPV_LOG_LEVEL_WARN  = 30,   /// "warn"  - possible problems
    MPV_LOG_LEVEL_INFO  = 40,   /// "info"  - informational message
    MPV_LOG_LEVEL_V     = 50,   /// "v"     - noisy informational message
    MPV_LOG_LEVEL_DEBUG = 60,   /// "debug" - very noisy technical information
    MPV_LOG_LEVEL_TRACE = 70,   /// "trace" - extremely noisy
} mpv_log_level;

#endif

#ifndef TSIM_LOG_H_
#define TSIM_LOG_H_

#include "tsim/tsim_macro.h"
#include <stdbool.h>

EXTERN_C_BEGIN

#if TSIM_USE_ASYNC_LOG
	#include "haclog/haclog.h"

	// use haclog
	#define LOG_LEVEL_TRACE HACLOG_LEVEL_TRACE
	#define LOG_LEVEL_DEBUG HACLOG_LEVEL_DEBUG
	#define LOG_LEVEL_INFO HACLOG_LEVEL_INFO
	#define LOG_LEVEL_WARNING HACLOG_LEVEL_WARNING
	#define LOG_LEVEL_ERROR HACLOG_LEVEL_ERROR
	#define LOG_LEVEL_FATAL HACLOG_LEVEL_FATAL

	#define LOG_TRACE(format, ...) HACLOG_TRACE(format, ##__VA_ARGS__)
	#define LOG_DEBUG(format, ...) HACLOG_DEBUG(format, ##__VA_ARGS__)
	#define LOG_INFO(format, ...) HACLOG_INFO(format, ##__VA_ARGS__)
	#define LOG_WARNING(format, ...) HACLOG_WARNING(format, ##__VA_ARGS__)
	#define LOG_ERROR(format, ...) HACLOG_ERROR(format, ##__VA_ARGS__)
	#define LOG_FATAL(format, ...) HACLOG_FATAL(format, ##__VA_ARGS__)

	#define LOG_ASSERT(x) HACLOG_ASSERT(x)
	#define LOG_ASSERT_MSG(x, format, ...) \
		HACLOG_ASSERT_MSG(x, format, ##__VA_ARGS__)
#else
	#include "muggle/c/log/log.h"

	// use muggle log
	#define LOG_LEVEL_TRACE MUGGLE_LOG_LEVEL_TRACE
	#define LOG_LEVEL_DEBUG MUGGLE_LOG_LEVEL_DEBUG
	#define LOG_LEVEL_INFO MUGGLE_LOG_LEVEL_INFO
	#define LOG_LEVEL_WARNING MUGGLE_LOG_LEVEL_WARNING
	#define LOG_LEVEL_ERROR MUGGLE_LOG_LEVEL_ERROR
	#define LOG_LEVEL_FATAL MUGGLE_LOG_LEVEL_FATAL

	#define LOG_TRACE(format, ...) MUGGLE_LOG_TRACE(format, ##__VA_ARGS__)
	#define LOG_DEBUG(format, ...) MUGGLE_LOG_DEBUG(format, ##__VA_ARGS__)
	#define LOG_INFO(format, ...) MUGGLE_LOG_INFO(format, ##__VA_ARGS__)
	#define LOG_WARNING(format, ...) MUGGLE_LOG_WARNING(format, ##__VA_ARGS__)
	#define LOG_ERROR(format, ...) MUGGLE_LOG_ERROR(format, ##__VA_ARGS__)
	#define LOG_FATAL(format, ...) MUGGLE_LOG_FATAL(format, ##__VA_ARGS__)

	#define LOG_ASSERT(x) MUGGLE_ASSERT(x)
	#define LOG_ASSERT_MSG(x, format, ...) \
		MUGGLE_ASSERT_MSG(x, format, ##__VA_ARGS__)
#endif

typedef void (*tsim_log_before_run_callback)();

/**
 * @brief 
 *
 * @param console_level  console output level; if it's -1, disable console log
 * @param file_level     file output level; if it's -1, disable file log
 * @param filepath       output filepath
 * @param use_rotate     use daily rotate log
 *                           - 1: use rotate
 *                           - 0: don't rotate
 * @param fn             before log run callback
 *
 * @return boolean
 */
TSIM_EXPORT
bool tsim_log_init(int console_level, int file_level, const char *filepath,
				   bool use_rotate, tsim_log_before_run_callback fn);

/**
 * @brief initialize log thread context
 */
TSIM_EXPORT
void tsim_log_init_thread_ctx();

/**
 * @brief cleanup log thread context
 */
TSIM_EXPORT
void tsim_log_cleanup_thread_ctx();

/**
 * @brief convert log string to level
 *
 * @param s  log string
 *
 * @return log level, when error, return -1
 */
TSIM_EXPORT
int tsim_log_str_to_level(const char *s);

/**
 * @brief convert log level to string
 *
 * @param level  log level
 *
 * @return string log level
 */
TSIM_EXPORT
const char *tsim_log_level_to_str(int level);

EXTERN_C_END

#endif // !TSIM_LOG_H_

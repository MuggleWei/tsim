#include "tsim_log.h"

#if TSIM_USE_ASYNC_LOG

	#include <string.h>
	#include <stdlib.h>

static bool log_add_console_handler(int level)
{
	static haclog_console_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_console_handler_init(&handler, 1) != 0) {
		fprintf(stderr, "failed init console handler");
		return false;
	}

	haclog_handler_set_level((haclog_handler_t *)&handler, level);
	haclog_context_add_handler((haclog_handler_t *)&handler);

	return true;
}

static bool log_add_file_handle(int level, const char *filepath)
{
	static haclog_file_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_file_handler_init(&handler, filepath, "a") != 0) {
		fprintf(stderr, "failed init file handler");
		return false;
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, level);
	haclog_context_add_handler((haclog_handler_t *)&handler);
	return true;
}

static bool log_add_timerot_handle(int level, const char *filepath)
{
	static haclog_file_time_rot_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_file_time_rotate_handler_init(
			&handler, filepath, HACLOG_TIME_ROTATE_UNIT_DAY, 1, 1) != 0) {
		fprintf(stderr, "failed init file time rotate handler");
		return false;
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, level);
	haclog_context_add_handler((haclog_handler_t *)&handler);

	return true;
}

bool tsim_log_init(int console_level, int file_level, const char *filepath,
				   bool use_rotate, tsim_log_before_run_callback fn)
{
	if (console_level >= 0) {
		if (!log_add_console_handler(console_level)) {
			fprintf(stderr, "failed add console log handler");
			return false;
		}
	}

	if (file_level >= 0) {
		if (use_rotate) {
			if (!log_add_timerot_handle(file_level, filepath)) {
				fprintf(stderr, "failed add file log handler");
				return false;
			}
		} else {
			if (!log_add_file_handle(file_level, filepath)) {
				fprintf(stderr, "failed add file log handler");
				return false;
			}
		}
	}

	if (fn != NULL) {
		haclog_context_set_before_run_cb(fn);
	}

	haclog_backend_run();

	return true;
}

void tsim_log_init_thread_ctx()
{
	haclog_thread_context_init();
}

void tsim_log_cleanup_thread_ctx()
{
	haclog_thread_context_cleanup();
}

#else

	#include "muggle/c/muggle_c.h"
	#include <stdio.h>

static int log_complicated_init_fmt(const muggle_log_msg_t *msg, char *buf,
									size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	unsigned long long tid = muggle_thread_current_readable_id();

	struct tm t;
	gmtime_r(&msg->ts.tv_sec, &t);

	const char *payload = "";
	if (msg->payload) {
		payload = msg->payload;
	}

	return (int)snprintf(
		buf, bufsize,
		"%s|%d-%02d-%02dT%02d:%02d:%02d.%09d|%s:%u|%s|%llu - %s\n", level,
		(int)t.tm_year + 1900, (int)t.tm_mon + 1, (int)t.tm_mday,
		(int)t.tm_hour, (int)t.tm_min, (int)t.tm_sec, (int)msg->ts.tv_nsec,
		filename, (unsigned int)msg->src_loc.line, msg->src_loc.func,
		(unsigned long long)tid, payload);
}

bool tsim_log_init(int console_level, int file_level, const char *filepath,
				   bool use_rotate, tsim_log_before_run_callback fn)
{
	MUGGLE_UNUSED(fn);

	static muggle_log_fmt_t formatter = { MUGGLE_LOG_FMT_LEVEL |
											  MUGGLE_LOG_FMT_FILE |
											  MUGGLE_LOG_FMT_FUNC |
											  MUGGLE_LOG_FMT_TIME,
										  log_complicated_init_fmt };
	muggle_logger_t *logger = muggle_logger_default();

	if (console_level >= 0) {
		static muggle_log_console_handler_t console_handler;
		muggle_log_console_handler_init(&console_handler, 1);
		muggle_log_handler_set_level((muggle_log_handler_t *)&console_handler,
									 console_level);
		muggle_log_handler_set_fmt((muggle_log_handler_t *)&console_handler,
								   &formatter);

		logger->add_handler(logger, (muggle_log_handler_t *)&console_handler);
	}

	if (file_level >= 0) {
		if (use_rotate) {
			static muggle_log_file_time_rot_handler_t rot_file_handler;
			int ret = muggle_log_file_time_rot_handler_init(
				&rot_file_handler, filepath, MUGGLE_LOG_TIME_ROTATE_UNIT_DAY, 1,
				false);
			if (ret != 0) {
				fprintf(stderr,
						"failed init file rotate handler with path: %s\n",
						filepath);
				return false;
			}
			muggle_log_handler_set_level(
				(muggle_log_handler_t *)&rot_file_handler, file_level);
			muggle_log_handler_set_fmt(
				(muggle_log_handler_t *)&rot_file_handler, &formatter);

			logger->add_handler(logger,
								(muggle_log_handler_t *)&rot_file_handler);
		} else {
			static muggle_log_file_handler_t file_handler;
			int ret =
				muggle_log_file_handler_init(&file_handler, filepath, "a");
			if (ret != 0) {
				fprintf(stderr, "failed init file handler with path: %s\n",
						filepath);
			}

			muggle_log_handler_set_level((muggle_log_handler_t *)&file_handler,
										 file_level);
			muggle_log_handler_set_fmt((muggle_log_handler_t *)&file_handler,
									   &formatter);
			logger->add_handler(logger, (muggle_log_handler_t *)&file_handler);
		}
	}

	return true;
}

void tsim_log_init_thread_ctx()
{
}

void tsim_log_cleanup_thread_ctx()
{
}

#endif

int tsim_log_str_to_level(const char *s)
{
	if (strcmp(s, "TRACE") == 0 || strcmp(s, "trace") == 0) {
		return LOG_LEVEL_TRACE;
	}
	if (strcmp(s, "DEBUG") == 0 || strcmp(s, "debug") == 0) {
		return LOG_LEVEL_DEBUG;
	}
	if (strcmp(s, "INFO") == 0 || strcmp(s, "info") == 0) {
		return LOG_LEVEL_INFO;
	}
	if (strcmp(s, "WARNING") == 0 || strcmp(s, "warning") == 0 ||
		strcmp(s, "WARN") == 0 || strcmp(s, "warn") == 0) {
		return LOG_LEVEL_WARNING;
	}
	if (strcmp(s, "ERROR") == 0 || strcmp(s, "error") == 0) {
		return LOG_LEVEL_ERROR;
	}
	if (strcmp(s, "FATAL") == 0 || strcmp(s, "fatal") == 0) {
		return LOG_LEVEL_FATAL;
	}
	return -1;
}

const char *tsim_log_level_to_str(int level)
{
	switch (level) {
	case LOG_LEVEL_TRACE: {
		return "TRACE";
	} break;
	case LOG_LEVEL_DEBUG: {
		return "DEBUG";
	} break;
	case LOG_LEVEL_INFO: {
		return "INFO";
	} break;
	case LOG_LEVEL_WARNING: {
		return "WARNING";
	} break;
	case LOG_LEVEL_ERROR: {
		return "ERROR";
	} break;
	case LOG_LEVEL_FATAL: {
		return "FATAL";
	} break;
	default: {
		return "(unknown)";
	} break;
	}
}

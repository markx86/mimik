#ifndef MIMIK_LOG_H
#define MIMIK_LOG_H

#include <log/printk.h>

#define LOG_COLOR_TRACE "\033[36m"
#define LOG_COLOR_SUCCESS "\033[32m"
#define LOG_COLOR_WARNING "\033[33m"
#define LOG_COLOR_ERROR "\033[31m"
#define LOG_COLOR_FATAL "\033[101m"
#define LOG_COLOR_RESET "\033[0m"

#define LOGTRACE(fmt, ...) \
	printk("[" LOG_COLOR_TRACE "*" LOG_COLOR_RESET "] " fmt "\n", ##__VA_ARGS__)
#define LOGSUCCESS(fmt, ...) \
	printk("[" LOG_COLOR_SUCCESS "+" LOG_COLOR_RESET "] " fmt "\n", ##__VA_ARGS__)
#define LOGWARNING(fmt, ...) \
	printk("[" LOG_COLOR_WARNING "~" LOG_COLOR_RESET "] " fmt "\n", ##__VA_ARGS__)
#define LOGERROR(fmt, ...) \
	printk("[" LOG_COLOR_ERROR "-" LOG_COLOR_RESET "] " fmt "\n", ##__VA_ARGS__)

#endif

#ifndef EQEMU_ERROR_LOG_H
#define EQEMU_ERROR_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string>

#include "../common/Mutex.h"

using namespace std;

/**
 * Dictates the log type specified in ErrorLog for Log(...)
 */
enum eqLogType
{
	log_debug,
	log_error,
	log_database,
	log_network,
	log_network_trace,
	log_network_error,
	log_world,
	log_world_error,
	log_client,
	log_client_error,
	_log_largest_type
};

/**
 * Basic error logging class.
 * Thread safe logging class that records time and date to both a file and to console(if exists).
 */
class ErrorLog
{
public:
	/**
	 * Constructor: opens the log file for writing and creates our mutex for writing to the log.
	 */
	ErrorLog(const char* file_name);

	/**
	 * Closes the file and destroys the mutex.
	 */
	~ErrorLog();

	/**
	 * Writes to the log system a variable message.
	 */
	void Log(eqLogType type, const char *message, ...);

protected:
	Mutex *log_mutex;
	FILE* error_log;
};

#endif


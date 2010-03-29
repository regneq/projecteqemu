/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2010  EQEMu Development Team (http://eqemulator.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "ErrorLog.h"

const char *eqLogTypes[_log_largest_type] =
{
	"Debug",
	"Error",
	"Database",
	"Network",
	"Network Trace",
	"Network Error",
	"World",
	"World Error",
	"Client",
	"Client Error"
};

ErrorLog::ErrorLog(const char* file_name)
{
	log_mutex = new Mutex();
	error_log = fopen(file_name, "w");
}

ErrorLog::~ErrorLog()
{
	log_mutex->lock();
	if(error_log)
	{
		fclose(error_log);
	}
	log_mutex->unlock();
	delete log_mutex;
}

void ErrorLog::Log(eqLogType type, const char *message, ...)
{
	if(type == _log_largest_type || error_log == NULL)
	{
		return;
	}

	va_list argptr;
	char *buffer = new char[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	time_t m_clock;
	struct tm *m_time;
	time(&m_clock);
	m_time = localtime(&m_clock);

	log_mutex->lock();
	printf("[%s] [%02d.%02d.%02d - %02d:%02d:%02d] %s\n", 
		eqLogTypes[type], 
		m_time->tm_mon+1, 
		m_time->tm_mday, 
		m_time->tm_year%100, 
		m_time->tm_hour, 
		m_time->tm_min, 
		m_time->tm_sec, 
		buffer);

	fprintf(error_log, "[%s] [%02d.%02d.%02d - %02d:%02d:%02d] %s\n", 
		eqLogTypes[type], 
		m_time->tm_mon+1, 
		m_time->tm_mday, 
		m_time->tm_year%100, 
		m_time->tm_hour, 
		m_time->tm_min, 
		m_time->tm_sec, 
		buffer);
	fflush(error_log);
	
	log_mutex->unlock();
	delete[] buffer;
}


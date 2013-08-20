#pragma once

#include <systemd/sd-daemon.h>
#include <iostream>


#define __LOG(PREFIX, TEXT) { std::cerr << PREFIX << TEXT << std::endl; }

#define LOG_FATAL(TEXT)    __LOG(SD_EMERG,   TEXT)
#define LOG_ALERT(TEXT)    __LOG(SD_ALERT,   TEXT)
#define LOG_CRITICAL(TEXT) __LOG(SD_CRIT,    TEXT)
#define LOG_ERROR(TEXT)    __LOG(SD_ERR,     TEXT)
#define LOG_WARNING(TEXT)  __LOG(SD_WARNING, TEXT)
#define LOG_NOTICE(TEXT)   __LOG(SD_NOTICE,  TEXT)
#define LOG_INFO(TEXT)     __LOG(SD_INFO,    TEXT)
#define LOG_DEBUG(TEXT)    __LOG(SD_DEBUG,   TEXT)


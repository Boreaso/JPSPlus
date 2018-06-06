#pragma once

#define __DEBUG__  0

#if __DEBUG__
#define LOG(format, ...)  printf(format, ##__VA_ARGS__)
#else
#define LOG(format,...)  
#endif 
#ifndef _FLOW_UTIL_H_
#define _FLOW_UTIL_H_

#include <time.h>

extern void time_parse_gpstime(struct tm * restrict tm, char const *time, size_t len);
extern void time_add_hours(struct tm * restrict tm, int hours);
extern void time_to_string_cn(const struct tm *tm, char * restrict dest, size_t size);

#endif

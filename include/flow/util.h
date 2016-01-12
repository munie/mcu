#ifndef _FLOW_UTIL_H_
#define _FLOW_UTIL_H_

extern void gpstime_to_normal(char * restrict normal, unsigned int size, char const * gpstime);
extern void gpstime_add_eight_hours(char * restrict time);

#endif

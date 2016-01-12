#ifndef _FLOW_UTIL_H_
#define _FLOW_UTIL_H_

extern void gpstime_to_normal(char * restrict normal, unsigned int normal_size,
    char const * gpstime, unsigned int gpstime_size);
extern void gpstime_add_eight_hours(char * restrict time);

#endif

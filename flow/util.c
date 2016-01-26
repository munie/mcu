// util.c

#include "util.h"
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#define CTOI(c) (c - 0x30)
#define ITOC(c) (c + 0x30)

int time_parse_gpstime(struct tm * restrict tm, char const *time, size_t len)
{
    if (len < 14) return -1;

    // 0 ~ 3 : year
    // 4 ~ 5 : mon
    // 6 ~ 7 : day
    // 8 ~ 9 : hour
    // 10 ~ 11 : min
    // 12 ~ 13 : sec
    // 20160229160601
    tm->tm_sec = CTOI(time[12]) * 10 + CTOI(time[13]);
    tm->tm_min = CTOI(time[10]) * 10 + CTOI(time[11]);
    tm->tm_hour = CTOI(time[8]) * 10 + CTOI(time[9]);
    tm->tm_mday = CTOI(time[6]) * 10 + CTOI(time[7]);
    tm->tm_mon = CTOI(time[4]) * 10 + CTOI(time[5]) - 1;
    tm->tm_year = CTOI(time[0]) * 1000 + CTOI(time[1]) * 100 + CTOI(time[2]) * 10 + CTOI(time[3]) - 1900;

    return 0;
}

void time_add_hours(struct tm * restrict tm, int hours)
{
    int hour, day, mon, year;
    bool isLeapYear = false;

    hour = tm->tm_hour + hours;
    day = tm->tm_mday;
    mon = tm->tm_mon + 1;
    year = tm->tm_year + 1900;
    if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) isLeapYear = true;

    if (hour >= 24) {
        hour -= 24;
        day++;
    }

    if (mon == 2 && isLeapYear) {
        if (day > 29) {
            day = 1;
            mon++;
        }
    } else if (mon == 2 && isLeapYear == false) {
        if (day > 28) {
            day = 1;
            mon++;
        }
    } else if (mon == 1 || mon == 3 || mon == 5 || mon == 7 || mon == 8 || mon == 10 || mon == 12) {
        if (day > 31) {
            day = 1;
            mon++;
        }
    } else /*if (mon == 2 || mon == 4 || mon ==6 || mon == 9 || mon == 11)*/ {
        if (day > 30) {
            day = 1;
            mon++;
        }
    }

    if (mon > 12) {
        mon = 1;
        year++;
    }

    tm->tm_hour = hour;
    tm->tm_mday = day;
    tm->tm_mon = mon - 1;
    tm->tm_year = year - 1900;
}

void time_to_string_cn(const struct tm *tm, char * restrict dest, size_t size)
{
    snprintf(dest, size, "%d-%d-%d %d:%d:%d",
        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
}

size_t unix_line_length(const char *str)
{
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '\n')
            return ++i;
    }

    return 0;
}

size_t windows_line_length(const char *str)
{
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '\n' && str[i-1] == '\r')
            return ++i;
    }

    return 0;
}

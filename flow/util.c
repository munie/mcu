// util.c

#include <stdbool.h>
#include <time.h>

#define CTOI(c) (c - 0x30)
#define ITOC(c) (c + 0x30)

void gpstime_to_normal(char * restrict normal, unsigned int size, char const * gpstime)
{
    // 2016-02-29 16:06:01
    for (int i = 0, j = 0; i < size && j < size; i++, j++) {
        if (j == 4 || j == 7) {
            normal[j] = '-';
            if (++j == size) break;
        } else if (j == 10) {
            normal[j] = ' ';
            if (++j == size) break;
        } else if (j == 13 || j == 16) {
            normal[j] = ':';
            if (++j == size) break;
        }
        normal[j] = gpstime[i];
    }
}

void gpstime_add_eight_hours(char * restrict time)
{
    // 0 ~ 3 : year
    // 4 ~ 5 : mon
    // 6 ~ 7 : day
    // 8 ~ 9 : hour
    // 10 ~ 11 : min
    // 12 ~ 13 : sec
    // 20160229160601 => 20160301000601

    int year, mon, day, hour;//, min, sec;
    bool isLeapYear = false;

    year = CTOI(time[0]) * 1000 + CTOI(time[1]) * 100 + CTOI(time[2]) * 10 + CTOI(time[3]);
    mon = CTOI(time[4]) * 10 + CTOI(time[5]);
    day = CTOI(time[6]) * 10 + CTOI(time[7]);
    hour = CTOI(time[8]) * 10 + CTOI(time[9]) + 8;
    //min = CTOI(time[10]) * 10 + CTOI(time[11]);
    //sec = CTOI(time[12]) * 10 + CTOI(time[13]);

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

    time[0] = ITOC(year / 1000 % 10);
    time[1] = ITOC(year / 100 % 10);
    time[2] = ITOC(year / 10 % 10);
    time[3] = ITOC(year % 10);
    time[4] = ITOC(mon / 10);
    time[5] = ITOC(mon % 10);
    time[6] = ITOC(day / 10);
    time[7] = ITOC(day % 10);
    time[8] = ITOC(hour / 10);
    time[9] = ITOC(hour % 10);
}

void chinese_time_add_eight_hours(char * restrict time)
{
    // 2016-02-29 16:06:01 => 2016-03-01 00:06:01
}

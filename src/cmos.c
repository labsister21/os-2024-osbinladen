#include "header/cpu/portio.h"
#include "header/driver/cmos.h"
#include "header/driver/graphics.h"
#include "header/stdlib/string.h"
  
unsigned char second;
unsigned char minute;
unsigned char hour;
 
int get_update_in_progress_flag() {
      out(cmos_address, 0x0A);
      return (in(cmos_data) & 0x80);
}
 
unsigned char get_RTC_register(int reg) {
    out(cmos_address, reg);
    return in(cmos_data);
}
 
void read_rtc() {
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char registerB;

    // Note: This uses the "read registers until you get the same values twice in a row" technique
    //       to avoid getting dodgy/inconsistent values due to RTC updates

    while (get_update_in_progress_flag());                // Make sure an update isn't in progress
    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;

        while (get_update_in_progress_flag());           // Make sure an update isn't in progress
        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
    } while( (last_second != second) || (last_minute != minute) || (last_hour != hour));

    registerB = get_RTC_register(0x0B);

    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
    }

    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    hour = (hour + 0x7) % 24;
}

void get_hour_minute_second(int* hourDest, int* minuteDest, int* secondDest){
    read_rtc();
    *hourDest = hour;
    *minuteDest = minute;
    *secondDest = second;
}

void show_HMS(){
    read_rtc();
    char hourStr[2] = {0};
    char minuteStr[2] = {0};
    char secondStr[2] = {0};

    intToStrPad(hour, hourStr, 2);
    intToStrPad(minute, minuteStr, 2);
    intToStrPad(second, secondStr, 2);

    draw_char_at(hourStr[0], TEXT_HEIGHT - 1, TEXT_WIDTH - 8, WHITE, BLACK);
    draw_char_at(hourStr[1], TEXT_HEIGHT - 1, TEXT_WIDTH - 7, WHITE, BLACK);
    draw_char_at(':', TEXT_HEIGHT - 1, TEXT_WIDTH - 6, WHITE, BLACK);
    draw_char_at(minuteStr[0], TEXT_HEIGHT - 1, TEXT_WIDTH - 5, WHITE, BLACK);
    draw_char_at(minuteStr[1], TEXT_HEIGHT - 1, TEXT_WIDTH - 4, WHITE, BLACK);
    draw_char_at(':', TEXT_HEIGHT - 1, TEXT_WIDTH - 3, WHITE, BLACK);
    draw_char_at(secondStr[0], TEXT_HEIGHT - 1, TEXT_WIDTH - 2, WHITE, BLACK);
    draw_char_at(secondStr[1], TEXT_HEIGHT - 1, TEXT_WIDTH - 1, WHITE, BLACK);
}
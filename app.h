#ifndef APP_H_
#define APP_H_

#define PAUSE_SMS_PWR_ON    120 //sec
#define PAUSE_SMS_PWR_OFF    120 //sec
#define INTERVAL_CHECK_BAT    90 //sec

#define FLAG_ALARM_DOOR    1
#define FLAG_ALARM_EXT_MOT    2
#define FLAG_ALARM_FRONT        3
#define FLAG_ALARM_SIDE         4
#define FLAG_WARN_COURTYARD     5
#define FLAG_WARN_COOK_ROOM     6
#define FLAG_WARN_FRONT         7
#define FLAG_WARN_SIDE          8
#define FLAG_ALARM_TEMPERATURE  9
#define FLAG_ALARM_SMOKE        10
#define FLAG_ALARM_SINGLE       11
#define FLAG_ALARM_SOS          12

#define CHIP_TEMP_ERROR_OFFSET 	-20    	  // Offset calibration in degrees Celsius
#define CHIP_TEMP_NR_OF_SAMPLES	30       // To get a useful accuracy we have to take the average of 100 samples
//void send_alarm_signal_if_needed(void);
//void guard_timer(void);
void send_sms_report(char *phone);
void check_temperature_smoke(void);
void debug_report(void);
void check_guard(void);
void check_alarm(void);
void set_flag_alarm(uint8_t flag);
void print_ram_free(void);
void power_control(void);
void charge_bat(void);
uint32_t get_time_without_power_s(void);
int16_t analog_read(uint8_t input);
void init_internal_temperature(void);
void set_flag_sos(uint8_t flag);
uint8_t get_flag_alarm(void);
void rst_default(void);
#endif /* APP_H_ */


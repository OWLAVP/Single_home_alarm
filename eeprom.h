#ifndef EEPROM_H_
#define EEPROM_H_


#define TOTAL_ADMIN_NUMBER      5
#define ADMIN_LIST					1

typedef struct
{
    uint32_t time_from_motion_s;
    //uint8_t time_sens_s;
    uint8_t interval_after_button_m;
    uint8_t pause_alarm_report_s;
    uint8_t power_report;
    uint8_t report_on_guard;
    uint8_t admin_mode;
    uint8_t guard;
    uint8_t autoguard;
    uint8_t first_usage; // по величине этого параметра определяется чистота EEPROM
    uint8_t first_m;
    uint8_t accept_calls;
    uint8_t accept_sms;
    uint8_t reset_count_mcu;
    uint8_t ext_temp;
    char admin_phone[TOTAL_ADMIN_NUMBER][14];
} config_t;

//config_t config;

void eeprom_read_config(char start);
void eeprom_save_config(void);
void EEPROM_update(void);
void EEPROM_save_local_guard(void);

uint8_t get_interval_power_off_report(void);
void set_power_report(uint8_t state);
uint8_t get_power_report(void);
void set_interval_power_on_report(uint8_t minute);
void set_device_number_phone(char *number);
char* get_device_number_phone(void);
uint8_t get_pause_alarm_report_s(void);
void set_pause_alarm_report_s(uint8_t second);
uint8_t get_accept_calls(void);
void set_accept_calls(uint8_t on_off);
uint8_t get_accept_sms(void);
void set_accept_sms(uint8_t on_off);
uint8_t get_guard(void);
void set_guard(uint8_t state);
uint8_t get_local_guard(void);
void set_local_guard(uint8_t state);
void set_report_on_guard(uint8_t on_off);
uint8_t get_report_on_guard(void);
void set_first_m(uint8_t state);
uint8_t get_first_m(void);
void set_autoguard(uint8_t state);
uint8_t get_autoguard(void);
void set_interval_after_button_m(uint8_t minute);
uint8_t get_interval_after_button_m(void);
void set_reset_count_mcu(uint8_t cnt);
uint8_t get_reset_count_mcu(void);
void inc_reset_count_mcu(void);
void set_ext_temp(uint8_t state);
uint8_t get_ext_temp(void);
char* get_phones(uint8_t  i);
void set_first_usage(uint8_t state);
uint8_t get_first_usage(void);
void set_admin_mode(uint8_t state);
uint8_t get_admin_mode(void);


#endif /* EEPROM_H_ */



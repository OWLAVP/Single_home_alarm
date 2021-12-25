#include "main.h"

static config_t config;
static config_t *p_config = &config;
//*******************************************************************************************************************
// здесь обязательно нужен высокий уровень оптимизации, т.к. между записями EEMWE EEWE должно быть не более 4 тактов
void __attribute__((optimize("-O3"))) EEPROM_write(uint16_t address, uint8_t data)
{
    char tsreg;
    tsreg = SREG;
    cli();
    while(EECR & (1<<EEPE)); // Wait for completion of previous write
    EEAR = address; // Set up address and data registers
    EEDR = data;
    EECR = (1<<EEMPE); // Write logical one to EEMWE
    EECR = (1<<EEPE); // Start eeprom write by setting EEWE
    SREG = tsreg;
}
//void EEPROM_write(uint16_t uiAddress, unsigned char ucData)
//{
//    /* Wait for completion of previous write */
//    while(EECR & (1<<EEPE))
//        ;
//    /* Set up address and Data Registers */
//    EEAR = uiAddress;
//    EEDR = ucData;
//    /* Write logical one to EEMPE */
//    EECR |= (1<<EEMPE);
//    /* Start eeprom write by setting EEPE */
//    EECR |= (1<<EEPE);
//}
//*******************************************************************************************************************

uint8_t EEPROM_read(uint16_t address)
{
    while(EECR & (1<<EEPE)); // Wait for completion of previous write
    EEAR = address; // Set up address register
    EECR |= (1<<EERE); // Start eeprom read by writing EERE
    return EEDR; // Return data from data register
}
//*******************************************************************************************************************

void EEPROM_write_buf(char *buf, uint16_t len, uint16_t address)
{
    while(len--)
        EEPROM_write(address++, *buf++);
}

//*******************************************************************************************************************

void EEPROM_read_buf(char *buf, uint16_t len, uint16_t address)
{
    while(len--)
        *buf++ = EEPROM_read(address++);
}

//*******************************************************************************************************************

void eeprom_read_config(char start)
{

    eeprom_read_block((void*)&config, 0, sizeof(config_t));
    //EEPROM_read_buf((char*)&config, sizeof(config_t), 0);
    if(p_config->first_usage != 0xAB)
    {
        memset(&config, 0, sizeof(config_t));
        softuart_puts_p(PSTR("eeprom_first_set"));
        p_config->interval_after_button_m = 1;
        p_config->pause_alarm_report_s = 1; //sec
        p_config->power_report = 1;
        //p_config->time_sens_s = 8;
//        p_config->mpause_count = 50;
        p_config->first_usage = 0xAB;
        //p_config->accept_calls = 1;
        //p_config->report_on_guard = 0;
        memcpy(p_config->admin_phone[0], "+380713318090", 13);
        //memcpy(p_config->admin_phone[1], "+380714779281", 13);
        //strcpy(p_config->admin_phone[0], "+380713318090");
        eeprom_save_config();
    }
    if(start)
    {
        set_time_from_motion_s(config.time_from_motion_s);
    }
}

//*******************************************************************************************************************

void eeprom_save_config(void)
{
    //softuart_puts_p(PSTR("eeprom_save_config"));
    config.time_from_motion_s = get_time_from_motion_s();
    eeprom_write_block((void*)&config, 0, sizeof(config_t));
    //EEPROM_write_buf((char*)&config, sizeof(config_t), 0);
}


//*******************************************************************************************************************

void EEPROM_update(void)
{
    //softuart_puts_p(PSTR("EEPROM_update"));
    eeprom_update_block((void*)&config, 0, sizeof(config_t));
}


//******************************************************************
void set_accept_calls(uint8_t state)
{
    p_config->accept_calls = state;
}
uint8_t get_accept_calls(void)
{
   return p_config->accept_calls;
}
//**********************************************************************
void set_accept_sms(uint8_t state)
{
    p_config->accept_sms = state;
}
uint8_t get_accept_sms(void)
{
   return p_config->accept_sms;
}
//***********************************************
//uint8_t get_interval_power_off_report(void)
//{
//    return p_config->interval_power_off_report_m;
//}
//void set_interval_power_off_report(uint8_t minute)
//{
//    p_config->interval_power_off_report_m = minute;
//}
//***********************************************
uint8_t get_power_report(void)
{
    return p_config->power_report;
}
void set_power_report(uint8_t state)
{
    p_config->power_report = state;
}

//***********************************************
uint8_t get_guard(void)
{
    return p_config->guard;
}
void set_guard(uint8_t state)
{
    p_config->guard = state;
}

//**********************************************************************
void set_report_on_guard(uint8_t state)
{
    p_config->report_on_guard = state;
}
uint8_t get_report_on_guard(void)
{
   return p_config->report_on_guard;
}
//**********************************************************************
void set_first_m(uint8_t state)
{
    p_config->first_m = state;
}
uint8_t get_first_m(void)
{
   return p_config->first_m;
}
//**********************************************************************
void set_autoguard(uint8_t state)
{
    p_config->autoguard = state;
}
uint8_t get_autoguard(void)
{
   return p_config->autoguard;
}
//**********************************************************************
void set_interval_after_button_m(uint8_t minute)
{
    p_config->interval_after_button_m = minute;
}
uint8_t get_interval_after_button_m(void)
{
   return p_config->interval_after_button_m;
}
//**********************************************************************
void set_reset_count_mcu(uint8_t cnt)
{
    p_config->reset_count_mcu = cnt;
}
uint8_t get_reset_count_mcu(void)
{
   return p_config->reset_count_mcu;
}
void inc_reset_count_mcu(void)
{
    p_config->reset_count_mcu++;
}

//**********************************************************************
char* get_phones(uint8_t  i)
{
    char *ptr = p_config->admin_phone[i];
    return ptr;
}
//***********************************************************************

void set_first_usage(uint8_t state)
{
    p_config->first_usage = state;
}
uint8_t get_first_usage(void)
{
   return p_config->first_usage;
}
//**********************************************************************
void set_admin_mode(uint8_t state)
{
    p_config->admin_mode = state;
}
uint8_t get_admin_mode(void)
{
   return p_config->admin_mode;
}
//**********************************************************************

//**********************************************************************
uint8_t get_pause_alarm_report_s(void)
{
    return p_config->pause_alarm_report_s;
}
void set_pause_alarm_report_s(uint8_t second)
{
    p_config->pause_alarm_report_s = second;
}

//**********************************************************************
void set_ext_temp(uint8_t state)
{
    p_config->ext_temp = state;
}
uint8_t get_ext_temp(void)
{
   return p_config->ext_temp;
}
//**********************************************************************

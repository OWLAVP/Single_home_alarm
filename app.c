#include "main.h"

static uint8_t flag_alarm = 0;
static uint32_t start_time_alarm = 0;
static uint32_t time_without_power_s = 0;

static void check_btn_SOS(void);
static uint8_t level_battery(void);
static uint8_t is_external_pwr(void);
static void door_sensors(void);
static void ext_motion_sensor(void);

//*******************************************************************************************************************
static void single_sens(void);
static void single_sens(void)
{
    static uint8_t motion_cnt = 0;
    uint8_t motion_d = get_single_motion_detected();
    if(motion_d && flag_alarm == 0)
    {
        if(get_first_m() == 0)
        {
            if(motion_cnt == 0)//if first motion - return
            {
                motion_cnt = 1;
                set_time_from_motion_s(0); // обнуляем таймер с момента последнего зафиксированного движения
                softuart_puts_p(PSTR("single_first_motion"));
                return;
            }
        }
        else motion_cnt = 0;
        if(motion_cnt == 1 && (get_time_from_motion_s() < 2)) // 2 seconds return
        {
            return;
        }
        start_time_alarm = get_time_s() + get_pause_alarm_report_s();
        flag_alarm = FLAG_ALARM_SINGLE;
        beep_non_block(500);
        softuart_puts_p(PSTR("FLAG_ALARM_SINGLE\r\n"));
    }
    if(motion_cnt == 1 && (get_time_from_motion_s() > 8))
    {
        softuart_puts_p(PSTR("rst motion")); //in 3 sec 2 motions
        motion_cnt = 0;
    }
}

//*******************************************************************************************************************
static void send_alarm_signal_if_needed(void);
static void send_alarm_signal_if_needed(void)
{

    if(flag_alarm == 0)
    {
        return;
    }
    if(get_pause_alarm_report_s() > 0)
    {
        if(get_time_s() < start_time_alarm)
        {
            return;
        }
    }

    if(flag_alarm == FLAG_ALARM_SINGLE)
    {

        //buzzer_non_block(60000); //60 sec
        softuart_puts_p(PSTR("alarm...SINGLE\r\n"));
        if(get_accept_calls() == 1)
        {
            alarm_play_file(AUDIO_ALARM_MOTION);
            delay_ms(2000);
        }
        if(get_accept_sms() == 1)
        {
            send_sms_admins(PSTR("ALARM! Invasion!!!"));
        }
        set_guard(0);
        EEPROM_update();
        flag_alarm = 0;
    }
    else if(flag_alarm == FLAG_ALARM_DOOR)
    {

        //buzzer_non_block(60000); //60 sec
        softuart_puts_p(PSTR("alarm...DOOR\r\n"));
        if(get_accept_calls() == 1)
        {
            alarm_play_file(AUDIO_ALARM_DOOR);
            delay_ms(2000);
        }
        if(get_accept_sms() == 1)
        {
            send_sms_admins(PSTR("ALARM! Invasion!!!"));
        }
        flag_alarm = 0;
        set_guard(0);
        EEPROM_update();
    }
    else if(flag_alarm == FLAG_ALARM_EXT_MOT)
    {
        //buzzer_non_block(60000); //60 sec
        softuart_puts_p(PSTR("alarm...EXT_MOT\r\n"));
        if(get_accept_calls() == 1)
        {
            alarm_play_file(AUDIO_ALARM_MOTION);
            delay_ms(2000);
        }
        if(get_accept_sms() == 1)
        {
            send_sms_admins(PSTR("ALARM! Invasion!!!"));
        }
        flag_alarm = 0;
        set_guard(0);
        EEPROM_update();
    }
}

//*******************************************************************************************************************
static void guard_timer(void);
static void guard_timer(void)
{
    if(get_guard() == 0)
    {
        uint16_t t = get_time_from_button_s();
        if(get_autoguard() == 0 && t!=0xFFFF)
        {
            set_autoguard(1);
            EEPROM_update();
        }
        if((t > 60*get_interval_after_button_m()) && (t != 0xFFFF))
        {
            set_guard(1);
            softuart_puts_p(PSTR("ON"));
            EEPROM_update();
            set_time_from_button_s(0xFFFF);
            beep_non_block(150);
            delay_ms(300);
            beep_non_block(150);
            if(get_report_on_guard() == 1)
                send_sms_admins(PSTR("Guard is on by button."));
        }
    }
}

//*******************************************************************************************************************
void check_guard(void)
{
    guard_timer();
    if(get_guard() == 1)
    {
        single_sens();
        door_sensors();
        ext_motion_sensor();
        send_alarm_signal_if_needed();
    }
    else
    {
        check_btn_SOS();
        flag_alarm = 0;
    }
}

//*******************************************************************************************************************

void send_sms_report(char *phone)
{
    send_USSD();

    char sms[160];
    char *ptr;

    ptr = sms;

    ptr += sprintf_P(ptr, PSTR("Deposit=%s;"), get_arr_dep());

    ptr += sprintf_P(ptr, PSTR("Guard=%d;"), get_guard());

    ptr += sprintf_P(ptr, PSTR("Temp_int=%dC;"), analog_read(INT_TEMP));

    if(get_ext_temp() == 1)
        ptr += sprintf_P(ptr, PSTR("Temp_ext=%dC;"), get_temperature());

    ptr += sprintf_P(ptr, PSTR("GSM=%d%%;"), mdm_get_signal_strength());

    ptr += sprintf_P(ptr, PSTR("rst_mcu=%u;"), get_reset_count_mcu());

    if(get_time_without_power_s() > 0)
    {
        ptr += sprintf_P(ptr, PSTR("Battery=%d%%, "), level_battery());
    }
    send_sms(sms, phone);
}



//********************************************************************************************************************
void check_temperature_smoke(void)
{

    static uint32_t time_stamp_s = 0;
    static uint8_t flag = 0;
    if(flag == 1)
        return;
    if(get_flag_smoke_sensor())
    {
        flag = 1;
        softuart_puts_ln("Alarm! SMOKE!!!");
        beep_non_block(500);
        //buzzer_non_block(60000); //60 sec
        //alarm_play_file(ALARM_SMOKE);
        //send_sms_admins(PSTR("Alarm! SMOKE!!!"));
    }
    if(get_time_s() > (time_stamp_s + 5))
    {
        time_stamp_s = get_time_s();
        if(get_ext_temp() == 1)
        {
            if(get_temperature() > 65)
            {
                flag = 1;
                softuart_puts_p(PSTR("Alarm! TEMPERATURE!!!"));
                buzzer_non_block(60000); //60 sec
                //  if(get_accept_calls() == 1)
                alarm_play_file(AUDIO_ALARM_TEMPERATURE);
                // if(get_accept_sms() == 1)
                send_sms_admins(PSTR("Alarm! TEMPERATURE!!!"));
            }
        }

        if(analog_read(INT_TEMP) > 65)
        {
            flag = 1;
            softuart_puts_p(PSTR("Alarm! Internal TEMPERATURE!!!"));
            buzzer_non_block(60000); //60 sec
            send_sms_admins(PSTR("Alarm! Internal TEMPERATURE > 65C !!!"));
        }
    }
}
//**********************************************************************************

static void check_btn_SOS(void)
{
    if(flag_alarm == FLAG_ALARM_SOS)
    {
        softuart_puts_p(PSTR("FLAG_ALARM_SOS\r\n"));
        flag_alarm = 0;
        alarm_play_file(AUDIO_SOS);
    }
}
//*******************************************************************************************************************
void set_flag_alarm(uint8_t flag)
{
    flag_alarm = flag;
}
//*******************************************************************************************************************
uint8_t get_flag_alarm(void)
{
    return get_val(flag_alarm);
}
//void event_p(const char *str)
//{
//    memset(config.last_event, 0, sizeof(config.last_event));
//    strcpy_P(config.last_event, str);
//    EEPROM_update();
//}
//*******************************************************************************************************************

//void check_rst_source(void)
//{
//    if((MCUSR & (1<<3)) == 0) // источником перезапуска был не сторожевой таймер
//    {
//        event_p(PSTR("Normal start"));
//    }
//    else
//        event_p(PSTR("last reboot: wdt "));
//}

//**********************************************************************************

void debug_report(void)
{
    uint32_t time;
    uint16_t d, h, m, s;
    char sms[160];
    char *ptr;

    time = get_time_from_start_s();
    d = time/(3600UL*24UL);
    time -= d*(3600UL*24UL);
    h = time/3600UL;
    time -= h*3600UL;
    m = time/60;
    s = time - m*60;
    ptr = sms;

    ptr += sprintf_P(ptr, PSTR("deposit=%s;"), get_arr_dep());

    ptr += sprintf_P(ptr, PSTR("t=%ud%uh%um%us;"), d, h, m, s);

    ptr += sprintf_P(ptr, PSTR("guard=%d;"), get_guard());

    ptr += sprintf_P(ptr, PSTR("temp=%dC;"), get_temperature());

    ptr += sprintf_P(ptr, PSTR("mcu=%u;"), get_reset_count_mcu());

    ptr += sprintf_P(ptr, PSTR("signal_gsm=%d%%;"), mdm_get_signal_strength());

    if(get_time_without_power_s() != 0)
    {
        ptr += sprintf_P(ptr, PSTR("Battery=%d%%, "), level_battery());
    }

    softuart_puts_ln(sms);
}

///****************************************************************************
static uint8_t level_battery(void)
{
    int16_t bat = analog_read(BATTERY);
    if(bat < 550)
    {
        reset_mcu(1);
        return 0;
    }
    else if(bat > 640)
        bat = 640;
    return map_s(bat, 550, 640, 0, 100);
}

//*****************************************************************************
void power_control(void)
{
    static uint32_t time_stamp_on = 0;
    static uint32_t time_stamp_off = 0;
    static uint32_t time_stamp_bat = 0;
    static uint8_t report_on = 0;
    static uint8_t report_off = 0;
    static uint8_t report_bat = 0;
    static uint8_t flag_time_on = 1;
    static uint8_t flag_time_off = 1;
    static uint8_t flag_guard = 0;
    static uint8_t flag_sms = 0;

    if(time_without_power_s == 0)   // power+++
    {
        if(get_power_report() == 1)
        {
            if(flag_guard == 0)
            {
                flag_guard = 1;
                report_bat = 0;
                //delay_s(15);    //для инициализации датчиков
                softuart_puts_p(PSTR("power+++"));
            }
            if(flag_time_on == 0)
            {
                time_stamp_on = get_time_s() + PAUSE_SMS_PWR_ON;
                flag_time_on = 1;
                report_on = 1;
            }
            if(report_on == 1)
            {
                if(flag_time_on == 1 && (time_stamp_on < get_time_s()))
                {
                    report_on = 0;
                    flag_time_off = 1;
                    flag_sms = 0;
                    softuart_puts_p(PSTR("sms=power on"));
                    send_sms_admins(PSTR("External power is on"));
                }
            }
        }
        return;
    }
    if(get_power_report() == 1)  ///  power---
    {
        time_stamp_off = get_val(time_without_power_s);
        time_stamp_on = get_time_s() + PAUSE_SMS_PWR_ON;
        if(flag_guard == 1)
        {
            flag_guard = 0;
            //set_guard(0);
            softuart_puts_p(PSTR("power---"));
        }
        if(flag_time_off == 1 && time_stamp_off > PAUSE_SMS_PWR_OFF)
        {
            report_off = 1;
        }
        if(report_off == 1)
        {
            report_off = 0;
            flag_time_on = 0;
            flag_time_off = 0;
            report_bat = 1;
            //time_stamp_bat = get_time_s() + INTERVAL_CHECK_BAT;
            softuart_puts_p(PSTR("sms=power_off"));
            send_sms_admins(PSTR("External power is off"));
        }
        if(report_bat == 1 && (get_time_s() > time_stamp_bat))
        {
            softuart_puts_p(PSTR("check battery"));
            time_stamp_bat = get_time_s() + INTERVAL_CHECK_BAT;

            if(level_battery() < 20 && flag_sms == 0)
            {
                flag_sms = 1;
                if(get_accept_sms() == 1)
                    send_sms_admins(PSTR("Battery LOW!!!"));
                softuart_puts_p(PSTR("sms=low bat"));
            }
        }
    }
}

//*****************************************************************************
void charge_bat(void)
{
    while(analog_read(BATTERY) < 550)
    {
        reset_soft_wdt();
        delay_s(5);
    }
}
//*****************************************************************************
uint32_t get_time_without_power_s(void)
{
    return time_without_power_s;
}
//*****************************************************************************

//********************************************************************************************************
void init_internal_temperature(void)
{
    ADMUX |= (1<<REFS1) | (1<<REFS0); //Internal 1.1V Voltage Reference
    if(F_CPU == 16000000)
    {
        ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 16Mhz F_CPU / 128 prescaler = 125Khz ADC clock source
    }
    else if(F_CPU == 7372800)
    {
        ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // 8Mhz F_CPU / 64 prescaler = 125Khz ADC clock source
        ADCSRA &= ~(1 << ADPS0);
    }
    else
    {
        ADCSRA |= (1 << ADPS1) | (1 << ADPS0); // 1Mhz F_CPU / 8 prescaler = 125Khz ADC clock source
        ADCSRA &= ~(1 << ADPS2);
    }
}


//*******************************************************************************************************************


//********************************************************************************************************************

//***********************************************************************************************
int16_t analog_read(uint8_t input)
{

    uint16_t ADC_cache=0;
    uint8_t i;
    long buffer=0;
    ADMUX &=  (~(1<<MUX3)) & (~(1<<MUX2)) & (~(1<<MUX1)) & (~(1<<MUX0));
    // Select the ADC reference voltage and channel. Must be done everytime because ADMUX can be overwritten in meantime
    switch(input)
    {
    case ANALOG0:
        ADMUX &=  (~(1<<MUX3)) & (~(1<<MUX2)) & (~(1<<MUX1)) & (~(1<<MUX0)); //---------- 0 0 0 0 0
        break;
    case ANALOG1:
        ADMUX |= (1<<MUX0);//-------------------------------------------------------------------------- 0 0 0 0 1
        break;
    case ANALOG2:
        ADMUX |= (1<<MUX1);//-------------------------------------------------------------------------- 0 0 0 1 0
        break;
    case ANALOG3:
        ADMUX |= (1<<MUX1) | (1<<MUX0);//-------------------------------------------------------------- 0 0 0 1 1
        break;
    case ANALOG4:
        ADMUX |= (1<<MUX2);//-------------------------------------------------------------------------- 0 0 1 0 0
        break;
    case ANALOG5:
        ADMUX |= (1<<MUX2) | (1<<MUX0);//-------------------------------------------------------------- 0 0 1 0 1
        break;
    case ANALOG6:
        ADMUX |= (1<<MUX2) | (1<<MUX1);//-------------------------------------------------------------- 0 0 1 1 0
        break;
    case ANALOG7:
        ADMUX |= (1<<MUX2) | (1<<MUX1) | (1<<MUX0);//-------------------------------------------------- 0 0 1 1 1
        break;
    case ANALOG8:
        ADMUX |= (1<<MUX3);         // ADC channel 8 - intelnal temperature sensor
        break;
    default:
        ADMUX &= (~(1<<MUX3)) & (~(1<<MUX2)) & (~(1<<MUX1)) & (~(1<<MUX0)); // set channel ADC0 default
    }

    ADCSRA |= (1 << ADEN);  // Power up the ADC
    ADCSRA |= (1 << ADIE);  // Enable the ADC Interrupt

    SMCR |= (1 << SM0);    // ADC Noise Reduction Sleep Mode
    delay_ms(20);         // Wait for voltages to become stable
    ADCSRA |= (1 << ADSC);  // Start converting

    // Take a reading and discard the first sample to prevent errors
    while(ADCSRA & (1 << ADSC));

    for(i=CHIP_TEMP_NR_OF_SAMPLES; i>0; i--)
    {
        // Enter Sleep Mode (ADC Noise Reduction Sleep Mode) and Start a new conversion
        SMCR |= (1 << SE);

        // Wait for ADC conversion
        while(ADCSRA & (1 << ADSC)); // ADSC is cleared when the conversion finishes

        // After the Sleep Mode this bit must be set to 0 according to the datasheet
        SMCR &= ~(1 << SE);

        // Read register atomically in case other interrupts are enabled
        cli();
        ADC_cache = ADCW;
        sei();

        buffer += ADC_cache;
    }
    //print_freeRam();
    // Shut down the ADC to save power
    ADCSRA &= ~(1 << ADEN);
    if(input == INT_TEMP)
        return ((buffer / CHIP_TEMP_NR_OF_SAMPLES) + CHIP_TEMP_ERROR_OFFSET) - 273;
    else if(input == BATTERY)
        return (buffer / CHIP_TEMP_NR_OF_SAMPLES);
    return 0;
}

ISR(ADC_vect)
{}

//*******************************************************************************************************************

void check_power(void)
{
    if(is_external_pwr())
        time_without_power_s = 0;
    else
        time_without_power_s++;
}

static uint8_t is_external_pwr(void)
{
    if(DigitalRead(EXT_POWER))
        return 1;
    else
    {
        return 0;
    }
}

//*******************************************************************************************************************

static void door_sensors(void)
{
    if(get_flag_open_door() && flag_alarm == 0)
    {
        set_time_from_motion_s(0); // обнуляем таймер с момента последнего зафиксированного движения
        start_time_alarm = get_time_s() + get_pause_alarm_report_s();
        flag_alarm = FLAG_ALARM_DOOR;
        beep_non_block(500);
        softuart_puts_p(PSTR("FLAG_ALARM_DOOR\r\n"));
    }

}

//*******************************************************************************************************************

//*******************************************************************************************************************

static void ext_motion_sensor(void)
{
    static uint8_t motion_cnt = 0;
    uint8_t motion_d = get_flag_ext_mot_sens_detected();
    if(motion_d && flag_alarm == 0)
    {
        if(get_first_m() == 0)
        {
            if(motion_cnt == 0)//if first motion - return
            {
                motion_cnt = 1;
                set_time_from_motion_s(0); // обнуляем таймер с момента последнего зафиксированного движения
                softuart_puts_p(PSTR("ext_first_motion"));
                return;
            }
        }
        else motion_cnt = 0;
        if(motion_cnt == 1 && (get_time_from_motion_s() < 2)) // 2 seconds return
        {
            return;
        }
        start_time_alarm = get_time_s() + get_pause_alarm_report_s();
        flag_alarm = FLAG_ALARM_EXT_MOT;
        beep_non_block(500);
        softuart_puts_p(PSTR("FLAG_ALARM_EXT_MOT\r\n"));
    }
    if(motion_cnt == 1 && (get_time_from_motion_s() > 8))
    {
        softuart_puts_p(PSTR("rst motion")); //in 3 sec 2 motions
        motion_cnt = 0;
    }
}
//*****************************************************************************

void rst_default(void)
{
    if(is_btn_rst_pressed())
    {
        set_first_usage(0);
        EEPROM_update();
        beep_non_block(100);
        delay_ms(100);
        beep_non_block(100);
        delay_ms(100);
        beep_non_block(100);
        delay_ms(100);
        cli(); // запрещаем прерывания и ждем перезагрузки по сторожевому таймеру
        while(1);
    }
}

//*****************************************************************************
static int freeRam ();
static int freeRam ()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


void print_ram_free(void)
{
    char ch[10];
    itoa(freeRam(), ch, 10);
    softuart_puts("free=");
    softuart_puts_ln(ch);
}

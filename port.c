#include "main.h"

static uint8_t single_motion_detected = 0;
static uint8_t flag_open_door = 0;
static uint8_t flag_ext_mot_sens_detected = 0;
static uint8_t flag_smoke_sensor = 0;

static void sensor_port_init(void);
static void button_port_init(void);
static void led_port_init(void);
static uint8_t is_single_sensor(void);
static uint8_t is_btn_guard_pressed(void);
static uint8_t is_btn_sos_pressed(void);
static uint8_t is_door_sensor(void);
static uint8_t is_smoke_sensor(void);
static void unused_port_init(void);
//*******************************************************************************************************************
static void sensor_port_init(void)
{
    PinMode(MOTION_SENSOR, INPUT);
    DigitalWrite(MOTION_SENSOR, LOW);

    PinMode(EXT_MOTION_SENSOR, INPUT);
    DigitalWrite(EXT_MOTION_SENSOR, HIGH);

    PinMode(DOOR, INPUT);
    DigitalWrite(DOOR, HIGH);

    PinMode(SMOKE_SENSOR, INPUT);
    DigitalWrite(SMOKE_SENSOR, HIGH);

    PinMode(EXT_POWER, INPUT);
    DigitalWrite(EXT_POWER, LOW);

    //PinMode(IBUTTON, INPUT);
    //DigitalWrite(IBUTTON, HIGH);
}

//*******************************************************************************************************************
static void button_port_init(void)
{
    PinMode(BTN_GUARD, INPUT);
    DigitalWrite(BTN_GUARD, HIGH);
    PinMode(BTN_SOS, INPUT);
    DigitalWrite(BTN_SOS, HIGH);
    PinMode(BTN_DEFAULT, INPUT);
    DigitalWrite(BTN_DEFAULT, HIGH);
}

//*******************************************************************************************************************
static void led_port_init(void)
{
    PinMode(RED_LED, OUTPUT);
    DigitalWrite(RED_LED, LOW);
    PinMode(GREEN_LED, OUTPUT);
    DigitalWrite(GREEN_LED, LOW);
}

//*******************************************************************************************************************
void port_init(void)
{
    PinMode(PIN_PWR_GSM, OUTPUT);
    DigitalWrite(PIN_PWR_GSM, HIGH);

    PinMode(PIN_BUZZER, OUTPUT);
    DigitalWrite(PIN_BUZZER, LOW);

    PinMode(PIN_BEEP, OUTPUT);
    DigitalWrite(PIN_BEEP, LOW);

    sensor_port_init();
    led_port_init();
    button_port_init();
    unused_port_init();
}

//*******************************************************************************************************************
static void unused_port_init(void)
{
//    PinMode(UNUSED5, OUTPUT);
//    DigitalWrite(UNUSED5, LOW);

}
////*******************************************************************************************************************
//// управление светодиодами. Вызывается с периодом 10 мс
void led_management(void)
{
    static uint8_t i = 0;
    static uint8_t j = 0;

    if(get_time_from_button_s() < 0xFFFF)
    {
        i++;
        if(i > 30) //every 300ms
        {
            DigitalLevelToggle(RED_LED);
            i = 0;
        }
        DigitalWrite(GREEN_LED, LOW);
    }
    else if(get_guard() == 1)
    {
        j++;
        if(j > 100) //every 1 sec
        {
            DigitalLevelToggle(RED_LED);
            j = 0;
        }
        DigitalWrite(GREEN_LED, LOW);

    }
    else if(get_guard() == 0)
    {
        DigitalWrite(RED_LED, LOW);
        led_gsm();
    }
}

/////////////////////////////////////
static uint8_t is_single_sensor(void)
{
    if(DigitalRead(MOTION_SENSOR))
        return 1;
    else
    {
        return 0;
    }
}
//***************************************************************************************
//// функция вызывается с периодом 10 мс
void check_single_sensor(void)
{
    static uint8_t cnt = 0;
    static uint8_t pause_count = 0;
    char motion;

    motion = is_single_sensor();
    if(motion && pause_count == 0)
    {
        cnt++;
        if(cnt >= 3)
        {
            single_motion_detected = 1;
            pause_count = 2;
            cnt = 0;
        }
    }
    else
    {
        cnt =0;

    }
    if(pause_count > 0)
    {
        pause_count--;

    }
    else
    {
        single_motion_detected = 0;
    }
}
///***************************************************************************************************
uint8_t get_single_motion_detected(void)
{
    return single_motion_detected;
}

///////////////////////////////////////////////////////
static uint8_t is_btn_guard_pressed(void)
{
    if(DigitalRead(BTN_GUARD))
        return 0;
    else
    {
        return 1;
    }

}
//*********************************************************************************************************************
static uint8_t is_btn_sos_pressed(void)
{
    if(DigitalRead(BTN_SOS))
        return 0;
    else
    {
        return 1;
    }

}
////*******************************************************************************************************************
uint8_t is_btn_rst_pressed(void)
{
    if(DigitalRead(BTN_DEFAULT))
        return 0;
    else
    {
        return 1;
    }

}
//*******************************************************************************************************************

//// функция вызывается с периодом 10 мс
void check_button(void)
{
    static uint8_t press_button = 0;
    static uint8_t button_state = 0;
    static uint8_t press_sos_button = 0;
    static uint8_t button_sos_state = 0;
    static uint8_t sos_state = 0;

    if(is_btn_guard_pressed())
    {
        if(get_guard() == 0)
        {
            if(press_button < 15)
                press_button++;
            else if(button_state == 0)
            {
                set_time_from_button_s(0);
                button_state = 1;
                EEPROM_update();
                beep_non_block(100);
            }
        }
        if(get_guard() == 1 || get_time_from_button_s() != 0xFFFF)
        {
            if(press_button < 5)
                press_button++;
            else if(button_state == 0)
            {
                buzzer_stop();
                set_guard(0);
                set_time_from_button_s(0xFFFF);
                button_state = 1;
                EEPROM_update();
                beep_non_block(100);
            }
        }

    }
    else
    {
        press_button = 0;
        button_state = 0;
    }

    if(sos_state == 1)
        return;
    if(is_btn_sos_pressed())
    {

        if(press_sos_button < 10)
            press_sos_button++;
        else if(button_sos_state == 0)
        {
            button_sos_state = 1;
            sos_state = 1;
            set_flag_alarm(FLAG_ALARM_SOS);
        }

    }
    else
    {
        press_sos_button = 0;
        button_sos_state = 0;
    }
}
//
//*******************************************************************************************************************


////*******************************************************************************************************************

//*******************************************************************************************************************

/////////////////////////////////////
static uint8_t is_door_sensor(void)
{
    if(DigitalRead(DOOR))
        return 1;
    else
    {
        return 0;
    }
}
//***************************************************************************************
//// функция вызывается с периодом 10 мс
void check_door_sensor(void)
{
    static uint8_t cnt = 0;
    static uint8_t pause_count = 0;
    char motion;

    motion = is_door_sensor();
    if(motion && pause_count == 0)
    {
        cnt++;
        if(cnt >= 20)
        {
            flag_open_door = 1;
            pause_count = 10;
            cnt = 0;
        }
    }
    else
    {
        cnt =0;

    }
    if(pause_count > 0)
    {
        pause_count--;

    }
    else
    {
        flag_open_door = 0;
    }
}

//*****************************************************************************
uint8_t get_flag_open_door(void)
{
    return flag_open_door;
}

/////////////////////////////////////
static uint8_t is_ext_mot_sensor(void)
{
    if(DigitalRead(EXT_MOTION_SENSOR))
        return 1;
    else
    {
        return 0;
    }
}
//***************************************************************************************
//// функция вызывается с периодом 10 мс
void check_ext_mot_sensor(void)
{
    static uint8_t cnt = 0;
    static uint8_t pause_count = 0;
    char motion;

    motion = is_ext_mot_sensor();
    if(motion && pause_count == 0)
    {
        cnt++;
        if(cnt >= 5)
        {
            flag_ext_mot_sens_detected = 1;
            pause_count = 3;
            cnt = 0;
        }
    }
    else
    {
        cnt =0;

    }
    if(pause_count > 0)
    {
        pause_count--;

    }
    else
    {
        flag_ext_mot_sens_detected = 0;
    }
}

//*****************************************************************************
uint8_t get_flag_ext_mot_sens_detected(void)
{
    return flag_ext_mot_sens_detected;
}

/////////////////////////////////////
static uint8_t is_smoke_sensor(void)
{
    if(DigitalRead(SMOKE_SENSOR))
        return 1;
    else
    {
        return 0;
    }
}
//***************************************************************************************
//// функция вызывается с периодом 10 мс
void check_smoke_sensor(void)
{
    static uint8_t cnt = 0;
    static uint8_t pause_count = 0;
    char motion;

    motion = is_smoke_sensor();
    if(motion && pause_count == 0)
    {
        cnt++;
        if(cnt >= 10)
        {
            flag_smoke_sensor = 1;
            pause_count = 5;
            cnt = 0;
        }
    }
    else
    {
        cnt =0;

    }
    if(pause_count > 0)
    {
        pause_count--;

    }
    else
    {
        flag_smoke_sensor = 0;
    }
}

//*****************************************************************************
uint8_t get_flag_smoke_sensor(void)
{
    return flag_smoke_sensor;
}

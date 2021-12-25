#include "main.h"

static uint32_t time_ms=0;
static uint32_t time_s=0;
static uint32_t time_m=0;
static uint8_t buz_on = 0;
static uint16_t buz_counter = 0;
static uint8_t beep_on = 0;
static uint16_t beep_counter = 0;
static uint16_t soft_wdt = 0;
static uint16_t reset_s_wdt = 0;
static uint32_t time_from_start_s=0; // таймер с начала работы прибора
static uint32_t time_from_motion_s=0; // таймер с момента последнего зафиксированного движения
static uint16_t time_from_button_s=0xFFFF; // таймер с момента последнего нажатия на кнопку

static void inc_time_from_motion(void);

//*******************************************************************************************************************

// период переполнения 10 мс
void timer1_init(void)
{
    cli(); // stop interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // 100 Hz (7372800/((71+1)*1024))
    OCR1A = 71;
    // CTC
    TCCR1B |= (1 << WGM12);
    // Prescaler 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);
    // Output Compare Match A Interrupt Enable
    TIMSK1 |= (1 << OCIE1A);

}


//*******************************************************************************************************************

void buzzer_non_block(uint16_t time_to_buz)
{
    cli();
    buz_on = 1;
    buz_counter = time_to_buz;
    DigitalWrite(PIN_BUZZER, HIGH);
    sei();
}

//*******************************************************************************************************************

void buzzer_stop(void)
{
    buz_on = 0;
    buz_counter = 0;
    DigitalWrite(PIN_BUZZER, LOW);
}
//**********************************************************
void beep_non_block(uint16_t time_beep)
{
    cli();
    beep_on = 1;
    beep_counter = time_beep;
    DigitalWrite(PIN_BEEP, HIGH);
    sei();
}
//*******************************************************************************************************************

static void beep_control(void);
static void beep_control(void)
{
    if(beep_on)
    {
        if(beep_counter >= 10)
            beep_counter -= 10;
        else
        {
            beep_counter = 0;
            beep_on = 0;
            DigitalWrite(PIN_BEEP, LOW);
        }
    }
}

static void buzzer_control(void);
static void buzzer_control(void)
{
    static uint8_t cnt = 0;
    if(buz_on)
    {
        if(buz_counter >= 10)
        {
            buz_counter -= 10;
            cnt++;
            if(cnt >= 40) // 400ms
            {
                DigitalLevelToggle(PIN_BUZZER);
                cnt = 0;
            }
        }
        else
        {
            buzzer_stop();
            cnt = 0;
        }
    }
}
//*******************************************************************************************************************

// прерывание вызывается с периодом 10 мс
ISR(TIMER1_COMPA_vect)
{
    static uint8_t i=0;
    static uint8_t j=0;
    led_management();
    check_button();
    check_single_sensor();
    check_door_sensor();
    check_ext_mot_sensor();
    check_smoke_sensor();
    beep_control();
    buzzer_control();
    time_ms+=10;
    i++;
    if(i>=100) // 1 секунда
    {
        i=0;
        time_s++;
        time_from_start_s++;
        if(time_from_button_s < 0xFFFF)
            time_from_button_s++;
        soft_wdt++;
        j++;
        check_power();
        inc_time_from_motion();

        if(j>=60)
        {
            j=0;
            time_m++;
        }
        if(soft_wdt > 600) // 1200 секунд 20 минут
        {
            if(reset_s_wdt < 0xFFFF)
                reset_s_wdt++;
            reset_mcu(1);
        }
    }
    _WDT_RESET(); // сброс сторожевого таймера
}

//*******************************************************************************************************************

uint32_t get_time_ms(void)
{
    return get_val(time_ms);
}
//*******************************************************************************************************************

uint32_t get_time_s(void)
{
    return get_val(time_s);
}

//*******************************************************************************************************************

uint32_t get_time_m(void)
{
    return get_val(time_m);
}

//*******************************************************************************************************************

void reset_soft_wdt(void)
{
    set_val(soft_wdt, 0);
}

//*******************************************************************************************************************
static void inc_time_from_motion(void)
{
    time_from_motion_s++; // счетчик переполнится через 136 лет
}


//*******************************************************************************************************************
// задержка на х милисекунд
void delay_ms(const uint16_t delay)
{
    uint32_t time_stamp = get_time_ms() + delay;
    while((get_time_ms() < time_stamp))
    {
        #if(DEBUG==0)
        _SLEEP();
        #endif
        while(is_queue_not_empty()) // тем временем, проверяем входной буфер
            get_message_from_mdm();
    }
}

//*******************************************************************************************************************
// задержка на х секунд
void delay_s(uint8_t delay)
{
    uint32_t time_stamp = get_time_s() + delay;
    while((get_time_s() < time_stamp))
    {
        #if(DEBUG==0)
        _SLEEP();
        #endif
        while(is_queue_not_empty()) // тем временем, проверяем входной буфер
            get_message_from_mdm();
    }
}
void delay_us(uint32_t _us)
{
    while (0 < _us)
    {
        _delay_us(1);
        --_us;
    }
}
//*******************************************************************************************************************

//// Функция задержки в мкс
//void _delay_us(uint16_t time_us)
//{
//    register uint16_t i;
//
//    for(i = 0; i < time_us; i++)
//    {
//        asm volatile(" PUSH  R0 ");
//        asm volatile(" POP   R0 ");
//    }
//}
////*******************************************************************************************************************
//// Функция задержки в мс
//void _delay_ms(uint32_t time_ms)
//{
//    register uint32_t i;
//
//    for(i = 0; i < time_ms; i++)
//    {
//        _delay_us(250);
//        _delay_us(250);
//        _delay_us(250);
//        _delay_us(250);
//    }
//}
//////////////////////////////////////////////////
void set_time_from_motion_s(uint8_t time)
{
    set_val(time_from_motion_s, time);
}

uint32_t get_time_from_motion_s()
{
    return get_val(time_from_motion_s);
}
/////////////////////// time_from_start_s //////////////////////////
uint32_t get_time_from_start_s()
{
    return get_val(time_from_start_s);
}

//////////////////// time_from_button_s //////////////////////////
void set_time_from_button_s(uint16_t time)
{
    set_val(time_from_button_s, time);
}

uint16_t get_time_from_button_s()
{
    return get_val(time_from_button_s);
}
//////////////////////////////////////////////

//*******************************************************************************************************************

//uint16_t get_rst_s_wdt(void)
//{
//    return get_val(reset_s_wdt);
//}
//*******************************************************************************



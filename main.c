#include "main.h"



int main(void)
{
    _WDT_RESET(); // сброс сторожевого таймера
    WDTCSR = (1<<WDE) | (1<<WDP0) | (1<<WDP3);
    port_init();
    timer1_init();
    softuart_init();
    init_uart();
    sei();
    rst_default();
    softuart_puts_p(PSTR("Start..."));
    init_internal_temperature();
    charge_bat();
    delay_ms(5000);
    if (setup_init(delay_ms, uart_send_str_p))
    {
        send_USSD();
        softuart_puts_p(PSTR("setup_ok"));
    }
    eeprom_read_config(1);
    if(get_ext_temp() == 1)
        init_ds18x20();
    reset_soft_wdt();
    delay_ms(5000);
    power_control();
    delete_all_sms();
    #if(DEBUG==1)
    print_ram_free();
    #endif
    beep_non_block(100);
    softuart_puts_p(PSTR("Init Ok"));
    uint32_t now = get_time_s();
    for(;;)
    {
        reset_soft_wdt();
        incoming_call_processing();
        check_guard();
        check_registration();
        check_deposit();
        power_control();
        check_temperature_smoke();
        get_sms();

        if(get_time_s() > now + 10)
        {
            #if(DEBUG==1)
            print_ram_free();
            #endif
            //debug_report();
            char debug_buffer[8];
            char debug_buffer2[12];
            //char debug_buffer3[12];
            sprintf(debug_buffer,"bat=%d", analog_read(BATTERY));
            softuart_puts_ln(debug_buffer);
            delay_ms(100);
            sprintf(debug_buffer2,"t_int=%dC", analog_read(INT_TEMP));
            softuart_puts_ln(debug_buffer2);
            //sprintf(debug_buffer3,"t_ext=%dC", get_temperature());
            //softuart_puts_ln(debug_buffer3);
            //softuart_puts_ln("123");
            now = get_time_s();
        }
        while(is_queue_not_empty())
        {
            get_message_from_mdm();
        }
        #if(DEBUG==0)
        _SLEEP();
        #endif
    }
    return 0;
}

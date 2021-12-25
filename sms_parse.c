#include "main.h"

#define SIZE_OF_SMS_TEXT_BUF 160
#define MAX_SMS_LENGTH       160


static const char help_text[MAX_SMS_LENGTH+1] PROGMEM = {"helpset;helpget;report;phones;reboot;resetdefault;"};
static const char help_get[MAX_SMS_LENGTH+1] PROGMEM = {"get:phones;calls;sms;interval;power_report;ext_temp;"};
static const char help_set[MAX_SMS_LENGTH+1] PROGMEM = {"set:interval=;calls=;sms=;guard=;autoguard=;power_report=;buzzer=;guard_report=;ext_temp=;"};

static char* set_phones(char *phones, char *dest, char max_num);
static char* get_param(char *str, char *sms_text);
static char* set_param(char *ptr);

char find_phone_in_phone_list(char *phone, char lst)
{
    uint8_t i;

    if(lst==ADMIN_LIST)
    {
        for(i=0; i<TOTAL_ADMIN_NUMBER; i++)
        {
            if(get_phones(i)[0] == '+')
                if(memcmp(phone, &get_phones(i)[0], 13) == 0)
                {
                    softuart_puts_p(PSTR("phone ok"));
                    return 1; // телефон найден в списке админов
                }
        }
    }
    softuart_puts_p(PSTR("not phone"));
    return 0;
}

//*******************************************************************************************************************

void process_sms_body(char *ptr)
{
    uint8_t i, err;
    //beep_ms(10);

    if(memcmp_P(ptr, PSTR("set:"), 4) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        ptr += 4;
        i=0;
        err = 0;
        while(ptr && *ptr)
        {
            ptr = set_param(ptr);
            //softuart_puts_ln(ptr);
            if(ptr)
                i++;
            else
                err = 1;
        }
        if(i && !err)
        {
            EEPROM_update();
            send_sms_p(PSTR("ok"), get_phone_number());
        }
        else
        {
            eeprom_read_config(0); // возвращаем всё в зад
            send_sms_p(PSTR("error"), get_phone_number());
        }
    }

    else if(memcmp_P(ptr, PSTR("get:"), 4) == 0)
    {
        char sms_text[SIZE_OF_SMS_TEXT_BUF];
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        ptr += 4;
        sms_text[0] = 0; // терминируем строку
        i=0;
        err = 0;

        while(ptr && *ptr)
        {

            ptr = get_param(ptr, &sms_text[strlen(sms_text)]);
            // softuart_puts_ln(ptr);
            if(ptr)
                i++;
            else
            {
                err = 1;
                break;
            }
            if(strlen(sms_text) > MAX_SMS_LENGTH)
            {
                err = 2;
                break;
            }
        }
        if(i && !err)
        {
            send_sms(sms_text, get_phone_number());
            softuart_puts_p(PSTR("send_sms\r\n"));
        }

        else if(err==1)
        {
            send_sms_p(PSTR("error"), get_phone_number());
        }

        else if(err==2)
        {
            send_sms_p(PSTR("resulting SMS text is too long"), get_phone_number());
        }

    }

    else if(memcmp_P(ptr, PSTR("resetdefault;"), 13) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        set_first_usage(0);
        EEPROM_update();
        send_sms_p(PSTR("Device reset default, send: phones=NUMBER_PHONES1,NUMBER_PHONES2;"), get_phone_number());
        reset_mcu(1);
    }

    else if(memcmp_P(ptr, PSTR("reboot;"), 7) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_p(PSTR("Reset command accepted."), get_phone_number());
        reset_mcu(1);
    }
    else if(memcmp_P(ptr, PSTR("help;"), 5) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_p(help_text, get_phone_number());
    }
    else if(memcmp_P(ptr, PSTR("helpget;"), 8) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_p(help_get, get_phone_number());
    }
    else if(memcmp_P(ptr, PSTR("helpset;"), 8) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_p(help_set, get_phone_number());
    }
    else if(memcmp_P(ptr, PSTR("report;"), 7) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_report(get_phone_number());
    }
    else if(memcmp_P(ptr, PSTR("phones="), 7) == 0)
    {
        if( get_admin_mode() == 0 || (find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 1))
        {
            ptr+=7;
            ptr = set_phones(ptr, &get_phones(0)[0], TOTAL_ADMIN_NUMBER);
            set_admin_mode(1);
            EEPROM_update();
            send_sms_p(PSTR("Set admins ok"), get_phone_number());
            //send_sms("ok", get_phone_number());
            softuart_puts_p(PSTR("Set admins ok"));
            return;
        }
        else
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
    }
//    else if(memcmp_P(ptr, PSTR("pay_code="), 9) == 0)
//    {
//        char code[22] = {'*', '1', '0', '0', '*'};
//        ptr+=9;
//        softuart_puts_ln(ptr);
//        if(isdigit(*ptr))
//        {
//            strncat(code, ptr, 16);
//            strcat(code, "#");
//        }
//        else
//        {
//            send_sms_p(PSTR("Error, wrong request"), get_phone_number());
//            return;
//        }
//        //softuart_puts_ln(code);
//        send_USSD((const char *)code);
//    }

}

//*******************************************************************************************************************

static char* get_param(char *str, char *sms_text)
{
    if(memcmp_P(str, PSTR("phones;"), 12) == 0)
    {
        uint8_t n, i;

        str += 12;
        sms_text += sprintf_P(sms_text, PSTR("phones="));
        n=0;
        for(i=0; i<TOTAL_ADMIN_NUMBER; i++)
        {
            if(get_phones(i)[0] != '+')
                break;
            sms_text += sprintf_P(sms_text, PSTR("%s,"), &get_phones(i)[0]);
            n++; // количество напечатанных телефонов
        }
        if(n)
            sms_text--;
        sprintf_P(sms_text, PSTR(";"));
        return str;
    }

    else if(memcmp_P(str, PSTR("interval;"), 9) == 0)
    {
        str += 9;
        sprintf_P(sms_text, PSTR("interval_after_button_m=%um;"), get_interval_after_button_m());
        return str;
    }

    else if(memcmp_P(str, PSTR("pause_alarm;"), 12) == 0)
    {
        str += 12;
        sprintf_P(sms_text, PSTR("pause_alarm=%us;"), get_pause_alarm_report_s());
        return str;
    }

    else if(memcmp_P(str, PSTR("autoguard;"), 10) == 0)
    {
        str += 10;
        sprintf_P(sms_text, PSTR("autoguard=%d;"), get_autoguard());
        return str;
    }

    else if(memcmp_P(str, PSTR("accept_calls;"), 13) == 0)
    {
        str += 13;
        sprintf_P(sms_text, PSTR("accept_calls=%d;"), get_accept_calls());
        return str;
    }

    else if(memcmp_P(str, PSTR("accept_sms;"), 11) == 0)
    {
        str += 11;
        sprintf_P(sms_text, PSTR("accept_sms=%d;"), get_accept_sms());
        return str;
    }

    else if(memcmp_P(str, PSTR("ext_temp;"), 9) == 0)
    {
        str += 9;
        sprintf_P(sms_text, PSTR("ext_temp=%d;"), get_ext_temp());
        return str;
    }

    return 0;
}

//*******************************************************************************************************************

static char* set_param(char *ptr)
{
    if(memcmp_P(ptr, PSTR("pause_alarm="), 12) == 0)
    {
        uint32_t interval;
        ptr+=12;
        if(isdigit(*ptr) == 0)
            return 0;
        interval = strtoul(ptr, &ptr, 10);
        if( interval>250 )
            return 0;
        if(*ptr != ';')
            return 0;
        set_pause_alarm_report_s(interval);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("interval="), 9) == 0)
    {
        uint32_t interval;
        ptr+=9;
        if(isdigit(*ptr) == 0)
            return 0;
        interval = strtoul(ptr, &ptr, 10);
        if( interval>250 )
            return 0;
        if(*ptr != ';')
            return 0;
        set_interval_after_button_m(interval);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("guard="), 6) == 0)
    {
        char temp;

        ptr+=6;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
        {
            temp = 1;
        }
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        set_guard(temp);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("firstm="), 7) == 0)
    {
        char temp;

        ptr+=7;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
            temp = 1;
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        set_first_m(temp);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("autoguard="), 10) == 0)
    {
        char temp;

        ptr+=10;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
            temp = 1;
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        set_autoguard(temp);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("calls="), 6) == 0)
    {
        char temp;

        ptr+=6;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
            temp = 1;
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        set_accept_calls(temp);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("sms="), 4) == 0)
    {
        char temp;

        ptr+=4;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
            temp = 1;
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        set_accept_sms(temp);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("power_report="), 13) == 0)
    {
        char temp;

        ptr+=13;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
            temp = 1;
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        set_power_report(temp);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("guard_report="), 13) == 0)
    {
        char temp;

        ptr+=13;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
        {
            temp = 1;
        }
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        set_report_on_guard(temp);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("ext_temp="), 9) == 0)
    {
        char temp;

        ptr+=9;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
            temp = 1;
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        set_ext_temp(temp);
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("buzzer="), 7) == 0)
    {
        char temp;

        ptr+=7;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
        {
            temp = 1;
        }
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        if(temp == 1)
            buzzer_non_block(60000);
        else if (temp == 0)
            buzzer_stop();
        ptr++;
        return ptr;
    }
    return 0;
}

//*******************************************************************************************************************
// принимает список телефонов через запятую, адрес назначения и максимальный размер области назначения

static char* set_phones(char *phones, char *dest, char max_num)
{
    uint8_t n, i;
    char *ptr = phones;
    n=0;

    for(uint8_t j = 0; j < TOTAL_ADMIN_NUMBER; j++)
    {
        get_phones(j)[0] = 0;
    }
    while(check_phone_string(ptr))
    {
        n++;
        if(n > max_num)
            return 0;
        ptr+=13;
        if(*ptr == ';')
            break;
        else if(*ptr == ',')
        {
            ptr++;
            continue;
        }
        else
            return 0;
    }
    ptr = phones;
    for(i=0; i<n; i++)
    {
        memcpy(&dest[i*14], ptr, 13);
        dest[i*14 + 13] = 0;
        ptr+=14;
    }
    memset(&dest[n*14], 0, 14*(max_num-n));
    set_admin_mode(1);
    return ptr;
}
//*******************************************************************************************************************

void set_my_phone(void)
{
    set_phones("+380713318090;", &get_phones(0)[0], 1);
    set_admin_mode(1);
}




























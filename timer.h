#ifndef TIMER_H_
#define TIMER_H_
void timer1_init(void);
uint32_t get_time_ms(void);
uint32_t get_time_s(void);
uint32_t get_time_m(void);
void delay_ms(const uint16_t delay);
void delay_s(uint8_t delay);
void delay_us(uint32_t _us);
void beep_non_block(uint16_t time_beep);
void reset_soft_wdt(void);
uint32_t get_time_from_start(void);

//void inc_time_from_motion(void);
void buzzer_non_block(uint16_t time_to_buz);
void buzzer_stop(void);

void set_time_from_motion_s(uint8_t time);
uint32_t get_time_from_motion_s();

uint32_t get_time_from_start_s();
void set_time_from_start_s(uint32_t time);


void set_time_from_button_s(uint16_t time);
uint16_t get_time_from_button_s();

uint16_t get_rst_s_wdt(void);
//void _delay_us(uint16_t time_us);
//void _delay_ms(uint32_t time_ms);
#endif /* TIMER_H_ */

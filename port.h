#ifndef PORT_H_
#define PORT_H_


#define FRONT_SENSOR_MASK (1<<0)
#define INSIDE_1_SENSOR_MASK  (1<<1) //PD4, PD5
#define INSIDE_2_SENSOR_MASK (1<<2)
#define SIDE_SENSOR_MASK  (1<<3) //PD4, PD5

void port_init(void);

void led_management(void);
void check_motion(void);
void check_button(void);
uint8_t get_motion_detected();
uint8_t is_btn_rst_pressed(void);
void check_single_sensor(void);
uint8_t get_single_motion_detected(void);
void check_power(void);
void check_door_sensor(void);
uint8_t get_flag_open_door(void);
void check_ext_mot_sensor(void);
uint8_t get_flag_ext_mot_sens_detected(void);
void check_smoke_sensor(void);
uint8_t get_flag_smoke_sensor(void);
//void power_control(void);
//uint32_t get_time_without_power_s(void);
//uint8_t is_smoke(void);
#endif /* PORT_H_ */

#pragma once

#define 	GPIO_HIGH		"1"
#define 	GPIO_LOW		"0"
#define 	GPIO_OUTPUT		"out"
#define 	GPIO_INPUT		"in"

float gp2y_read_Vo_when_off(int fd);
float gp2y_read_Vo_when_clean(int fd);
void gp2y_init();
float gp2y_get_dust_density(int fd);
void gpio_set_value(char* value);
void gpio_set_mode(char* mode);
void gpio_init(char* fpath);

#pragma once

unsigned int ads1115_get_reg_val(int fd, unsigned char* reg_addr);
void ads1115_reset(int fd);
unsigned int ads1115_init(char* fpath, unsigned int slave_addr);

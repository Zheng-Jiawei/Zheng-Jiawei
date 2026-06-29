#ifndef __M203_H
#define __M203_H


#include "main.h"

void M203_Init(void);
void Data_in(uint8_t s1);
void Clk(uint8_t s1);
void Cmd(uint8_t s1);
void Rstn(uint8_t s1);
void Rst(uint8_t s1);
void Set_Sl_all(uint8_t s1);
void Set_Bl_all(uint8_t s1);
void Set_Wl_all(uint8_t s1);
void Set_Sl_single(uint16_t Row, uint8_t GBL_num);
void Set_Bl_single(uint16_t Row, uint8_t GBL_num);
void Set_Wl_single(uint16_t Col, uint8_t GWL_num);
void GBL_GWL_GSL_Init(void);
void GBL_select(uint8_t num);
void GWL_select(uint8_t num);
void GSL_select(uint8_t num, uint8_t model);


void Set_Bl_single_rows(uint16_t Row_start, uint16_t Row);
void Set_Bl_Reg(uint8_t Reg_num, uint8_t single);


void Set_Mux_Reg(uint8_t Mux_num);
#endif /* __M203_H */






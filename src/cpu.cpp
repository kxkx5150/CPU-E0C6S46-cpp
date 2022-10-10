#include "cpu.h"
#include "tamago.h"

#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))


CPU::CPU(Tamago *_tamago)
{
    tamago = _tamago;
}
void CPU::cpu_set_speed(u8_t speed)
{
    speed_ratio = speed;
}
u32_t CPU::cpu_get_depth(void)
{
    return call_depth;
}
void CPU::generate_interrupt(int_slot_t slot, u8_t bit)
{
    interrupts[slot].factor_flag_reg = interrupts[slot].factor_flag_reg | (0x1 << bit);
    if (interrupts[slot].mask_reg & (0x1 << bit)) {
        interrupts[slot].triggered = 1;
    }
}
void CPU::cpu_set_input_pin(pin_t pin, pin_state_t state)
{
    inputs[pin & 0x4].states = (inputs[pin & 0x4].states & ~(0x1 << (pin & 0x3))) | (state << (pin & 0x3));
    if (state == PIN_STATE_LOW) {
        switch ((pin & 0x4) >> 2) {
            case 0:
                generate_interrupt(INT_K00_K03_SLOT, pin & 0x3);
                break;
            case 1:
                generate_interrupt(INT_K10_K13_SLOT, pin & 0x3);
                break;
        }
    }
}
void CPU::cpu_sync_ref_timestamp(void)
{
    ref_ts = tamago->hal_get_timestamp();
}
u4_t CPU::get_io(u12_t n)
{
    u4_t tmp;
    switch (n) {
        case REG_CLK_INT_FACTOR_FLAGS:
            tmp                                              = interrupts[INT_CLOCK_TIMER_SLOT].factor_flag_reg;
            interrupts[INT_CLOCK_TIMER_SLOT].factor_flag_reg = 0;
            return tmp;
        case REG_SW_INT_FACTOR_FLAGS:
            tmp                                            = interrupts[INT_STOPWATCH_SLOT].factor_flag_reg;
            interrupts[INT_STOPWATCH_SLOT].factor_flag_reg = 0;
            return tmp;
        case REG_PROG_INT_FACTOR_FLAGS:
            tmp                                             = interrupts[INT_PROG_TIMER_SLOT].factor_flag_reg;
            interrupts[INT_PROG_TIMER_SLOT].factor_flag_reg = 0;
            return tmp;
        case REG_SERIAL_INT_FACTOR_FLAGS:
            tmp                                         = interrupts[INT_SERIAL_SLOT].factor_flag_reg;
            interrupts[INT_SERIAL_SLOT].factor_flag_reg = 0;
            return tmp;
        case REG_K00_K03_INT_FACTOR_FLAGS:
            tmp                                          = interrupts[INT_K00_K03_SLOT].factor_flag_reg;
            interrupts[INT_K00_K03_SLOT].factor_flag_reg = 0;
            return tmp;
        case REG_K10_K13_INT_FACTOR_FLAGS:
            tmp                                          = interrupts[INT_K10_K13_SLOT].factor_flag_reg;
            interrupts[INT_K10_K13_SLOT].factor_flag_reg = 0;
            return tmp;
        case REG_CLOCK_INT_MASKS:
            return interrupts[INT_CLOCK_TIMER_SLOT].mask_reg;
        case REG_SW_INT_MASKS:
            return interrupts[INT_STOPWATCH_SLOT].mask_reg & 0x3;
        case REG_PROG_INT_MASKS:
            return interrupts[INT_PROG_TIMER_SLOT].mask_reg & 0x1;
        case REG_SERIAL_INT_MASKS:
            return interrupts[INT_SERIAL_SLOT].mask_reg & 0x1;
        case REG_K00_K03_INT_MASKS:
            return interrupts[INT_K00_K03_SLOT].mask_reg;
        case REG_K10_K13_INT_MASKS:
            return interrupts[INT_K10_K13_SLOT].mask_reg;
        case REG_PROG_TIMER_DATA_L:
            return prog_timer_data & 0xF;
        case REG_PROG_TIMER_DATA_H:
            return (prog_timer_data >> 4) & 0xF;
        case REG_PROG_TIMER_RELOAD_DATA_L:
            return prog_timer_rld & 0xF;
        case REG_PROG_TIMER_RELOAD_DATA_H:
            return (prog_timer_rld >> 4) & 0xF;
        case REG_K00_K03_INPUT_PORT:
            return inputs[0].states;
        case REG_K10_K13_INPUT_PORT:
            return inputs[1].states;
        case REG_K40_K43_BZ_OUTPUT_PORT:
            return GET_IO_MEMORY(memory, n);
        case REG_CPU_OSC3_CTRL:
            return GET_IO_MEMORY(memory, n);
        case REG_LCD_CTRL:
            return GET_IO_MEMORY(memory, n);
        case REG_LCD_CONTRAST:
            break;
        case REG_SVD_CTRL:
            return GET_IO_MEMORY(memory, n) & 0x7;
        case REG_BUZZER_CTRL1:
            return GET_IO_MEMORY(memory, n);
        case REG_BUZZER_CTRL2:
            return GET_IO_MEMORY(memory, n) & 0x3;
        case REG_CLK_WD_TIMER_CTRL:
            break;
        case REG_SW_TIMER_CTRL:
            break;
        case REG_PROG_TIMER_CTRL:
            return !!prog_timer_enabled;
        case REG_PROG_TIMER_CLK_SEL:
            break;
        default:;
    }
    return 0;
}
void CPU::set_io(u12_t n, u4_t v)
{
    switch (n) {
        case REG_CLOCK_INT_MASKS:
            interrupts[INT_CLOCK_TIMER_SLOT].mask_reg = v;
            break;
        case REG_SW_INT_MASKS:
            interrupts[INT_STOPWATCH_SLOT].mask_reg = v;
            break;
        case REG_PROG_INT_MASKS:
            interrupts[INT_PROG_TIMER_SLOT].mask_reg = v;
            break;
        case REG_SERIAL_INT_MASKS:
            interrupts[INT_K10_K13_SLOT].mask_reg = v;
            break;
        case REG_K00_K03_INT_MASKS:
            interrupts[INT_SERIAL_SLOT].mask_reg = v;
            break;
        case REG_K10_K13_INT_MASKS:
            interrupts[INT_K10_K13_SLOT].mask_reg = v;
            break;
        case REG_PROG_TIMER_RELOAD_DATA_L:
            prog_timer_rld = v | (prog_timer_rld & 0xF0);
            break;
        case REG_PROG_TIMER_RELOAD_DATA_H:
            prog_timer_rld = (prog_timer_rld & 0xF) | (v << 4);
            break;
        case REG_K00_K03_INPUT_PORT:
            break;
        case REG_K40_K43_BZ_OUTPUT_PORT:
            //
            tamago->hal_play_frequency(!(v & 0x8));
            break;
        case REG_CPU_OSC3_CTRL:
            break;
        case REG_LCD_CTRL:
            break;
        case REG_LCD_CONTRAST:
            break;
        case REG_SVD_CTRL:
            break;
        case REG_BUZZER_CTRL1:
            break;
        case REG_BUZZER_CTRL2:
            break;
        case REG_CLK_WD_TIMER_CTRL:
            break;
        case REG_SW_TIMER_CTRL:
            break;
        case REG_PROG_TIMER_CTRL:
            if (v & 0x2) {
                prog_timer_data = prog_timer_rld;
            }
            if ((v & 0x1) && !prog_timer_enabled) {
                prog_timer_timestamp = tick_counter;
            }
            prog_timer_enabled = v & 0x1;
            break;
        case REG_PROG_TIMER_CLK_SEL:
            break;
        default:;
    }
}
void CPU::set_lcd(u12_t n, u4_t v)
{
    u8_t i;
    u8_t seg, com0;
    seg  = ((n & 0x7F) >> 1);
    com0 = (((n & 0x80) >> 7) * 8 + (n & 0x1) * 4);
    for (i = 0; i < 4; i++) {
        tamago->hw_set_lcd_pin(seg, com0 + i, (v >> i) & 0x1);
    }
}
u4_t CPU::get_memory(u12_t n)
{
    u4_t res = 0;
    if (n < MEM_RAM_SIZE) {

        res = GET_RAM_MEMORY(memory, n);
    } else if (n >= MEM_DISPLAY1_ADDR && n < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) {

        res = GET_DISP1_MEMORY(memory, n);
    } else if (n >= MEM_DISPLAY2_ADDR && n < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) {

        res = GET_DISP2_MEMORY(memory, n);
    } else if (n >= MEM_IO_ADDR && n < (MEM_IO_ADDR + MEM_IO_SIZE)) {

        res = get_io(n);
    } else {

        return 0;
    }

    return res;
}
void CPU::set_memory(u12_t n, u4_t v)
{
    if (n < MEM_RAM_SIZE) {
        SET_RAM_MEMORY(memory, n, v);

    } else if (n >= MEM_DISPLAY1_ADDR && n < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) {
        SET_DISP1_MEMORY(memory, n, v);
        set_lcd(n, v);

    } else if (n >= MEM_DISPLAY2_ADDR && n < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) {
        SET_DISP2_MEMORY(memory, n, v);
        set_lcd(n, v);

    } else if (n >= MEM_IO_ADDR && n < (MEM_IO_ADDR + MEM_IO_SIZE)) {
        SET_IO_MEMORY(memory, n, v);
        set_io(n, v);

    } else {

        return;
    }
}
void CPU::cpu_refresh_hw(void)
{
    static const struct range
    {
        u12_t addr;
        u12_t size;
    } refresh_locs[] = {
        {MEM_DISPLAY1_ADDR, MEM_DISPLAY1_SIZE},
        {MEM_DISPLAY2_ADDR, MEM_DISPLAY2_SIZE},
        {REG_BUZZER_CTRL1, 1},
        {REG_K40_K43_BZ_OUTPUT_PORT, 1},
        {0, 0},
    };
    for (int i = 0; refresh_locs[i].size != 0; i++) {
        for (u12_t n = refresh_locs[i].addr; n < (refresh_locs[i].addr + refresh_locs[i].size); n++) {
            set_memory(n, GET_MEMORY(memory, n));
        }
    }
}
u4_t CPU::get_rq(u12_t rq)
{
    switch (rq & 0x3) {
        case 0x0:
            return a;
        case 0x1:
            return b;
        case 0x2:
            return M(x);
        case 0x3:
            return M(y);
    }
    return 0;
}
void CPU::set_rq(u12_t rq, u4_t v)
{
    switch (rq & 0x3) {
        case 0x0:
            a = v;
            break;
        case 0x1:
            b = v;
            break;
        case 0x2:
            SET_M(x, v);
            break;
        case 0x3:
            SET_M(y, v);
            break;
    }
}

//

void CPU::op_pset_cb(u8_t arg0, u8_t arg1)
{
    np = arg0;
}
void CPU::op_jp_cb(u8_t arg0, u8_t arg1)
{
    next_pc = arg0 | (np << 8);
}
void CPU::op_jp_c_cb(u8_t arg0, u8_t arg1)
{
    if (flags & FLAG_C) {
        next_pc = arg0 | (np << 8);
    }
}
void CPU::op_jp_nc_cb(u8_t arg0, u8_t arg1)
{
    if (!(flags & FLAG_C)) {
        next_pc = arg0 | (np << 8);
    }
}
void CPU::op_jp_z_cb(u8_t arg0, u8_t arg1)
{
    if (flags & FLAG_Z) {
        next_pc = arg0 | (np << 8);
    }
}
void CPU::op_jp_nz_cb(u8_t arg0, u8_t arg1)
{
    if (!(flags & FLAG_Z)) {
        next_pc = arg0 | (np << 8);
    }
}
void CPU::op_jpba_cb(u8_t arg0, u8_t arg1)
{
    next_pc = a | (b << 4) | (np << 8);
}
void CPU::op_call_cb(u8_t arg0, u8_t arg1)
{
    pc = (pc + 1) & 0x1FFF;
    SET_M(sp - 1, PCP);
    SET_M(sp - 2, PCSH);
    SET_M(sp - 3, PCSL);
    sp      = (sp - 3) & 0xFF;
    next_pc = TO_PC(PCB, NPP, arg0);
    call_depth++;
}
void CPU::op_calz_cb(u8_t arg0, u8_t arg1)
{
    pc = (pc + 1) & 0x1FFF;
    SET_M(sp - 1, PCP);
    SET_M(sp - 2, PCSH);
    SET_M(sp - 3, PCSL);
    sp      = (sp - 3) & 0xFF;
    next_pc = TO_PC(PCB, 0, arg0);
    call_depth++;
}
void CPU::op_ret_cb(u8_t arg0, u8_t arg1)
{
    next_pc = M(sp) | (M(sp + 1) << 4) | (M(sp + 2) << 8) | (PCB << 12);
    sp      = (sp + 3) & 0xFF;
    call_depth--;
}
void CPU::op_rets_cb(u8_t arg0, u8_t arg1)
{
    next_pc = M(sp) | (M(sp + 1) << 4) | (M(sp + 2) << 8) | (PCB << 12);
    sp      = (sp + 3) & 0xFF;
    next_pc = (pc + 1) & 0x1FFF;
    call_depth--;
}
void CPU::op_retd_cb(u8_t arg0, u8_t arg1)
{
    next_pc = M(sp) | (M(sp + 1) << 4) | (M(sp + 2) << 8) | (PCB << 12);
    sp      = (sp + 3) & 0xFF;
    SET_M(x, arg0 & 0xF);
    SET_M(x + 1, (arg0 >> 4) & 0xF);
    x = ((x + 2) & 0xFF) | (XP << 8);
    call_depth--;
}
void CPU::op_nop5_cb(u8_t arg0, u8_t arg1)
{
}
void CPU::op_nop7_cb(u8_t arg0, u8_t arg1)
{
}
void CPU::op_halt_cb(u8_t arg0, u8_t arg1)
{
    tamago->hal_halt();
}
void CPU::op_inc_x_cb(u8_t arg0, u8_t arg1)
{
    x = ((x + 1) & 0xFF) | (XP << 8);
}
void CPU::op_inc_y_cb(u8_t arg0, u8_t arg1)
{
    y = ((y + 1) & 0xFF) | (YP << 8);
}
void CPU::op_ld_x_cb(u8_t arg0, u8_t arg1)
{
    x = arg0 | (XP << 8);
}
void CPU::op_ld_y_cb(u8_t arg0, u8_t arg1)
{
    y = arg0 | (YP << 8);
}
void CPU::op_ld_xp_r_cb(u8_t arg0, u8_t arg1)
{
    x = XHL | (RQ(arg0) << 8);
}
void CPU::op_ld_xh_r_cb(u8_t arg0, u8_t arg1)
{
    x = XL | (RQ(arg0) << 4) | (XP << 8);
}
void CPU::op_ld_xl_r_cb(u8_t arg0, u8_t arg1)
{
    x = RQ(arg0) | (XH << 4) | (XP << 8);
}
void CPU::op_ld_yp_r_cb(u8_t arg0, u8_t arg1)
{
    y = YHL | (RQ(arg0) << 8);
}
void CPU::op_ld_yh_r_cb(u8_t arg0, u8_t arg1)
{
    y = YL | (RQ(arg0) << 4) | (YP << 8);
}
void CPU::op_ld_yl_r_cb(u8_t arg0, u8_t arg1)
{
    y = RQ(arg0) | (YH << 4) | (YP << 8);
}
void CPU::op_ld_r_xp_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, XP);
}
void CPU::op_ld_r_xh_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, XH);
}
void CPU::op_ld_r_xl_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, XL);
}
void CPU::op_ld_r_yp_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, YP);
}
void CPU::op_ld_r_yh_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, YH);
}
void CPU::op_ld_r_yl_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, YL);
}
void CPU::op_adc_xh_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = XH + arg0 + C;
    x   = XL | ((tmp & 0xF) << 4) | (XP << 8);
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!(tmp & 0xF)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_adc_xl_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = XL + arg0 + C;
    x   = (tmp & 0xF) | (XH << 4) | (XP << 8);
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!(tmp & 0xF)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_adc_yh_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = YH + arg0 + C;
    y   = YL | ((tmp & 0xF) << 4) | (YP << 8);
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!(tmp & 0xF)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_adc_yl_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = YL + arg0 + C;
    y   = (tmp & 0xF) | (YH << 4) | (YP << 8);
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!(tmp & 0xF)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_cp_xh_cb(u8_t arg0, u8_t arg1)
{
    if (XH < arg0) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (XH == arg0) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_cp_xl_cb(u8_t arg0, u8_t arg1)
{
    if (XL < arg0) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (XL == arg0) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_cp_yh_cb(u8_t arg0, u8_t arg1)
{
    if (YH < arg0) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (YH == arg0) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_cp_yl_cb(u8_t arg0, u8_t arg1)
{
    if (YL < arg0) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (YL == arg0) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_ld_r_i_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, arg1);
}
void CPU::op_ld_r_q_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, RQ(arg1));
}
void CPU::op_ld_a_mn_cb(u8_t arg0, u8_t arg1)
{
    a = M(arg0);
}
void CPU::op_ld_b_mn_cb(u8_t arg0, u8_t arg1)
{
    b = M(arg0);
}
void CPU::op_ld_mn_a_cb(u8_t arg0, u8_t arg1)
{
    SET_M(arg0, a);
}
void CPU::op_ld_mn_b_cb(u8_t arg0, u8_t arg1)
{
    SET_M(arg0, b);
}
void CPU::op_ldpx_mx_cb(u8_t arg0, u8_t arg1)
{
    SET_M(x, arg0);
    x = ((x + 1) & 0xFF) | (XP << 8);
}
void CPU::op_ldpx_r_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, RQ(arg1));
    x = ((x + 1) & 0xFF) | (XP << 8);
}
void CPU::op_ldpy_my_cb(u8_t arg0, u8_t arg1)
{
    SET_M(y, arg0);
    y = ((y + 1) & 0xFF) | (YP << 8);
}
void CPU::op_ldpy_r_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, RQ(arg1));
    y = ((y + 1) & 0xFF) | (YP << 8);
}
void CPU::op_lbpx_cb(u8_t arg0, u8_t arg1)
{
    SET_M(x, arg0 & 0xF);
    SET_M(x + 1, (arg0 >> 4) & 0xF);
    x = ((x + 2) & 0xFF) | (XP << 8);
}
void CPU::op_set_cb(u8_t arg0, u8_t arg1)
{
    flags |= arg0;
}
void CPU::op_rst_cb(u8_t arg0, u8_t arg1)
{
    flags &= arg0;
}
void CPU::op_scf_cb(u8_t arg0, u8_t arg1)
{
    SET_C();
}
void CPU::op_rcf_cb(u8_t arg0, u8_t arg1)
{
    CLEAR_C();
}
void CPU::op_szf_cb(u8_t arg0, u8_t arg1)
{
    SET_Z();
}
void CPU::op_rzf_cb(u8_t arg0, u8_t arg1)
{
    CLEAR_Z();
}
void CPU::op_sdf_cb(u8_t arg0, u8_t arg1)
{
    SET_D();
}
void CPU::op_rdf_cb(u8_t arg0, u8_t arg1)
{
    CLEAR_D();
}
void CPU::op_ei_cb(u8_t arg0, u8_t arg1)
{
    SET_I();
}
void CPU::op_di_cb(u8_t arg0, u8_t arg1)
{
    CLEAR_I();
}
void CPU::op_inc_sp_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp + 1) & 0xFF;
}
void CPU::op_dec_sp_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp - 1) & 0xFF;
}
void CPU::op_push_r_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp - 1) & 0xFF;
    SET_M(sp, RQ(arg0));
}
void CPU::op_push_xp_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp - 1) & 0xFF;
    SET_M(sp, XP);
}
void CPU::op_push_xh_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp - 1) & 0xFF;
    SET_M(sp, XH);
}
void CPU::op_push_xl_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp - 1) & 0xFF;
    SET_M(sp, XL);
}
void CPU::op_push_yp_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp - 1) & 0xFF;
    SET_M(sp, YP);
}
void CPU::op_push_yh_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp - 1) & 0xFF;
    SET_M(sp, YH);
}
void CPU::op_push_yl_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp - 1) & 0xFF;
    SET_M(sp, YL);
}
void CPU::op_push_f_cb(u8_t arg0, u8_t arg1)
{
    sp = (sp - 1) & 0xFF;
    SET_M(sp, flags);
}
void CPU::op_pop_r_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, M(sp));
    sp = (sp + 1) & 0xFF;
}
void CPU::op_pop_xp_cb(u8_t arg0, u8_t arg1)
{
    x  = XL | (XH << 4) | (M(sp) << 8);
    sp = (sp + 1) & 0xFF;
}
void CPU::op_pop_xh_cb(u8_t arg0, u8_t arg1)
{
    x  = XL | (M(sp) << 4) | (XP << 8);
    sp = (sp + 1) & 0xFF;
}
void CPU::op_pop_xl_cb(u8_t arg0, u8_t arg1)
{
    x  = M(sp) | (XH << 4) | (XP << 8);
    sp = (sp + 1) & 0xFF;
}
void CPU::op_pop_yp_cb(u8_t arg0, u8_t arg1)
{
    y  = YL | (YH << 4) | (M(sp) << 8);
    sp = (sp + 1) & 0xFF;
}
void CPU::op_pop_yh_cb(u8_t arg0, u8_t arg1)
{
    y  = YL | (M(sp) << 4) | (YP << 8);
    sp = (sp + 1) & 0xFF;
}
void CPU::op_pop_yl_cb(u8_t arg0, u8_t arg1)
{
    y  = M(sp) | (YH << 4) | (YP << 8);
    sp = (sp + 1) & 0xFF;
}
void CPU::op_pop_f_cb(u8_t arg0, u8_t arg1)
{
    flags = M(sp);
    sp    = (sp + 1) & 0xFF;
}
void CPU::op_ld_sph_r_cb(u8_t arg0, u8_t arg1)
{
    sp = SPL | (RQ(arg0) << 4);
}
void CPU::op_ld_spl_r_cb(u8_t arg0, u8_t arg1)
{
    sp = RQ(arg0) | (SPH << 4);
}
void CPU::op_ld_r_sph_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, SPH);
}
void CPU::op_ld_r_spl_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, SPL);
}
void CPU::op_add_r_i_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = RQ(arg0) + arg1;
    if (D) {
        if (tmp >= 10) {
            SET_RQ(arg0, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_RQ(arg0, tmp);
            CLEAR_C();
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
        if (tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_add_r_q_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = RQ(arg0) + RQ(arg1);
    if (D) {
        if (tmp >= 10) {
            SET_RQ(arg0, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_RQ(arg0, tmp);
            CLEAR_C();
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
        if (tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_adc_r_i_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = RQ(arg0) + arg1 + C;
    if (D) {
        if (tmp >= 10) {
            SET_RQ(arg0, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_RQ(arg0, tmp);
            CLEAR_C();
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
        if (tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_adc_r_q_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = RQ(arg0) + RQ(arg1) + C;
    if (D) {
        if (tmp >= 10) {
            SET_RQ(arg0, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_RQ(arg0, tmp);
            CLEAR_C();
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
        if (tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_sub_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = RQ(arg0) - RQ(arg1);
    if (D) {
        if (tmp >> 4) {
            SET_RQ(arg0, (tmp - 6) & 0xF);
        } else {
            SET_RQ(arg0, tmp);
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
    }
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_sbc_r_i_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = RQ(arg0) - arg1 - C;
    if (D) {
        if (tmp >> 4) {
            SET_RQ(arg0, (tmp - 6) & 0xF);
        } else {
            SET_RQ(arg0, tmp);
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
    }
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_sbc_r_q_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = RQ(arg0) - RQ(arg1) - C;
    if (D) {
        if (tmp >> 4) {
            SET_RQ(arg0, (tmp - 6) & 0xF);
        } else {
            SET_RQ(arg0, tmp);
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
    }
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_and_r_i_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, RQ(arg0) & arg1);
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_and_r_q_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, RQ(arg0) & RQ(arg1));
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_or_r_i_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, RQ(arg0) | arg1);
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_or_r_q_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, RQ(arg0) | RQ(arg1));
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_xor_r_i_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, RQ(arg0) ^ arg1);
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_xor_r_q_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, RQ(arg0) ^ RQ(arg1));
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_cp_r_i_cb(u8_t arg0, u8_t arg1)
{
    if (RQ(arg0) < arg1) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (RQ(arg0) == arg1) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_cp_r_q_cb(u8_t arg0, u8_t arg1)
{
    if (RQ(arg0) < RQ(arg1)) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (RQ(arg0) == RQ(arg1)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_fan_r_i_cb(u8_t arg0, u8_t arg1)
{
    if (!(RQ(arg0) & arg1)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_fan_r_q_cb(u8_t arg0, u8_t arg1)
{
    if (!(RQ(arg0) & RQ(arg1))) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_rlc_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = (RQ(arg0) << 1) | C;
    if (RQ(arg0) & 0x8) {
        SET_C();
    } else {
        CLEAR_C();
    }
    SET_RQ(arg0, tmp & 0xF);
}
void CPU::op_rrc_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = (RQ(arg0) >> 1) | (C << 3);
    if (RQ(arg0) & 0x1) {
        SET_C();
    } else {
        CLEAR_C();
    }
    SET_RQ(arg0, tmp & 0xF);
}
void CPU::op_inc_mn_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = M(arg0) + 1;
    SET_M(arg0, tmp & 0xF);
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!M(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_dec_mn_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = M(arg0) - 1;
    SET_M(arg0, tmp & 0xF);
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!M(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}
void CPU::op_acpx_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = M(x) + RQ(arg0) + C;
    if (D) {
        if (tmp >= 10) {
            SET_M(x, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_M(x, tmp);
            CLEAR_C();
        }
    } else {
        SET_M(x, tmp & 0xF);
        if (tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if (!M(x)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
    x = ((x + 1) & 0xFF) | (XP << 8);
}
void CPU::op_acpy_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = M(y) + RQ(arg0) + C;
    if (D) {
        if (tmp >= 10) {
            SET_M(y, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_M(y, tmp);
            CLEAR_C();
        }
    } else {
        SET_M(y, tmp & 0xF);
        if (tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if (!M(y)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
    y = ((y + 1) & 0xFF) | (YP << 8);
}
void CPU::op_scpx_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = M(x) - RQ(arg0) - C;
    if (D) {
        if (tmp >> 4) {
            SET_M(x, (tmp - 6) & 0xF);
        } else {
            SET_M(x, tmp);
        }
    } else {
        SET_M(x, tmp & 0xF);
    }
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!M(x)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
    x = ((x + 1) & 0xFF) | (XP << 8);
}
void CPU::op_scpy_cb(u8_t arg0, u8_t arg1)
{
    u8_t tmp;
    tmp = M(y) - RQ(arg0) - C;
    if (D) {
        if (tmp >> 4) {
            SET_M(y, (tmp - 6) & 0xF);
        } else {
            SET_M(y, tmp);
        }
    } else {
        SET_M(y, tmp & 0xF);
    }
    if (tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if (!M(y)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
    y = ((y + 1) & 0xFF) | (YP << 8);
}
void CPU::op_not_cb(u8_t arg0, u8_t arg1)
{
    SET_RQ(arg0, ~RQ(arg0) & 0xF);
    if (!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

//

timestamp_t CPU::wait_for_cycles(timestamp_t since, u8_t cycles)
{
    timestamp_t deadline;
    tick_counter += cycles;
    if (speed_ratio == 0) {
        return tamago->hal_get_timestamp();
    }
    deadline = since + (cycles * ts_freq) / (TICK_FREQUENCY * speed_ratio);
    tamago->hal_sleep_until(deadline);
    return deadline;
}
void CPU::process_interrupts(void)
{
    u8_t i;
    for (i = 0; i < INT_SLOT_NUM; i++) {
        if (interrupts[i].triggered) {

            SET_M(sp - 1, PCP);
            SET_M(sp - 2, PCSH);
            SET_M(sp - 3, PCSL);
            sp = (sp - 3) & 0xFF;
            CLEAR_I();
            np = TO_NP(NBP, 1);
            pc = TO_PC(PCB, 1, interrupts[i].vector);
            call_depth++;
            ref_ts                  = wait_for_cycles(ref_ts, 12);
            interrupts[i].triggered = 0;
        }
    }
}
void CPU::cpu_reset(void)
{
    u13_t i;
    pc    = TO_PC(0, 1, 0x00);
    np    = TO_NP(0, 1);
    a     = 0;
    b     = 0;
    x     = 0;
    y     = 0;
    sp    = 0;
    flags = 0;
    for (i = 0; i < MEM_BUFFER_SIZE; i++) {
        memory[i] = 0;
    }
    SET_IO_MEMORY(memory, REG_K40_K43_BZ_OUTPUT_PORT, 0xF);
    SET_IO_MEMORY(memory, REG_LCD_CTRL, 0x8);
    cpu_sync_ref_timestamp();
}
bool_t CPU::cpu_init(const u12_t *program, breakpoint_t *breakpoints, u32_t freq)
{
    g_program     = program;
    g_breakpoints = breakpoints;
    ts_freq       = freq;
    cpu_reset();
    return 0;
}
int CPU::cpu_step(void)
{
    u12_t         op;
    u8_t          i;
    breakpoint_t *bp = g_breakpoints;

    op = g_program[pc];
    for (i = 0; ops[i].log != 0; i++) {
        if ((op & ops[i].mask) == ops[i].code) {
            break;
        }
    }

    if (ops[i].log == 0) {

        return 1;
    }

    next_pc = (pc + 1) & 0x1FFF;
    ref_ts  = wait_for_cycles(ref_ts, precycles);

    if (ops[i].mask_arg0 != 0) {
        CALL_MEMBER_FN(*this, ops[i].cb)
        ((op & ops[i].mask_arg0) >> ops[i].shift_arg0, op & ~(ops[i].mask | ops[i].mask_arg0));
    } else {
        CALL_MEMBER_FN(*this, ops[i].cb)
        ((op & ~ops[i].mask) >> ops[i].shift_arg0, 0);
    }

    pc        = next_pc;
    precycles = ops[i].cycles;

    if (i > 0) {
        np = (pc >> 8) & 0x1F;
    }

    if (tick_counter - clk_timer_timestamp >= TIMER_1HZ_PERIOD) {
        do {
            clk_timer_timestamp += TIMER_1HZ_PERIOD;
        } while (tick_counter - clk_timer_timestamp >= TIMER_1HZ_PERIOD);
        generate_interrupt(INT_CLOCK_TIMER_SLOT, 3);
    }

    if (prog_timer_enabled && tick_counter - prog_timer_timestamp >= TIMER_256HZ_PERIOD) {
        do {
            prog_timer_timestamp += TIMER_256HZ_PERIOD;
            prog_timer_data--;
            if (prog_timer_data == 0) {
                prog_timer_data = prog_timer_rld;
                generate_interrupt(INT_PROG_TIMER_SLOT, 0);
            }
        } while (tick_counter - prog_timer_timestamp >= TIMER_256HZ_PERIOD);
    }

    if (I && i > 0) {
        process_interrupts();
    }

    while (bp != 0) {
        if (bp->addr == pc) {
            return 1;
        }
        bp = bp->next;
    }
    return 0;
}

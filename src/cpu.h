#ifndef _CPU_H_
#define _CPU_H_
#include <stdint.h>
#include "cpu_def.h"


typedef uint8_t  bool_t;
typedef uint8_t  u4_t;
typedef uint8_t  u5_t;
typedef uint8_t  u8_t;
typedef uint16_t u12_t;
typedef uint16_t u13_t;
typedef uint32_t u32_t;
typedef uint32_t timestamp_t;

typedef enum
{
    LOG_ERROR  = 0x1,
    LOG_INFO   = (0x1 << 1),
    LOG_MEMORY = (0x1 << 2),
    LOG_CPU    = (0x1 << 3),
} log_level_t;

typedef enum
{
    PIN_K00 = 0x0,
    PIN_K01 = 0x1,
    PIN_K02 = 0x2,
    PIN_K03 = 0x3,
    PIN_K10 = 0X4,
    PIN_K11 = 0X5,
    PIN_K12 = 0X6,
    PIN_K13 = 0X7,
} pin_t;

typedef enum
{
    PIN_STATE_LOW  = 0,
    PIN_STATE_HIGH = 1,
} pin_state_t;

typedef enum
{
    INT_PROG_TIMER_SLOT  = 0,
    INT_SERIAL_SLOT      = 1,
    INT_K10_K13_SLOT     = 2,
    INT_K00_K03_SLOT     = 3,
    INT_STOPWATCH_SLOT   = 4,
    INT_CLOCK_TIMER_SLOT = 5,
    INT_SLOT_NUM,
} int_slot_t;

typedef struct breakpoint
{
    u13_t              addr;
    struct breakpoint *next;
} breakpoint_t;

typedef struct
{
    u4_t states;
} input_port_t;

typedef struct
{
    u4_t   factor_flag_reg;
    u4_t   mask_reg;
    bool_t triggered;
    u8_t   vector;
} interrupt_t;


class Tamago;
class CPU {
  private:
    typedef void (CPU::*proc_t)(u8_t, u8_t);
    typedef struct
    {
        char  *log;
        u12_t  code;
        u12_t  mask;
        u12_t  shift_arg0;
        u12_t  mask_arg0;
        u8_t   cycles;
        proc_t cb;
    } op_t;

  public:
    Tamago *tamago = nullptr;

  private:
    const u12_t *g_program = 0;

    u13_t pc, next_pc;
    u12_t x, y;
    u4_t  a, b;
    u5_t  np;
    u8_t  sp;
    u4_t  flags;

    breakpoint_t *g_breakpoints        = 0;
    u32_t         call_depth           = 0;
    u32_t         clk_timer_timestamp  = 0;
    u32_t         prog_timer_timestamp = 0;
    bool_t        prog_timer_enabled   = 0;
    u8_t          prog_timer_data      = 0;
    u8_t          prog_timer_rld       = 0;
    u32_t         tick_counter         = 0;
    u32_t         ts_freq;
    u8_t          speed_ratio = 1;
    timestamp_t   ref_ts;

    u8_t         memory[MEM_BUFFER_SIZE];
    input_port_t inputs[2] = {{0}};

    u8_t precycles = 0;

    interrupt_t interrupts[INT_SLOT_NUM] = {
        {0x0, 0x0, 0, 0x0C}, {0x0, 0x0, 0, 0x0A}, {0x0, 0x0, 0, 0x08},
        {0x0, 0x0, 0, 0x06}, {0x0, 0x0, 0, 0x04}, {0x0, 0x0, 0, 0x02},
    };

  public:
    CPU(Tamago *_tamago);

    void  cpu_set_speed(u8_t speed);
    u32_t cpu_get_depth(void);

    void generate_interrupt(int_slot_t slot, u8_t bit);
    void cpu_set_input_pin(pin_t pin, pin_state_t state);
    void cpu_sync_ref_timestamp(void);

    u4_t get_io(u12_t n);
    void set_io(u12_t n, u4_t v);
    void set_lcd(u12_t n, u4_t v);

    u4_t get_memory(u12_t n);
    void set_memory(u12_t n, u4_t v);

    void cpu_refresh_hw(void);
    u4_t get_rq(u12_t rq);
    void set_rq(u12_t rq, u4_t v);

    timestamp_t wait_for_cycles(timestamp_t since, u8_t cycles);
    void        process_interrupts(void);

    void   cpu_reset(void);
    bool_t cpu_init(const u12_t *program, breakpoint_t *breakpoints, u32_t freq);
    int    cpu_step(void);

  private:
    void op_pset_cb(u8_t arg0, u8_t arg1);
    void op_jp_cb(u8_t arg0, u8_t arg1);
    void op_jp_c_cb(u8_t arg0, u8_t arg1);
    void op_jp_nc_cb(u8_t arg0, u8_t arg1);
    void op_jp_z_cb(u8_t arg0, u8_t arg1);
    void op_jp_nz_cb(u8_t arg0, u8_t arg1);
    void op_jpba_cb(u8_t arg0, u8_t arg1);

    void op_call_cb(u8_t arg0, u8_t arg1);
    void op_calz_cb(u8_t arg0, u8_t arg1);

    void op_ret_cb(u8_t arg0, u8_t arg1);
    void op_rets_cb(u8_t arg0, u8_t arg1);
    void op_retd_cb(u8_t arg0, u8_t arg1);

    void op_nop5_cb(u8_t arg0, u8_t arg1);
    void op_nop7_cb(u8_t arg0, u8_t arg1);

    void op_halt_cb(u8_t arg0, u8_t arg1);
    void op_inc_x_cb(u8_t arg0, u8_t arg1);
    void op_inc_y_cb(u8_t arg0, u8_t arg1);

    void op_ld_x_cb(u8_t arg0, u8_t arg1);
    void op_ld_y_cb(u8_t arg0, u8_t arg1);
    void op_ld_xp_r_cb(u8_t arg0, u8_t arg1);
    void op_ld_xh_r_cb(u8_t arg0, u8_t arg1);
    void op_ld_xl_r_cb(u8_t arg0, u8_t arg1);
    void op_ld_yp_r_cb(u8_t arg0, u8_t arg1);
    void op_ld_yh_r_cb(u8_t arg0, u8_t arg1);
    void op_ld_yl_r_cb(u8_t arg0, u8_t arg1);
    void op_ld_r_xp_cb(u8_t arg0, u8_t arg1);
    void op_ld_r_xh_cb(u8_t arg0, u8_t arg1);
    void op_ld_r_xl_cb(u8_t arg0, u8_t arg1);
    void op_ld_r_yp_cb(u8_t arg0, u8_t arg1);
    void op_ld_r_yh_cb(u8_t arg0, u8_t arg1);
    void op_ld_r_yl_cb(u8_t arg0, u8_t arg1);

    void op_adc_xh_cb(u8_t arg0, u8_t arg1);
    void op_adc_xl_cb(u8_t arg0, u8_t arg1);
    void op_adc_yh_cb(u8_t arg0, u8_t arg1);
    void op_adc_yl_cb(u8_t arg0, u8_t arg1);

    void op_cp_xh_cb(u8_t arg0, u8_t arg1);
    void op_cp_xl_cb(u8_t arg0, u8_t arg1);
    void op_cp_yh_cb(u8_t arg0, u8_t arg1);
    void op_cp_yl_cb(u8_t arg0, u8_t arg1);

    void op_ld_r_i_cb(u8_t arg0, u8_t arg1);
    void op_ld_r_q_cb(u8_t arg0, u8_t arg1);
    void op_ld_a_mn_cb(u8_t arg0, u8_t arg1);
    void op_ld_b_mn_cb(u8_t arg0, u8_t arg1);
    void op_ld_mn_a_cb(u8_t arg0, u8_t arg1);
    void op_ld_mn_b_cb(u8_t arg0, u8_t arg1);
    void op_ldpx_mx_cb(u8_t arg0, u8_t arg1);
    void op_ldpx_r_cb(u8_t arg0, u8_t arg1);
    void op_ldpy_my_cb(u8_t arg0, u8_t arg1);
    void op_ldpy_r_cb(u8_t arg0, u8_t arg1);
    void op_lbpx_cb(u8_t arg0, u8_t arg1);

    void op_set_cb(u8_t arg0, u8_t arg1);
    void op_rst_cb(u8_t arg0, u8_t arg1);
    void op_scf_cb(u8_t arg0, u8_t arg1);
    void op_rcf_cb(u8_t arg0, u8_t arg1);
    void op_szf_cb(u8_t arg0, u8_t arg1);
    void op_rzf_cb(u8_t arg0, u8_t arg1);
    void op_sdf_cb(u8_t arg0, u8_t arg1);
    void op_rdf_cb(u8_t arg0, u8_t arg1);
    void op_ei_cb(u8_t arg0, u8_t arg1);
    void op_di_cb(u8_t arg0, u8_t arg1);

    void op_inc_sp_cb(u8_t arg0, u8_t arg1);
    void op_dec_sp_cb(u8_t arg0, u8_t arg1);

    void op_push_r_cb(u8_t arg0, u8_t arg1);
    void op_push_xp_cb(u8_t arg0, u8_t arg1);
    void op_push_xh_cb(u8_t arg0, u8_t arg1);
    void op_push_xl_cb(u8_t arg0, u8_t arg1);
    void op_push_yp_cb(u8_t arg0, u8_t arg1);
    void op_push_yh_cb(u8_t arg0, u8_t arg1);
    void op_push_yl_cb(u8_t arg0, u8_t arg1);
    void op_push_f_cb(u8_t arg0, u8_t arg1);
    void op_pop_r_cb(u8_t arg0, u8_t arg1);
    void op_pop_xp_cb(u8_t arg0, u8_t arg1);
    void op_pop_xh_cb(u8_t arg0, u8_t arg1);
    void op_pop_xl_cb(u8_t arg0, u8_t arg1);
    void op_pop_yp_cb(u8_t arg0, u8_t arg1);
    void op_pop_yh_cb(u8_t arg0, u8_t arg1);
    void op_pop_yl_cb(u8_t arg0, u8_t arg1);
    void op_pop_f_cb(u8_t arg0, u8_t arg1);

    void op_ld_sph_r_cb(u8_t arg0, u8_t arg1);
    void op_ld_spl_r_cb(u8_t arg0, u8_t arg1);
    void op_ld_r_sph_cb(u8_t arg0, u8_t arg1);
    void op_ld_r_spl_cb(u8_t arg0, u8_t arg1);

    void op_add_r_i_cb(u8_t arg0, u8_t arg1);
    void op_add_r_q_cb(u8_t arg0, u8_t arg1);
    void op_adc_r_i_cb(u8_t arg0, u8_t arg1);
    void op_adc_r_q_cb(u8_t arg0, u8_t arg1);
    void op_sub_cb(u8_t arg0, u8_t arg1);
    void op_sbc_r_i_cb(u8_t arg0, u8_t arg1);
    void op_sbc_r_q_cb(u8_t arg0, u8_t arg1);

    void op_and_r_i_cb(u8_t arg0, u8_t arg1);
    void op_and_r_q_cb(u8_t arg0, u8_t arg1);
    void op_or_r_i_cb(u8_t arg0, u8_t arg1);
    void op_or_r_q_cb(u8_t arg0, u8_t arg1);
    void op_xor_r_i_cb(u8_t arg0, u8_t arg1);
    void op_xor_r_q_cb(u8_t arg0, u8_t arg1);

    void op_cp_r_i_cb(u8_t arg0, u8_t arg1);
    void op_cp_r_q_cb(u8_t arg0, u8_t arg1);

    void op_fan_r_i_cb(u8_t arg0, u8_t arg1);
    void op_fan_r_q_cb(u8_t arg0, u8_t arg1);

    void op_rlc_cb(u8_t arg0, u8_t arg1);
    void op_rrc_cb(u8_t arg0, u8_t arg1);

    void op_inc_mn_cb(u8_t arg0, u8_t arg1);
    void op_dec_mn_cb(u8_t arg0, u8_t arg1);

    void op_acpx_cb(u8_t arg0, u8_t arg1);
    void op_acpy_cb(u8_t arg0, u8_t arg1);
    void op_scpx_cb(u8_t arg0, u8_t arg1);
    void op_scpy_cb(u8_t arg0, u8_t arg1);
    void op_not_cb(u8_t arg0, u8_t arg1);

  private:
    const op_t ops[109] = {
        {(char *)"PSET #0x%02X            ", 0xE40, MASK_7B, 0, 0, 5, &CPU::op_pset_cb},
        {(char *)"JP   #0x%02X            ", 0x000, MASK_4B, 0, 0, 5, &CPU::op_jp_cb},
        {(char *)"JP   C #0x%02X          ", 0x200, MASK_4B, 0, 0, 5, &CPU::op_jp_c_cb},
        {(char *)"JP   NC #0x%02X         ", 0x300, MASK_4B, 0, 0, 5, &CPU::op_jp_nc_cb},
        {(char *)"JP   Z #0x%02X          ", 0x600, MASK_4B, 0, 0, 5, &CPU::op_jp_z_cb},
        {(char *)"JP   NZ #0x%02X         ", 0x700, MASK_4B, 0, 0, 5, &CPU::op_jp_nz_cb},
        {(char *)"JPBA                  ", 0xFE8, MASK_12B, 0, 0, 5, &CPU::op_jpba_cb},
        {(char *)"CALL #0x%02X            ", 0x400, MASK_4B, 0, 0, 7, &CPU::op_call_cb},
        {(char *)"CALZ #0x%02X            ", 0x500, MASK_4B, 0, 0, 7, &CPU::op_calz_cb},
        {(char *)"RET                   ", 0xFDF, MASK_12B, 0, 0, 7, &CPU::op_ret_cb},
        {(char *)"RETS                  ", 0xFDE, MASK_12B, 0, 0, 12, &CPU::op_rets_cb},
        {(char *)"RETD #0x%02X            ", 0x100, MASK_4B, 0, 0, 12, &CPU::op_retd_cb},
        {(char *)"NOP5                  ", 0xFFB, MASK_12B, 0, 0, 5, &CPU::op_nop5_cb},
        {(char *)"NOP7                  ", 0xFFF, MASK_12B, 0, 0, 7, &CPU::op_nop7_cb},
        {(char *)"HALT                  ", 0xFF8, MASK_12B, 0, 0, 5, &CPU::op_halt_cb},
        {(char *)"INC  X #0x%02X          ", 0xEE0, MASK_12B, 0, 0, 5, &CPU::op_inc_x_cb},
        {(char *)"INC  Y #0x%02X          ", 0xEF0, MASK_12B, 0, 0, 5, &CPU::op_inc_y_cb},
        {(char *)"LD   X #0x%02X          ", 0xB00, MASK_4B, 0, 0, 5, &CPU::op_ld_x_cb},
        {(char *)"LD   Y #0x%02X          ", 0x800, MASK_4B, 0, 0, 5, &CPU::op_ld_y_cb},
        {(char *)"LD   XP R(#0x%02X)      ", 0xE80, MASK_10B, 0, 0, 5, &CPU::op_ld_xp_r_cb},
        {(char *)"LD   XH R(#0x%02X)      ", 0xE84, MASK_10B, 0, 0, 5, &CPU::op_ld_xh_r_cb},
        {(char *)"LD   XL R(#0x%02X)      ", 0xE88, MASK_10B, 0, 0, 5, &CPU::op_ld_xl_r_cb},
        {(char *)"LD   YP R(#0x%02X)      ", 0xE90, MASK_10B, 0, 0, 5, &CPU::op_ld_yp_r_cb},
        {(char *)"LD   YH R(#0x%02X)      ", 0xE94, MASK_10B, 0, 0, 5, &CPU::op_ld_yh_r_cb},
        {(char *)"LD   YL R(#0x%02X)      ", 0xE98, MASK_10B, 0, 0, 5, &CPU::op_ld_yl_r_cb},
        {(char *)"LD   R(#0x%02X) XP      ", 0xEA0, MASK_10B, 0, 0, 5, &CPU::op_ld_r_xp_cb},
        {(char *)"LD   R(#0x%02X) XH      ", 0xEA4, MASK_10B, 0, 0, 5, &CPU::op_ld_r_xh_cb},
        {(char *)"LD   R(#0x%02X) XL      ", 0xEA8, MASK_10B, 0, 0, 5, &CPU::op_ld_r_xl_cb},
        {(char *)"LD   R(#0x%02X) YP      ", 0xEB0, MASK_10B, 0, 0, 5, &CPU::op_ld_r_yp_cb},
        {(char *)"LD   R(#0x%02X) YH      ", 0xEB4, MASK_10B, 0, 0, 5, &CPU::op_ld_r_yh_cb},
        {(char *)"LD   R(#0x%02X) YL      ", 0xEB8, MASK_10B, 0, 0, 5, &CPU::op_ld_r_yl_cb},
        {(char *)"ADC  XH #0x%02X         ", 0xA00, MASK_8B, 0, 0, 7, &CPU::op_adc_xh_cb},
        {(char *)"ADC  XL #0x%02X         ", 0xA10, MASK_8B, 0, 0, 7, &CPU::op_adc_xl_cb},
        {(char *)"ADC  YH #0x%02X         ", 0xA20, MASK_8B, 0, 0, 7, &CPU::op_adc_yh_cb},
        {(char *)"ADC  YL #0x%02X         ", 0xA30, MASK_8B, 0, 0, 7, &CPU::op_adc_yl_cb},
        {(char *)"CP   XH #0x%02X         ", 0xA40, MASK_8B, 0, 0, 7, &CPU::op_cp_xh_cb},
        {(char *)"CP   XL #0x%02X         ", 0xA50, MASK_8B, 0, 0, 7, &CPU::op_cp_xl_cb},
        {(char *)"CP   YH #0x%02X         ", 0xA60, MASK_8B, 0, 0, 7, &CPU::op_cp_yh_cb},
        {(char *)"CP   YL #0x%02X         ", 0xA70, MASK_8B, 0, 0, 7, &CPU::op_cp_yl_cb},
        {(char *)"LD   R(#0x%02X) #0x%02X   ", 0xE00, MASK_6B, 4, 0x030, 5, &CPU::op_ld_r_i_cb},
        {(char *)"LD   R(#0x%02X) Q(#0x%02X)", 0xEC0, MASK_8B, 2, 0x00C, 5, &CPU::op_ld_r_q_cb},
        {(char *)"LD   A M(#0x%02X)       ", 0xFA0, MASK_8B, 0, 0, 5, &CPU::op_ld_a_mn_cb},
        {(char *)"LD   B M(#0x%02X)       ", 0xFB0, MASK_8B, 0, 0, 5, &CPU::op_ld_b_mn_cb},
        {(char *)"LD   M(#0x%02X) A       ", 0xF80, MASK_8B, 0, 0, 5, &CPU::op_ld_mn_a_cb},
        {(char *)"LD   M(#0x%02X) B       ", 0xF90, MASK_8B, 0, 0, 5, &CPU::op_ld_mn_b_cb},
        {(char *)"LDPX MX #0x%02X         ", 0xE60, MASK_8B, 0, 0, 5, &CPU::op_ldpx_mx_cb},
        {(char *)"LDPX R(#0x%02X) Q(#0x%02X)", 0xEE0, MASK_8B, 2, 0x00C, 5, &CPU::op_ldpx_r_cb},
        {(char *)"LDPY MY #0x%02X         ", 0xE70, MASK_8B, 0, 0, 5, &CPU::op_ldpy_my_cb},
        {(char *)"LDPY R(#0x%02X) Q(#0x%02X)", 0xEF0, MASK_8B, 2, 0x00C, 5, &CPU::op_ldpy_r_cb},
        {(char *)"LBPX #0x%02X            ", 0x900, MASK_4B, 0, 0, 5, &CPU::op_lbpx_cb},
        {(char *)"SET  #0x%02X            ", 0xF40, MASK_8B, 0, 0, 7, &CPU::op_set_cb},
        {(char *)"RST  #0x%02X            ", 0xF50, MASK_8B, 0, 0, 7, &CPU::op_rst_cb},
        {(char *)"SCF                   ", 0xF41, MASK_12B, 0, 0, 7, &CPU::op_scf_cb},
        {(char *)"RCF                   ", 0xF5E, MASK_12B, 0, 0, 7, &CPU::op_rcf_cb},
        {(char *)"SZF                   ", 0xF42, MASK_12B, 0, 0, 7, &CPU::op_szf_cb},
        {(char *)"RZF                   ", 0xF5D, MASK_12B, 0, 0, 7, &CPU::op_rzf_cb},
        {(char *)"SDF                   ", 0xF44, MASK_12B, 0, 0, 7, &CPU::op_sdf_cb},
        {(char *)"RDF                   ", 0xF5B, MASK_12B, 0, 0, 7, &CPU::op_rdf_cb},
        {(char *)"EI                    ", 0xF48, MASK_12B, 0, 0, 7, &CPU::op_ei_cb},
        {(char *)"DI                    ", 0xF57, MASK_12B, 0, 0, 7, &CPU::op_di_cb},
        {(char *)"INC  SP               ", 0xFDB, MASK_12B, 0, 0, 5, &CPU::op_inc_sp_cb},
        {(char *)"DEC  SP               ", 0xFCB, MASK_12B, 0, 0, 5, &CPU::op_dec_sp_cb},
        {(char *)"PUSH R(#0x%02X)         ", 0xFC0, MASK_10B, 0, 0, 5, &CPU::op_push_r_cb},
        {(char *)"PUSH XP               ", 0xFC4, MASK_12B, 0, 0, 5, &CPU::op_push_xp_cb},
        {(char *)"PUSH XH               ", 0xFC5, MASK_12B, 0, 0, 5, &CPU::op_push_xh_cb},
        {(char *)"PUSH XL               ", 0xFC6, MASK_12B, 0, 0, 5, &CPU::op_push_xl_cb},
        {(char *)"PUSH YP               ", 0xFC7, MASK_12B, 0, 0, 5, &CPU::op_push_yp_cb},
        {(char *)"PUSH YH               ", 0xFC8, MASK_12B, 0, 0, 5, &CPU::op_push_yh_cb},
        {(char *)"PUSH YL               ", 0xFC9, MASK_12B, 0, 0, 5, &CPU::op_push_yl_cb},
        {(char *)"PUSH F                ", 0xFCA, MASK_12B, 0, 0, 5, &CPU::op_push_f_cb},
        {(char *)"POP  R(#0x%02X)         ", 0xFD0, MASK_10B, 0, 0, 5, &CPU::op_pop_r_cb},
        {(char *)"POP  XP               ", 0xFD4, MASK_12B, 0, 0, 5, &CPU::op_pop_xp_cb},
        {(char *)"POP  XH               ", 0xFD5, MASK_12B, 0, 0, 5, &CPU::op_pop_xh_cb},
        {(char *)"POP  XL               ", 0xFD6, MASK_12B, 0, 0, 5, &CPU::op_pop_xl_cb},
        {(char *)"POP  YP               ", 0xFD7, MASK_12B, 0, 0, 5, &CPU::op_pop_yp_cb},
        {(char *)"POP  YH               ", 0xFD8, MASK_12B, 0, 0, 5, &CPU::op_pop_yh_cb},
        {(char *)"POP  YL               ", 0xFD9, MASK_12B, 0, 0, 5, &CPU::op_pop_yl_cb},
        {(char *)"POP  F                ", 0xFDA, MASK_12B, 0, 0, 5, &CPU::op_pop_f_cb},
        {(char *)"LD   SPH R(#0x%02X)     ", 0xFE0, MASK_10B, 0, 0, 5, &CPU::op_ld_sph_r_cb},
        {(char *)"LD   SPL R(#0x%02X)     ", 0xFF0, MASK_10B, 0, 0, 5, &CPU::op_ld_spl_r_cb},
        {(char *)"LD   R(#0x%02X) SPH     ", 0xFE4, MASK_10B, 0, 0, 5, &CPU::op_ld_r_sph_cb},
        {(char *)"LD   R(#0x%02X) SPL     ", 0xFF4, MASK_10B, 0, 0, 5, &CPU::op_ld_r_spl_cb},
        {(char *)"ADD  R(#0x%02X) #0x%02X   ", 0xC00, MASK_6B, 4, 0x030, 7, &CPU::op_add_r_i_cb},
        {(char *)"ADD  R(#0x%02X) Q(#0x%02X)", 0xA80, MASK_8B, 2, 0x00C, 7, &CPU::op_add_r_q_cb},
        {(char *)"ADC  R(#0x%02X) #0x%02X   ", 0xC40, MASK_6B, 4, 0x030, 7, &CPU::op_adc_r_i_cb},
        {(char *)"ADC  R(#0x%02X) Q(#0x%02X)", 0xA90, MASK_8B, 2, 0x00C, 7, &CPU::op_adc_r_q_cb},
        {(char *)"SUB  R(#0x%02X) Q(#0x%02X)", 0xAA0, MASK_8B, 2, 0x00C, 7, &CPU::op_sub_cb},
        {(char *)"SBC  R(#0x%02X) #0x%02X   ", 0xB40, MASK_6B, 4, 0x030, 7, &CPU::op_sbc_r_i_cb},
        {(char *)"SBC  R(#0x%02X) Q(#0x%02X)", 0xAB0, MASK_8B, 2, 0x00C, 7, &CPU::op_sbc_r_q_cb},
        {(char *)"AND  R(#0x%02X) #0x%02X   ", 0xC80, MASK_6B, 4, 0x030, 7, &CPU::op_and_r_i_cb},
        {(char *)"AND  R(#0x%02X) Q(#0x%02X)", 0xAC0, MASK_8B, 2, 0x00C, 7, &CPU::op_and_r_q_cb},
        {(char *)"OR   R(#0x%02X) #0x%02X   ", 0xCC0, MASK_6B, 4, 0x030, 7, &CPU::op_or_r_i_cb},
        {(char *)"OR   R(#0x%02X) Q(#0x%02X)", 0xAD0, MASK_8B, 2, 0x00C, 7, &CPU::op_or_r_q_cb},
        {(char *)"XOR  R(#0x%02X) #0x%02X   ", 0xD00, MASK_6B, 4, 0x030, 7, &CPU::op_xor_r_i_cb},
        {(char *)"XOR  R(#0x%02X) Q(#0x%02X)", 0xAE0, MASK_8B, 2, 0x00C, 7, &CPU::op_xor_r_q_cb},
        {(char *)"CP   R(#0x%02X) #0x%02X   ", 0xDC0, MASK_6B, 4, 0x030, 7, &CPU::op_cp_r_i_cb},
        {(char *)"CP   R(#0x%02X) Q(#0x%02X)", 0xF00, MASK_8B, 2, 0x00C, 7, &CPU::op_cp_r_q_cb},
        {(char *)"FAN  R(#0x%02X) #0x%02X   ", 0xD80, MASK_6B, 4, 0x030, 7, &CPU::op_fan_r_i_cb},
        {(char *)"FAN  R(#0x%02X) Q(#0x%02X)", 0xF10, MASK_8B, 2, 0x00C, 7, &CPU::op_fan_r_q_cb},
        {(char *)"RLC  R(#0x%02X)         ", 0xAF0, MASK_8B, 0, 0, 7, &CPU::op_rlc_cb},
        {(char *)"RRC  R(#0x%02X)         ", 0xE8C, MASK_10B, 0, 0, 5, &CPU::op_rrc_cb},
        {(char *)"INC  M(#0x%02X)         ", 0xF60, MASK_8B, 0, 0, 7, &CPU::op_inc_mn_cb},
        {(char *)"DEC  M(#0x%02X)         ", 0xF70, MASK_8B, 0, 0, 7, &CPU::op_dec_mn_cb},
        {(char *)"ACPX R(#0x%02X)         ", 0xF28, MASK_10B, 0, 0, 7, &CPU::op_acpx_cb},
        {(char *)"ACPY R(#0x%02X)         ", 0xF2C, MASK_10B, 0, 0, 7, &CPU::op_acpy_cb},
        {(char *)"SCPX R(#0x%02X)         ", 0xF38, MASK_10B, 0, 0, 7, &CPU::op_scpx_cb},
        {(char *)"SCPY R(#0x%02X)         ", 0xF3C, MASK_10B, 0, 0, 7, &CPU::op_scpy_cb},
        {(char *)"NOT  R(#0x%02X)         ", 0xD0F, 0xFCF, 4, 0, 7, &CPU::op_not_cb},
        {0, 0, 0, 0, 0, 0, 0},
    };
};
#endif

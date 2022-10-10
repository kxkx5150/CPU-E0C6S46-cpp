#define MEM_BUFFER_SIZE    (MEM_RAM_SIZE + MEM_DISPLAY1_SIZE + MEM_DISPLAY2_SIZE + MEM_IO_SIZE) / 2
#define RAM_TO_MEMORY(n)   ((n - MEM_RAM_ADDR) / 2)
#define DISP1_TO_MEMORY(n) ((n - MEM_DISPLAY1_ADDR + MEM_RAM_SIZE) / 2)
#define DISP2_TO_MEMORY(n) ((n - MEM_DISPLAY2_ADDR + MEM_RAM_SIZE + MEM_DISPLAY1_SIZE) / 2)
#define IO_TO_MEMORY(n)    ((n - MEM_IO_ADDR + MEM_RAM_SIZE + MEM_DISPLAY1_SIZE + MEM_DISPLAY2_SIZE) / 2)

#define MASK_4B  0xF00
#define MASK_6B  0xFC0
#define MASK_7B  0xFE0
#define MASK_8B  0xFF0
#define MASK_10B 0xFFC
#define MASK_12B 0xFFF

#define SET_RAM_MEMORY(buffer, n, v)                                                                                   \
    {                                                                                                                  \
        buffer[RAM_TO_MEMORY(n)] = (buffer[RAM_TO_MEMORY(n)] & ~(0xF << (((n) % 2) << 2))) | ((v)&0xF)                 \
                                                                                                 << (((n) % 2) << 2);  \
    }
#define SET_DISP1_MEMORY(buffer, n, v)                                                                                 \
    {                                                                                                                  \
        buffer[DISP1_TO_MEMORY(n)] =                                                                                   \
            (buffer[DISP1_TO_MEMORY(n)] & ~(0xF << (((n) % 2) << 2))) | ((v)&0xF) << (((n) % 2) << 2);                 \
    }
#define SET_DISP2_MEMORY(buffer, n, v)                                                                                 \
    {                                                                                                                  \
        buffer[DISP2_TO_MEMORY(n)] =                                                                                   \
            (buffer[DISP2_TO_MEMORY(n)] & ~(0xF << (((n) % 2) << 2))) | ((v)&0xF) << (((n) % 2) << 2);                 \
    }
#define SET_IO_MEMORY(buffer, n, v)                                                                                    \
    {                                                                                                                  \
        buffer[IO_TO_MEMORY(n)] = (buffer[IO_TO_MEMORY(n)] & ~(0xF << (((n) % 2) << 2))) | ((v)&0xF)                   \
                                                                                               << (((n) % 2) << 2);    \
    }
#define SET_MEMORY(buffer, n, v)                                                                                       \
    {                                                                                                                  \
        if ((n) < (MEM_RAM_ADDR + MEM_RAM_SIZE)) {                                                                     \
            SET_RAM_MEMORY(buffer, n, v);                                                                              \
        } else if ((n) < MEM_DISPLAY1_ADDR) {                                                                          \
        } else if ((n) < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) {                                                    \
            SET_DISP1_MEMORY(buffer, n, v);                                                                            \
        } else if ((n) < MEM_DISPLAY2_ADDR) {                                                                          \
        } else if ((n) < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) {                                                    \
            SET_DISP2_MEMORY(buffer, n, v);                                                                            \
        } else if ((n) < MEM_IO_ADDR) {                                                                                \
        } else if ((n) < (MEM_IO_ADDR + MEM_IO_SIZE)) {                                                                \
            SET_IO_MEMORY(buffer, n, v);                                                                               \
        } else {                                                                                                       \
        }                                                                                                              \
    }

#define GET_RAM_MEMORY(buffer, n)   ((buffer[RAM_TO_MEMORY(n)] >> (((n) % 2) << 2)) & 0xF)
#define GET_DISP1_MEMORY(buffer, n) ((buffer[DISP1_TO_MEMORY(n)] >> (((n) % 2) << 2)) & 0xF)
#define GET_DISP2_MEMORY(buffer, n) ((buffer[DISP2_TO_MEMORY(n)] >> (((n) % 2) << 2)) & 0xF)
#define GET_IO_MEMORY(buffer, n)    ((buffer[IO_TO_MEMORY(n)] >> (((n) % 2) << 2)) & 0xF)
#define GET_MEMORY(buffer, n)                                                                                          \
    ((buffer[((n) < (MEM_RAM_ADDR + MEM_RAM_SIZE))             ? RAM_TO_MEMORY(n)                                      \
             : ((n) < MEM_DISPLAY1_ADDR)                       ? 0                                                     \
             : ((n) < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) ? DISP1_TO_MEMORY(n)                                    \
             : ((n) < MEM_DISPLAY2_ADDR)                       ? 0                                                     \
             : ((n) < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) ? DISP2_TO_MEMORY(n)                                    \
             : ((n) < MEM_IO_ADDR)                             ? 0                                                     \
             : ((n) < (MEM_IO_ADDR + MEM_IO_SIZE))             ? IO_TO_MEMORY(n)                                       \
                                                               : 0] >>                                                             \
      (((n) % 2) << 2)) &                                                                                              \
     0xF)

#define MEM_RAM_ADDR      0x000
#define MEM_RAM_SIZE      0x280
#define MEM_DISPLAY1_ADDR 0xE00
#define MEM_DISPLAY1_SIZE 0x050
#define MEM_DISPLAY2_ADDR 0xE80
#define MEM_DISPLAY2_SIZE 0x050
#define MEM_IO_ADDR       0xF00
#define MEM_IO_SIZE       0x080

#define TICK_FREQUENCY     32768
#define TIMER_1HZ_PERIOD   32768
#define TIMER_256HZ_PERIOD 128

#define REG_CLK_INT_FACTOR_FLAGS     0xF00
#define REG_SW_INT_FACTOR_FLAGS      0xF01
#define REG_PROG_INT_FACTOR_FLAGS    0xF02
#define REG_SERIAL_INT_FACTOR_FLAGS  0xF03
#define REG_K00_K03_INT_FACTOR_FLAGS 0xF04
#define REG_K10_K13_INT_FACTOR_FLAGS 0xF05
#define REG_CLOCK_INT_MASKS          0xF10
#define REG_SW_INT_MASKS             0xF11
#define REG_PROG_INT_MASKS           0xF12
#define REG_SERIAL_INT_MASKS         0xF13
#define REG_K00_K03_INT_MASKS        0xF14
#define REG_K10_K13_INT_MASKS        0xF15
#define REG_PROG_TIMER_DATA_L        0xF24
#define REG_PROG_TIMER_DATA_H        0xF25
#define REG_PROG_TIMER_RELOAD_DATA_L 0xF26
#define REG_PROG_TIMER_RELOAD_DATA_H 0xF27
#define REG_K00_K03_INPUT_PORT       0xF40
#define REG_K10_K13_INPUT_PORT       0xF42
#define REG_K40_K43_BZ_OUTPUT_PORT   0xF54
#define REG_CPU_OSC3_CTRL            0xF70
#define REG_LCD_CTRL                 0xF71
#define REG_LCD_CONTRAST             0xF72
#define REG_SVD_CTRL                 0xF73
#define REG_BUZZER_CTRL1             0xF74
#define REG_BUZZER_CTRL2             0xF75
#define REG_CLK_WD_TIMER_CTRL        0xF76
#define REG_SW_TIMER_CTRL            0xF77
#define REG_PROG_TIMER_CTRL          0xF78
#define REG_PROG_TIMER_CLK_SEL       0xF79

#define PCS  (pc & 0xFF)
#define PCSL (pc & 0xF)
#define PCSH ((pc >> 4) & 0xF)
#define PCP  ((pc >> 8) & 0xF)
#define PCB  ((pc >> 12) & 0x1)

#define TO_PC(bank, page, step) ((step & 0xFF) | ((page & 0xF) << 8) | (bank & 0x1) << 12)
#define TO_NP(bank, page)       ((page & 0xF) | (bank & 0x1) << 4)

#define NBP          ((np >> 4) & 0x1)
#define NPP          (np & 0xF)
#define XHL          (x & 0xFF)
#define XL           (x & 0xF)
#define XH           ((x >> 4) & 0xF)
#define XP           ((x >> 8) & 0xF)
#define YHL          (y & 0xFF)
#define YL           (y & 0xF)
#define YH           ((y >> 4) & 0xF)
#define YP           ((y >> 8) & 0xF)
#define M(n)         get_memory(n)
#define SET_M(n, v)  set_memory(n, v)
#define RQ(i)        get_rq(i)
#define SET_RQ(i, v) set_rq(i, v)
#define SPL          (sp & 0xF)
#define SPH          ((sp >> 4) & 0xF)
#define FLAG_C       (0x1 << 0)
#define FLAG_Z       (0x1 << 1)
#define FLAG_D       (0x1 << 2)
#define FLAG_I       (0x1 << 3)
#define C            !!(flags & FLAG_C)
#define Z            !!(flags & FLAG_Z)
#define D            !!(flags & FLAG_D)
#define I            !!(flags & FLAG_I)

#define SET_C()                                                                                                        \
    {                                                                                                                  \
        flags |= FLAG_C;                                                                                               \
    }
#define CLEAR_C()                                                                                                      \
    {                                                                                                                  \
        flags &= ~FLAG_C;                                                                                              \
    }
#define SET_Z()                                                                                                        \
    {                                                                                                                  \
        flags |= FLAG_Z;                                                                                               \
    }
#define CLEAR_Z()                                                                                                      \
    {                                                                                                                  \
        flags &= ~FLAG_Z;                                                                                              \
    }
#define SET_D()                                                                                                        \
    {                                                                                                                  \
        flags |= FLAG_D;                                                                                               \
    }
#define CLEAR_D()                                                                                                      \
    {                                                                                                                  \
        flags &= ~FLAG_D;                                                                                              \
    }
#define SET_I()                                                                                                        \
    {                                                                                                                  \
        flags |= FLAG_I;                                                                                               \
    }
#define CLEAR_I()                                                                                                      \
    {                                                                                                                  \
        flags &= ~FLAG_I;                                                                                              \
    }

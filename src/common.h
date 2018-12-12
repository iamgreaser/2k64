#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <time.h>

#include <SDL.h>
#include <signal.h>

#define RAM_TO_FILE 1

#if RAM_TO_FILE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
extern int ram_fd;
#endif

struct vr4300;
struct rsp;

extern uint32_t dpc_start;
extern uint32_t dpc_end;
extern uint32_t dpc_end_saved;
extern uint32_t dpc_current;
extern uint32_t dpc_status;

uint32_t fullrandu32(void);
uint64_t fullrandu64(void);
float n64_roundf(float v);
double n64_round(double v);

void n64_update_interrupts(void);
void n64_set_interrupt(int flag);
void n64_clear_interrupt(int flag);

enum mipserr n64primary_mem_read(struct vr4300 *C, uint64_t addr, uint32_t mask, uint32_t *data);
void n64primary_mem_write(struct vr4300 *C, uint64_t addr, uint32_t mask, uint32_t data);
enum mipserr n64rsp_mem_read(struct rsp *rsp, uint64_t addr, uint32_t mask, uint32_t *data);
void n64rsp_mem_write(struct rsp *rsp, uint64_t addr, uint32_t mask, uint32_t data);

#if 0
#define DEBUG_AI 1
#define DEBUG_DP 1
#define DEBUG_MI 1
#define DEBUG_PI 1
#define DEBUG_RI 1
#define DEBUG_SI 1
#define DEBUG_SP 1
#define DEBUG_VI 1
#define DEBUG_RDREG 1
#endif
#define DEBUG_SP 1

#if 1
#define rsp_debug_printf printf
#else
#define rsp_debug_printf(...)
#endif

#if 1
#define rdp_debug_printf printf
#else
#define rdp_debug_printf(...)
#endif

#define RAM_SIZE_WORDS (8*1024*256)
#define RAM_SIZE_BYTES (4*RAM_SIZE_WORDS)

#if RAM_TO_FILE
extern uint32_t *ram;
extern uint32_t *rsp_mem;
extern uint32_t *pifmem;
#else
extern uint32_t ram[RAM_SIZE_WORDS];
extern uint32_t rsp_mem[8*256];
extern uint32_t pifmem[2*256];
#endif

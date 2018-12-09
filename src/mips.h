#include "mips-common.h"

#include "mips-defs.h"

struct MIPSNAME
{
	// main stuff
	SREG regs[32]; // regs[0] must be 0 at all times!
	SREG pc;
	UREG rlo, rhi;

	// cop0 regs
	union {
		UREG i[32];
		struct {
#ifdef MIPS_IS_RSP
			UREG dma_cache; // 0
			UREG dma_dram; // 1
			UREG dma_read_length; // 2
			UREG dma_write_length; // 3

			UREG sp_status; // 4
			UREG _05; // 5
			UREG _06; // 6
			UREG sp_reserved; // 7

			UREG _08; // 8
			UREG _09; // 9
			UREG _10; // 10
			UREG cmd_status; // 11

			UREG _12; // 12
			UREG _13; // 13
			UREG _14; // 14
			UREG _15; // 15

			UREG _16; // 16
			UREG _17; // 17
			UREG _18; // 18
			UREG _19; // 19

			UREG _20; // 20
			UREG _21; // 21
			UREG _22; // 22
			UREG _23; // 23

			UREG _24; // 24
			UREG _25; // 25
			UREG _26; // 26
			UREG _27; // 27

			UREG _28; // 28
			UREG _29; // 29
			UREG _30; // 30
			UREG _31; // 31

#else
			UREG index; // 0
			UREG random; // 1
			UREG entrylo0; // 2
			UREG entrylo1; // 3

			UREG context; // 4
			UREG pagemask; // 5
			UREG wired; // 6
			UREG _07; // 7

			UREG badvaddr; // 8
			UREG count; // 9
			UREG entryhi; // 10
			UREG compare; // 11

			UREG sr; // 12
			UREG cause; // 13
			UREG epc; // 14
			UREG prid; // 15

			UREG config; // 16
			UREG lladdr; // 17
			UREG watchlo; // 18
			UREG watchhi; // 19

			UREG xcontext; // 20
			UREG _21; // 21
			UREG _22; // 22
			UREG _23; // 23

			UREG _24; // 24
			UREG _25; // 25
			UREG perr; // 26
			UREG cacheerr; // 27

			UREG taglo; // 28
			UREG taghi; // 29
			UREG errorepc; // 30
			UREG _31; // 31
#endif
		} n;
	} c0;

#ifndef MIPS_IS_RSP
	bool llbit;
#endif

#ifdef MIPS_HAS_FPU
	// FPU regs
	union {
		uint64_t di[32];
		float sf[32][2];
		double df[32];
	} c1;

	bool coc1;
#endif

#ifdef MIPS_IS_RSP
	// cop2 regs
	union {
		uint8_t b[32][16];
		uint16_t h[32][8];
		uint32_t w[32][4];
		uint64_t d[32][2];
	} c2;

	// cop2 accumulator
	uint16_t c2acc[3][8];

	uint32_t c2divin;
	uint32_t c2divout;

	// cop2 control regs
	union {
		uint32_t i[32];
		struct {
			uint32_t vco;
			uint32_t vcc;
			uint32_t vce;
		} n;
	} cc2;

	// RSP special
	bool rsp_intr;
#endif

	// timing
	int64_t tickrem;
	uint64_t ticktime;

	// pipelining
	uint32_t pl0_op;
	SREG pl0_pc;
	bool pl0_is_branch;

	// TLB
#ifdef MIPS_MMU_ENTRIES
	struct MIPSXNAME(_tlbent) {
		UREG entrylo[2];
		UREG entryhi;
		UREG pagemask;
	} tlb[MIPS_MMU_ENTRIES];
#endif

	// hooks
	//enum mipserr (*f_mem_read)(struct MIPSNAME *, uint64_t addr, uint32_t mask, uint32_t *data);
	//void (*f_mem_write)(struct MIPSNAME *, uint64_t addr, uint32_t mask, uint32_t data);
};

enum mipserr MIPSXNAME(_read32)(struct MIPSNAME *C, UREG addr, uint32_t *data);

#include "mips-core.h"

#include "mips-undefs.h"


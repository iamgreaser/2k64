
#ifdef MIPS_IS_VR4300
#define MIPS_VERSION 3
#define MIPS_MMU_ENTRIES 32
#endif
#ifdef MIPS_IS_RSP
#define MIPS_VERSION 1
#endif

#if MIPS_VERSION >= 3
#define UREG uint64_t
#define SREG int64_t
#define REGSIZE 64
#if 0
#define SIGNEX32R(C,ridx) { \
	(C)->regs[ridx] = (UREG)(SREG)(int32_t)(uint32_t)((C)->regs[ridx]); \
	printf("signex %016llX\n", (C)->regs[ridx]); \
}
#else
#define SIGNEX32R(C,ridx) {  \
	(C)->regs[ridx] = (UREG)(SREG)(int32_t)(uint32_t)((C)->regs[ridx]); \
}
#endif

#else
#define UREG uint32_t
#define SREG int32_t
#define REGSIZE 32
#define SIGNEX32R(C,ridx) {}
#endif

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
			UREG _11; // 11

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

#ifdef MIPS_IS_RSP
	// cop2 regs
	union {
		uint8_t b[32][16];
		uint16_t h[32][8];
		uint32_t w[32][4];
		uint64_t d[32][2];
	} c2;

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
	enum mipserr (*f_mem_read)(struct MIPSNAME *, uint64_t addr, uint32_t mask, uint32_t *data);
	void (*f_mem_write)(struct MIPSNAME *, uint64_t addr, uint32_t mask, uint32_t data);
};

enum mipserr MIPSXNAME(_read32)(struct MIPSNAME *C, UREG addr, uint32_t *data);

void MIPSXNAME(_cpu_reset)(struct MIPSNAME *C)
{
	// set PC
#ifdef MIPS_IS_RSP
	C->pc = (SREG)(int32_t)0x00000000;
#else
	C->pc = (SREG)(int32_t)0xBFC00000;
#endif

	// flush pipeline
	C->pl0_op = 0x00000000;
	C->pl0_pc = C->pc;
	C->pl0_is_branch = false;

#ifdef MIPS_IS_RSP

#else
	// set bits in c0_sr
	C->c0.n.sr &= ~(C0SR_TS | C0SR_SR | C0SR_RP);
	C->c0.n.sr |=  (C0SR_ERL | C0SR_BEV);

	// set bits in c0_config
	C->c0.n.config &= ~(C0CONFIG_EP_mask);
	C->c0.n.config |=  (C0CONFIG_BE);

	// set random
	C->c0.n.random = 31;

	// set divmode
	// XXX: we don't know what this is yet, so do something plausible
	C->c0.n.config |=  (0x0<<C0CONFIG_EC_shift); // 1:2 scale
#endif
}

void MIPSXNAME(_throw_exception)(struct MIPSNAME *C, UREG epc, enum mipserr cause, bool bd)
{
	assert(cause >= MER_Int && cause <= MER_Ov);

	printf("Throw exception %016llX, cause %d\n", (unsigned long long)epc, ((int)cause)-1);

	for(int i = 0; i < 32; i++) {
		printf("$%s = %016llX\n", mips_gpr_names[i], C->regs[i]);
	}

#if 0
	for(int i = 0x00200000; i < 0x00202000; i++) {
		uint32_t mdata = 0x55555555;
		if(i%16 == 0) {
			printf("%08X:", i);
		}
		int e = MIPSXNAME(_read8_unchecked)(C, i, &mdata);
		printf(" %02X", mdata);
		if((i+1)%16 == 0) {
			printf("\n");
		}
	}
#endif

	uint32_t opdata = 0xFFFFFFFF;
#ifdef MIPS_IS_RSP
	C->f_mem_read(C, (epc&0x0FFF)|0x1000, 0xFFFFFFFF, &opdata);
	printf("op refetch = %08X\n", opdata);

	// Halt RSP.
	C->c0.n.sp_status |= 0x00000001;
#else
	MIPSXNAME(_read32)(C, epc, &opdata);
	printf("op refetch = %08X\n", opdata);
	printf("badvaddr = %016llX\n", (unsigned long long)C->c0.n.badvaddr);
	//fflush(stdout); abort();

	// Set up PC + op
	C->c0.n.epc = epc;
	C->pl0_op = 0x00000000;
	C->pc = ((C->c0.n.sr & C0SR_BEV) != 0 ? 0xBFC00200 : 0x80000000);
	C->pc = (SREG)(int32_t)C->pc;
	if(cause == MER_TLBL || cause == MER_TLBS) {
		// TODO: XTLB
	} else {
		C->pc += 0x180;
	}

	// Set up cause fields
	C->c0.n.cause &= 0x3000FF00;
	C->c0.n.cause |= ((((uint32_t)(cause-1))&31)<<2);
	if(bd) {
		// Branch delay slots have special treatment
		C->c0.n.cause |= (1<<31);
		C->c0.n.epc -= 4;
	}

	// Disable interrupts (IE=0) + enter kernel mode (KUc=0)
	C->c0.n.sr = (C->c0.n.sr&~0x3F) | ((C->c0.n.sr<<2)&0x3C);
#endif
}

enum mipserr MIPSXNAME(_probe_interrupts)(struct MIPSNAME *C)
{
#ifdef MIPS_IS_RSP

#else
	if((C->c0.n.sr & C0SR_IE) != 0) {
		if((C->c0.n.sr & C->c0.n.cause & 0x0000FF00) != 0) {
			// Interrupt ready, you should fire an exception now
			return MER_Int;
		}
	}
#endif

	return MER_NONE;
}

void MIPSXNAME(_badvaddr_cond_set)(struct MIPSNAME *C, enum mipserr e, UREG addr)
{
	switch(e)
	{
		case MER_Mod:
		case MER_TLBL:
		case MER_TLBS:
		case MER_AdEL:
		case MER_AdES:
#ifndef MIPS_IS_RSP
			C->c0.n.badvaddr = addr;
#endif
			break;
		default:
			break;
	}
}

enum mipserr MIPSXNAME(_calc_addr)(struct MIPSNAME *C, UREG *addr, bool is_write)
{
#if REGSIZE > 32
	// TODO: 64-bit addressing mode
	// (should this be defined as two different CPUs?)
	//*addr |= 0xFFFFFFFF00000000LLU;
#endif

#ifdef MIPS_IS_RSP
	*addr &= 0x00000FFF;
	return MER_NONE;
#else
	// Usermode cannot access the upper region
	if(*addr >= 0x80000000LLU) {
		if((C->c0.n.sr & C0SR_KSU_mask) != C0SR_KSU_Kernel
				&& (C->c0.n.sr & (C0SR_EXL|C0SR_ERL)) == 0) {

			if((C->c0.n.sr & C0SR_KSU_mask) != C0SR_KSU_Supervisor
				|| (*addr & 0xE0000000) != 0xC0000000) {
				printf("YOU AIN'T THE KERNEL\n");
				return (is_write ? MER_AdES : MER_AdEL);
			} 
		}
	}

	if(*addr >= (UREG)(SREG)(int32_t)0x80000000LLU && *addr <= (UREG)(SREG)(int32_t)0xBFFFFFFFLLU) {
		enum mipserr ret = ((*addr&0x20000000)!=0
			? MER_UNCACHEABLE : MER_NONE);
		*addr &= 0x1FFFFFFF;
		return ret;
	} else {

#ifdef MIPS_MMU_ENTRIES
		// Handle TLB
		// TODO: not do this the slow way
		int good_tlb = -1;
		for(int i = 0; i < MIPS_MMU_ENTRIES; i++) {
			// Get TLB entry
			struct MIPSXNAME(_tlbent) *T = &C->tlb[i];

			// Page mask check
			//uint32_t pmask = ((T->pagemask<<1)+1);
			uint32_t pmask = T->pagemask;
			//printf("tlb %2d: %08X %08X %08X %08X\n", i, T->entryhi, T->pagemask, pmask, *addr);
			if((*addr & ~pmask) != (T->entryhi & ~pmask)) {
				//continue;
				break; // checking for TLB shutdowns is slow, fuck that
			}

			// TLB shutdown check
			if(good_tlb != -1) {
				// FIXME: kludge to force this to somehow work when TLB not initialised
				// we're preventing invalid TLBs from making a TLB shutdown happen
				uint32_t pmask = T->pagemask;
				uint32_t entidx = (*addr & pmask)>>12;
				uint32_t lo = T->entrylo[entidx&1];
				if((lo & 2) == 0) {
					continue;
				}

				printf("tlb %2d: %08X %08X %08X %08X\n", i, T->entryhi, T->pagemask, pmask, *addr);
				printf("TLB SHUTDOWN, FUCK THIS SHIT\n");
				fflush(stdout);
				abort();
			}

			// Use this index
			good_tlb = i;
			//break;
		}

		if(good_tlb != -1) {
			// Select subentry
			struct MIPSXNAME(_tlbent) *T = &C->tlb[good_tlb];
			uint32_t pmask = T->pagemask;
			uint32_t entidx = (*addr & pmask)>>12;

			// XXX: this could be SIMDed
			// (or of course we could just store shifts instead of masks)
			while(pmask >= 1) {
				entidx >>= 2;
				pmask >>= 2;
			}

			// Get entry
			uint32_t lo = T->entrylo[entidx&1];

			// ASID/global check
			if((lo & 1) != 0 || ((T->entryhi^C->c0.n.entryhi)&0xFF) == 0) {
				// Do translation
				uint32_t basepmask = T->pagemask;
				*addr = (*addr&basepmask) | (lo&~basepmask);
				return (((lo>>2)&0x7) == 0x2 ? MER_UNCACHEABLE : MER_NONE);
			}
		}

		return (is_write ? MER_TLBS : MER_TLBL);
#else
		// TODO: handle cache
		*addr &= 0x7FFFFFFF;
		return MER_UNCACHEABLE;
#endif
	}
#endif
}

enum mipserr MIPSXNAME(_read32_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t *data)
{
#ifdef MIPS_IS_RSP
	return C->f_mem_read(C, addr & 0x00000FFF, 0xFFFFFFFF, data);
#else
	return C->f_mem_read(C, addr, 0xFFFFFFFF, data);
#endif
}

enum mipserr MIPSXNAME(_read16_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t *data)
{
	uint32_t mask = 0xFFFF<<(((~addr)&2)<<3);
	enum mipserr e = C->f_mem_read(C, addr, mask, data);
	//enum mipserr e = C->f_mem_read(C, addr, 0xFFFFFFFF, data);
	if(e != MER_NONE) {
		return e;
	}
	*data >>= (((~addr)&2)<<3);
	*data &= 0xFFFF;
	return e;
}

enum mipserr MIPSXNAME(_read8_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t *data)
{
	uint32_t mask = 0xFF<<(((~addr)&3)<<3);
	enum mipserr e = C->f_mem_read(C, addr, mask, data);
	//enum mipserr e = C->f_mem_read(C, addr, 0xFFFFFFFF, data);
	if(e != MER_NONE) {
		return e;
	}
	*data >>= (((~addr)&3)<<3);
	*data &= 0xFF;
	return e;
}

enum mipserr MIPSXNAME(_write32_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t data)
{
	C->f_mem_write(C, addr, 0xFFFFFFFF, data);
	return MER_NONE;
}

enum mipserr MIPSXNAME(_write16_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t data)
{
	data &= 0xFFFF;
	data |= (data<<16);
	C->f_mem_write(C, addr, 0xFFFF<<(((~addr)&2)<<3), data);
	return MER_NONE;
}

enum mipserr MIPSXNAME(_write8_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t data)
{
	data &= 0xFF;
	data |= (data<<8);
	data |= (data<<16);
	C->f_mem_write(C, addr, 0xFF<<(((~addr)&3)<<3), data);
	return MER_NONE;
}

enum mipserr MIPSXNAME(_read32l)(struct MIPSNAME *C, UREG addr, uint32_t *data)
{
	// Get address
	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform read
	uint32_t shamt = ((addr)&3U)<<3U;
	uint32_t omask = 0xFFFFFFFFU>>shamt;
	uint32_t imask = 0xFFFFFFFFU<<shamt;
	uint32_t mdata = 0;
	e = C->f_mem_read(C, addr, imask, &mdata);
	//printf("R mask %08X %08X %d %08X\n", imask, omask, shamt, mdata);
	if(e == MER_NONE) {
		*data = (*data & ~omask) | ((mdata>>shamt) & omask);
	}
	MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
	return e;
}

enum mipserr MIPSXNAME(_read32r)(struct MIPSNAME *C, UREG addr, uint32_t *data)
{
	// Get address
	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform read
	uint32_t shamt = ((~addr)&3U)<<3U;
	uint32_t omask = 0xFFFFFFFFU>>shamt;
	uint32_t imask = 0xFFFFFFFFU<<shamt;
	uint32_t mdata = 0;
	e = C->f_mem_read(C, addr, imask, &mdata);
	//printf("L mask %08X %08X %d %08X\n", imask, omask, shamt, mdata);
	if(e == MER_NONE) {
		*data = (*data & ~omask) | ((mdata>>shamt) & omask);
	}
	MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
	return e;
}

enum mipserr MIPSXNAME(_read32)(struct MIPSNAME *C, UREG addr, uint32_t *data)
{
	// Get address
	if((addr&3) != 0) {
		MIPSXNAME(_badvaddr_cond_set)(C, MER_AdEL, addr);
		return MER_AdEL;
	}

	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform read
	e = MIPSXNAME(_read32_unchecked)(C, addr, data);
	MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
	return e;
}

enum mipserr MIPSXNAME(_read16)(struct MIPSNAME *C, UREG addr, uint32_t *data)
{
	// Get address
	if((addr&1) != 0) {
		MIPSXNAME(_badvaddr_cond_set)(C, MER_AdEL, addr);
		return MER_AdEL;
	}

	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform read
	e = MIPSXNAME(_read16_unchecked)(C, addr, data);
	MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
	return e;
}

enum mipserr MIPSXNAME(_read8)(struct MIPSNAME *C, UREG addr, uint32_t *data)
{
	// Get address
	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform read
	e = MIPSXNAME(_read8_unchecked)(C, addr, data);
	MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
	return e;
}

enum mipserr MIPSXNAME(_write32l)(struct MIPSNAME *C, UREG addr, uint32_t data)
{
	// Get address
	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform write
	uint32_t shamt = ((addr)&3U)<<3U;
	uint32_t omask = 0xFFFFFFFFU<<shamt;
	C->f_mem_write(C, addr, omask, data<<shamt);
	return MER_NONE;
}

enum mipserr MIPSXNAME(_write32r)(struct MIPSNAME *C, UREG addr, uint32_t data)
{
	// Get address
	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform write
	uint32_t shamt = ((~addr)&3U)<<3U;
	uint32_t omask = 0xFFFFFFFFU<<shamt;
	C->f_mem_write(C, addr, omask, data<<shamt);
	return MER_NONE;
}

enum mipserr MIPSXNAME(_write32)(struct MIPSNAME *C, UREG addr, uint32_t data)
{
	// Get address
	if((addr&3) != 0) {
		MIPSXNAME(_badvaddr_cond_set)(C, MER_AdES, addr);
		return MER_AdES;
	}

	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform write
	e = MIPSXNAME(_write32_unchecked)(C, addr, data);
	MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
	return e;
}

enum mipserr MIPSXNAME(_write16)(struct MIPSNAME *C, UREG addr, uint32_t data)
{
	// Get address
	if((addr&1) != 0) {
		MIPSXNAME(_badvaddr_cond_set)(C, MER_AdES, addr);
		return MER_AdES;
	}

	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform write
	e = MIPSXNAME(_write16_unchecked)(C, addr, data);
	MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
	return e;
}

enum mipserr MIPSXNAME(_write8)(struct MIPSNAME *C, UREG addr, uint32_t data)
{
	// Get address
	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform write
	e = MIPSXNAME(_write8_unchecked)(C, addr, data);
	MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
	return e;
}

enum mipserr MIPSXNAME(_fetch_op)(struct MIPSNAME *C, uint32_t *data)
{
	// Get address
	UREG addr = C->pc;
	if((addr&3) != 0) {
		MIPSXNAME(_badvaddr_cond_set)(C, MER_AdEL, addr);
		return MER_AdEL;
	}

#ifdef MIPS_IS_RSP
	// Special-case the RSP
	enum mipserr e = C->f_mem_read(C, (addr&0x0FFF)|0x1000, 0xFFFFFFFF, data);
	if(e != MER_NONE) {
		if(e == MER_DBE) {
			e = MER_IBE;
		}
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

#else
	enum mipserr e = MIPSXNAME(_calc_addr)(C, &addr, false);
	if(e != MER_NONE && e != MER_UNCACHEABLE) {
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}

	// Perform read
	e = MIPSXNAME(_read32_unchecked)(C, addr, data);
	if(e != MER_NONE) {
		if(e == MER_DBE) {
			e = MER_IBE;
		}
		MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
		return e;
	}
#endif

	// Advance
	C->pc += 4;
	return MER_NONE;
}

enum mipserr MIPSXNAME(_run_op)(struct MIPSNAME *C)
{
	uint32_t mdata;
	enum mipserr e;
	e = MER_NONE;

	assert(C->regs[0] == 0);

	C->pc = (SREG)(int32_t)C->pc;

	// Fetch op
	uint32_t op = C->pl0_op;
	SREG op_pc = C->pl0_pc;
	bool op_was_branch = C->pl0_is_branch;
	SREG new_pc = C->pc;
	uint32_t new_op = 0xFFFFFFFF; // shut the compiler up
	//enum mipserr e_ifetch = MIPSXNAME(_fetch_op)(C, &new_op);
	enum mipserr e_ifetch = MIPSXNAME(_fetch_op)(C, &(C->pl0_op));
	new_op = C->pl0_op;

	//printf("OPDATA: %016llX: %08X\n", op_pc, op);

	// Run op
	C->pl0_is_branch = false;

	//
	// OPCODE EXECUTION BEG
	//
	uint32_t rs = (op>>21U)&0x1FU;
	uint32_t rt = (op>>16U)&0x1FU;
	uint32_t rd = (op>>11U)&0x1FU;
	uint32_t shamt = (op>>6U)&0x1FU;

	switch(op>>26U) {
		// SPECIAL
		case 0: switch(op&0x3F) {
			// S(LL|RL|RA)
			case 0: // SLL
				if(rd != 0) {
					C->regs[rd] = C->regs[rt] << shamt;
					SIGNEX32R(C, rd);
				} break;
			case 2: // SRL
				if(rd != 0) {
					C->regs[rd] = (uint32_t)C->regs[rt] >> (uint32_t)shamt;
					SIGNEX32R(C, rd);
				} break;
			case 3: // SRA
				if(rd != 0) {
					C->regs[rd] = ((int32_t)C->regs[rt]) >> (int32_t)shamt;
					SIGNEX32R(C, rd);
				} break;

			// S(LL|RL|RA)V
			case 4: // SLLV
				if(rd != 0) {
					C->regs[rd] = C->regs[rt] << (C->regs[rs]&0x1F);
					SIGNEX32R(C, rd);
				} break;
			case 6: // SRLV
				if(rd != 0) {
					C->regs[rd] = ((uint32_t)C->regs[rt]) >> (uint32_t)(C->regs[rs]&0x1F);
					SIGNEX32R(C, rd);
				} break;
			case 7: // SRAV
				if(rd != 0) {
					C->regs[rd] = ((int32_t)C->regs[rt]) >> (int32_t)(C->regs[rs]&0x1F);
					SIGNEX32R(C, rd);
				} break;

			// J(AL)?R
			case 9: // JALR
				if(rd != 0) {
					C->regs[rd] = op_pc + 8;
				}
			case 8: // JR
				C->pc = C->regs[rs];
				C->pl0_is_branch = true;
				//printf("JR NEW PC: %016llX\n", C->pc);
				break;

			// SWIs
			case 12: // SYSCALL
				MIPSXNAME(_throw_exception)(C, op_pc, MER_Syscall, op_was_branch);
				return MER_Syscall;
			case 13: // BREAK
				MIPSXNAME(_throw_exception)(C, op_pc, MER_Bp, op_was_branch);
				return MER_Bp;

#ifndef MIPS_IS_RSP
			case 15: // SYNC
				// No idea how this works sadly
				break;
#endif

			// M(F|T)(LO|HI)
			case 16: // MFHI
				if(rd != 0) { C->regs[rd] = C->rhi; }
				break;
			case 17: // MTHI
				C->rhi = C->regs[rd];
				break;
			case 18: // MFLO
				if(rd != 0) { C->regs[rd] = C->rlo; }
				break;
			case 19: // MTLO
				C->rlo = C->regs[rd];
				break;

			// (MULT|DIV)U?
			case 24: // MULT
				printf("MULT\n");
				{
					int64_t in_a = (int64_t)(int32_t)C->regs[rs];
					int64_t in_b = (int64_t)(int32_t)C->regs[rt];
					int64_t res = in_a * in_b;
					C->rlo = res&0xFFFFFFFFU;
					C->rhi = res>>32U;
					C->rlo = (SREG)(int32_t)C->rlo;
					C->rhi = (SREG)(int32_t)C->rhi;
				} break;
			case 25: // MULTU
				{
					uint64_t in_a = (uint64_t)(uint32_t)C->regs[rs];
					uint64_t in_b = (uint64_t)(uint32_t)C->regs[rt];
					uint64_t res = in_a * in_b;
					C->rlo = res&0xFFFFFFFFU;
					C->rhi = res>>32U;
					C->rlo = (SREG)(int32_t)C->rlo;
					C->rhi = (SREG)(int32_t)C->rhi;
#if 0
					printf("MULTU %016llX * %016llX = %016llX = %016llX : %016llX\n",
						in_a,
						in_b,
						res,
						C->rhi,
						C->rlo);
#endif
				} break;
			case 26: // DIV
				printf("DIV\n");
				// TODO: find result of zero division on THIS particular MIPS version
				// TODO: find out how the hell the remainder is calculated
				{
					int32_t in_a = (int32_t)C->regs[rs];
					int32_t in_b = (int32_t)C->regs[rt];
					if(in_b == 0) {
						C->rlo = (SREG)(in_a >= 0
							? (UREG)(SREG)-1
							: (UREG)1);
						C->rhi = (SREG)in_a;
					} else {
						int32_t quo = in_a / in_b;
						int32_t rem = in_a % in_b;
						C->rlo = (SREG)quo;
						C->rhi = (SREG)rem;
					}
				} break;
			case 27: // DIVU
				printf("DIVU\n");
				{
					uint32_t in_a = (uint32_t)C->regs[rs];
					uint32_t in_b = (uint32_t)C->regs[rt];
					if(in_b == 0) {
						C->rlo = (SREG)-1;
						C->rhi = (SREG)(int32_t)in_a;
					} else {
						uint32_t quo = in_a / in_b;
						uint32_t rem = in_a % in_b;
						C->rlo = (SREG)(int32_t)quo;
						C->rhi = (SREG)(int32_t)rem;
					}
				} break;

			// (ADD|SUB)U?
			case 32: // ADD
#ifndef MIPS_IS_RSP
				{ 
					uint32_t s1 = C->regs[rs];
					uint32_t s2 = C->regs[rt];
					uint32_t dv = s1 + s2;

					// Trap on signed overflow
					// Which is *always* horrible to detect
					// if sgn(s1)==sgn(s2) && sgn(dv)!=sgn(s1)
					if((0x80000000 & (dv^s1) & ~(s1^s2)) != 0) {
						MIPSXNAME(_throw_exception)(C, op_pc, MER_Ov, op_was_branch);
						return MER_Ov;
					}

					if(rd != 0) {
						C->regs[rd] = dv;
						SIGNEX32R(C, rd);
					}
				} break;
#else
			// FALL THROUGH
#endif
			case 33: // ADDU
				if(rd != 0) {
					C->regs[rd] = C->regs[rs] + C->regs[rt];
					SIGNEX32R(C, rd);
				} break;
			case 34: // SUB
#ifndef MIPS_IS_RSP
				{ 
					uint32_t s1 = C->regs[rs];
					uint32_t s2 = C->regs[rt];
					uint32_t dv = s1 - s2;

					// Trap on signed overflow
					// if sgn(s1)!=sgn(s2) && sgn(dv)!=sgn(s1)
					if((0x80000000 & (dv^s1) & (s1^s2)) != 0) {
						MIPSXNAME(_throw_exception)(C, op_pc, MER_Ov, op_was_branch);
						return MER_Ov;
					}

					if(rd != 0) {
						C->regs[rd] = dv;
						SIGNEX32R(C, rd);
					}
				} break;
#else
			// FALL THROUGH
#endif
			case 35: // SUBU
				if(rd != 0) {
					C->regs[rd] = C->regs[rs] - C->regs[rt];
					SIGNEX32R(C, rd);
				} break;

			// Logic ops
			case 36: // AND
				if(rd != 0) {
					C->regs[rd] = C->regs[rs] & C->regs[rt];
				} break;
			case 37: // OR
				if(rd != 0) {
					C->regs[rd] = C->regs[rs] | C->regs[rt];
				} break;
			case 38: // XOR
				if(rd != 0) {
					C->regs[rd] = C->regs[rs] ^ C->regs[rt];
				} break;
			case 39: // NOR
				if(rd != 0) {
					C->regs[rd] = ~(C->regs[rs] | C->regs[rt]);
				} break;

			// SLTU?
			case 42: // SLT
				if(rd != 0) {
					C->regs[rd] = ((SREG)C->regs[rs] < (SREG)C->regs[rt]) ? 1 : 0;
				} break;
			case 43: // SLTU
				if(rd != 0) {
					C->regs[rd] = ((UREG)C->regs[rs] < (UREG)C->regs[rt]) ? 1 : 0;
				} break;
			default:
				// Invalid opcode
				printf("RI %2u %08X -> %08X %d (special)\n"
					, (op&0x3FU), op_pc, new_pc, op_was_branch
					);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		} break;

		// REGIMM
		case 1: switch(rt) {
			case 0: // BLTZ
				if(((SREG)C->regs[rs]) < 0) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} break;
			case 1: // BGEZ
				if(((SREG)C->regs[rs]) >= 0) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} break;

#if 1
			case 2: // BLTZL
				if(((SREG)C->regs[rs]) < 0) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} else {
					new_op = 0;
				} break;
			case 3: // BGEZL
				if(((SREG)C->regs[rs]) >= 0) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} else {
					new_op = 0;
				} break;
#endif

			case 16: // BLTZAL
				C->regs[31] = op_pc + 8;
				if(((SREG)C->regs[rs]) < 0) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} break;
			case 17: // BGEZAL
				C->regs[31] = op_pc + 8;
				if(((SREG)C->regs[rs]) >= 0) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} break;

#if 1
			case 18: // BLTZALL
				C->regs[31] = op_pc + 8;
				if(((SREG)C->regs[rs]) < 0) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} else {
					new_op = 0;
				} break;
			case 19: // BGEZALL
				C->regs[31] = op_pc + 8;
				if(((SREG)C->regs[rs]) >= 0) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} else {
					new_op = 0;
				} break;
#endif

			default:
				// Invalid opcode
				printf("RI %2u %08X -> %08X %d (regimm)\n"
					, rt, op_pc, new_pc, op_was_branch
					);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		} break;

		// J(AL)?
		case 3: // JAL
			C->regs[31] = op_pc + 8;
		case 2: // J
			C->pc = op_pc;
			C->pc &= 0xF0000000U;
			C->pc |= (op<<2)&0x0FFFFFFC;
			C->pl0_is_branch = true;
			//printf("J NEW PC: %016llX\n", C->pc);
			break;

		// B(EQ|NE|(LE|GT)Z)
		// XXX: do we add from new_pc or do we do pc+4+imm?
		case 4: // BEQ
			if(C->regs[rs] == C->regs[rt]) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} break;
		case 5: // BNE
			if(C->regs[rs] != C->regs[rt]) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} break;
		case 6: // BLEZ
			if(C->regs[rs] <= 0) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} break;
		case 7: // BGTZ
			if(C->regs[rs] > 0) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} break;

		// ADDIU?
		case 8: // ADDI
#ifndef MIPS_IS_RSP
			{ 
				int32_t s1 = (int32_t)(C->regs[rs]);
				int32_t s2 = (int32_t)(int16_t)op;
				int32_t dv = s1 + s2;

				// Trap on signed overflow
				if((s2 < 0) ? dv > s1 : dv < s1) {
					MIPSXNAME(_throw_exception)(C, op_pc, MER_Ov, op_was_branch);
					return MER_Ov;
				}

				if(rt != 0) {
					C->regs[rt] = (uint32_t)dv;
				}
				C->regs[rt] = (SREG)(int32_t)(C->regs[rt]);
				SIGNEX32R(C, rt);
			} break;
#else
			// FALL THROUGH
#endif
		case 9: // ADDIU
			if(rt != 0) {
				C->regs[rt] = C->regs[rs] + (SREG)(int16_t)op;
				SIGNEX32R(C, rt);
			} break;

		// IDT you lied to me, you told me these were SUBI/SUBIU
		// SLTIU?
		case 10: // SLTI
			if(rt != 0) {
				C->regs[rt] = ((SREG)C->regs[rs] < (SREG)(int16_t)op) ? 1 : 0;
			} break;
		case 11: // SLTIU
			if(rt != 0) {
				C->regs[rt] = ((UREG)C->regs[rs] < (UREG)(uint16_t)op) ? 1 : 0;
			} break;

		// Logic ops + LUI
		case 12: // ANDI
			if(rt != 0) {
				C->regs[rt] = C->regs[rs] & (op&0xFFFFU);
			} break;
		case 13: // ORI
			if(rt != 0) {
				C->regs[rt] = C->regs[rs] | (op&0xFFFFU);
			} break;
		case 14: // XORI
			if(rt != 0) {
				C->regs[rt] = C->regs[rs] ^ (op&0xFFFFU);
			} break;
		case 15: // LUI
			if(rt != 0) {
				C->regs[rt] = (op&0xFFFFU)<<16U;
				SIGNEX32R(C, rt);
			} break;

		// COP0
		case 16:
#ifndef MIPS_IS_RSP
		if((C->c0.n.sr & C0SR_KSU_mask) != C0SR_KSU_Kernel
				&& (C->c0.n.sr & (C0SR_EXL|C0SR_ERL)) == 0
				&& (C->c0.n.sr & C0SR_CU(0)) == 0
				) {
			C->c0.n.cause &= ~0x30000000;
			C->c0.n.cause |= 0<<28;
			MIPSXNAME(_throw_exception)(C, op_pc, MER_CpU, op_was_branch);
			return MER_CpU;
		} else
#endif
		switch(rs) {
			case 0: // MFCz
			switch(rd) {

#ifdef MIPS_IS_RSP
				case 1: // DMA_DRAM
					if(rt != 0) {
						C->regs[rt] = C->c0.n.dma_dram;
						SIGNEX32R(C, rt);
					} break;
				case 5: // DMA_FULL
					if(rt != 0) {
						C->regs[rt] = (C->c0.n.sp_status>>3)&0x1;
						SIGNEX32R(C, rt);
					} break;
				case 6: // DMA_BUSY
					if(rt != 0) {
						C->regs[rt] = (C->c0.n.sp_status>>2)&0x1;
						SIGNEX32R(C, rt);
					} break;
				case 7: // SP_RESERVED
					if(rt != 0) {
						C->regs[rt] = C->c0.n.sp_reserved;
						SIGNEX32R(C, rt);
					}
					C->c0.n.sp_reserved |= 0x1;
					break;

#else
				case 8: // c0_badvaddr
					if(rt != 0) {
						C->regs[rt] = C->c0.n.badvaddr;
						SIGNEX32R(C, rt);
					} break;
				case 10: // c0_entryhi
					if(rt != 0) {
						C->regs[rt] = C->c0.n.entryhi;
						SIGNEX32R(C, rt);
					} break;
				case 12: // c0_sr
					if(rt != 0) {
						C->regs[rt] = C->c0.n.sr;
						SIGNEX32R(C, rt);
					} break;
				case 13: // c0_cause
					if(rt != 0) {
						C->regs[rt] = C->c0.n.cause;
						SIGNEX32R(C, rt);
					} break;
				case 14: // c0_epc
					if(rt != 0) {
						C->regs[rt] = C->c0.n.epc;
						SIGNEX32R(C, rt);
					} break;
				case 16: // c0_config
					if(rt != 0) {
						C->regs[rt] = C->c0.n.config;
						SIGNEX32R(C, rt);
					} break;
#endif

				default:
					printf("RI MFC0 %2u %08X -> %08X %d (COP0)\n"
						, rd, op_pc, new_pc, op_was_branch
						);
					MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
					return MER_RI;
			} break;

			case 4: // MTCz
			switch(rd) {

#ifdef MIPS_IS_RSP
				case 0: // DMA_CACHE
					printf("DMA_CACHE %08X\n", C->regs[rt]);
					C->c0.n.dma_cache = C->regs[rt] & 0x1FFF;
					break;
				case 1: // DMA_DRAM
					printf("DMA_DRAM %08X\n", C->regs[rt]);
					C->c0.n.dma_dram = C->regs[rt] & 0xFFFFFF;
					break;
				case 2: // DMA_READ_LENGTH
					printf("DMA_READ_LENGTH %08X\n", C->regs[rt]);
					C->c0.n.dma_read_length = C->regs[rt];
					break;
				//case 3: // DMA_WRITE_LENGTH
					//C->c0.n.dma_write_length = C->regs[rt];
					//break;
#else
				case 0: // c0_index
					C->c0.n.index = C->regs[rt] & 63;
					break;

				case 2: // c0_entrylo0
					C->c0.n.entrylo0 = C->regs[rt];
					break;
				case 3: // c0_entrylo1
					C->c0.n.entrylo1 = C->regs[rt];
					break;

				case 5: // c0_pagemask
					C->c0.n.pagemask = (C->regs[rt] & 0x01FFE000)|0x1FFF;
					break;

				case 8: // c0_badvaddr
					C->c0.n.badvaddr = C->regs[rt];
					break;

				case 9: // c0_count
					C->c0.n.count = C->regs[rt];
					break;

				case 10: // c0_entryhi
					C->c0.n.entryhi = C->regs[rt];
					break;

				case 11: // c0_compare
					C->c0.n.compare = C->regs[rt];
					break;

				case 12: // c0_sr
					C->c0.n.sr &= ~0xFF77FFFF;
					C->c0.n.sr |= (C->regs[rt] & 0xFF57FFFF);
					e = MIPSXNAME(_probe_interrupts)(C);
					if(e != MER_NONE) {
						e_ifetch = e;
					}
					break;

				case 13: // c0_cause
					C->c0.n.cause &= ~0x00000300;
					C->c0.n.cause |= (C->regs[rt] & 0x00000300);
					e = MIPSXNAME(_probe_interrupts)(C);
					if(e != MER_NONE) {
						e_ifetch = e;
					}
					break;

				case 14: // c0_epc
					C->c0.n.epc = C->regs[rt];
					break;

				case 16: // c0_config
					C->c0.n.config &= ~0x0F00800F;
					C->c0.n.config |= (C->regs[rt] & 0x0F00800F);
					break;

				case 28: // c0_taglo
					C->c0.n.taglo &= ~0x0FFFFFC0;
					C->c0.n.taglo |= (C->regs[rt] & 0x0FFFFFC0);
					break;

				case 29: // c0_taghi
					C->c0.n.taghi &= ~0x00000000;
					C->c0.n.taghi |= (C->regs[rt] & 0x00000000);
					break;
#endif

				default:
					printf("RI MTC0 %2u %08X -> %08X %d (COP0)\n"
						, rd, op_pc, new_pc, op_was_branch
						);
					MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
					return MER_RI;

			} break;

			case 16: switch(op&0x3F) {
#ifdef MIPS_MMU_ENTRIES
				case 2: // TLBWI
				case 6: // TLBWR
				{
					// FIXME: we should calculate c0_random properly
					int idx = ((op&4) == 0
						? C->c0.n.index
						: fullrandu32());
					idx &= (MIPS_MMU_ENTRIES-1);

					C->tlb[idx].entrylo[0] = C->c0.n.entrylo0;
					C->tlb[idx].entrylo[1] = C->c0.n.entrylo1;
					C->tlb[idx].entryhi = C->c0.n.entryhi;
					C->tlb[idx].pagemask = C->c0.n.pagemask;
					printf("entryhi %08X\n", C->c0.n.entryhi);
					printf("entrylo0 %08X\n", C->c0.n.entrylo0);
					printf("entrylo1 %08X\n", C->c0.n.entrylo1);
					printf("pagemask %08X\n", C->c0.n.pagemask);
					printf("idx %d\n", idx);
				} break;
#endif
				default:
					printf("RI op %2u %08X -> %08X %d (COP0)\n"
						, op&0x3F, op_pc, new_pc, op_was_branch
						);
					MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
					return MER_RI;

			} break;

			default:
				printf("RI %2u %2u %08X -> %08X %d (COP0)\n"
					, rs, op&0x3F, op_pc, new_pc, op_was_branch
					);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		} break;

#ifdef MIPS_IS_RSP
		// COP2
		case 18:
		switch(rs) {
			case 0: // MFCz
			switch(rd) {
				default:
					printf("RI MFC2 %2u %08X -> %08X %d (COP2)\n"
						, rd, op_pc, new_pc, op_was_branch
						);
					MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
					return MER_RI;
			} break;

			case 4: // MTCz
			switch(rd) {
				default:
					printf("RI MTC2 %2u %08X -> %08X %d (COP2)\n"
						, rd, op_pc, new_pc, op_was_branch
						);
					MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
					return MER_RI;
			} break;

			case 16: switch(op&0x3F) {

				default:
					printf("RI op %2u %08X -> %08X %d (COP2)\n"
						, op&0x3F, op_pc, new_pc, op_was_branch
						);
					MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
					return MER_RI;

			} break;

			default:
				printf("RI %2u %2u %08X -> %08X %d (COP2)\n"
					, rs, op&0x3F, op_pc, new_pc, op_was_branch
					);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		} break;


#else
		// COPx
		case 17: if((C->c0.n.sr & C0SR_CU(1)) == 0) {
			C->c0.n.cause &= ~0x30000000;
			C->c0.n.cause |= 1<<28;
			MIPSXNAME(_throw_exception)(C, op_pc, MER_CpU, op_was_branch);
			return MER_CpU;
		} else {
			printf("RI %2u %2u %08X -> %08X %d (COP1)\n"
				, rs, op&0x3F, op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;
		} break;

		case 18: if((C->c0.n.sr & C0SR_CU(2)) == 0) {
			C->c0.n.cause &= ~0x30000000;
			C->c0.n.cause |= 1<<28;
			MIPSXNAME(_throw_exception)(C, op_pc, MER_CpU, op_was_branch);
			return MER_CpU;
		} else {
			printf("RI %2u %2u %08X -> %08X %d (COP2)\n"
				, rs, op&0x3F, op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;
		} break;

		case 19: if((C->c0.n.sr & C0SR_CU(3)) == 0) {
			C->c0.n.cause &= ~0x30000000;
			C->c0.n.cause |= 3<<28;
			MIPSXNAME(_throw_exception)(C, op_pc, MER_CpU, op_was_branch);
			return MER_CpU;
		} else {
			printf("RI %2u %2u %08X -> %08X %d (COP3)\n"
				, rs, op&0x3F, op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;
		} break;
#endif

		// B(EQ|NE|(LE|GT)Z)L
		// XXX: do we add from new_pc or do we do pc+4+imm?
#ifndef MIPS_IS_RSP
		case 20: // BEQL
			if(C->regs[rs] == C->regs[rt]) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} else {
				new_op = 0;
			} break;
		case 21: // BNEL
			if(C->regs[rs] != C->regs[rt]) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} else {
				new_op = 0;
			} break;
		case 22: // BLEZL
			if(C->regs[rs] <= 0) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} else {
				new_op = 0;
			} break;
		case 23: // BGTZL
			if(C->regs[rs] >= 0) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} else {
				new_op = 0;
			} break;
#endif

		// Basic loads
		case 32: // LB
			e = MIPSXNAME(_read8)(C, C->regs[rs]+(SREG)(int16_t)op, &mdata);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			if(rt != 0) {
				C->regs[rt] = (SREG)(int8_t)mdata;
			}
			break;
		case 33: // LH
			e = MIPSXNAME(_read16)(C, C->regs[rs]+(SREG)(int16_t)op, &mdata);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			if(rt != 0) {
				C->regs[rt] = (SREG)(int16_t)mdata;
			}
			break;
		case 35: // LW
			e = MIPSXNAME(_read32)(C, C->regs[rs]+(SREG)(int16_t)op, &mdata);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			if(rt != 0) {
				C->regs[rt] = (SREG)(int32_t)mdata;
			}
			break;

		// Unsigned loads
		case 36: // LBU
			e = MIPSXNAME(_read8)(C, C->regs[rs]+(SREG)(int16_t)op, &mdata);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			if(rt != 0) {
				C->regs[rt] = (uint32_t)(uint8_t)mdata;
			}
			break;
		case 37: // LHU
			e = MIPSXNAME(_read16)(C, C->regs[rs]+(SREG)(int16_t)op, &mdata);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			if(rt != 0) {
				C->regs[rt] = (uint32_t)(uint16_t)mdata;
			}
			break;

		// Unaligned loads
		case 34: // LWL
			mdata = C->regs[rt];
			e = MIPSXNAME(_read32l)(C, C->regs[rs]+(SREG)(int16_t)op, &mdata);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			if(rt != 0) {
				C->regs[rt] = mdata;
				SIGNEX32R(C, rt);
			}
			break;
		case 38: // LWR
			mdata = C->regs[rt];
			e = MIPSXNAME(_read32r)(C, C->regs[rs]+(uint32_t)(int32_t)(int16_t)op, &mdata);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			if(rt != 0) {
				C->regs[rt] = mdata;
				SIGNEX32R(C, rt);
			}
			break;

		// Basic stores
		case 40: // SB
			e = MIPSXNAME(_write8)(C, C->regs[rs]+(SREG)(int32_t)(int16_t)op, C->regs[rt]);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			break;
		case 41: // SH
			e = MIPSXNAME(_write16)(C, C->regs[rs]+(SREG)(int32_t)(int16_t)op, C->regs[rt]);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			break;
		case 43: // SW
			e = MIPSXNAME(_write32)(C, C->regs[rs]+(SREG)(int32_t)(int16_t)op, C->regs[rt]);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			break;

		// Unaligned stores
		case 42: // SWL
			e = MIPSXNAME(_write32l)(C, C->regs[rs]+(SREG)(int32_t)(int16_t)op, C->regs[rt]);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			break;
		case 46: // SWR
			e = MIPSXNAME(_write32r)(C, C->regs[rs]+(SREG)(int32_t)(int16_t)op, C->regs[rt]);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			break;

		// CACHE
		case 47: // CACHE
			// TODO!
			if(rt != 8 && rt != 9) {
				printf("CACHE %2u %08X\n", rt, C->regs[rs]+(uint32_t)(int32_t)(int16_t)op);
			}
			break;

#ifdef MIPS_IS_RSP
		// RSP vector unit op (normally LWC2)
		case 50: {
			uint32_t v_base = (op>>21)&0x1F;
			uint32_t v_vt = (op>>16)&0x1F;
			uint32_t v_opcode = (op>>11)&0x1F;
			uint32_t v_element = (op>>7)&0xF;
			uint32_t v_offset = (uint32_t)(((int32_t)(op<<25))>>25);
			switch(v_opcode)
			{
				// LQV
				case 4: {
					assert(v_element == 0);
					C->c2.w[v_vt][0] = v_base;
					C->c2.w[v_vt][1] = v_base;
					C->c2.w[v_vt][2] = v_base;
					C->c2.w[v_vt][3] = v_base;
				} break;

				default:
					printf("RSP load op %2d %2d %2d %2d %2d\n",
						v_base, v_vt, v_opcode, v_element, v_offset);
					MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
					return MER_RI;
			}
		} break;
#endif

#ifndef MIPS_IS_RSP
		// for some reason krom's tests need these

		case 39: // LWU
			e = MIPSXNAME(_read32)(C, C->regs[rs]+(SREG)(int16_t)op, &mdata);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			if(rt != 0) {
				C->regs[rt] = (SREG)(UREG)(uint32_t)mdata;
			}
			break;

		case 55: // LD
		{
			uint32_t mdata0, mdata1;
			e = MIPSXNAME(_read32)(C, C->regs[rs]+(SREG)(int16_t)op, &mdata0);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			e = MIPSXNAME(_read32)(C, C->regs[rs]+4+(SREG)(int16_t)op, &mdata1);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			if(rt != 0) {
				C->regs[rt] = ((SREG)(int32_t)mdata0)<<32;
				C->regs[rt] |= ((SREG)(UREG)(uint32_t)mdata1);
			}
		} break;

		case 63: // SD
		{
			// FIXME this is probably wrong
			e = MIPSXNAME(_write32)(C, C->regs[rs]+(SREG)(int16_t)op, C->regs[rt]>>32);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
			e = MIPSXNAME(_write32)(C, C->regs[rs]+4+(SREG)(int16_t)op, C->regs[rt]);
			if(e != MER_NONE) {
				MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
				return e;
			}
		} break;
#endif

		default:
			// Invalid opcode
			printf("RI %2u %08X -> %08X %d (main)\n"
				, (op>>26U), op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;
	}

	//
	// OPCODE EXECUTION END
	//

	// Check fetch result
	if(e_ifetch != MER_NONE) {
		MIPSXNAME(_throw_exception)(C, new_pc, e_ifetch, C->pl0_is_branch);
		return e_ifetch;
	}

#if 0
	//if(new_pc == op_pc - 4) {
	if(new_op == 0x0411FFFF) {
	//if(new_pc == (SREG)(int32_t)0xA40015F8) {
		MIPSXNAME(_throw_exception)(C, op_pc, MER_Int, op_was_branch);
		printf("OP OLD: %016llX: %08X\n", op_pc, op);
		printf("OP NEW: %016llX: %08X\n", new_pc, new_op);
	}
#endif

	// Copy new op
	C->pl0_op = new_op;
	C->pl0_pc = new_pc;

	// Return
	return MER_NONE;
}

void MIPSXNAME(_cpu_init)(struct MIPSNAME *C)
{
	// initialise main regs
	for(int i = 1; i < 32; i++) {
		C->regs[i] = fullrandu64();
		C->regs[i] = 0;
	}
	C->regs[0] = 0;
	C->rlo = fullrandu64();
	C->rhi = fullrandu64();

	// initialise cop0 regs
	for(int i = 0; i < 32; i++) {
		C->c0.i[i] = fullrandu64();
	}

#ifdef MIPS_IS_RSP
	C->c0.n.sp_status = 0x00000001;
	C->c0.n.dma_read_length = 0;
	C->c0.n.dma_write_length = 0;
	C->rsp_intr = false;

#else
	// apply masks
	C->c0.n.index &= 0x80003F00;
	C->c0.n.random &= 0x00003F00;
	C->c0.n.entrylo0 &= 0xFFFFFF00;
	C->c0.n.entrylo1 &= 0xFFFFFF00;
	C->c0.n.context &= 0xFFFFFFFC;
	C->c0.n.entryhi &= 0xFFFFFFC0;
	C->c0.n.sr &= 0xF27FFF3F;
	C->c0.n.cause &= 0xB800037C;
	C->c0.n.config &= 0x00000000;
	C->c0.n.config |= 0x0006E460;

	// apply standard values
	C->c0.n.prid = 0x00000F00; // TODO get valid ID

	// clamp random
	if((C->c0.n.random>>8U) < 8U) {
		C->c0.n.random = 8U<<8U;
	}
#endif

	// perform reset
	MIPSXNAME(_cpu_reset)(C);

	// initialise clock
	C->tickrem = 0;
	C->ticktime = 0;
}

#undef UREG
#undef SREG
#undef REGSIZE
#undef SIGNEX32R

#undef MIPS_VERSION
#undef MIPS_MMU_ENTRIES


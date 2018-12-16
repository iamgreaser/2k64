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

void MIPSXNAME(_print_reg_dump)(struct MIPSNAME *C)
{
	for(int i = 0; i < 32; i++) {
		//printf("$%s = %016llX\n", mips_gpr_names[i], C->regs[i]);
#ifdef MIPS_IS_RSP
		printf("$%s = %08X | $c0_%-2d = %08X | $v%-2d = %016llX%016llX\n", mips_gpr_names[i], C->regs[i], i, C->c0.i[i], i, C->c2.d[i][0], C->c2.d[i][1]);
#else
		printf("$%s = %016llX | $c0_%-2d = %016llX | $f%-2d = %016llX\n", mips_gpr_names[i], C->regs[i], i, C->c0.i[i], i, C->c1.di[i]);
#endif
	}
}

void MIPSXNAME(_throw_exception)(struct MIPSNAME *C, UREG epc, enum mipserr cause, bool bd)
{
	assert(cause >= MER_Int && cause <= MER_Ov);

#ifdef MIPS_IS_RSP
	bool show_throws = true;
#else
	bool show_throws = (cause != MER_Int);
#endif

	if(show_throws) {
		printf("Throw exception %016llX, cause %d\n", (unsigned long long)epc, ((int)cause)-1);
	}

	if(show_throws) { // && cause != MER_Int) {
		MIPSXNAME(_print_reg_dump)(C);
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
	MIPS_MEM_READ(C, (epc&0x0FFF)|0x1000, 0xFFFFFFFF, &opdata);
	printf("op refetch = %08X\n", opdata);

	// Halt RSP.
	C->c0.n.sp_status |= 0x00000001;
	printf("RSP HALTED\n");
#else
	if(show_throws) {
		MIPSXNAME(_read32)(C, epc, &opdata);
		printf("op refetch = %08X\n", opdata);
		printf("badvaddr = %016llX\n", (unsigned long long)C->c0.n.badvaddr);
	}
	//fflush(stdout); abort();

	// Set up PC + op
	if((C->c0.n.sr & C0SR_EXL) == 0) {
		C->c0.n.epc = epc;
	}
	C->pl0_op = 0x00000000;
	C->pc = ((C->c0.n.sr & C0SR_BEV) != 0 ? 0xBFC00200 : 0x80000000);
	C->pc = (SREG)(int32_t)C->pc;
	if(cause == MER_TLBL || cause == MER_TLBS) {
		// TODO: XTLB
		if((C->c0.n.sr & C0SR_EXL) != 0) {
			C->pc += 0x180;
		}
	} else {
		C->pc += 0x180;
	}

	// FIXME there's some special handling here somewhere

	// Set up cause fields
	C->c0.n.cause &= 0xB000FF00;
	C->c0.n.cause |= ((((uint32_t)(cause-1))&31)<<2);
	if((C->c0.n.sr & C0SR_EXL) == 0) {
		C->c0.n.cause &= ~(1<<31);
		if(bd) {
			// Branch delay slots have special treatment
			C->c0.n.cause |= (1<<31);
			C->c0.n.epc -= 4;
		}
	}

	// Enter exception mode
	C->c0.n.sr |= C0SR_EXL;
	if(show_throws) {
		printf("new PC = %08X\n", C->pc);
	}
#endif
}

enum mipserr MIPSXNAME(_probe_interrupts)(struct MIPSNAME *C)
{
#ifdef MIPS_IS_RSP

#else
	if((C->c0.n.sr & (C0SR_EXL|C0SR_ERL|C0SR_IE)) == C0SR_IE) {
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
#ifndef MIPS_IS_RSP
			C->c0.n.context &= ~0x007FFFF0;
			C->c0.n.context |= (addr>>(13-4)) & 0x007FFFF0;
			C->c0.n.context |= (SREG)(int32_t)C->c0.n.context;
			C->c0.n.entryhi &= 0x1FFF;
			C->c0.n.entryhi |= addr & ~0x1FFF;
#endif
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
				printf("YOU AIN'T THE KERNEL (%016llX -> %08X)\n", C->pl0_pc, C->c0.n.sr);
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
				continue;
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
			break; // checking for TLB shutdowns is slow, fuck that
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
	addr &= 0xFFF;
#endif
	return MIPS_MEM_READ(C, addr, 0xFFFFFFFF, data);
}

enum mipserr MIPSXNAME(_read16_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t *data)
{
#ifdef MIPS_IS_RSP
	addr &= 0xFFF;
#endif
	uint32_t mask = 0xFFFF<<(((~addr)&2)<<3);
	enum mipserr e = MIPS_MEM_READ(C, addr, mask, data);
	//enum mipserr e = MIPS_MEM_READ(C, addr, 0xFFFFFFFF, data);
	if(e != MER_NONE) {
		return e;
	}
	*data >>= (((~addr)&2)<<3);
	*data &= 0xFFFF;
	return e;
}

enum mipserr MIPSXNAME(_read8_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t *data)
{
#ifdef MIPS_IS_RSP
	addr &= 0xFFF;
#endif
	uint32_t mask = 0xFF<<(((~addr)&3)<<3);
	enum mipserr e = MIPS_MEM_READ(C, addr, mask, data);
	//enum mipserr e = MIPS_MEM_READ(C, addr, 0xFFFFFFFF, data);
	if(e != MER_NONE) {
		return e;
	}
	*data >>= (((~addr)&3)<<3);
	*data &= 0xFF;
	return e;
}

enum mipserr MIPSXNAME(_write32_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t data)
{
	MIPS_MEM_WRITE(C, addr, 0xFFFFFFFF, data);
	return MER_NONE;
}

enum mipserr MIPSXNAME(_write16_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t data)
{
	data &= 0xFFFF;
	data |= (data<<16);
	MIPS_MEM_WRITE(C, addr, 0xFFFF<<(((~addr)&2)<<3), data);
	return MER_NONE;
}

enum mipserr MIPSXNAME(_write8_unchecked)(struct MIPSNAME *C, uint64_t addr, uint32_t data)
{
	data &= 0xFF;
	data |= (data<<8);
	data |= (data<<16);
	MIPS_MEM_WRITE(C, addr, 0xFF<<(((~addr)&3)<<3), data);
	return MER_NONE;
}

#ifndef MIPS_IS_RSP
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
	e = MIPS_MEM_READ(C, addr, imask, &mdata);
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
	e = MIPS_MEM_READ(C, addr, imask, &mdata);
	//printf("L mask %08X %08X %d %08X\n", imask, omask, shamt, mdata);
	if(e == MER_NONE) {
		*data = (*data & ~omask) | ((mdata>>shamt) & omask);
	}
	MIPSXNAME(_badvaddr_cond_set)(C, e, addr);
	return e;
}
#endif

enum mipserr MIPSXNAME(_read32)(struct MIPSNAME *C, UREG addr, uint32_t *data)
{
	// Get address
	if((addr&3) != 0) {
		printf("BAD ADDR read 32-bit %08X @ %016llX\n", addr, C->pl0_pc);
#ifdef MIPS_IS_RSP
		addr &= ~0x3;
#else
		MIPSXNAME(_badvaddr_cond_set)(C, MER_AdEL, addr);
		return MER_AdEL;
#endif
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
		printf("BAD ADDR read 16-bit %08X @ %016llX\n", addr, C->pl0_pc);
		printf("k1=%016llX\n", C->regs[27]);
#ifdef MIPS_IS_RSP
		addr &= ~0x1;
#else
		MIPSXNAME(_badvaddr_cond_set)(C, MER_AdEL, addr);
		return MER_AdEL;
#endif
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

#ifndef MIPS_IS_RSP
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
	MIPS_MEM_WRITE(C, addr, omask, data<<shamt);
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
	MIPS_MEM_WRITE(C, addr, omask, data<<shamt);
	return MER_NONE;
}
#endif

enum mipserr MIPSXNAME(_write32)(struct MIPSNAME *C, UREG addr, uint32_t data)
{
	// Get address
	if((addr&3) != 0) {
		printf("BAD ADDR write 32-bit %08X @ %016llX\n", addr, C->pl0_pc);
#ifdef MIPS_IS_RSP
		addr &= ~3;
#else
		MIPSXNAME(_badvaddr_cond_set)(C, MER_AdES, addr);
		return MER_AdES;
#endif
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
		printf("BAD ADDR write 16-bit %08X @ %016llX\n", addr, C->pl0_pc);
#ifdef MIPS_IS_RSP
		addr &= ~1;
#else
		MIPSXNAME(_badvaddr_cond_set)(C, MER_AdES, addr);
		return MER_AdES;
#endif
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
	enum mipserr e = MIPS_MEM_READ(C, (addr&0x0FFF)|0x1000, 0xFFFFFFFF, data);
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

#ifndef MIPS_IS_RSP
	C->c0.n.count += 1;
	if((C->c0.n.count&0x1FFFFFFF) == ((C->c0.n.compare & 0xFFFFFFFF)<<1)) {
		printf("COUNT INTR\n");
		C->c0.n.cause |= 0x8000;
	}
	if(C->tickwait > 0) {
		C->tickwait -= 1;
		return e;
	}
#endif

#ifdef MIPS_IS_RSP
	C->pc &= 0xFFF;
#else
	C->pc = (SREG)(int32_t)C->pc;
#endif

	// Fetch op
	uint32_t op = C->pl0_op;
	SREG op_pc = C->pl0_pc;
	bool op_was_branch = C->pl0_is_branch;
	SREG new_pc = C->pc;
	uint32_t new_op = 0xFFFFFFFF; // shut the compiler up
	//enum mipserr e_ifetch = MIPSXNAME(_fetch_op)(C, &new_op);
	enum mipserr e_ifetch = MIPSXNAME(_fetch_op)(C, &(C->pl0_op));
	new_op = C->pl0_op;

#ifdef MIPS_IS_RSP
	op_pc &= 0xFFF;
	op_pc |= 0x04001000;
#endif

#ifdef MIPS_IS_RSP
#if 0
	printf("OPDATA: %016llX: %08X\n", op_pc, op);
	MIPSXNAME(_print_reg_dump)(C);
#endif
#endif
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

#include "ops-main.h"


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
	//if(new_op == 0x0411FFFF) {
	if(op_pc >= 0x70049950 && op_pc <= 0x70049970) {
	//if(new_pc == (SREG)(int32_t)0xA40015F8) {
		//MIPSXNAME(_throw_exception)(C, op_pc, MER_Int, op_was_branch);
		printf("OP OLD: %016llX: %08X\n", op_pc, op);
		//printf("OP NEW: %016llX: %08X\n", new_pc, new_op);
	}
#endif

	// Copy new op
	C->pl0_op = new_op;
	C->pl0_pc = new_pc;

	// Handle interrupts
	e = MIPSXNAME(_probe_interrupts)(C);
	if(e != MER_NONE) {
		MIPSXNAME(_throw_exception)(C, new_pc, e, C->pl0_is_branch);
		return e;
	}

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
	C->c0.n.context &= 0xFFFFFFF0;
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

	C->llbit = false;
#endif

	// perform reset
	MIPSXNAME(_cpu_reset)(C);

	// initialise clock
	C->tickrem = 0;
	C->ticktime = 0;
	C->tickwait = 0;
}


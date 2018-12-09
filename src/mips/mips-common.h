#ifndef _MIPS_COMMON_H
#define _MIPS_COMMON_H 1
enum mipserr
{
	// not errors, just extra info
	MER_UNCACHEABLE = -1, // for MIPSNAME##_calc_addr

	// very much not an error
	MER_NONE = 0,

	// subtract 1 for actual exception number
	MER_Int  =  0+1, // Interrupt
	MER_Mod  =  1+1, // TLB modification
	MER_TLBL =  2+1, // TLB load
	MER_TLBS =  3+1, // TLB store
	MER_AdEL =  4+1, // Address error load (incl ifetch)
	MER_AdES =  5+1, // Address error store
	MER_IBE  =  6+1, // Bus error on ifetch
	MER_DBE  =  7+1, // Bus error on dfetch (NOT dstore)
	MER_Syscall = 8+1, // SYSCALL instruction
	MER_Bp   =  9+1, // BREAK instruction (breakpoint)
	MER_RI   = 10+1, // Reserved instruction
	MER_CpU  = 11+1, // Coprocessor unusable
	MER_Ov   = 12+1, // Arithmetic overflow

	// MIPS3+ exceptions
	MER_Tr   = 13+1, // Trap exception
	MER_FPE  = 15+1, // Trap exception
	MER_WATCH= 23+1, // Trap exception
};

// c0_sr MIPS3
#define C0SR_IE (1<<0)
#define C0SR_EXL (1<<1)
#define C0SR_ERL (1<<2)
#define C0SR_KSU_Kernel (0x0<<3)
#define C0SR_KSU_Supervisor (0x1<<3)
#define C0SR_KSU_User (0x2<<3)
#define C0SR_KSU_mask (0x3<<3)
#define C0SR_KSU_shift 3
#define C0SR_UX (1<<5)
#define C0SR_SX (1<<6)
#define C0SR_KX (1<<7)
#define C0SR_IM(x) (1<<((x)+8))
#define C0SR_IM_mask (0xFF<<8)
#define C0SR_IM_shift 8
#define C0SR_CH (1<<18)
#define C0SR_SR (1<<20)
#define C0SR_TS (1<<21)
#define C0SR_BEV (1<<22)
#define C0SR_ITS (1<<24)
#define C0SR_DS_mask (0x1FF<<16)
#define C0SR_DS_shift 16
#define C0SR_RE (1<<25)
#define C0SR_FR (1<<26)
#define C0SR_RP (1<<27)
#define C0SR_CU(x) (1<<((x)+28))
#define C0SR_CU_mask (0xF<<28)
#define C0SR_CU_shift 28

// c0_config MIPS3
#define C0CONFIG_K0_mask (0x7<<0)
#define C0CONFIG_K0_shift 0
#define C0CONFIG_CU (1<<3)
#define C0CONFIG_BE (1<<15)
#define C0CONFIG_EP_mask (0xF<<24)
#define C0CONFIG_EP_shift 24
#define C0CONFIG_EC_mask (0x7<<28)
#define C0CONFIG_EC_shift 28

static const char *mips_gpr_names[32] = {
	"z0", // "zero" but I like to fit these into two chars
	"at",
	"v0", "v1",
	"a0", "a1", "a2", "a3",

	// this is either t0+ (O32) or a4+ (N32/N64) depending on calling convention
	// no, N64 doesn't mean Nintendo 64 here!
	"t0", "t1", "t2", "t3",

	"t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9",
	"k0", "k1",
	"gp", "sp", "fp", "ra",
};

#endif


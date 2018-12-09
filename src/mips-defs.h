#ifdef MIPS_IS_VR4300
#define MIPS_VERSION 3
#define MIPS_MMU_ENTRIES 32
#define MIPS_HAS_FPU 1
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


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
#ifdef MIPS_IS_RSP
			C->c0.n.sp_status |= 0x02; // broke
			C->c0.n.sp_status |= 0x01; // halt
			return MER_Bp;
#else
			MIPSXNAME(_throw_exception)(C, op_pc, MER_Bp, op_was_branch);
			return MER_Bp;
#endif

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

#ifndef MIPS_IS_RSP
		// DS(LL|RL|RA)V
		case 20: // DSLLV
			if(rd != 0) {
				C->regs[rd] = C->regs[rt] << (C->regs[rs]&0x3F);
			} break;
		case 22: // DSRLV
			if(rd != 0) {
				C->regs[rd] = ((uint64_t)C->regs[rt]) >> (uint64_t)(C->regs[rs]&0x3F);
			} break;
		case 23: // DSRAV
			if(rd != 0) {
				C->regs[rd] = ((int64_t)C->regs[rt]) >> (int64_t)(C->regs[rs]&0x3F);
			} break;
#endif

#ifndef MIPS_IS_RSP
		// (MULT|DIV)U?
		case 24: // MULT
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
#endif

#ifndef MIPS_IS_RSP
		// D(MULT|DIV)U?
		// Using GCC's __int128 b/c can't be arsed right now with a portable approach
		case 28: // DMULT
			{
				__int128 in_a = (__int128)(int64_t)C->regs[rs];
				__int128 in_b = (__int128)(int64_t)C->regs[rt];
				__int128 res = in_a * in_b;
				C->rlo = res&0xFFFFFFFFFFFFFFFFU;
				C->rhi = res>>64U;
			} break;
		case 29: // DMULTU
			{
				unsigned __int128 in_a = (unsigned __int128)(uint64_t)C->regs[rs];
				unsigned __int128 in_b = (unsigned __int128)(uint64_t)C->regs[rt];
				unsigned __int128 res = in_a * in_b;
				C->rlo = res&0xFFFFFFFFFFFFFFFFU;
				C->rhi = res>>64U;
#if 0
				printf("DMULTU %016llX * %016llX = %016llX : %016llX\n",
					in_a,
					in_b,
					C->rhi,
					C->rlo);
#endif
			} break;
		case 30: // DDIV
			// TODO: find result of zero division on THIS particular MIPS version
			// TODO: find out how the hell the remainder is calculated
			{
				int64_t in_a = (int64_t)C->regs[rs];
				int64_t in_b = (int64_t)C->regs[rt];
				if(in_b == 0) {
					C->rlo = (SREG)(in_a >= 0
						? (UREG)(SREG)-1
						: (UREG)1);
					C->rhi = (SREG)in_a;
				} else {
					int64_t quo = in_a / in_b;
					int64_t rem = in_a % in_b;
					C->rlo = (SREG)quo;
					C->rhi = (SREG)rem;
				}
			} break;
		case 31: // DDIVU
			{
				uint64_t in_a = (uint64_t)C->regs[rs];
				uint64_t in_b = (uint64_t)C->regs[rt];
				if(in_b == 0) {
					C->rlo = (SREG)-1;
					C->rhi = (SREG)(int64_t)in_a;
				} else {
					uint64_t quo = in_a / in_b;
					uint64_t rem = in_a % in_b;
					C->rlo = (SREG)(int64_t)quo;
					C->rhi = (SREG)(int64_t)rem;
				}
			} break;
#endif

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

		// D(ADD|SUB)U?
		case 44: // DADD
#ifndef MIPS_IS_RSP
			{
				uint64_t s1 = C->regs[rs];
				uint64_t s2 = C->regs[rt];
				uint64_t dv = s1 + s2;

				// Trap on signed overflow
				// Which is *always* horrible to detect
				// if sgn(s1)==sgn(s2) && sgn(dv)!=sgn(s1)
				if((0x8000000000000000ULL & (dv^s1) & ~(s1^s2)) != 0) {
					MIPSXNAME(_throw_exception)(C, op_pc, MER_Ov, op_was_branch);
					return MER_Ov;
				}

				if(rd != 0) {
					C->regs[rd] = dv;
				}
			} break;
		case 45: // DADDU
			if(rd != 0) {
				C->regs[rd] = C->regs[rs] + C->regs[rt];
			} break;
		case 46: // DSUB
			{
				uint64_t s1 = C->regs[rs];
				uint64_t s2 = C->regs[rt];
				uint64_t dv = s1 - s2;

				// Trap on signed overflow
				// if sgn(s1)!=sgn(s2) && sgn(dv)!=sgn(s1)
				if((0x8000000000000000ULL & (dv^s1) & (s1^s2)) != 0) {
					MIPSXNAME(_throw_exception)(C, op_pc, MER_Ov, op_was_branch);
					return MER_Ov;
				}

				if(rd != 0) {
					C->regs[rd] = dv;
				}
			} break;
		case 47: // DSUBU
			if(rd != 0) {
				C->regs[rd] = C->regs[rs] - C->regs[rt];
			} break;
#endif

#ifndef MIPS_IS_RSP
		// DS(LL|RL|RA)
		case 56: // DSLL
			if(rd != 0) {
				C->regs[rd] = C->regs[rt] << shamt;
			} break;
		case 58: // DSRL
			if(rd != 0) {
				C->regs[rd] = (uint64_t)C->regs[rt] >> (uint64_t)shamt;
			} break;
		case 59: // DSRA
			if(rd != 0) {
				C->regs[rd] = ((int64_t)C->regs[rt]) >> (int64_t)shamt;
			} break;

		// DS(LL|RL|RA)32
		case 60: // DSLL32
			if(rd != 0) {
				C->regs[rd] = C->regs[rt] << (shamt+32);
			} break;
		case 62: // DSRL32
			if(rd != 0) {
				C->regs[rd] = (uint64_t)C->regs[rt] >> (uint64_t)(shamt+32);
			} break;
		case 63: // DSRA32
			if(rd != 0) {
				C->regs[rd] = ((int64_t)C->regs[rt]) >> (int64_t)(shamt+32);
			} break;
#endif

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
		case 16: // BLTZAL
			C->regs[31] = op_pc + 8;
		case 0: // BLTZ
			if(((SREG)C->regs[rs]) < 0) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} break;
		case 17: // BGEZAL
			C->regs[31] = op_pc + 8;
		case 1: // BGEZ
			if(((SREG)C->regs[rs]) >= 0) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} break;

#ifndef MIPS_IS_RSP
		case 18: // BLTZALL
			C->regs[31] = op_pc + 8;
		case 2: // BLTZL
			if(((SREG)C->regs[rs]) < 0) {
				C->pc = new_pc + (((SREG)(int16_t)op)<<2);
				C->pl0_is_branch = true;
			} else {
				new_op = 0;
			} break;
		case 19: // BGEZALL
			C->regs[31] = op_pc + 8;
		case 3: // BGEZL
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
	{
#include "ops-cop0.h"
	} break;

#ifdef MIPS_IS_RSP
	// COP2
	case 18: {
		uint32_t vd = ((op>> 6)&0x1F);
		uint32_t vs = ((op>>11)&0x1F);
		uint32_t vt = ((op>>16)&0x1F);
		uint32_t el = ((op>>21)&0xF);
		uint32_t elbeg = elparamtab[el][0];
		uint32_t elend = elparamtab[el][1];
		uint32_t elstep = elparamtab[el][2];
#include "ops-rsp.h"
	} break;


#else
	// COPx
	case 17: if((C->c0.n.sr & C0SR_CU(1)) == 0) {
		C->c0.n.cause &= ~0x30000000;
		C->c0.n.cause |= 1<<28;
		MIPSXNAME(_throw_exception)(C, op_pc, MER_CpU, op_was_branch);
		return MER_CpU;
	} else {
#ifdef MIPS_HAS_FPU
#include "ops-fpu.h"
#else
		printf("RI %2u %2u %08X -> %08X %d (COP1)\n"
			, rs, op&0x3F, op_pc, new_pc, op_was_branch
			);
		MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
		return MER_RI;
#endif
	} break;

	case 18: if((C->c0.n.sr & C0SR_CU(2)) == 0) {
		C->c0.n.cause &= ~0x30000000;
		C->c0.n.cause |= 2<<28;
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

#ifndef MIPS_IS_RSP
	// DADDIU?
	case 24: // DADDI
		{
			int64_t s1 = (int64_t)(C->regs[rs]);
			int64_t s2 = (int64_t)(int16_t)op;
			int64_t dv = s1 + s2;

			// Trap on signed overflow
			if((s2 < 0) ? dv > s1 : dv < s1) {
				MIPSXNAME(_throw_exception)(C, op_pc, MER_Ov, op_was_branch);
				return MER_Ov;
			}

			if(rt != 0) {
				C->regs[rt] = dv;
			}
		} break;
	case 25: // DADDIU
		if(rt != 0) {
			C->regs[rt] = C->regs[rs] + (SREG)(int16_t)op;
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
#ifndef MIPS_IS_RSP
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
#endif

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
#ifndef MIPS_IS_RSP
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
#endif

	// CACHE
	case 47: // CACHE
		// TODO!
		if(rt != 0 && rt != 1 && rt != 8 && rt != 9) {
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
			// LBV
			case 0: {
				rsp_debug_printf("LBV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				MIPSXNAME(_read8)(C, C->regs[v_base]+(v_offset<<0), &mdata);
				if((v_element&0x1) == 0) {
					C->c2.h[v_vt][v_element>>1] &= ~0xFF00;
					C->c2.h[v_vt][v_element>>1] |= mdata<<8;
				} else {
					C->c2.h[v_vt][v_element>>1] &= ~0x00FF;
					C->c2.h[v_vt][v_element>>1] |= mdata&0xFF;
				}
			} break;

			// LSV
			case 1: {
				rsp_debug_printf("LSV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert((v_element&0x1) == 0);
				MIPSXNAME(_read8)(C, C->regs[v_base]+(v_offset<<1), &mdata);
				C->c2.h[v_vt][v_element>>1] = mdata;
			} break;

			// LDV
			case 3: {
				rsp_debug_printf("LDV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert((v_element&0x7) == 0);
				MIPSXNAME(_read8)(C, C->regs[v_base]+(v_offset<<1), &mdata);
				for(int i = 0; i < 4; i++) {
					MIPSXNAME(_read16)(C, C->regs[v_base]+(i<<1)+(v_offset<<3), &mdata);
					C->c2.h[v_vt][i+(v_element>>1)] = mdata;
				}
			} break;

			// LQV
			case 4: {
				rsp_debug_printf("LQV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert(v_element == 0);
				// FIXME this is wrong
				for(int i = 0; i < 8; i++) {
					MIPSXNAME(_read16)(C, C->regs[v_base]+(i<<1)+(v_offset<<4), &mdata);
					C->c2.h[v_vt][i] = mdata;
				}
			} break;

			// LPV
			case 6: {
				rsp_debug_printf("LPV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert(v_element == 0);
				for(int i = 0; i < 8; i++) {
					MIPSXNAME(_read8)(C, C->regs[v_base]+(i<<1)+(v_offset<<4), &mdata);
					C->c2.h[v_vt][i] = mdata<<8;
				}
			} break;

			// LUV
			case 7: {
				rsp_debug_printf("LUV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert(v_element == 0);
				for(int i = 0; i < 8; i++) {
					MIPSXNAME(_read8)(C, C->regs[v_base]+(i<<1)+(v_offset<<4), &mdata);
					C->c2.h[v_vt][i] = (mdata&0xFF)<<7;
				}
			} break;

			// LTV
			case 11: {
				rsp_debug_printf("LTV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert((v_element&0x1) == 0);
				for(int i = 0; i < 8; i++) {
					MIPSXNAME(_read16)(C,
						C->regs[v_base]
						+(i<<1)
						+(v_offset<<4),
						&mdata);
					C->c2.h[(v_vt&0x18)|i][(i-(v_element>>1))&0x7] = mdata;
				}
			} break;

			default:
				printf("RSP load op %2d %2d %2d %2d %2d\n",
					v_base, v_vt, v_opcode, v_element, v_offset);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		}
	} break;

	// RSP vector unit op (normally SWC2)
	case 58: {
		uint32_t v_base = (op>>21)&0x1F;
		uint32_t v_vt = (op>>16)&0x1F;
		uint32_t v_opcode = (op>>11)&0x1F;
		uint32_t v_element = (op>>7)&0xF;
		uint32_t v_offset = (uint32_t)(((int32_t)(op<<25))>>25);
		switch(v_opcode)
		{
			// SSV
			case 1: {
				rsp_debug_printf("SSV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert((v_element&0x1) == 0);
				MIPSXNAME(_write16)(C,
					C->regs[v_base]
					+(v_offset<<1),
					C->c2.h[v_vt][((v_element>>1)&0x7)]
				);
			} break;

			// SDV
			case 3: {
				rsp_debug_printf("SDV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert((v_element&0x7) == 0);
				for(int i = 0; i < 4; i++) {
					MIPSXNAME(_write16)(C,
						C->regs[v_base]
						+(i<<1)
						+(v_offset<<3),
						C->c2.h[v_vt][((i+(v_element>>1))&0x7)]
					);
				}
			} break;

			// SQV
			case 4: {
				// FIXME this is wrong
				rsp_debug_printf("SQV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert(v_element == 0);
				for(int i = 0; i < 8; i++) {
					MIPSXNAME(_write16)(C, C->regs[v_base]+(i<<1)+(v_offset<<4), C->c2.h[v_vt][i]);
				}
			} break;

			// SWV
			case 10: {
				rsp_debug_printf("SWV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert((v_element&0x1) == 0);
				for(int i = 0; i < 8; i++) {
					MIPSXNAME(_write16)(C,
						C->regs[v_base]
						+(i<<1)
						+(v_offset<<4),
						C->c2.h[v_vt][((i+(v_element>>1))&0x7)]
					);
				}
			} break;

			// STV
			case 11: {
				rsp_debug_printf("STV %2u %2u %2u %2u %d\n", v_base, v_vt, v_opcode, v_element, v_offset);
				assert((v_element&0x1) == 0);
				for(int i = 0; i < 8; i++) {
					MIPSXNAME(_write16)(C,
						C->regs[v_base]
						+(i<<1)
						+(v_offset<<4),
						C->c2.h[
							(v_vt&0x18)
							|((i+(v_element>>1))&0x7)
						][i]);
				}
			} break;

			default:
				printf("RSP store op %2d %2d %2d %2d %2d\n",
					v_base, v_vt, v_opcode, v_element, v_offset);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		}
	} break;
#endif

#ifdef MIPS_HAS_FPU
	// LWC1
	case 49: if((C->c0.n.sr & C0SR_CU(1)) == 0) {
		C->c0.n.cause &= ~0x30000000;
		C->c0.n.cause |= 1<<28;
		MIPSXNAME(_throw_exception)(C, op_pc, MER_CpU, op_was_branch);
		return MER_CpU;
	} else {
		e = MIPSXNAME(_read32)(C, C->regs[rs]+(SREG)(int16_t)op, &mdata);
		if(e != MER_NONE) {
			MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
			return e;
		}
		C->c1.di[rt]  = ((SREG)(int32_t)mdata); // ?
	} break;

	// LDC1
	case 53: if((C->c0.n.sr & C0SR_CU(1)) == 0) {
		C->c0.n.cause &= ~0x30000000;
		C->c0.n.cause |= 1<<28;
		MIPSXNAME(_throw_exception)(C, op_pc, MER_CpU, op_was_branch);
		return MER_CpU;
	} else {
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

		C->c1.di[rt]  = ((SREG)(int32_t)mdata0)<<32;
		C->c1.di[rt] |= ((SREG)(UREG)(uint32_t)mdata1);
	} break;

	// SWC1
	case 57: if((C->c0.n.sr & C0SR_CU(1)) == 0) {
		C->c0.n.cause &= ~0x30000000;
		C->c0.n.cause |= 1<<28;
		MIPSXNAME(_throw_exception)(C, op_pc, MER_CpU, op_was_branch);
		return MER_CpU;
	} else {
		e = MIPSXNAME(_write32)(C, C->regs[rs]+(SREG)(int16_t)op, C->c1.di[rt]);
		if(e != MER_NONE) {
			MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
			return e;
		}
	} break;

	// SDC1
	case 61: if((C->c0.n.sr & C0SR_CU(1)) == 0) {
		C->c0.n.cause &= ~0x30000000;
		C->c0.n.cause |= 1<<28;
		MIPSXNAME(_throw_exception)(C, op_pc, MER_CpU, op_was_branch);
		return MER_CpU;
	} else {
		// FIXME this is probably wrong
		e = MIPSXNAME(_write32)(C, C->regs[rs]+(SREG)(int16_t)op, C->c1.di[rt]>>32);
		if(e != MER_NONE) {
			MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
			return e;
		}
		e = MIPSXNAME(_write32)(C, C->regs[rs]+4+(SREG)(int16_t)op, C->c1.di[rt]);
		if(e != MER_NONE) {
			MIPSXNAME(_throw_exception)(C, op_pc, e, op_was_branch);
			return e;
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


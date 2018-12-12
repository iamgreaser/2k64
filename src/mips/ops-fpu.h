// TODO: set cause and throw exception!
#define TRAP_ON_NAN_FS_FT_S { \
	if(isnan(REG_SF(fs)) || isnan(REG_SF(ft))) { \
		printf("TODO: TRAP ON NaN\n"); \
		fflush(stdout); \
		if(0) abort(); \
	} \
}

#define TRAP_ON_NAN_FS_FT_D { \
	if(isnan(REG_DF(fs)) || isnan(REG_DF(ft))) { \
		printf("TODO: TRAP ON NaN\n"); \
		fflush(stdout); \
		if(0) abort(); \
	} \
}

#define REG_SF(r) C->c1.sf[(r)&~0x1][(r)&0x1]
#define REG_DF(r) C->c1.df[(r)]

{
	uint32_t fd = (op>>6)&0x1F;
	uint32_t fs = (op>>11)&0x1F;
	uint32_t ft = (op>>16)&0x1F;
	switch(rs)
	{
		// MFC1
		case 0:
			//printf("MFC1 %d %016llX\n", rd, C->c1.di[rd]);
			C->regs[rt] = (SREG)(int32_t)C->c1.di[rd];
			break;

		// CFC1
		case 2:
			break;

		// MTC1
		case 4:
			//printf("MTC1 %d %016llX\n", rd, C->regs[rt]);
			C->c1.di[rd] = (SREG)(int32_t)C->regs[rt];
			break;

		// CTC1
		case 6:
			break;

		// BC1[FT]L?
		case 8: switch(ft) {
			// BC1F
			case 0:
				if(!C->coc1) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} break;
			// BC1T
			case 1:
				if(C->coc1) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} break;
			// BC1FL
			case 2:
				if(!C->coc1) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} else {
					new_op = 0;
				} break;
			// BC1TL
			case 3:
				if(C->coc1) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
				} else {
					new_op = 0;
				} break;

			default:
				printf("RI BC1 %2u %08X -> %08X %d (COP1)\n"
					, ft, op_pc, new_pc, op_was_branch
					);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		} break;

		// S instructions
		case 16: switch(op&0x3F) {
			case 0: // ADD.S
				REG_SF(fd) = REG_SF(fs) + REG_SF(ft);
				break;

			case 1: // SUB.S
				REG_SF(fd) = REG_SF(fs) - REG_SF(ft);
				break;

			case 2: // MUL.S
				REG_SF(fd) = REG_SF(fs) * REG_SF(ft);
				break;

			case 3: // DIV.S
				REG_SF(fd) = REG_SF(fs) / REG_SF(ft);
				break;

			case 4: // SQRT.S
				REG_SF(fd) = sqrtf(REG_SF(fs));
				break;

			case 5: // ABS.S
				REG_SF(fd) = fabsf(REG_SF(fs));
				break;

			case 6: // MOV.S
				REG_SF(fd) = REG_SF(fs);
				break;

			case 7: // NEG.S
				REG_SF(fd) = -REG_SF(fs);
				break;

			case 8: // ROUND.L.S
				C->c1.di[fd] = n64_roundf(REG_SF(fs));
				break;

			case 9: // TRUNC.L.S
				C->c1.di[fd] = truncf(REG_SF(fs));
				break;

			case 10: // CEIL.L.S
				C->c1.di[fd] = ceilf(REG_SF(fs));
				break;

			case 11: // FLOOR.L.S
				C->c1.di[fd] = floorf(REG_SF(fs));
				break;

			case 12: // ROUND.W.S
				C->c1.di[fd] = n64_roundf(REG_SF(fs));
				break;

			case 13: // TRUNC.W.S
				C->c1.di[fd] = truncf(REG_SF(fs));
				break;

			case 14: // CEIL.W.S
				C->c1.di[fd] = ceilf(REG_SF(fs));
				break;

			case 15: // FLOOR.W.S
				C->c1.di[fd] = floorf(REG_SF(fs));
				break;

			case 33: // CVT.D.S
				REG_DF(fd) = REG_SF(fs);
				break;

			case 36: // CVT.W.S
				C->c1.di[fd] = (SREG)(int32_t)REG_SF(fs);
				break;
			case 37: // CVT.L.S
				C->c1.di[fd] = REG_SF(fs);
				break;

			case 56: // C.SF.S
				TRAP_ON_NAN_FS_FT_S;
			case 48: // C.F.S
				C->coc1 = false;
				break;
			case 57: // C.NGLE.S
				TRAP_ON_NAN_FS_FT_S;
			case 49: // C.UN.S
				C->coc1 = (isnanf(REG_SF(fs)) || isnanf(REG_SF(ft)));
				break;
			case 58: // C.SEQ.S
				TRAP_ON_NAN_FS_FT_S;
			case 50: // C.EQ.S
				C->coc1 = (REG_SF(fs) == REG_SF(ft));
				break;
			case 59: // C.NGL.S
				TRAP_ON_NAN_FS_FT_S;
			case 51: // C.UEQ.S
				C->coc1 = (isnanf(REG_SF(fs)) || isnanf(REG_SF(ft))) || (REG_SF(fs) == REG_SF(ft));
				break;
			case 60: // C.LT.S
				TRAP_ON_NAN_FS_FT_S;
			case 52: // C.OLT.S
				C->coc1 = (REG_SF(fs) < REG_SF(ft));
				break;
			case 61: // C.NGE.S
				TRAP_ON_NAN_FS_FT_S;
			case 53: // C.ULT.S
				C->coc1 = (isnanf(REG_SF(fs)) || isnanf(REG_SF(ft))) || (REG_SF(fs) < REG_SF(ft));
				break;
			case 62: // C.LE.S
				TRAP_ON_NAN_FS_FT_S;
			case 54: // C.OLE.S
				C->coc1 = (REG_SF(fs) <= REG_SF(ft));
				break;
			case 63: // C.NGT.S
				TRAP_ON_NAN_FS_FT_S;
			case 55: // C.ULE.S
				C->coc1 = (isnanf(REG_SF(fs)) || isnanf(REG_SF(ft))) || (REG_SF(fs) <= REG_SF(ft));
				break;

			default:
				printf("RI %2u %2u %08X -> %08X %d (COP1)\n"
					, rs, op&0x3F, op_pc, new_pc, op_was_branch
					);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		} break;

		// D instructions
		case 17: switch(op&0x3F) {
			case 0: // ADD.D
				REG_DF(fd) = REG_DF(fs) + REG_DF(ft);
				break;

			case 1: // SUB.D
				REG_DF(fd) = REG_DF(fs) - REG_DF(ft);
				break;

			case 2: // MUL.D
				REG_DF(fd) = REG_DF(fs) * REG_DF(ft);
				break;

			case 3: // DIV.D
				REG_DF(fd) = REG_DF(fs) / REG_DF(ft);
				break;

			case 4: // SQRT.D
				REG_DF(fd) = sqrt(REG_DF(fs));
				break;

			case 5: // ABS.D
				REG_DF(fd) = fabs(REG_DF(fs));
				break;

			case 6: // MOV.D
				REG_DF(fd) = REG_DF(fs);
				break;

			case 7: // NEG.D
				REG_DF(fd) = -REG_DF(fs);
				break;

			case 8: // ROUND.L.D
				C->c1.di[fd] = n64_round(REG_DF(fs));
				break;

			case 9: // TRUNC.L.D
				C->c1.di[fd] = trunc(REG_DF(fs));
				break;

			case 10: // CEIL.L.D
				C->c1.di[fd] = ceil(REG_DF(fs));
				break;

			case 11: // FLOOR.L.D
				C->c1.di[fd] = floor(REG_DF(fs));
				break;

			case 12: // ROUND.W.D
				C->c1.di[fd] = n64_round(REG_DF(fs));
				break;

			case 13: // TRUNC.W.D
				C->c1.di[fd] = trunc(REG_DF(fs));
				break;

			case 14: // CEIL.W.D
				C->c1.di[fd] = ceil(REG_DF(fs));
				break;

			case 15: // FLOOR.W.D
				C->c1.di[fd] = floor(REG_DF(fs));
				break;

			case 32: // CVT.S.D
				REG_SF(fd) = REG_DF(fs);
				break;

			case 36: // CVT.W.D
				C->c1.di[fd] = (SREG)(int32_t)REG_DF(fs);
				break;
			case 37: // CVT.L.D
				C->c1.di[fd] = REG_DF(fs);
				break;

			case 56: // C.SF.D
				TRAP_ON_NAN_FS_FT_D;
			case 48: // C.F.D
				C->coc1 = false;
				break;
			case 57: // C.NGLE.D
				TRAP_ON_NAN_FS_FT_D;
			case 49: // C.UN.D
				C->coc1 = (isnan(REG_DF(fs)) || isnan(REG_DF(ft)));
				break;
			case 58: // C.SEQ.D
				TRAP_ON_NAN_FS_FT_D;
			case 50: // C.EQ.D
				C->coc1 = (REG_DF(fs) == REG_DF(ft));
				break;
			case 59: // C.NGL.D
				TRAP_ON_NAN_FS_FT_D;
			case 51: // C.UEQ.D
				C->coc1 = (isnan(REG_DF(fs)) || isnan(REG_DF(ft))) || (REG_DF(fs) == REG_DF(ft));
				break;
			case 60: // C.LT.D
				TRAP_ON_NAN_FS_FT_D;
			case 52: // C.OLT.D
				C->coc1 = (REG_DF(fs) < REG_DF(ft));
				break;
			case 61: // C.NGE.D
				TRAP_ON_NAN_FS_FT_D;
			case 53: // C.ULT.D
				C->coc1 = (isnan(REG_DF(fs)) || isnan(REG_DF(ft))) || (REG_DF(fs) < REG_DF(ft));
				break;
			case 62: // C.LE.D
				TRAP_ON_NAN_FS_FT_D;
			case 54: // C.OLE.D
				C->coc1 = (REG_DF(fs) <= REG_DF(ft));
				break;
			case 63: // C.NGT.D
				TRAP_ON_NAN_FS_FT_D;
			case 55: // C.ULE.D
				C->coc1 = (isnan(REG_DF(fs)) || isnan(REG_DF(ft))) || (REG_DF(fs) <= REG_DF(ft));
				break;

			default:
				printf("RI %2u %2u %08X -> %08X %d (COP1)\n"
					, rs, op&0x3F, op_pc, new_pc, op_was_branch
					);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		} break;

		// W instructions
		case 20: switch(op&0x3F) {
			case 32: // CVT.S.W
				REG_SF(fd) = (SREG)(int32_t)C->c1.di[fs];
				break;
			case 33: // CVT.D.W
				REG_DF(fd) = (SREG)(int32_t)C->c1.di[fs];
				break;
		} break;

		// L instructions
		case 21: switch(op&0x3F) {
			case 32: // CVT.S.L
				REG_SF(fd) = (SREG)C->c1.di[fs];
				break;
			case 33: // CVT.D.L
				REG_DF(fd) = (SREG)C->c1.di[fs];
				break;
		} break;

		default:
			printf("RI %2u %2u %08X -> %08X %d (COP1)\n"
				, rs, op&0x3F, op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;
	}
}

#undef REG_SF
#undef REG_DF


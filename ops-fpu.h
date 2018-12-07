// TODO: set cause and throw exception!
#define TRAP_ON_NAN_FS_FT_S { \
	if(isnan(C->c1.sf[fs][0]) || isnan(C->c1.sf[ft][0])) { \
		printf("TODO: TRAP ON NaN\n"); \
		fflush(stdout); \
		abort(); \
	} \
}

#define TRAP_ON_NAN_FS_FT_D { \
	if(isnan(C->c1.df[fs]) || isnan(C->c1.df[ft])) { \
		printf("TODO: TRAP ON NaN\n"); \
		fflush(stdout); \
		abort(); \
	} \
}

{
	uint32_t fd = (op>>6)&0x1F;
	uint32_t fs = (op>>11)&0x1F;
	uint32_t ft = (op>>16)&0x1F;
	switch(rs)
	{
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
			case 1:
				if(C->coc1) {
					C->pc = new_pc + (((SREG)(int16_t)op)<<2);
					C->pl0_is_branch = true;
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
				C->c1.sf[fd][0] = C->c1.sf[fs][0] + C->c1.sf[ft][0];
				break;

			case 1: // SUB.S
				C->c1.sf[fd][0] = C->c1.sf[fs][0] - C->c1.sf[ft][0];
				break;

			case 2: // MUL.S
				C->c1.sf[fd][0] = C->c1.sf[fs][0] * C->c1.sf[ft][0];
				break;

			case 3: // DIV.S
				C->c1.sf[fd][0] = C->c1.sf[fs][0] / C->c1.sf[ft][0];
				break;

			case 4: // SQRT.S
				C->c1.sf[fd][0] = sqrtf(C->c1.sf[fs][0]);
				break;

			case 5: // ABS.S
				C->c1.sf[fd][0] = fabsf(C->c1.sf[fs][0]);
				break;

			case 6: // MOV.S
				C->c1.sf[fd][0] = C->c1.sf[fs][0];
				break;

			case 7: // NEG.S
				C->c1.sf[fd][0] = -C->c1.sf[fs][0];
				break;

			case 8: // ROUND.L.S
				C->c1.di[fd] = n64_roundf(C->c1.sf[fs][0]);
				break;

			case 9: // TRUNC.L.S
				C->c1.di[fd] = truncf(C->c1.sf[fs][0]);
				break;

			case 10: // CEIL.L.S
				C->c1.di[fd] = ceilf(C->c1.sf[fs][0]);
				break;

			case 11: // FLOOR.L.S
				C->c1.di[fd] = floorf(C->c1.sf[fs][0]);
				break;

			case 12: // ROUND.W.S
				C->c1.di[fd] = n64_roundf(C->c1.sf[fs][0]);
				break;

			case 13: // TRUNC.W.S
				C->c1.di[fd] = truncf(C->c1.sf[fs][0]);
				break;

			case 14: // CEIL.W.S
				C->c1.di[fd] = ceilf(C->c1.sf[fs][0]);
				break;

			case 15: // FLOOR.W.S
				C->c1.di[fd] = floorf(C->c1.sf[fs][0]);
				break;

			case 33: // CVT.D.S
				C->c1.df[fd] = C->c1.sf[fs][0];
				break;

			case 36: // CVT.W.S
				C->c1.di[fd] = (SREG)(int32_t)C->c1.sf[fs][0];
				break;
			case 37: // CVT.L.S
				C->c1.di[fd] = C->c1.sf[fs][0];
				break;

			case 56: // C.SF.S
				TRAP_ON_NAN_FS_FT_S;
			case 48: // C.F.S
				C->coc1 = false;
				break;
			case 57: // C.NGLE.S
				TRAP_ON_NAN_FS_FT_S;
			case 49: // C.UN.S
				C->coc1 = (isnanf(C->c1.sf[fs][0]) || isnanf(C->c1.sf[ft][0]));
				break;
			case 58: // C.SEQ.S
				TRAP_ON_NAN_FS_FT_S;
			case 50: // C.EQ.S
				C->coc1 = (C->c1.sf[fs][0] == C->c1.sf[ft][0]);
				break;
			case 59: // C.NGL.S
				TRAP_ON_NAN_FS_FT_S;
			case 51: // C.UEQ.S
				C->coc1 = (isnanf(C->c1.sf[fs][0]) || isnanf(C->c1.sf[ft][0])) || (C->c1.sf[fs][0] == C->c1.sf[ft][0]);
				break;
			case 60: // C.LT.S
				TRAP_ON_NAN_FS_FT_S;
			case 52: // C.OLT.S
				C->coc1 = (C->c1.sf[fs][0] < C->c1.sf[ft][0]);
				break;
			case 61: // C.NGE.S
				TRAP_ON_NAN_FS_FT_S;
			case 53: // C.ULT.S
				C->coc1 = (isnanf(C->c1.sf[fs][0]) || isnanf(C->c1.sf[ft][0])) || (C->c1.sf[fs][0] < C->c1.sf[ft][0]);
				break;
			case 62: // C.LE.S
				TRAP_ON_NAN_FS_FT_S;
			case 54: // C.OLE.S
				C->coc1 = (C->c1.sf[fs][0] <= C->c1.sf[ft][0]);
				break;
			case 63: // C.NGT.S
				TRAP_ON_NAN_FS_FT_S;
			case 55: // C.ULE.S
				C->coc1 = (isnanf(C->c1.sf[fs][0]) || isnanf(C->c1.sf[ft][0])) || (C->c1.sf[fs][0] <= C->c1.sf[ft][0]);
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
				C->c1.df[fd] = C->c1.df[fs] + C->c1.df[ft];
				break;

			case 1: // SUB.D
				C->c1.df[fd] = C->c1.df[fs] - C->c1.df[ft];
				break;

			case 2: // MUL.D
				C->c1.df[fd] = C->c1.df[fs] * C->c1.df[ft];
				break;

			case 3: // DIV.D
				C->c1.df[fd] = C->c1.df[fs] / C->c1.df[ft];
				break;

			case 4: // SQRT.D
				C->c1.df[fd] = sqrt(C->c1.df[fs]);
				break;

			case 5: // ABS.D
				C->c1.df[fd] = fabs(C->c1.df[fs]);
				break;

			case 6: // MOV.D
				C->c1.df[fd] = C->c1.df[fs];
				break;

			case 7: // NEG.D
				C->c1.df[fd] = -C->c1.df[fs];
				break;

			case 8: // ROUND.L.D
				C->c1.di[fd] = n64_round(C->c1.df[fs]);
				break;

			case 9: // TRUNC.L.D
				C->c1.di[fd] = trunc(C->c1.df[fs]);
				break;

			case 10: // CEIL.L.D
				C->c1.di[fd] = ceil(C->c1.df[fs]);
				break;

			case 11: // FLOOR.L.D
				C->c1.di[fd] = floor(C->c1.df[fs]);
				break;

			case 12: // ROUND.W.D
				C->c1.di[fd] = n64_round(C->c1.df[fs]);
				break;

			case 13: // TRUNC.W.D
				C->c1.di[fd] = trunc(C->c1.df[fs]);
				break;

			case 14: // CEIL.W.D
				C->c1.di[fd] = ceil(C->c1.df[fs]);
				break;

			case 15: // FLOOR.W.D
				C->c1.di[fd] = floor(C->c1.df[fs]);
				break;

			case 32: // CVT.S.D
				C->c1.sf[fd][0] = C->c1.df[fs];
				break;

			case 36: // CVT.W.D
				C->c1.di[fd] = (SREG)(int32_t)C->c1.df[fs];
				break;
			case 37: // CVT.L.D
				C->c1.di[fd] = C->c1.df[fs];
				break;

			case 56: // C.SF.D
				TRAP_ON_NAN_FS_FT_D;
			case 48: // C.F.D
				C->coc1 = false;
				break;
			case 57: // C.NGLE.D
				TRAP_ON_NAN_FS_FT_D;
			case 49: // C.UN.D
				C->coc1 = (isnan(C->c1.df[fs]) || isnan(C->c1.df[ft]));
				break;
			case 58: // C.SEQ.D
				TRAP_ON_NAN_FS_FT_D;
			case 50: // C.EQ.D
				C->coc1 = (C->c1.df[fs] == C->c1.df[ft]);
				break;
			case 59: // C.NGL.D
				TRAP_ON_NAN_FS_FT_D;
			case 51: // C.UEQ.D
				C->coc1 = (isnan(C->c1.df[fs]) || isnan(C->c1.df[ft])) || (C->c1.df[fs] == C->c1.df[ft]);
				break;
			case 60: // C.LT.D
				TRAP_ON_NAN_FS_FT_D;
			case 52: // C.OLT.D
				C->coc1 = (C->c1.df[fs] < C->c1.df[ft]);
				break;
			case 61: // C.NGE.D
				TRAP_ON_NAN_FS_FT_D;
			case 53: // C.ULT.D
				C->coc1 = (isnan(C->c1.df[fs]) || isnan(C->c1.df[ft])) || (C->c1.df[fs] < C->c1.df[ft]);
				break;
			case 62: // C.LE.D
				TRAP_ON_NAN_FS_FT_D;
			case 54: // C.OLE.D
				C->coc1 = (C->c1.df[fs] <= C->c1.df[ft]);
				break;
			case 63: // C.NGT.D
				TRAP_ON_NAN_FS_FT_D;
			case 55: // C.ULE.D
				C->coc1 = (isnan(C->c1.df[fs]) || isnan(C->c1.df[ft])) || (C->c1.df[fs] <= C->c1.df[ft]);
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
				C->c1.sf[fd][0] = (SREG)(int32_t)C->c1.di[fs];
				break;
			case 33: // CVT.D.W
				C->c1.df[fd] = (SREG)(int32_t)C->c1.di[fs];
				break;
		} break;

		// L instructions
		case 21: switch(op&0x3F) {
			case 32: // CVT.S.L
				C->c1.sf[fd][0] = (SREG)C->c1.di[fs];
				break;
			case 33: // CVT.D.L
				C->c1.df[fd] = (SREG)C->c1.di[fs];
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

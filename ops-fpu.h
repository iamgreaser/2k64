{
	uint32_t fd = (op>>6)&0x1F;
	uint32_t fs = (op>>11)&0x1F;
	uint32_t ft = (op>>16)&0x1F;
	switch(rs)
	{
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

			default:
				printf("RI %2u %2u %08X -> %08X %d (COP1)\n"
					, rs, op&0x3F, op_pc, new_pc, op_was_branch
					);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		} break;

		default:
			printf("RI %2u %2u %08X -> %08X %d (COP1)\n"
				, rs, op&0x3F, op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;
	}
}

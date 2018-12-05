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

	case 2: // CFCz
	switch(rd) {
		case 0:
			C->regs[rt] = C->cc2.n.vco;
			break;
		case 1:
			C->regs[rt] = C->cc2.n.vcc;
			break;
		case 2:
			C->regs[rt] = C->cc2.n.vce;
			break;
		default:
			printf("RI CFC2 %2u %08X -> %08X %d (COP2)\n"
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


	case 6: // CTCz
	switch(rd) {
		default:
			printf("RI CTC2 %2u %08X -> %08X %d (COP2)\n"
				, rd, op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;
	} break;

	case 16: case 17: case 18: case 19:
	case 20: case 21: case 22: case 23:
	case 24: case 25: case 26: case 27:
	case 28: case 29: case 30: case 31:
	{
		switch(op&0x3F) {

			case 16: // VADD
				printf("VADD %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int32_t r = (int32_t)(int16_t)C->c2.h[vs][i] + (int32_t)(int16_t)C->c2.h[vt][i] + ((C->cc2.n.vco>>i)&1);
					C->c2acc[0][i] = r&0xFFFF;
					if(r > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(r < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)r;
					}
				}
				C->cc2.n.vco = 0;
				break;

			case 20: // VADDC
				printf("VADDC %2u %2u %2u %X\n", vd, vs, vt, el);
				C->cc2.n.vco = 0;
				for(int i = 0; i < 8; i++) {
					uint32_t r = C->c2.h[vs][i] + C->c2.h[vt][i];
					C->c2acc[0][i] = (uint16_t)(uint32_t)r;
					if(r >= 0x10000) {
						C->cc2.n.vco |=  (1<<i);
					}
					C->c2.h[vd][i] = (uint16_t)r;
				}
				break;

			case 29: // VSAR
				printf("VSAR %2u %2u %2u %X\n", vd, vs, vt, el);
				switch(el)
				{

					case 0x8:
						for(int i = 0; i < 8; i++) {
							C->c2.h[vd][i] = C->c2acc[2][i];
							C->c2acc[2][i] = C->c2.h[vs][i];
						}
						break;
					case 0x9:
						for(int i = 0; i < 8; i++) {
							C->c2.h[vd][i] = C->c2acc[1][i];
							C->c2acc[1][i] = C->c2.h[vs][i];
						}
						break;
					case 0xA:
						for(int i = 0; i < 8; i++) {
							C->c2.h[vd][i] = C->c2acc[0][i];
							C->c2acc[0][i] = C->c2.h[vs][i];
						}
						break;

					default:
						printf("UNHANDLED VSAR EL CASE %2u %2u %2u %X\n", vd, vs, vt, el);
						fflush(stdout);
						abort();
						break;
				}
				break;

			default:
				printf("RI op %2u %08X -> %08X %d (COP2)\n"
					, op&0x3F, op_pc, new_pc, op_was_branch
					);
				MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
				return MER_RI;
		}
	} break;

	default:
		printf("RI %2u %2u %08X -> %08X %d (COP2)\n"
			, rs, op&0x3F, op_pc, new_pc, op_was_branch
			);
		MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
		return MER_RI;
}


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
					int j = elparamtab[el][i];
					int32_t r = (int32_t)(int16_t)C->c2.h[vs][i] + (int32_t)(int16_t)C->c2.h[vt][j] + ((C->cc2.n.vco>>i)&1);
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

			case 17: // VSUB
				printf("VSUB %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = (int32_t)(int16_t)C->c2.h[vs][i] - (int32_t)(int16_t)C->c2.h[vt][j] - ((C->cc2.n.vco>>(i+8))&1);
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

			case 19: // VABS
				printf("VABS %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = (int16_t)C->c2.h[vt][j];
					int32_t s = (int16_t)C->c2.h[vs][i];
					if(s < 0) {
						r = -r;
					} else if(s == 0) {
						r = 0;
					}
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 20: // VADDC
				printf("VADDC %2u %2u %2u %X\n", vd, vs, vt, el);
				C->cc2.n.vco = 0;
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					uint32_t r = C->c2.h[vs][i] + C->c2.h[vt][j];
					C->c2acc[0][i] = (uint16_t)(uint32_t)r;
					if(r >= 0x10000) {
						C->cc2.n.vco |=  (1<<i);
					}
					C->c2.h[vd][i] = (uint16_t)r;
				}
				break;

			case 21: // VSUBC
				printf("VSUBC %2u %2u %2u %X\n", vd, vs, vt, el);
				C->cc2.n.vco = 0;
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i] - C->c2.h[vt][j];
					C->c2acc[0][i] = (uint16_t)(uint32_t)r;
					if(r < 0) {
						C->cc2.n.vco |= (0x101<<i);
					} else if(r > 0) {
						C->cc2.n.vco |= (0x100<<i);
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

			case 32: // VLT
				printf("VLT %2u %2u %2u %X\n", vd, vs, vt, el);
				C->cc2.n.vcc = 0;
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i];
					if((int16_t)C->c2.h[vs][i] < (int16_t)C->c2.h[vt][j]) {
						C->cc2.n.vcc |= 0x001<<i;
					} else {
						r = C->c2.h[vt][j];
					}
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				C->cc2.n.vco = 0;
				C->cc2.n.vce = 0;
				break;

			case 33: // VEQ
				printf("VEQ %2u %2u %2u %X\n", vd, vs, vt, el);
				C->cc2.n.vcc &= ~0xFF;
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i];
					if(C->c2.h[vs][i] == C->c2.h[vt][j]) {
						C->cc2.n.vcc |= 0x001<<i;
					} else {
						r = C->c2.h[vt][j];
					}
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				C->cc2.n.vco = 0;
				C->cc2.n.vce = 0;
				break;

			case 34: // VNE
				printf("VNE %2u %2u %2u %X\n", vd, vs, vt, el);
				C->cc2.n.vcc = 0;
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i];
					if(C->c2.h[vs][i] != C->c2.h[vt][j]) {
						C->cc2.n.vcc |= 0x001<<i;
					} else {
						r = C->c2.h[vt][j];
					}
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				C->cc2.n.vco = 0;
				C->cc2.n.vce = 0;
				break;

			case 35: // VGE
				printf("VGE %2u %2u %2u %X\n", vd, vs, vt, el);
				C->cc2.n.vcc = 0;
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i];
					if((int16_t)C->c2.h[vs][i] >= (int16_t)C->c2.h[vt][j]) {
						C->cc2.n.vcc |= 0x001<<i;
					} else {
						r = C->c2.h[vt][j];
					}
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				C->cc2.n.vco = 0;
				C->cc2.n.vce = 0;
				break;

			case 40: // VAND
				printf("VAND %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i] & C->c2.h[vt][j];
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 41: // VNAND
				printf("VNAND %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = ~(C->c2.h[vs][i] & C->c2.h[vt][j]);
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 42: // VOR
				printf("VOR %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i] | C->c2.h[vt][j];
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 43: // VNOR
				printf("VNOR %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = ~(C->c2.h[vs][i] | C->c2.h[vt][j]);
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 44: // VXOR
				printf("VXOR %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i] ^ C->c2.h[vt][j];
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 45: // VNXOR
				printf("VNXOR %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = ~(C->c2.h[vs][i] ^ C->c2.h[vt][j]);
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 51: // VMOV
				printf("VMOV %2u[%2u] %2u[%2u]\n", vd, vs, vt, el);
				{
					uint16_t r = (uint16_t)C->c2.h[vt][el&0x7];
					C->c2.h[vd][vs&0x7] = r;
					for(int i = 0; i < 8; i++) {
						C->c2acc[0][i] = C->c2.h[vt][i];
					}
				}
				break;

			case 55:
				// VNOP
				printf("VNOP\n");
				break;

			case 63:
				// VNULL
				printf("VNULL\n");
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


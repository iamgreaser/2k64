#define GET_ACC_FOR_LANE(i) (((uint16_t)C->c2acc[0][i])|(((uint32_t)(uint16_t)C->c2acc[1][i])<<16)|(((int64_t)(int16_t)C->c2acc[2][i])<<32))
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

			case 0: // VMULF
				rsp_debug_printf("VMULF %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t r = (int64_t)(int16_t)C->c2.h[vs][i] * (int64_t)(int16_t)C->c2.h[vt][j];
					r <<= 1;
					r += 0x8000;
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rm > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rh < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rm;
					}
				}
				break;

			case 1: // VMULU
				rsp_debug_printf("VMULU %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t r = (int64_t)(int16_t)C->c2.h[vs][i] * (int64_t)(int16_t)C->c2.h[vt][j];
					r <<= 1;
					r += 0x8000;
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rm > 0xFFFF) {
						C->c2acc[2][i] = 0xFFFF;
						C->c2.h[vd][i] = 0x0000;
					} else if(rm < 0x0000) {
						C->c2acc[2][i] = 0xFFFF;
						C->c2.h[vd][i] = 0x0000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rm;
					}
				}
				break;

			case 4: // VMUDL
				rsp_debug_printf("VMUDL %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t r = (uint64_t)(uint16_t)C->c2.h[vs][i] * (uint64_t)(uint16_t)C->c2.h[vt][j];
					r >>= 16;
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rl > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rl < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rl;
					}
				}
				break;

			case 5: // VMUDM
				rsp_debug_printf("VMUDM %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t r = (int64_t)(int16_t)C->c2.h[vs][i] * (uint64_t)(uint16_t)C->c2.h[vt][j];
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rm > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rm < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rm;
					}
				}
				break;

			case 6: // VMUDN
				rsp_debug_printf("VMUDN %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t r = (uint64_t)(uint16_t)C->c2.h[vs][i] * (int64_t)(int16_t)C->c2.h[vt][j];
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rl > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rl < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rl;
					}
				}
				break;

			case 7: // VMUDH
				rsp_debug_printf("VMUDH %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t r = (int64_t)(int16_t)C->c2.h[vs][i] * (int64_t)(int16_t)C->c2.h[vt][j];
					r <<= 16;
					int32_t rl = r;
					int32_t rm = (r>>16);
					int32_t rh = (r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rm > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rm < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rm;
					}
				}
				break;

			case 8: // VMACF
				rsp_debug_printf("VMACF %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t acc = GET_ACC_FOR_LANE(i);
					int64_t r = (int64_t)(int16_t)C->c2.h[vs][i] * (int64_t)(int16_t)C->c2.h[vt][j];
					r <<= 1;
					r += acc;
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rm > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rm < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rm;
					}
				}
				break;

			case 9: // VMACU
				rsp_debug_printf("VMACU %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t acc = GET_ACC_FOR_LANE(i);
					int64_t r = (int64_t)(int16_t)C->c2.h[vs][i] * (int64_t)(int16_t)C->c2.h[vt][j];
					r <<= 1;
					r += acc;
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rm > 0xFFFF) {
						C->c2.h[vd][i] = 0xFFFF;
					} else if(rm < 0x0000) {
						C->c2.h[vd][i] = 0xFFFF;
					} else {
						C->c2.h[vd][i] = (uint16_t)rm;
					}
				}
				break;

			case 12: // VMADL
				rsp_debug_printf("VMADL %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t acc = GET_ACC_FOR_LANE(i);
					int64_t r = (uint64_t)(uint16_t)C->c2.h[vs][i] * (uint64_t)(uint16_t)C->c2.h[vt][j];
					r >>= 16;
					r += acc;
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rl > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rl < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rl;
					}
				}
				break;

			case 13: // VMADM
				rsp_debug_printf("VMADM %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t acc = GET_ACC_FOR_LANE(i);
					int64_t r = (int64_t)(int16_t)C->c2.h[vs][i] * (uint64_t)(uint16_t)C->c2.h[vt][j];
					r += acc;
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rm > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rm < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rm;
					}
				}
				break;

			case 14: // VMADN
				rsp_debug_printf("VMADN %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t acc = GET_ACC_FOR_LANE(i);
					int64_t r = (uint64_t)(uint16_t)C->c2.h[vs][i] * (int64_t)(int16_t)C->c2.h[vt][j];
					r += acc;
					int32_t rl = (int32_t)(int16_t)r;
					int32_t rm = (int32_t)(int16_t)(r>>16);
					int32_t rh = (int32_t)(int16_t)(r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rl > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rl < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rl;
					}
				}
				break;

			case 15: // VMADH
				rsp_debug_printf("VMADH %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int64_t acc = GET_ACC_FOR_LANE(i);
					int64_t r = (int64_t)(int16_t)C->c2.h[vs][i] * (int64_t)(int16_t)C->c2.h[vt][j];
					r <<= 16;
					r += acc;
					int32_t rl = r;
					int32_t rm = (r>>16);
					int32_t rh = (r>>32);
					C->c2acc[0][i] = rl;
					C->c2acc[1][i] = rm;
					C->c2acc[2][i] = rh;
					if(rm > 0x7FFF) {
						C->c2.h[vd][i] = 0x7FFF;
					} else if(rm < -0x8000) {
						C->c2.h[vd][i] = 0x8000;
					} else {
						C->c2.h[vd][i] = (uint16_t)rm;
					}
				}
				break;

			case 16: // VADD
				rsp_debug_printf("VADD %2u %2u %2u %X\n", vd, vs, vt, el);
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
				rsp_debug_printf("VSUB %2u %2u %2u %X\n", vd, vs, vt, el);
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
				rsp_debug_printf("VABS %2u %2u %2u %X\n", vd, vs, vt, el);
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
				rsp_debug_printf("VADDC %2u %2u %2u %X\n", vd, vs, vt, el);
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
				rsp_debug_printf("VSUBC %2u %2u %2u %X\n", vd, vs, vt, el);
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
				rsp_debug_printf("VSAR %2u %2u %2u %X\n", vd, vs, vt, el);
				switch(el)
				{
					case 0x8:
						for(int i = 0; i < 8; i++) {
							C->c2.h[vd][i] = C->c2acc[2][i];
							if(vt == 0) {
								C->c2acc[2][i] = C->c2.h[vs][i];
							}
						}
						break;
					case 0x9:
						for(int i = 0; i < 8; i++) {
							C->c2.h[vd][i] = C->c2acc[1][i];
							if(vt == 0) {
								C->c2acc[1][i] = C->c2.h[vs][i];
							}
						}
						break;
					case 0xA:
						for(int i = 0; i < 8; i++) {
							C->c2.h[vd][i] = C->c2acc[0][i];
							if(vt == 0) {
								C->c2acc[0][i] = C->c2.h[vs][i];
							}
						}
						break;

					default:
						//printf("UNHANDLED VSAR EL CASE %2u %2u %2u %X\n", vd, vs, vt, el);
						for(int i = 0; i < 8; i++) {
							C->c2.h[vd][i] = 0;
						}
						break;
				}
				break;

			case 32: // VLT
				rsp_debug_printf("VLT %2u %2u %2u %X\n", vd, vs, vt, el);
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
				rsp_debug_printf("VEQ %2u %2u %2u %X\n", vd, vs, vt, el);
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
				rsp_debug_printf("VNE %2u %2u %2u %X\n", vd, vs, vt, el);
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
				rsp_debug_printf("VGE %2u %2u %2u %X\n", vd, vs, vt, el);
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
				rsp_debug_printf("VAND %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i] & C->c2.h[vt][j];
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 41: // VNAND
				rsp_debug_printf("VNAND %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = ~(C->c2.h[vs][i] & C->c2.h[vt][j]);
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 42: // VOR
				rsp_debug_printf("VOR %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i] | C->c2.h[vt][j];
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 43: // VNOR
				rsp_debug_printf("VNOR %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = ~(C->c2.h[vs][i] | C->c2.h[vt][j]);
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 44: // VXOR
				rsp_debug_printf("VXOR %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = C->c2.h[vs][i] ^ C->c2.h[vt][j];
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 45: // VNXOR
				rsp_debug_printf("VNXOR %2u %2u %2u %X\n", vd, vs, vt, el);
				for(int i = 0; i < 8; i++) {
					int j = elparamtab[el][i];
					int32_t r = ~(C->c2.h[vs][i] ^ C->c2.h[vt][j]);
					C->c2acc[0][i] = r;
					C->c2.h[vd][i] = r;
				}
				break;

			case 48: // VRCP
				rsp_debug_printf("VRCP %2u %2u %2u %X\n", vd, vs, vt, el);
				{
					int de = vs;

					C->c2divin = abs((int16_t)C->c2.h[vt][el&0x7]);
					int lshift = 0;
					while(lshift < 32) {
						if((C->c2divin & (0x80000000U>>lshift)) != 0) {
							break;
						}
						lshift++;
					}

					if(C->c2divin == 0) {
						lshift = 16;
					}

					uint32_t addr = ((C->c2divin<<(lshift+1))>>(22+1))&0x3FF;
					uint32_t romdata = (0x10000|rsp_reciprocal_rom[addr])<<14;
					int rshift = (~lshift)&0x1F;
					C->c2divout = romdata>>rshift;
					if((int16_t)C->c2.h[vt][el&0x7] < 0) {
						C->c2divout ^= 0xFFFFFFFF;
					}

					for(int i = 0; i < 8; i++) {
						int j = elparamtab[el][i];
						C->c2acc[0][i] = C->c2.h[vt][j];
					}
					C->c2.h[vd][de] = C->c2divout;
				}

				break;

			case 51: // VMOV
				rsp_debug_printf("VMOV %2u[%2u] %2u[%2u]\n", vd, vs, vt, el);
				{
					uint16_t r = (uint16_t)C->c2.h[vt][el&0x7];
					C->c2.h[vd][vs&0x7] = r;
					for(int i = 0; i < 8; i++) {
						C->c2acc[0][i] = C->c2.h[vt][i];
					}
				}
				break;

			case 52: // VRSQ
				rsp_debug_printf("VRSQ %2u %2u %2u %X\n", vd, vs, vt, el);

				break;

			case 55:
				// VNOP
				rsp_debug_printf("VNOP\n");
				break;

			case 63:
				// VNULL
				rsp_debug_printf("VNULL\n");
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


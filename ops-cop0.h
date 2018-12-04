switch(rs) {
	case 0: // MFCz
	switch(rd) {

#ifdef MIPS_IS_RSP
		case 1: // DMA_DRAM
			if(rt != 0) {
				C->regs[rt] = C->c0.n.dma_dram;
				SIGNEX32R(C, rt);
			} break;
		case 5: // DMA_FULL
			if(rt != 0) {
				C->regs[rt] = (C->c0.n.sp_status>>3)&0x1;
				SIGNEX32R(C, rt);
			} break;
		case 6: // DMA_BUSY
			if(rt != 0) {
				C->regs[rt] = (C->c0.n.sp_status>>2)&0x1;
				SIGNEX32R(C, rt);
			} break;
		case 7: // SP_RESERVED
			if(rt != 0) {
				C->regs[rt] = C->c0.n.sp_reserved;
				SIGNEX32R(C, rt);
			}
			C->c0.n.sp_reserved |= 0x1;
			break;

#else
		case 4: // c0_context
			if(rt != 0) {
				C->regs[rt] = C->c0.n.context;
				SIGNEX32R(C, rt);
			} break;
		case 8: // c0_badvaddr
			if(rt != 0) {
				C->regs[rt] = C->c0.n.badvaddr;
				SIGNEX32R(C, rt);
			} break;
		case 10: // c0_entryhi
			if(rt != 0) {
				C->regs[rt] = C->c0.n.entryhi;
				SIGNEX32R(C, rt);
			} break;
		case 12: // c0_sr
			if(rt != 0) {
				C->regs[rt] = C->c0.n.sr;
				SIGNEX32R(C, rt);
			} break;
		case 13: // c0_cause
			if(rt != 0) {
				C->regs[rt] = C->c0.n.cause;
				SIGNEX32R(C, rt);
			} break;
		case 14: // c0_epc
			if(rt != 0) {
				C->regs[rt] = C->c0.n.epc;
				SIGNEX32R(C, rt);
			} break;
		case 16: // c0_config
			if(rt != 0) {
				C->regs[rt] = C->c0.n.config;
				SIGNEX32R(C, rt);
			} break;
#endif

		default:
			printf("RI MFC0 %2u %08X -> %08X %d (COP0)\n"
				, rd, op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;
	} break;

	case 4: // MTCz
	switch(rd) {

#ifdef MIPS_IS_RSP
		case 0: // DMA_CACHE
			printf("DMA_CACHE %08X\n", C->regs[rt]);
			C->c0.n.dma_cache = C->regs[rt] & 0x1FFF;
			break;
		case 1: // DMA_DRAM
			printf("DMA_DRAM %08X\n", C->regs[rt]);
			C->c0.n.dma_dram = C->regs[rt] & 0xFFFFFF;
			break;
		case 2: // DMA_READ_LENGTH
			printf("DMA_READ_LENGTH %08X\n", C->regs[rt]);
			C->c0.n.dma_read_length = C->regs[rt];
			break;
		//case 3: // DMA_WRITE_LENGTH
			//C->c0.n.dma_write_length = C->regs[rt];
			//break;
#else
		case 0: // c0_index
			C->c0.n.index = C->regs[rt] & 63;
			break;

		case 2: // c0_entrylo0
			C->c0.n.entrylo0 = (SREG)(int32_t)C->regs[rt];
			break;
		case 3: // c0_entrylo1
			C->c0.n.entrylo1 = (SREG)(int32_t)C->regs[rt];
			break;

		case 4: // c0_context
			C->c0.n.context = (SREG)(int32_t)(C->regs[rt] & 0xFFFFFFF0);
			break;

		case 5: // c0_pagemask
			C->c0.n.pagemask = (C->regs[rt] & 0x01FFE000)|0x1FFF;
			break;

		case 6: // c0_wired
			C->c0.n.wired = C->regs[rt] & 0x3F;
			break;

		case 8: // c0_badvaddr
			C->c0.n.badvaddr = (SREG)(int32_t)C->regs[rt];
			break;

		case 9: // c0_count
			C->c0.n.count = (SREG)(int32_t)C->regs[rt];
			break;

		case 10: // c0_entryhi
			C->c0.n.entryhi = (SREG)(int32_t)C->regs[rt];
			break;

		case 11: // c0_compare
			C->c0.n.compare = (SREG)(int32_t)C->regs[rt];
			break;

		case 12: // c0_sr
			C->c0.n.sr &= ~0xFF77FFFF;
			C->c0.n.sr |= (C->regs[rt] & 0xFF57FFFF);
			printf("set SR = %08X\n", C->c0.n.sr);
			e = MIPSXNAME(_probe_interrupts)(C);
			if(e != MER_NONE) {
				e_ifetch = e;
			}
			break;

		case 13: // c0_cause
			C->c0.n.cause &= ~0x00000300;
			C->c0.n.cause |= (C->regs[rt] & 0x00000300);
			e = MIPSXNAME(_probe_interrupts)(C);
			if(e != MER_NONE) {
				e_ifetch = e;
			}
			break;

		case 14: // c0_epc
			C->c0.n.epc = C->regs[rt];
			break;

		case 16: // c0_config
			C->c0.n.config &= ~0x0F00800F;
			C->c0.n.config |= (C->regs[rt] & 0x0F00800F);
			break;

		case 28: // c0_taglo
			C->c0.n.taglo &= ~0x0FFFFFC0;
			C->c0.n.taglo |= (C->regs[rt] & 0x0FFFFFC0);
			break;

		case 29: // c0_taghi
			C->c0.n.taghi &= ~0x00000000;
			C->c0.n.taghi |= (C->regs[rt] & 0x00000000);
			break;
#endif

		default:
			printf("RI MTC0 %2u %08X -> %08X %d (COP0)\n"
				, rd, op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;

	} break;

	case 16: switch(op&0x3F) {
#ifdef MIPS_MMU_ENTRIES
		case 2: // TLBWI
		case 6: // TLBWR
		{
			// FIXME: we should calculate c0_random properly
			int wired = C->c0.n.wired;
			int idx = ((op&4) == 0
				? C->c0.n.index
				: fullrandu32());
			if((op&4) != 0) {
				idx = (idx-wired) % (32-wired) + wired;
			}
			idx &= (MIPS_MMU_ENTRIES-1);

			C->tlb[idx].entrylo[0] = C->c0.n.entrylo0;
			C->tlb[idx].entrylo[1] = C->c0.n.entrylo1;
			C->tlb[idx].entryhi = C->c0.n.entryhi;
			C->tlb[idx].pagemask = C->c0.n.pagemask;
			printf("entryhi %08X\n", C->c0.n.entryhi);
			printf("entrylo0 %08X\n", C->c0.n.entrylo0);
			printf("entrylo1 %08X\n", C->c0.n.entrylo1);
			printf("pagemask %08X\n", C->c0.n.pagemask);
			printf("idx %d\n", idx);
		} break;

		case 24: // ERET
		{
			printf("ERET start (SR=%08X PC=%08X)\n", C->c0.n.sr, C->pc);
			if((C->c0.n.sr & C0SR_ERL) != 0) {
				C->pc = C->c0.n.errorepc;
				C->c0.n.sr &= ~C0SR_ERL;
			} else {
				C->pc = C->c0.n.epc;
				C->c0.n.sr &= ~C0SR_EXL;
			}
			printf("ERET (SR=%08X PC=%08X)\n", C->c0.n.sr, C->pc);

			C->llbit = false;

			// flush pipeline
			C->pl0_op = 0x00000000;
			C->pl0_pc = C->pc;
			C->pl0_is_branch = false;
		} break;
#endif

		default:
			printf("RI op %2u %08X -> %08X %d (COP0)\n"
				, op&0x3F, op_pc, new_pc, op_was_branch
				);
			MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
			return MER_RI;

	} break;

	default:
		printf("RI %2u %2u %08X -> %08X %d (COP0)\n"
			, rs, op&0x3F, op_pc, new_pc, op_was_branch
			);
		MIPSXNAME(_throw_exception)(C, op_pc, MER_RI, op_was_branch);
		return MER_RI;
}

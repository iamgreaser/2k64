#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <time.h>

uint32_t fullrandu32(void)
{
	// I do NOT care about Windows for this
	// (If *you* do, drop the >>16s.)
	uint32_t v0 = (rand()>>16)&0x7FF;
	uint32_t v1 = (rand()>>16)&0x7FF;
	uint32_t v2 = (rand()>>16)&0x3FF;

	return v0|(v1<<11)|(v2<<22);
}

uint64_t fullrandu64(void)
{
	uint64_t v0 = fullrandu32();
	uint64_t v1 = fullrandu32();

	return v0|(v1<<32U);
}

#include "mips-common.h"

// VR4300 core
#define MIPS_IS_VR4300
#define MIPSNAME vr4300
#define MIPSXNAME(x) vr4300##x
#include "mips.h"
#undef MIPSNAME
#undef MIPSXNAME
#undef MIPS_IS_VR4300

// RSP core
#define MIPS_IS_RSP
#define MIPSNAME rsp
#define MIPSXNAME(x) rsp##x
#include "mips.h"
#undef MIPSNAME
#undef MIPSXNAME
#undef MIPS_IS_RSP

uint32_t pifmem[2*256];
uint32_t ram[8*1024*256];
uint32_t cartmem[64*1024*256];

uint32_t rsp_mem[8*256];
uint32_t rsp_status = 0x00000001;
bool rsp_intr = false;

uint32_t pi_dram_addr = 0;
uint32_t pi_cart_addr = 0;
uint32_t pi_rd_len = 0;
uint32_t pi_wr_len = 0;
uint32_t pi_status = 0;

static struct vr4300 vr4300_baseinst;

enum mipserr n64primary_mem_read(struct vr4300 *C, uint64_t addr, uint32_t mask, uint32_t *data)
{
	uint32_t data_out = 0xFFFFFFFFU;

	if(addr <= 0x03EFFFFFU) {
		data_out = ram[(addr & (sizeof(ram)-1))>>2];

	} else if(addr >= 0x10000000U && addr < 0x10000000U+sizeof(cartmem)) {
		data_out = cartmem[(addr-0x10000000U)>>2];

	} else if(addr >= 0x1FC00000U && addr < 0x1FC00000U+sizeof(pifmem)) {
		data_out = pifmem[(addr-0x1FC00000U)>>2];

		if((addr&0x7FF) >= 0x7C0) {
			printf("PIF RAM read %016llX pc %016llX op %08X mask %08X\n",
				addr, C->pl0_pc, C->pl0_op, mask);
			//usleep(100000);
		}

	} else if(addr >= 0x03F00000 && addr < 0x03FFFFFF) {
		printf("RDREG read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
		switch(addr)
		{
			case 0x03F0000C:
				data_out = 0xFFFFFFFF;
				break;

			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04000000U && addr < 0x04001FFFU) {
		data_out = rsp_mem[(addr&0x1FFF)>>2];

	} else if(addr >= 0x04040000U && addr < 0x0404FFFFU) {
		printf("SP read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
		switch(addr)
		{
			case 0x04040010:
				data_out = rsp_status;
				break;
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04100000U && addr < 0x041FFFFFU) {
		printf("DP read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
		switch(addr)
		{
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04300000 && addr < 0x043FFFFF) {
		printf("MI read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
		switch(addr)
		{
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04600000U && addr < 0x046FFFFFU) {
		printf("PI read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
		switch(addr)
		{
			case 0x04600010: // PI_STATUS_REG
				data_out = pi_status;
				break;
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04700000U && addr < 0x047FFFFFU) {
		printf("RI read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
		switch(addr)
		{
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04800000U && addr < 0x048FFFFFU) {
		printf("SI read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
		switch(addr)
		{
			default:
				data_out = 0;
				break;
		}

	} else {
		printf("buserr %08X (%016llX)\n", addr, (unsigned long long)C->pc);
		printf("(%016llX)\n", (unsigned long long)C->regs[31]);
		return MER_DBE;
	}

	*data = (*data & ~mask) | (data_out & mask);
	return MER_NONE;
}

void n64primary_mem_write(struct vr4300 *C, uint64_t addr, uint32_t mask, uint32_t data)
{
	uint32_t data_in = data & mask;
	uint32_t *data_out_ptr = NULL;

	if(addr <= 0x03EFFFFFU) {
		data_out_ptr = &ram[(addr & (sizeof(ram)-1))>>2];

	} else if(addr >= 0x03F00000 && addr < 0x03FFFFFF) {
		printf("RDREG write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		return;

	} else if(addr >= 0x04000000 && addr < 0x04001FFF) {
		data_out_ptr = &rsp_mem[(addr&0x1FFF)>>2];
		//printf("SP RAM write %016llX mask %08X data %08X\n",
			//(unsigned long long)addr, mask, data);

	} else if(addr >= 0x04040000 && addr < 0x0404FFFF) {
		printf("SP write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);

		switch(addr)
		{
			case 0x04040010:
				if((data & 0x00000001) != 0) { rsp_status &= ~0x0001; }
				if((data & 0x00000002) != 0) { rsp_status |=  0x0001; }
				if((data & 0x00000004) != 0) { rsp_status &= ~0x0002; }
				if((data & 0x00000008) != 0) { rsp_intr = false; }
				if((data & 0x00000010) != 0) { rsp_intr = true; }
				if((data & 0x00000020) != 0) { rsp_status &= ~0x0020; }
				if((data & 0x00000040) != 0) { rsp_status |=  0x0020; }
				if((data & 0x00000080) != 0) { rsp_status &= ~0x0040; }
				if((data & 0x00000100) != 0) { rsp_status |=  0x0040; }
				if((data & 0x00000200) != 0) { rsp_status &= ~0x0080; }
				if((data & 0x00000400) != 0) { rsp_status |=  0x0080; }
				if((data & 0x00000800) != 0) { rsp_status &= ~0x0100; }
				if((data & 0x00001000) != 0) { rsp_status |=  0x0100; }
				if((data & 0x00002000) != 0) { rsp_status &= ~0x0200; }
				if((data & 0x00004000) != 0) { rsp_status |=  0x0200; }
				if((data & 0x00008000) != 0) { rsp_status &= ~0x0400; }
				if((data & 0x00010000) != 0) { rsp_status |=  0x0400; }
				if((data & 0x00020000) != 0) { rsp_status &= ~0x0800; }
				if((data & 0x00040000) != 0) { rsp_status |=  0x0800; }
				if((data & 0x00080000) != 0) { rsp_status &= ~0x1000; }
				if((data & 0x00100000) != 0) { rsp_status |=  0x1000; }
				if((data & 0x00200000) != 0) { rsp_status &= ~0x2000; }
				if((data & 0x00400000) != 0) { rsp_status |=  0x2000; }
				break;
			default:
				break;
		}
		return;

	} else if(addr >= 0x04100000 && addr < 0x041FFFFF) {
		printf("DP write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		return;

	} else if(addr >= 0x04200000 && addr < 0x042FFFFF) {
		printf("DP span write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		return;

	} else if(addr >= 0x04300000 && addr < 0x043FFFFF) {
		printf("MI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		return;

	} else if(addr >= 0x04400000 && addr < 0x044FFFFF) {
		printf("VI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		return;

	} else if(addr >= 0x04500000 && addr < 0x045FFFFF) {
		printf("AI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		return;

	} else if(addr >= 0x04600000 && addr < 0x046FFFFF) {
		printf("PI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		switch(addr)
		{
			case 0x04600000: // PI_DRAM_ADDR_REG
				pi_dram_addr = data & 0xFFFFFF;
				break;
			case 0x04600004: // PI_CART_ADDR_REG
				// FIXME: apparently all 32 bits are used
				pi_cart_addr = data & 0xFFFFFF;
				break;
			case 0x04600008: // PI_RD_LEN_REG
				pi_rd_len = data & 0xFFFFFF;
				pi_rd_len += 1;
				pi_status |= 0x1;
				break;
			case 0x0460000C: // PI_WR_LEN_REG
				pi_wr_len = data & 0xFFFFFF;
				pi_wr_len += 1;
				pi_status |= 0x1;
				break;
			case 0x04600010: // PI_STATUS_REG
				break;
			default:
				break;
		}

		// FIXME: this is lazy and should actually emulate separately
		while((pi_wr_len&~3) != 0) {
			uint32_t data = cartmem[(pi_cart_addr>>2)%(sizeof(cartmem)/sizeof(uint32_t))];
			ram[(pi_dram_addr>>2)%(sizeof(ram)/sizeof(uint32_t))] = data;
			//n64primary_mem_read (C, pi_cart_addr, 0xFF<<((~pi_cart_addr)&0x3), &mdata);
			//n64primary_mem_write(C, pi_dram_addr, 0xFF<<((~pi_dram_addr)&0x3), &mdata);
			//printf("Copy byte %08X <- %08X\n", pi_dram_addr, pi_cart_addr);
			pi_dram_addr += 4;
			pi_cart_addr += 4;
			pi_wr_len -= 4;
		}
		pi_status &= ~0x1;

		return;

	} else if(addr >= 0x04700000 && addr < 0x047FFFFF) {
		printf("RI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		return;

	} else if(addr >= 0x04800000 && addr < 0x048FFFFF) {
		printf("SI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		return;

	} else if(addr >= 0x1FC007C0U && addr < 0x1FC00000U+sizeof(pifmem)) {
		data_out_ptr = &pifmem[((addr&0x3F)+0x7C0)>>2];
		printf("PIF RAM write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);

		if(addr == 0x1FC007FC) {
			// HACK.
			if((data&mask&0xFF) == 0x30) {
				data_in |= 0x00000080;
			}
		}

	} else {
		// writes cannot throw bus errors
		printf("unknown write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
		return;
	}

	assert(data_out_ptr != NULL);
	*data_out_ptr = (*data_out_ptr & ~mask) | (data_in & mask);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	struct vr4300 *C = &vr4300_baseinst;

	srand((unsigned int)time(NULL));

	printf("EMU INIT\n");

	fp = fopen(argv[1], "rb");
	size_t pif_len = fread(pifmem, 1, sizeof(pifmem), fp);
	fclose(fp);
	printf(" PIF ROM len: %d bytes\n", pif_len);

	fp = fopen(argv[2], "rb");
	size_t cart_len = fread(cartmem, 1, sizeof(cartmem), fp);
	fclose(fp);
	printf("CART ROM len: %d bytes\n", cart_len);

	for(size_t i = 0; i < sizeof(pifmem)/4; i++) {
		uint32_t v = pifmem[i];
		uint32_t nv = 0;
		nv |= ((v>>0)&0xFF)<<24;
		nv |= ((v>>8)&0xFF)<<16;
		nv |= ((v>>16)&0xFF)<<8;
		nv |= ((v>>24)&0xFF)<<0;
		pifmem[i] = nv;
	}

	for(size_t i = 0; i < sizeof(cartmem)/4; i++) {
		uint32_t v = cartmem[i];
		uint32_t nv = 0;
		nv |= ((v>>0)&0xFF)<<24;
		nv |= ((v>>8)&0xFF)<<16;
		nv |= ((v>>16)&0xFF)<<8;
		nv |= ((v>>24)&0xFF)<<0;
		cartmem[i] = nv;
	}

	vr4300_cpu_init(C);

	// PIF hacks
	pifmem[0x7FC>>2] = 0;
	pifmem[0x7E4>>2] = 0; // we have nothing in the expansion slot!

#if 0
	// ROM checksum hacks
	cartmem[0x0010>>2] = 0x6285AD5B; // A3
	cartmem[0x0014>>2] = 0x5258B8E7; // S0
#endif

#if 0
	// ROM DL hacks
	memcpy(ram, cartmem, 4*1024*1024);
	C->c0.n.sr = 0x34000000;
	C->pl0_op = 0;
	C->pc = (int64_t)(int32_t)cartmem[0x0008>>2];
	printf("new pc: %08X\n", C->pc);
#endif

	C->f_mem_read  = n64primary_mem_read;
	C->f_mem_write = n64primary_mem_write;

	printf("EMU START\n####################\n\n");

	for(;;)
	{
		//printf("PC=%016llX OP=%08X\n", (unsigned long long)(C->pl0_pc), C->pl0_op);
		enum mipserr e = vr4300_run_op(C);
		//printf("PC=%016llX\n", (unsigned long long)(C->pc));
		//usleep(10000);
		/*
		if(e != MER_NONE) {
			printf("$$$\n\nERR %3d - epc = %08X, sr = %08X, cause = %08X\n",
				e-1, C->c0.n.epc, C->c0.n.sr, C->c0.n.cause);
			//break;
		}
		*/
	}

	printf("EMU END\n");
	
	return 0;
}


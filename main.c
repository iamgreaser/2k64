#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <time.h>

#include <SDL.h>
#include <signal.h>

#define DEBUG_DP 1
#define DEBUG_SP 1
#define DEBUG_VI 1

#define RAM_TO_FILE 0

#if RAM_TO_FILE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
int ram_fd = -1;
#endif

SDL_Window *window;
SDL_Surface *window_surface;
SDL_Surface *surface;
SDL_Renderer *renderer;

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

// the N64 apparently rounds to zero in case of ambiguity
float n64_roundf(float v)
{
	float tv = truncf(v);
	float rem = v - tv;
	if(rem > 0.5f) {
		return tv + 1;
	} else if(rem < -0.5f) {
		return tv - 1;
	} else {
		return tv;
	}
}

double n64_round(double v)
{
	double tv = trunc(v);
	double rem = v - tv;
	if(rem > 0.5) {
		return tv + 1;
	} else if(rem < -0.5) {
		return tv - 1;
	} else {
		return tv;
	}
}

#include "mips-common.h"

struct vr4300;
struct rsp;
enum mipserr n64primary_mem_read(struct vr4300 *C, uint64_t addr, uint32_t mask, uint32_t *data);
void n64primary_mem_write(struct vr4300 *C, uint64_t addr, uint32_t mask, uint32_t data);
enum mipserr n64rsp_mem_read(struct rsp *rsp, uint64_t addr, uint32_t mask, uint32_t *data);
void n64rsp_mem_write(struct rsp *rsp, uint64_t addr, uint32_t mask, uint32_t data);

// VR4300 core
#define MIPS_IS_VR4300
#define MIPSNAME vr4300
#define MIPSXNAME(x) vr4300##x
#define MIPS_MEM_READ n64primary_mem_read
#define MIPS_MEM_WRITE n64primary_mem_write
#include "mips.h"
#undef MIPS_MEM_READ
#undef MIPS_MEM_WRITE
#undef MIPSNAME
#undef MIPSXNAME
#undef MIPS_IS_VR4300

// RSP core
#define MIPS_IS_RSP
#define MIPSNAME rsp
#define MIPSXNAME(x) rsp##x
#define MIPS_MEM_READ n64rsp_mem_read
#define MIPS_MEM_WRITE n64rsp_mem_write
#include "mips.h"
#undef MIPS_MEM_READ
#undef MIPS_MEM_WRITE
#undef MIPSNAME
#undef MIPSXNAME
#undef MIPS_IS_RSP

uint32_t pifimg[2*256];
#define RAM_SIZE_WORDS (8*1024*256)
#define RAM_SIZE_BYTES (4*RAM_SIZE_WORDS)
#if RAM_TO_FILE
uint32_t *ram;
uint32_t *rsp_mem;
uint32_t *pifmem;
#else
uint32_t ram[RAM_SIZE_WORDS];
uint32_t rsp_mem[8*256];
uint32_t pifmem[2*256];
#endif
uint32_t cartmem[64*1024*256];


uint32_t pi_dram_addr = 0;
uint32_t pi_cart_addr = 0;
uint32_t pi_rd_len = 0;
uint32_t pi_wr_len = 0;
uint32_t pi_status = 0;

uint32_t vi_status_reg = 0;
uint32_t vi_origin_reg = 0;
uint32_t vi_width_reg = 640;
uint32_t vi_x_scale_reg = 0;
uint32_t vi_y_scale_reg = 0;

struct {
	char *cic_type;
	uint32_t pif_seed;
	uint8_t pif_rom_0x0040[16];
} cic_boot_codes[] = {
	{
		"6102",
		0x00003F3F,
		{
			0x40, 0x80, 0x68, 0x00, 0x40, 0x80, 0x48, 0x00,
			0x40, 0x80, 0x58, 0x00, 0x3c, 0x08, 0xa4, 0x70,
		},
	},
	{
		"6105",
		0x0000913F,
		{
			0x03, 0xa0, 0x48, 0x20, 0x8d, 0x28, 0xf0, 0x10,
			0x8d, 0x6a, 0x00, 0x44, 0x01, 0x48, 0x50, 0x26,
		},
	},
};

static struct vr4300 vr4300_baseinst;
static struct rsp rsp_baseinst;

uint64_t global_clock = 0;

enum mipserr n64primary_mem_read(struct vr4300 *C, uint64_t addr, uint32_t mask, uint32_t *data)
{
	uint32_t data_out = 0xFFFFFFFFU;

	if(addr < RAM_SIZE_BYTES) {
		//data_out = ram[(addr & (RAM_SIZE_BYTES-1))>>2];
		data_out = ram[addr>>2];

	} else if(addr <= 0x03EFFFFFU) {
		data_out = 0x00000000;

	} else if(addr >= 0x10000000U && addr < 0x10000000U+sizeof(cartmem)) {
		//printf("CART CPU read %016llX\n", addr);
		assert(addr-0x10000000U >= 0U);
		assert(addr-0x10000000U < sizeof(cartmem));
		data_out = cartmem[(addr-0x10000000U)>>2];

	} else if(addr >= 0x1FC00000U && addr < 0x1FC00000U+4*2*256) {
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
		struct rsp *rsp = &rsp_baseinst;
#if DEBUG_SP
		printf("SP read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
#endif
		switch(addr)
		{
			case 0x04040010:
				data_out = rsp->c0.n.sp_status;
				break;
			case 0x0404001C:
				data_out = rsp->c0.n.sp_reserved;
				rsp->c0.n.sp_reserved = 1;
				break;
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04080000U && addr < 0x0408FFFFU) {
		struct rsp *rsp = &rsp_baseinst;

#if DEBUG_SP
		printf("SP8 read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
#endif
		switch(addr)
		{
			case 0x04080000:
				data_out = rsp->pc & 0xFFF;
				break;
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04100000U && addr < 0x041FFFFFU) {
#if DEBUG_DP
		printf("DP read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
#endif
		switch(addr)
		{
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04300000 && addr < 0x043FFFFF) {
#if DEBUG_MI
		printf("MI read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
#endif
		switch(addr)
		{
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04400000 && addr < 0x044FFFFF) {
#if DEBUG_VI
		if(addr != 0x04400010) {
			printf("VI read %016llX mask %08X\n",
				(unsigned long long)addr, mask);
		}
#endif
		switch(addr)
		{
			case 0x04400010:
				data_out = (global_clock / 6250) % 262;
				break;
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04600000U && addr < 0x046FFFFFU) {
#if DEBUG_PI
		printf("PI read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
#endif
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
#if DEBUG_RI
		printf("RI read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
#endif
		switch(addr)
		{
			case 0x04700010:
				data_out = 0;
				break;
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x04800000U && addr < 0x048FFFFFU) {
#if DEBUG_SI
		printf("SI read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
#endif
		switch(addr)
		{
			default:
				data_out = 0;
				break;
		}

	} else if(addr >= 0x05000000U && addr < 0x05FFFFFFU) {
#if DEBUG_SI
		printf("CartDom2Addr1 read %016llX mask %08X\n",
			(unsigned long long)addr, mask);
#endif
		data_out = 0;


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

	if(addr == 0x00000318) {
		//if(C->pl0_pc != 0xFFFFFFFFA400039CULL) {
		{
		// FIXME: DETECT EXPANSION PAK PROPERLY
		data = 8*1024*1024;
		//printf("DEBUG ABORT: %08X (%016llX)\n", data, C->pl0_pc);
		//fsync(ram_fd);
		//abort();
		}
	}

	if(addr < RAM_SIZE_BYTES) {
		//data_out_ptr = &ram[(addr & (RAM_SIZE_BYTES-1))>>2];
		data_out_ptr = &ram[addr>>2];

	} else if(addr <= 0x03EFFFFFU) {
		return;

	} else if(addr >= 0x03F00000 && addr < 0x03FFFFFF) {
#ifdef DEBUG_RDREG
		printf("RDREG write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
#endif
		return;

	} else if(addr >= 0x04000000 && addr < 0x04001FFF) {
		data_out_ptr = &rsp_mem[(addr&0x1FFF)>>2];
		//printf("SP RAM write %016llX mask %08X data %08X\n",
			//(unsigned long long)addr, mask, data);

	} else if(addr >= 0x04040000 && addr < 0x0404FFFF) {
		struct rsp *rsp = &rsp_baseinst;

#ifdef DEBUG_SP
		printf("SP write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
#endif

		switch(addr)
		{
			case 0x04040000: // DMA_CACHE
				printf("DMA_CACHE %08X\n", data);
				rsp->c0.n.dma_cache = data & 0x1FFF;
				break;
			case 0x04040004: // DMA_DRAM
				printf("DMA_DRAM %08X\n", data);
				rsp->c0.n.dma_dram = data & 0xFFFFFF;
				break;
			case 0x04040008: // DMA_READ_LENGTH
				printf("DMA_READ_LENGTH %08X\n", data);
				rsp->c0.n.dma_read_length = data;
				break;
			case 0x04040010:
				if((data & 0x00000001) != 0) { rsp->c0.n.sp_status &= ~0x0001; }
				if((data & 0x00000002) != 0) { rsp->c0.n.sp_status |=  0x0001; }
				if((data & 0x00000004) != 0) { rsp->c0.n.sp_status &= ~0x0002; }
				if((data & 0x00000008) != 0) { rsp->rsp_intr = false; }
				if((data & 0x00000010) != 0) { rsp->rsp_intr = true; }
				if((data & 0x00000020) != 0) { rsp->c0.n.sp_status &= ~0x0020; }
				if((data & 0x00000040) != 0) { rsp->c0.n.sp_status |=  0x0020; }
				if((data & 0x00000080) != 0) { rsp->c0.n.sp_status &= ~0x0040; }
				if((data & 0x00000100) != 0) { rsp->c0.n.sp_status |=  0x0040; }
				if((data & 0x00000200) != 0) { rsp->c0.n.sp_status &= ~0x0080; }
				if((data & 0x00000400) != 0) { rsp->c0.n.sp_status |=  0x0080; }
				if((data & 0x00000800) != 0) { rsp->c0.n.sp_status &= ~0x0100; }
				if((data & 0x00001000) != 0) { rsp->c0.n.sp_status |=  0x0100; }
				if((data & 0x00002000) != 0) { rsp->c0.n.sp_status &= ~0x0200; }
				if((data & 0x00004000) != 0) { rsp->c0.n.sp_status |=  0x0200; }
				if((data & 0x00008000) != 0) { rsp->c0.n.sp_status &= ~0x0400; }
				if((data & 0x00010000) != 0) { rsp->c0.n.sp_status |=  0x0400; }
				if((data & 0x00020000) != 0) { rsp->c0.n.sp_status &= ~0x0800; }
				if((data & 0x00040000) != 0) { rsp->c0.n.sp_status |=  0x0800; }
				if((data & 0x00080000) != 0) { rsp->c0.n.sp_status &= ~0x1000; }
				if((data & 0x00100000) != 0) { rsp->c0.n.sp_status |=  0x1000; }
				if((data & 0x00200000) != 0) { rsp->c0.n.sp_status &= ~0x2000; }
				if((data & 0x00400000) != 0) { rsp->c0.n.sp_status |=  0x2000; }
				break;

			case 0x0404001C:
				rsp->c0.n.sp_reserved = 0;
				break;

			default:
				break;
		}
		return;

	} else if(addr >= 0x04080000 && addr < 0x0408FFFF) {
		struct rsp *rsp = &rsp_baseinst;

#ifdef DEBUG_SP
		printf("SP8 write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
#endif

		switch(addr)
		{
			case 0x04080000:
				rsp->pc = data & 0xFFF;
				//rsp->pl0_pc = (data-4) & 0xFFF;
				rsp->pl0_op = 0;
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
#if DEBUG_MI
		printf("MI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
#endif
		return;

	} else if(addr >= 0x04400000 && addr < 0x044FFFFF) {
#if DEBUG_VI
		printf("VI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
#endif

		switch(addr)
		{
			case 0x04400000: // VI_STATUS_REG
				vi_status_reg = data & 0x00FFFFFF;
				break;
			case 0x04400004: // VI_ORIGIN_REG
				vi_origin_reg = data & 0x00FFFFFF;
				break;
			case 0x04400008: // VI_WIDTH_REG
				vi_width_reg = data & 0x00000FFF;
				break;
			case 0x04400030: // VI_X_SCALE_REG
				vi_x_scale_reg = data & 0x0FFF0FFF;
				break;
			case 0x04400034: // VI_Y_SCALE_REG
				vi_y_scale_reg = data & 0x0FFF0FFF;
				break;
		}
		return;

	} else if(addr >= 0x04500000 && addr < 0x045FFFFF) {
#if DEBUG_AI
		printf("AI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
#endif
		return;

	} else if(addr >= 0x04600000 && addr < 0x046FFFFF) {
#if DEBUG_PI
		printf("PI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
#endif
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
#if 0
		while((pi_wr_len&~3) != 0) {
			uint32_t data = cartmem[(pi_cart_addr>>2)%(sizeof(cartmem)/sizeof(uint32_t))];
			ram[(pi_dram_addr>>2)%(RAM_SIZE_BYTES/sizeof(uint32_t))] = data;
			//n64primary_mem_read (C, pi_cart_addr, 0xFF<<((~pi_cart_addr)&0x3), &mdata);
			//n64primary_mem_write(C, pi_dram_addr, 0xFF<<((~pi_dram_addr)&0x3), &mdata);
			//printf("Copy byte %08X <- %08X\n", pi_dram_addr, pi_cart_addr);
			pi_dram_addr += 4;
			pi_cart_addr += 4;
			pi_wr_len -= 4;
		}
#endif
		if(pi_wr_len != 0) {
			uint32_t *src = &cartmem[(pi_cart_addr>>2)%(sizeof(cartmem)/sizeof(uint32_t))];
			uint32_t *dst = &ram[(pi_dram_addr>>2)%(RAM_SIZE_BYTES/sizeof(uint32_t))];
			memcpy(dst, src, pi_wr_len);
			pi_wr_len = 0;
		}
		pi_status &= ~0x1;

		return;

	} else if(addr >= 0x04700000 && addr < 0x047FFFFF) {
#if DEBUG_RI
		printf("RI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
#endif
		return;

	} else if(addr >= 0x04800000 && addr < 0x048FFFFF) {
#if DEBUG_SI
		printf("SI write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);
#endif
		return;

	} else if(addr >= 0x1FC007C0U && addr < 0x1FC00000U+4*2*256) {
		data_out_ptr = &pifmem[((addr&0x3F)+0x7C0)>>2];
		printf("PIF RAM write %016llX mask %08X data %08X\n",
			(unsigned long long)addr, mask, data);

		if(addr == 0x1FC007FC) {
			// HACK.
			switch(data&mask&0xFF)
			{
				case 0x30:
					data_in |= 0x00000080;
					break;
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

enum mipserr n64rsp_mem_read(struct rsp *rsp, uint64_t addr, uint32_t mask, uint32_t *data)
{
	addr &= 0x1FFF;
	*data = (*data & ~mask) | (rsp_mem[(0x0000+addr)>>2] & mask);
	return MER_NONE;
}

void n64rsp_mem_write(struct rsp *rsp, uint64_t addr, uint32_t mask, uint32_t data)
{
	addr &= 0xFFF;
	uint32_t *data_out_ptr = &rsp_mem[(0x0000+addr)>>2];
	*data_out_ptr = (*data_out_ptr & ~mask) | (data & mask);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	struct vr4300 *C = &vr4300_baseinst;
	struct rsp *rsp = &rsp_baseinst;

	srand((unsigned int)time(NULL));

	printf("EMU INIT\n");

#if RAM_TO_FILE
	ram_fd = open("ram.data", O_RDWR|O_CREAT, 0755);
	assert(ram_fd >= 3);
	ftruncate(ram_fd, RAM_SIZE_BYTES + 4*(8*256 + 2*256));
	ram = mmap(NULL, RAM_SIZE_BYTES + 4*(8*256 + 2*256), PROT_READ|PROT_WRITE, MAP_SHARED, ram_fd, 0);
	assert(ram != (void *)MAP_FAILED);
	assert(ram != NULL);
	rsp_mem = ram + RAM_SIZE_WORDS;
	pifmem = rsp_mem + 8*256;
	memset(ram, 0, RAM_SIZE_BYTES);
#endif

	fp = fopen(argv[1], "rb");
	size_t pif_len = fread(pifimg, 1, sizeof(pifimg), fp);
	fclose(fp);
	printf(" PIF ROM len: %d bytes\n", pif_len);

	fp = fopen(argv[2], "rb");
	size_t cart_len = fread(cartmem, 1, sizeof(cartmem), fp);
	fclose(fp);
	printf("CART ROM len: %d bytes\n", cart_len);

	for(size_t i = 0; i < sizeof(pifimg)/4; i++) {
		uint32_t v = pifimg[i];
		uint32_t nv = 0;
		nv |= ((v>>0)&0xFF)<<24;
		nv |= ((v>>8)&0xFF)<<16;
		nv |= ((v>>16)&0xFF)<<8;
		nv |= ((v>>24)&0xFF)<<0;
		pifimg[i] = nv;
	}
	memcpy(pifmem, pifimg, sizeof(pifimg));

	uint32_t pifseed = 0x00000000;

	// do this before byteswapping
	for(size_t i = 0; i < sizeof(cic_boot_codes)/sizeof(cic_boot_codes[0]); i++) {
		if(!memcmp(
				0x0040+(uint8_t *)cartmem,
				cic_boot_codes[i].pif_rom_0x0040,
				sizeof(cic_boot_codes[i].pif_rom_0x0040))) {

			pifseed = cic_boot_codes[i].pif_seed;
			printf("CIC type %s detected\n", cic_boot_codes[i].cic_type);
			break;
		}
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
	if(pifseed == 0) {
		printf("UNDETECTED CIC TYPE! This will most likely lock up\n");
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);

	// Undo SDL's signal stealing bullshit
	signal(SIGINT,  SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	window = SDL_CreateWindow("2k64",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640, 480,
		0);
	window_surface = SDL_GetWindowSurface(window);
	surface = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_RGBX8888);
	renderer = SDL_CreateSoftwareRenderer(window_surface);

	vr4300_cpu_init(C);
	rsp_cpu_init(rsp);

	// PIF hacks
	pifmem[0x7FC>>2] = 0;

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

	//C->f_mem_read  = n64primary_mem_read;
	//C->f_mem_write = n64primary_mem_write;

	//rsp->f_mem_read  = n64rsp_mem_read;
	//rsp->f_mem_write = n64rsp_mem_write;

	printf("EMU START\n####################\n\n");

	pifmem[0x7E4>>2] = pifseed;
	pifmem[0x7E4>>2] = pifseed;

	for(;;)
	{
		//printf("PC=%016llX OP=%08X\n", (unsigned long long)(C->pl0_pc), C->pl0_op);
		enum mipserr e_cpu = vr4300_run_op(C);
		if((rsp->c0.n.sp_status & 0x00000001) == 0x00000000) {
			enum mipserr e_rsp = rsp_run_op(rsp);
#if 0
			if(rsp->pl0_op != 0) {
				printf("RSP op %08X: %08X %2d %2d %2d %2d %2d %2d\n", rsp->pl0_pc, rsp->pl0_op,
					(rsp->pl0_op>>26)&0x3F,
					(rsp->pl0_op>>21)&0x1F,
					(rsp->pl0_op>>16)&0x1F,
					(rsp->pl0_op>>11)&0x1F,
					(rsp->pl0_op>>6)&0x1F,
					(rsp->pl0_op>>0)&0x3F);
			}
#endif

			if(rsp->c0.n.dma_read_length != 0) {
				printf("RSP DMA read RSP=%08X <- MEM=%08X len %08X\n",
					rsp->c0.n.dma_cache,
					rsp->c0.n.dma_dram,
					rsp->c0.n.dma_read_length);

				int length = rsp->c0.n.dma_read_length & 0xFFF;
				int count = (rsp->c0.n.dma_read_length>>12) & 0xFF;
				int skip = (rsp->c0.n.dma_read_length>>20) & 0xFFF;
				int dst = rsp->c0.n.dma_cache;
				int src = rsp->c0.n.dma_dram;

				length += 7; length &= ~7;

				assert((dst & 0x7) == 0);
				assert((src & 0x7) == 0);
				assert((length & 0x7) == 0);
				assert((skip & 0x7) == 0);

				for(int y = 0; y <= count; y++) {
					for(int x = 0; x < length>>3; x++) {
						assert(src+8 <= RAM_SIZE_BYTES);
						assert(dst+8 <= 0x2000);
						rsp_mem[(dst>>2)+0] = ram[(src>>2)+0];
						rsp_mem[(dst>>2)+1] = ram[(src>>2)+1];
						dst += 8;
						src += 8;
						dst &= 0x1FF8;
						src &= RAM_SIZE_BYTES-8;
					}
					dst += skip;
					dst &= 0x1FF8;
				}
				rsp->c0.n.dma_read_length = 0;
				rsp->c0.n.dma_cache = dst;
				rsp->c0.n.dma_dram = src;
			}
		}
		global_clock += 1;
#if 0
		// useful for brute forcing the CIC seed
		if(C->pl0_op == 0x0411FFFF) {
			pifseed += 0x00000100;
			printf("FAILED - next seed: %08X (%016llX)\n", pifseed, C->regs[7]);
			memcpy(pifmem, pifimg, sizeof(pifimg));
			pifmem[0x7FC>>2] = 0;
			pifmem[0x7E4>>2] = pifseed;
			vr4300_cpu_init(C);
		}
#endif
		//printf("PC=%016llX\n", (unsigned long long)(C->pc));
		//usleep(10000);
		/*
		if(e != MER_NONE) {
			printf("$$$\n\nERR %3d - epc = %08X, sr = %08X, cause = %08X\n",
				e-1, C->c0.n.epc, C->c0.n.sr, C->c0.n.cause);
			//break;
		}
		*/

		if(global_clock % (6250*262) == 0) {
			printf(" - NEW FRAME - \n");
			SDL_LockSurface(surface);
			//printf("%d %d\n", vi_origin_reg, vi_width_reg);
			for(int y = 0; y < 480; y++) {
			for(int x = 0; x < 640; x++) {
				uint32_t offs = vi_origin_reg;
				offs += y*vi_width_reg*4;
				offs += x*4;
				uint32_t data = *(uint32_t *)(((uint8_t *)ram) + offs);
				((uint32_t *)(surface->pixels + surface->pitch*y))[x] = data;
			}
			}
			SDL_UnlockSurface(surface);
			SDL_BlitSurface(surface, NULL, window_surface, NULL);
			SDL_UpdateWindowSurface(window);
		}
	}

	printf("EMU END\n");
	
	return 0;
}


switch(rdp_tile_size) {
	// 4bpp
	case 0: switch(rdp_tile_format) {

		// Color Index
		case 2: {
#define GET_TEX_DATA() \
			uint32_t data = rdp_tmem[(tmem_offs+(data_s>>3))&0x3FF]; \
			data >>= ((~data_s)&0x7)*4; \
			data &= 0xF; \
			APPLY_PALETTE_RGBA16(data);
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
		} break;

		// Intensity+Alpha
		case 3: {
#define GET_TEX_DATA() \
			uint32_t data = rdp_tmem[(tmem_offs+(data_s>>3))&0x3FF]; \
			data >>= ((~data_s)&0x7)*4; \
			data &= 0xF; \
			data  = (0 \
				| 0x11111100U * (data) \
				| 0x000000FFU * ((data==0)?0:1) \
				);
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
		} break;

		// Intensity
		default:
		case 4: {
#define GET_TEX_DATA() \
			uint32_t data = rdp_tmem[(tmem_offs+(data_s>>3))&0x3FF]; \
			data >>= ((~data_s)&0x7)*4; \
			data &= 0xF; \
			data *= 0x11111100U; \
			data |= 0x000000FFU;

#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
		} break;
	} break;

	// 8bpp
	case 1: switch(rdp_tile_format) {
		// Color Index
		case 2: {
#define GET_TEX_DATA() \
			uint32_t data = rdp_tmem[(tmem_offs+(data_s>>2))&0x3FF]; \
			data >>= ((~data_s)&0x3)*8; \
			data &= 0xFF; \
			APPLY_PALETTE_RGBA16(data);
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
		} break;

		// Intensity+Alpha
		case 3: {
#define GET_TEX_DATA() \
			uint32_t data = rdp_tmem[(tmem_offs+(data_s>>2))&0x3FF]; \
			data >>= ((~data_s)&0x3)*8; \
			data &= 0xFF; \
			data  = (0 \
				| 0x11111100U * (data>>4) \
				| 0x000000FFU * (data&0xF) \
				);
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
		} break;

		// Intensity
		default:
		case 4: {
#define GET_TEX_DATA() \
			uint32_t data = rdp_tmem[(tmem_offs+(data_s>>2))&0x3FF]; \
			data >>= ((~data_s)&0x3)*8; \
			data &= 0xFF; \
			data *= 0x01010100U; \
			data |= 0x000000FFU;
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
		} break;

	} break;


	// 16bpp
	case 2: {
#define GET_TEX_DATA() \
		uint32_t data = rdp_tmem[(tmem_offs+(data_s>>1))&0x3FF]; \
		data >>= ((~data_s)&0x1)*16; \
		data = (0 \
			| (((data>>0)&0x1)*0xFF) \
			| (((((data>>1)&0x1F)*0x21)>>2)<<8) \
			| (((((data>>6)&0x1F)*0x21)>>2)<<16) \
			| (((((data>>11)&0x1F)*0x21)>>2)<<24) \
		);

#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
	} break;

	default:
	// 32bpp
	case 3: {

#define GET_TEX_DATA() \
		uint32_t data = rdp_tmem[(tmem_offs+data_s)&0x3FF];
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
	} break;
}



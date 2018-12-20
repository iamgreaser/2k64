// 4bpp Color Index
#define TEX_FUNCTION_NAME tex_rect_rdp_4p
#define GET_TEX_DATA() \
	uint32_t data = rdp_tmem[(tmem_offs+(data_s>>3))&0x3FF]; \
	data >>= ((~data_s)&0x7)*4; \
	data &= 0xF; \
	APPLY_PALETTE_RGBA16(data);
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
#undef TEX_FUNCTION_NAME

// 4bpp Intensity + Alpha
#define TEX_FUNCTION_NAME tex_rect_rdp_4ia
#define GET_TEX_DATA() \
	uint32_t data = rdp_tmem[(tmem_offs+(data_s>>3))&0x3FF]; \
	data >>= ((~data_s)&0x7)*4; \
	data &= 0xF; \
	data  = (0 \
		| 0x11111100U * (((data>>1)*0x49)>>1) \
		| 0x000000FFU * (((data&0x1)==0)?0:1) \
		);
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
#undef TEX_FUNCTION_NAME

// 4bpp Intensity
#define TEX_FUNCTION_NAME tex_rect_rdp_4i
#define GET_TEX_DATA() \
	uint32_t data = rdp_tmem[(tmem_offs+(data_s>>3))&0x3FF]; \
	data >>= ((~data_s)&0x7)*4; \
	data &= 0xF; \
	data *= 0x11111100U; \
	data |= 0x000000FFU;

#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
#undef TEX_FUNCTION_NAME

// 8bpp Color Index
#define TEX_FUNCTION_NAME tex_rect_rdp_8p
#define GET_TEX_DATA() \
	uint32_t data = rdp_tmem[(tmem_offs+(data_s>>2))&0x3FF]; \
	data >>= ((~data_s)&0x3)*8; \
	data &= 0xFF; \
	APPLY_PALETTE_RGBA16(data);
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
#undef TEX_FUNCTION_NAME

// 8bpp Intensity + Alpha
#define TEX_FUNCTION_NAME tex_rect_rdp_8ia
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
#undef TEX_FUNCTION_NAME

// 8bpp Intensity
#define TEX_FUNCTION_NAME tex_rect_rdp_8i
#define GET_TEX_DATA() \
	uint32_t data = rdp_tmem[(tmem_offs+(data_s>>2))&0x3FF]; \
	data >>= ((~data_s)&0x3)*8; \
	data &= 0xFF; \
	data *= 0x01010100U; \
	data |= 0x000000FFU;
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
#undef TEX_FUNCTION_NAME

// 16bpp Intensity + Alpha
#define TEX_FUNCTION_NAME tex_rect_rdp_16ia
#define GET_TEX_DATA() \
	uint32_t data = rdp_tmem[(tmem_offs+(data_s>>1))&0x3FF]; \
	data >>= ((~data_s)&0x1)*16; \
	data &= 0xFFFF; \
	data = (0 \
		| ((data&0xFF)) \
		| ((data>>8)*0x01010100) \
	);
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
#undef TEX_FUNCTION_NAME

// "16bpp" YUV
#define TEX_FUNCTION_NAME tex_rect_rdp_16yuv
#define GET_TEX_DATA() \
	uint32_t data = rdp_tmem[(tmem_offs+(data_s>>1))&0x3FF]; \
	{ \
		int32_t cy = (data>>(16*((~data_s)&0x1)))&0xFF; \
		int32_t cu = ((data>>24)&0xFF) - 0x80; \
		int32_t cv = ((data>>8)&0xFF) - 0x80; \
		/* This step is done in the TF (that's here!) */ \
		int32_t cr = cy + ((rdp_convert_k0*cv + (1<<6))>>7); \
		int32_t cg = cy + ((rdp_convert_k1*cu + rdp_convert_k2*cv + (1<<6))>>7); \
		int32_t cb = cy + ((rdp_convert_k3*cu + (1<<6))>>7); \
		/* This step is done in the CC */ \
		cr = (((cr - (rdp_convert_k4<<1))*rdp_convert_k5 + (1<<6))>>7) + cr; \
		cg = (((cg - (rdp_convert_k4<<1))*rdp_convert_k5 + (1<<6))>>7) + cg; \
		cb = (((cb - (rdp_convert_k4<<1))*rdp_convert_k5 + (1<<6))>>7) + cb; \
		/* And here's a final clamp... */ \
		cr = (cr < 0 ? 0 : cr > 0xFF ? 0xFF : cr); \
		cg = (cg < 0 ? 0 : cg > 0xFF ? 0xFF : cg); \
		cb = (cb < 0 ? 0 : cb > 0xFF ? 0xFF : cb); \
		data = (0 \
			| (cr<<24) \
			| (cg<<16) \
			| (cb<<8) \
			| 0xFF \
		); \
	}
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
#undef TEX_FUNCTION_NAME

// 16bpp RGBA
#define TEX_FUNCTION_NAME tex_rect_rdp_16rgba
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
#undef TEX_FUNCTION_NAME

// 32bpp RGBA
#define TEX_FUNCTION_NAME tex_rect_rdp_32rgba
#define GET_TEX_DATA() \
	uint32_t data = rdp_tmem[(tmem_offs+data_s)&0x3FF];
#include "rdp/tex-rect-per-fmt.h"
#undef GET_TEX_DATA
#undef TEX_FUNCTION_NAME

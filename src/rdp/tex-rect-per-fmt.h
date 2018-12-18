#define APPLY_PALETTE_RGBA16(data) \
	data = rdp_tmem[(0x200+((data+(rdp_tile_palette<<4))<<1))&0x3FF]>>(16*((~data)&0x1)); \
	data &= 0xFFFF; \
	data = (0 \
		| (((data>>0)&0x1)*0xFF) \
		| (((((data>>1)&0x1F)*0x21)>>2)<<8) \
		| (((((data>>6)&0x1F)*0x21)>>2)<<16) \
		| (((((data>>11)&0x1F)*0x21)>>2)<<24) \
	);

#define DRAW_FULL() \
	for(int y = yl; y < yh; y++, acc_t += dtdy) { \
		int32_t acc_s = s; \
		uint32_t dram_offs = (rdp_color_image_addr>>2); \
		uint32_t tmem_offs = (rdp_tile_tmem_addr<<1); \
		dram_offs += (dram_stride>>OFFS_Y_SHIFT)*y; \
		tmem_offs += tmem_stride*((acc_t>>rdp_tile_shift_t)&rdp_tile_mask_t); \
		for(int x = xl; x < xh; x++, acc_s += dsdx) { \
			rdp_cooldown += 1; \
			uint32_t data_s = ((acc_s>>rdp_tile_shift_s)&rdp_tile_mask_s); \
			GET_TEX_DATA(); \
			uint32_t src_data = data; \
			uint32_t dst_data = READ_DEST_COLOR(); \
			uint32_t s0 = src_data&0x00FF00FF; \
			uint32_t s1 = src_data&0xFF00FF00; \
			uint32_t d0 = dst_data&0x00FF00FF; \
			uint32_t d1 = dst_data&0xFF00FF00; \
			uint32_t als = src_data&0xFF; \
			if(als >= 0x7F) { als += 1; } \
			uint32_t ald = 0x100-als; \
			uint32_t c0 = (als*s0+ald*d0); \
			uint32_t c1 = (als*(s1>>8)+ald*(d1>>8)); \
			c0 >>= 8; \
			c0 &= 0x00FF00FF; \
			c1 &= 0xFF00FF00; \
			c0 |= c1; \
			WRITE_PIXEL(c0); \
		} \
	}


#define IN_PIXEL_16 ((uint32_t)(((uint16_t *)&ram[dram_offs&(RAM_SIZE_WORDS-1)])[x^0x1]))
#define IN_PIXEL_32 ram[(dram_offs+x)&(RAM_SIZE_WORDS-1)]
#define OUT_PIXEL_16 ((uint16_t *)&ram[dram_offs&(RAM_SIZE_WORDS-1)])[x^0x1]
#define OUT_PIXEL_32 ram[(dram_offs+x)&(RAM_SIZE_WORDS-1)]

rdp_debug_printf("Render %d,%d -> %d,%d w=%d\n", xl, yl, xh, yh, tmem_stride);

switch(rdp_color_image_size)
{
	case 2: { // 16bpp
#define OFFS_Y_SHIFT 1
#define READ_DEST_COLOR() (0 \
	| (((IN_PIXEL_16>>0)&0x1)*0x000000FF) \
	| (((((IN_PIXEL_16>>1)&0x1F)*0x21)>>2)<<8) \
	| (((((IN_PIXEL_16>>6)&0x1F)*0x21)>>2)<<16) \
	| (((((IN_PIXEL_16>>11)&0x1F)*0x21)>>2)<<24) \
)
#define WRITE_PIXEL(c) OUT_PIXEL_16 = (0 \
	| ((c>>7)&0x1) \
	| (((c>>11)&0x1F)<<1) \
	| (((c>>19)&0x1F)<<6) \
	| (((c>>27)&0x1F)<<11) \
);
		DRAW_FULL();
#undef OFFS_Y_SHIFT
#undef READ_DEST_COLOR
#undef WRITE_PIXEL
	} break;

	case 3: { // 32bpp
#define OFFS_Y_SHIFT 0
#define READ_DEST_COLOR() IN_PIXEL_32
#define WRITE_PIXEL(c) OUT_PIXEL_32 = (c);
		DRAW_FULL();
#undef OFFS_Y_SHIFT
#undef READ_DEST_COLOR
#undef WRITE_PIXEL
	} break;
}

#undef OUT_PIXEL_16
#undef OUT_PIXEL_32

#undef DRAW_FULL

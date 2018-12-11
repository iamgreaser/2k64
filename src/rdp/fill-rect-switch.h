#define DRAW_FULL() \
	for(int y = yl; y < yh; y++) { \
		uint32_t offs = (rdp_color_image_addr>>2); \
		offs += ((rdp_color_image_width+1)>>OFFS_Y_SHIFT)*y; \
		for(int x = xl; x < xh; x++) { \
			CALC_FILL_COLOR(); \
			WRITE_PIXEL(READ_COLOR()); \
		} \
	}

#define OUT_PIXEL_16 ((uint16_t *)&ram[offs&(RAM_SIZE_WORDS-1)])[x^0x1]
#define OUT_PIXEL_32 ram[(offs+x)&(RAM_SIZE_WORDS-1)]

switch(rdp_color_image_size)
{
	case 2: { // 16bpp
#define OFFS_Y_SHIFT 1
#define WRITE_PIXEL(c) OUT_PIXEL_16 = (c);
#ifdef IS_FILL_MODE
#define READ_COLOR() (FILL_COLOR>>(16*(x&0x1)))
#else
#define READ_COLOR() (0 \
	| (((FILL_COLOR>>11)&0x1F)<<1) \
	| (((FILL_COLOR>>19)&0x1F)<<6) \
	| (((FILL_COLOR>>27)&0x1F)<<11) \
)
#endif
		DRAW_FULL();
#undef OFFS_Y_SHIFT
#undef WRITE_PIXEL
#undef READ_COLOR
	} break;

	case 3: { // 32bpp
#define OFFS_Y_SHIFT 0
#define WRITE_PIXEL(c) OUT_PIXEL_32 = (c);
#define READ_COLOR() FILL_COLOR
		DRAW_FULL();
#undef OFFS_Y_SHIFT
#undef WRITE_PIXEL
#undef READ_COLOR
	} break;
}

#undef OUT_PIXEL_16
#undef OUT_PIXEL_32

#undef DRAW_FULL

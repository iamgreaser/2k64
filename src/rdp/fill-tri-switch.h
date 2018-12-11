#define GEN_EFF_X(x0, x1) \
	int32_t eff_##x0 = x0>>16; \
	int32_t eff_##x1 = x1>>16; \
	if(eff_##x0 < rdp_scissor_xh>>2) { eff_##x0 = rdp_scissor_xh>>2; } \
	if(eff_##x1 > (rdp_scissor_xl>>2)-1) { eff_##x1 = (rdp_scissor_xl>>2)-1; }

#define OUT_PIXEL_16 ((uint16_t *)&ram[offs&(RAM_SIZE_WORDS-1)])[x^0x1]
#define OUT_PIXEL_32 ram[(offs+x)&(RAM_SIZE_WORDS-1)]

#define DRAW_HALF(y_top, y_bottom, dx0, x_left, x_right) \
	for(int y = y_top>>4; y < y_bottom>>4; y++, x0 += dx0, x1 += dxhdy) { \
		uint32_t offs = (rdp_color_image_addr>>2); \
		offs += ((rdp_color_image_width+1)>>OFFS_Y_SHIFT)*y; \
		GEN_EFF_X(x_left, x_right); \
		for(int x = eff_##x_left; x < eff_##x_right; x++) { \
			rdp_cooldown += 1; \
			CALC_FILL_COLOR(); \
			WRITE_PIXEL(READ_COLOR()); \
		} \
	}

#define DRAW_FULL(x_left, x_right) \
	DRAW_HALF(yh, ym, dxmdy, x_left, x_right); \
	x0 = xl; \
	DRAW_HALF(ym, yl, dxldy, x_left, x_right);

switch(rdp_color_image_size)
{
	case 2: // 16bpp
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
	if(!left_major) {
		DRAW_FULL(x0, x1);
	} else {
		DRAW_FULL(x1, x0);
	} break;
#undef OFFS_Y_SHIFT
#undef WRITE_PIXEL
#undef READ_COLOR
	case 3: // 32bpp
#define OFFS_Y_SHIFT 0
#define WRITE_PIXEL(c) OUT_PIXEL_32 = (c);
#define READ_COLOR() FILL_COLOR
	if(!left_major) {
		DRAW_FULL(x0, x1);
	} else {
		DRAW_FULL(x1, x0);
	} break;
#undef OFFS_Y_SHIFT
#undef WRITE_PIXEL
#undef READ_COLOR
}
#undef DRAW_FULL
#undef DRAW_HALF
#undef FILL_COLOR

#undef OUT_PIXEL_16
#undef OUT_PIXEL_32

#undef GEN_EFF_X


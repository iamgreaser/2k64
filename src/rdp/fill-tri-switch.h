#define GEN_EFF_X(x0, x1) \
	int32_t eff_##x0 = x0>>16; \
	int32_t eff_##x1 = x1>>16; \
	if(eff_##x0 < rdp_scissor_xh>>2) { eff_##x0 = rdp_scissor_xh>>2; } \
	if(eff_##x1 > rdp_scissor_xl>>2) { eff_##x1 = rdp_scissor_xl>>2; }

#define OUT_PIXEL ram[(offs+x)&(RAM_SIZE_WORDS-1)]
#if 0
#define FILL_COLOR rdp_fill_color
#else
#define FILL_COLOR rdp_blend_color
#endif

switch(rdp_color_image_size)
{
	case 3: // 32bpp
	if(!left_major) {
		for(int y = yh>>4; y < ym>>4; y++, x0 += dxmdy, x1 += dxhdy) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += (rdp_color_image_width+1)*y;
			GEN_EFF_X(x0, x1);
			for(int x = eff_x0; x < eff_x1; x++) {
				OUT_PIXEL = FILL_COLOR;
			}
		}
		x0 = xl;
		for(int y = ym>>4; y < yl>>4; y++, x0 += dxldy, x1 += dxhdy) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += (rdp_color_image_width+1)*y;
			GEN_EFF_X(x0, x1);
			for(int x = eff_x0; x < eff_x1; x++) {
				OUT_PIXEL = FILL_COLOR;
			}
		}
	} else {
		for(int y = yh>>4; y < ym>>4; y++, x0 += dxmdy, x1 += dxhdy) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += (rdp_color_image_width+1)*y;
			GEN_EFF_X(x1, x0);
			for(int x = eff_x1; x < eff_x0; x++) {
				OUT_PIXEL = FILL_COLOR;
			}
		}
		x0 = xl;
		for(int y = ym>>4; y < yl>>4; y++, x0 += dxldy, x1 += dxhdy) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += (rdp_color_image_width+1)*y;
			GEN_EFF_X(x1, x0);
			for(int x = eff_x1; x < eff_x0; x++) {
				OUT_PIXEL = FILL_COLOR;
			}
		}
	} break;
}

#undef GEN_EFF_X


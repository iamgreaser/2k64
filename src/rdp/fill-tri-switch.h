switch(rdp_color_image_size)
{
	case 3: // 32bpp
	if(!left_major) {
		for(int y = yh>>4; y < ym>>4; y++, x0 += dxmdy, x1 += dxhdy) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += (rdp_color_image_width+1)*y;
			for(int x = x0>>16; x < x1>>16; x++) {
				ram[(offs+x)&(RAM_SIZE_WORDS-1)] = rdp_fill_color;
			}
		}
		x0 = xl;
		for(int y = ym>>4; y < yl>>4; y++, x0 += dxldy, x1 += dxhdy) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += (rdp_color_image_width+1)*y;
			for(int x = x0>>16; x < x1>>16; x++) {
				ram[(offs+x)&(RAM_SIZE_WORDS-1)] = rdp_fill_color;
			}
		}
	} else {
		for(int y = yh>>4; y < ym>>4; y++, x0 += dxmdy, x1 += dxhdy) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += (rdp_color_image_width+1)*y;
			for(int x = x1>>16; x < x0>>16; x++) {
				ram[(offs+x)&(RAM_SIZE_WORDS-1)] = rdp_fill_color;
			}
		}
		x0 = xl;
		for(int y = ym>>4; y < yl>>4; y++, x0 += dxldy, x1 += dxhdy) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += (rdp_color_image_width+1)*y;
			for(int x = x1>>16; x < x0>>16; x++) {
				ram[(offs+x)&(RAM_SIZE_WORDS-1)] = rdp_fill_color;
			}
		}
	} break;
}


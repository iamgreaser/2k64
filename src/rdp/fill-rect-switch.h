switch(rdp_color_image_size)
{
	case 2: // 15bpp
		// FIXME this isn't actually right AFAIK
		xl >>= 1;
		xh >>= 1;
		for(int y = yl; y < yh; y++) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += ((rdp_color_image_width+1)>>1)*y;
			for(int x = xl; x < xh; x++) {
				ram[(offs+x)&RAM_SIZE_WORDS-1] = rdp_fill_color;
			}
		}
		break;
	case 3: // 32bpp
		for(int y = yl; y < yh; y++) {
			uint32_t offs = (rdp_color_image_addr>>2);
			offs += (rdp_color_image_width+1)*y;
			for(int x = xl; x < xh; x++) {
				ram[(offs+x)&RAM_SIZE_WORDS-1] = rdp_fill_color;
			}
		}
		break;
}

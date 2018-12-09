switch(rdp_color_image_size)
{
	case 3: // 32bpp
		for(int y = yl; y < yh; y++, acc_t += dtdy) {
			int16_t acc_s = s;
			uint32_t dram_offs = (rdp_color_image_addr>>2);
			uint32_t tmem_offs = (rdp_tile_tmem_addr<<3);
			tmem_offs += tmem_stride*((acc_t>>rdp_tile_shift_t)&rdp_tile_mask_t);
			dram_offs += dram_stride*y;
			for(int x = xl; x < xh; x++, acc_s += dsdx) {
				uint32_t data = rdp_tmem[(tmem_offs
					+((acc_s>>rdp_tile_shift_s)&rdp_tile_mask_s)
				)&0x3FF];

				// FIXME use colour combiners for once
				uint32_t *src_data = &data;
				uint32_t *dst_data = &ram[(dram_offs+x)&(RAM_SIZE_WORDS-1)];
				uint32_t s0 = *src_data&0x00FF00FF;
				uint32_t s1 = *src_data&0xFF00FF00;
				uint32_t d0 = *dst_data&0x00FF00FF;
				uint32_t d1 = *dst_data&0xFF00FF00;
				uint32_t als = *src_data&0xFF;
				als += (als>>7);
				uint32_t ald = 0x100-als;
				uint32_t c0 = (als*s0+ald*d0);
				uint32_t c1 = (als*(s1>>8)+ald*(d1>>8));
				c0 >>= 8;
				c0 &= 0x00FF00FF;
				c1 &= 0xFF00FF00;
				*dst_data = c0|c1;
			}
		}
		break;
}

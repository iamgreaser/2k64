{
	for(int iy = 0, ay = ((vi_y_scale_reg>>16)&0xFFF); iy < 480; iy++, ay += (vi_y_scale_reg&0xFFF)) {
	for(int ix = 0, ax = ((vi_x_scale_reg>>16)&0xFFF); ix < 640; ix++, ax += (vi_x_scale_reg&0xFFF)) {
		uint32_t offs = vi_origin_reg;
		int x = ax>>10;
		int y = ay>>11;
#if VI_BITS == 32
		offs += y*vi_width_reg*4;
		offs += x*4;
		offs &= (RAM_SIZE_BYTES-4);
		uint32_t data = *(uint32_t *)(((uint8_t *)ram) + offs);
		((uint32_t *)(surface->pixels + surface->pitch*iy))[ix] = data;
#else
#if VI_BITS == 16
		offs += y*vi_width_reg*2;
		offs += x*2;
		offs ^= 2;
		offs &= (RAM_SIZE_BYTES-2);
		uint32_t idata = *(uint16_t *)(((uint8_t *)ram) + offs);
		uint32_t data = (0
			|(((((idata>>1 )&0x1F)*0x21)>>2)<<8)
			|(((((idata>>6 )&0x1F)*0x21)>>2)<<16)
			|(((((idata>>11)&0x1F)*0x21)>>2)<<24)
		);
		((uint32_t *)(surface->pixels + surface->pitch*iy))[ix] = data;
#else
#error "VI_BITS must be 16 or 32"
#endif
#endif
	}
	}
}

#undef VI_BITS


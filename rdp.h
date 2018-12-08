#define RDSR_USE_XBUS (1<<0)
#define RDSR_RDP_FROZEN (1<<1)
#define RDSR_RDP_FLUSHED (1<<2)
#define RDSR_GCLK_ALIVE (1<<3)
#define RDSR_TMEM_BUSY (1<<4)
#define RDSR_PIPELINE_BUSY (1<<5)
#define RDSR_CMD_BUSY (1<<6)
#define RDSR_CMD_READY (1<<7)
#define RDSR_DMA_BUSY (1<<8)
#define RDSR_CMD_START_VALID (1<<9)
#define RDSR_CMD_END_VALID (1<<10)

uint32_t rdp_tmem[0x400];

uint64_t rdp_cmd_buffer[22];
uint64_t rdp_cmd_len = 0;

// 0x2D Set Scissor
uint32_t rdp_scissor_xh = 0;
uint32_t rdp_scissor_yh = 0;
uint32_t rdp_scissor_xl = 0;
uint32_t rdp_scissor_yl = 0;
bool rdp_scissor_f = false;
bool rdp_scissor_o = false;

// 0x2F Set Other Modes
// I refuse to split this into multiple fields.
#define RDOM_ATOMIC_PRIM (1<<55)
#define RDOM_CYCLE_TYPE_MASK (0x3<<52)
#define RDOM_CYCLE_TYPE_1CYCLE (0x0<<52)
#define RDOM_CYCLE_TYPE_2CYCLE (0x1<<52)
#define RDOM_CYCLE_TYPE_COPY (0x2<<52)
#define RDOM_CYCLE_TYPE_FILL (0x3<<52)
#define RDOM_PERSP_TEX_EN (1<<51)
#define RDOM_DETAIL_TEX_EN (1<<50)
#define RDOM_SHARPEN_TEX_EN (1<<49)
#define RDOM_TEX_LOD_EN (1<<48)
#define RDOM_EN_TLUT (1<<47)
#define RDOM_TLUT_TYPE_MASK (0x1<<46)
#define RDOM_TLUT_TYPE_RGBA (0x0<<46)
#define RDOM_TLUT_TYPE_IA (0x1<<46)
#define RDOM_SAMPLE_TYPE_MASK (0x1<<45)
#define RDOM_SAMPLE_TYPE_1 (0x0<<45)
#define RDOM_SAMPLE_TYPE_2 (0x1<<45)
#define RDOM_MID_TEXEL (1<<44)
#define RDOM_BI_LERP_0 (1<<43)
#define RDOM_BI_LERP_1 (1<<42)
#define RDOM_CONVERT_ONE (1<<41)
#define RDOM_KEY_EN (1<<40)
#define RDOM_RGB_DITHER_SEL_MASK (0x3<<38)
#define RDOM_RGB_DITHER_SEL_MAGIC (0x0<<38)
#define RDOM_RGB_DITHER_SEL_STANDARD (0x1<<38)
#define RDOM_RGB_DITHER_SEL_NOISE (0x2<<38)
#define RDOM_RGB_DITHER_SEL_NONE (0x3<<38)
#define RDOM_ALPHA_DITHER_SEL_MASK (0x3<<36)
#define RDOM_ALPHA_DITHER_SEL_PATTERN (0x0<<36)
#define RDOM_ALPHA_DITHER_SEL_NOTPATTERN (0x1<<36)
#define RDOM_ALPHA_DITHER_SEL_NOISE (0x2<<36)
#define RDOM_ALPHA_DITHER_SEL_NONE (0x3<<36)
#define RDOM_BLEND_M1A_CYCLE0_SHIFT 30
#define RDOM_BLEND_M1A_CYCLE0_MASK (0x3<<RDOM_BLEND_M1A_CYCLE0_SHIFT)
#define RDOM_BLEND_M1A_CYCLE1_SHIFT 28
#define RDOM_BLEND_M1A_CYCLE1_MASK (0x3<<RDOM_BLEND_M1A_CYCLE1_SHIFT)
#define RDOM_BLEND_M1B_CYCLE0_SHIFT 26
#define RDOM_BLEND_M1B_CYCLE0_MASK (0x3<<RDOM_BLEND_M1B_CYCLE0_SHIFT)
#define RDOM_BLEND_M1B_CYCLE1_SHIFT 24
#define RDOM_BLEND_M1B_CYCLE1_MASK (0x3<<RDOM_BLEND_M1B_CYCLE1_SHIFT)
#define RDOM_BLEND_M2A_CYCLE0_SHIFT 22
#define RDOM_BLEND_M2A_CYCLE0_MASK (0x3<<RDOM_BLEND_M2A_CYCLE0_SHIFT)
#define RDOM_BLEND_M2A_CYCLE1_SHIFT 20
#define RDOM_BLEND_M2A_CYCLE1_MASK (0x3<<RDOM_BLEND_M2A_CYCLE1_SHIFT)
#define RDOM_BLEND_M2B_CYCLE0_SHIFT 18
#define RDOM_BLEND_M2B_CYCLE0_MASK (0x3<<RDOM_BLEND_M2B_CYCLE0_SHIFT)
#define RDOM_BLEND_M2B_CYCLE1_SHIFT 16
#define RDOM_BLEND_M2B_CYCLE1_MASK (0x3<<RDOM_BLEND_M2B_CYCLE1_SHIFT)
#define RDOM_FORCE_BLEND (1<<14)
#define RDOM_ALPHA_CVG_SELECT (1<<13)
#define RDOM_CVG_TIMES_ALPHA (1<<12)
#define RDOM_Z_MODE_MASK (0x3<<10)
#define RDOM_Z_MODE_OPAQUE (0x0<<10)
#define RDOM_Z_MODE_INTERPENETRATING (0x1<<10)
#define RDOM_Z_MODE_TRANSPARENT (0x2<<10)
#define RDOM_Z_MODE_DECAL (0x3<<10)
#define RDOM_CVG_DEST_MASK (0x3<<8)
#define RDOM_CVG_DEST_CLAMP (0x0<<8)
#define RDOM_CVG_DEST_WRAP (0x1<<8)
#define RDOM_CVG_DEST_ZAP (0x2<<8)
#define RDOM_CVG_DEST_SAVE (0x3<<8)
#define RDOM_COLOR_ON_CVG (1<<7)
#define RDOM_IMAGE_READ_EN (1<<6)
#define RDOM_Z_UPDATE_EN (1<<5)
#define RDOM_Z_COMPARE_EN (1<<4)
#define RDOM_ANTIALIAS_EN (1<<3)
#define RDOM_Z_SOURCE_SEL (1<<2)
#define RDOM_DITHER_ALPHA_EN (1<<1)
#define RDOM_ALPHA_COMPARE_EN (1<<0)
uint64_t rdp_other_modes = 0;

// 0x35 Set Tile
uint32_t rdp_tile_format = 0;
uint32_t rdp_tile_size = 0;
uint32_t rdp_tile_line = 0;
uint32_t rdp_tile_tmem_addr = 0;
uint32_t rdp_tile_idx = 0;
uint32_t rdp_tile_palette = 0;
bool rdp_tile_ct = false;
bool rdp_tile_mt = false;
uint32_t rdp_tile_mask_t = 0;
uint32_t rdp_tile_shift_t = 0+5;
bool rdp_tile_cs = false;
bool rdp_tile_ms = false;
uint32_t rdp_tile_mask_s = 0;
uint32_t rdp_tile_shift_s = 0+5;

// 0x37 Set Fill Color
uint32_t rdp_fill_color = 0;

// 0x38 Set Fog Color
uint32_t rdp_fog_color = 0;

// 0x39 Set Blend Color
uint32_t rdp_blend_color = 0;

// 0x3D Set Texture Image
uint32_t rdp_texture_image_format = 0;
uint32_t rdp_texture_image_size = 0;
uint32_t rdp_texture_image_width = 0;
uint32_t rdp_texture_image_addr = 0;

// 0x3E Set Z Image
uint32_t rdp_z_image_addr = 0;

// 0x3F Set Color Image
uint32_t rdp_color_image_format = 0;
uint32_t rdp_color_image_size = 0;
uint32_t rdp_color_image_width = 0;
uint32_t rdp_color_image_addr = 0;

void rdp_run_one_command(void) {
	uint64_t cmd;

	if((dpc_status & RDSR_USE_XBUS) != 0) {
		// XBUS DMEM DMA
		uint64_t cmdh = rsp_mem[((dpc_current>>2)+0) & 0x3FF];
		uint64_t cmdl = rsp_mem[((dpc_current>>2)+1) & 0x3FF];
		cmd = (cmdh<<32)|cmdl;
	} else {
		// Primary DMA
		uint64_t cmdh = ram[((dpc_current>>2)+0) & (RAM_SIZE_WORDS-1)];
		uint64_t cmdl = ram[((dpc_current>>2)+1) & (RAM_SIZE_WORDS-1)];
		cmd = (cmdh<<32)|cmdl;
	}
	assert(rdp_cmd_len >= 0 && rdp_cmd_len < sizeof(rdp_cmd_buffer)/sizeof(rdp_cmd_buffer[0]));
	rdp_cmd_buffer[rdp_cmd_len++] = cmd;
	cmd = rdp_cmd_buffer[0];
	switch(cmd>>56) {
		case 0x24:
		case 0x25: {
			if(rdp_cmd_len < 2) break;
			bool is_flipped = (((cmd>>56)&0x1) != 0);

			if(is_flipped) {
				rdp_debug_printf("RDP %016llX Texture Rectangle Flip\n", cmd);
			} else {
				rdp_debug_printf("RDP %016llX Texture Rectangle\n", cmd);
			}

			uint32_t sl = (cmd>>44)&0xFFF;
			uint32_t tl = (cmd>>32)&0xFFF;
			uint32_t tile = (cmd>>24)&0x7;
			uint32_t sh = (cmd>>12)&0xFFF;
			uint32_t th = (cmd>>0)&0xFFF;
			rdp_debug_printf("Render %d,%d -> %d,%d\n", sl, tl, sh, th);

			int16_t s = (rdp_cmd_buffer[1]>>48);
			int16_t t = (rdp_cmd_buffer[1]>>32);
			int16_t dsdx = (rdp_cmd_buffer[1]>>16);
			int16_t dtdy = (rdp_cmd_buffer[1]);
			int32_t yl = (cmd>>0)&0xFFF;
			int32_t xl = (cmd>>12)&0xFFF;
			int32_t yh = (cmd>>32)&0xFFF;
			int32_t xh = (cmd>>44)&0xFFF;

			if(xh < rdp_scissor_xh) { xh = rdp_scissor_xh; }
			if(yh < rdp_scissor_yh) { yh = rdp_scissor_yh; }
			if(xl > rdp_scissor_xl) { xl = rdp_scissor_xl; }
			if(yl > rdp_scissor_yl) { yl = rdp_scissor_yl; }

			xh >>= 2;
			yh >>= 2;
			xl >>= 2;
			yl >>= 2;
			int tmem_stride = (rdp_tile_line+0)<<3;
			int dram_stride = (rdp_color_image_width+1);
			rdp_debug_printf("Render %d,%d -> %d,%d w=%d\n", xl, yl, xh, yh, tmem_stride);
			int16_t acc_t = t;
			switch(rdp_color_image_size)
			{
				case 3: // 32bpp
					for(int y = yl; y < yh; y++, acc_t += dtdy) {
						int16_t acc_s = s;
						uint32_t tmem_offs = (rdp_tile_tmem_addr<<3);
						uint32_t dram_offs = (rdp_color_image_addr>>2);
						tmem_offs += tmem_stride*((acc_t>>rdp_tile_shift_t)&rdp_tile_mask_t);
						dram_offs += dram_stride*y;
						for(int x = xl; x < xh; x++, acc_s += dsdx) {
							uint32_t data = rdp_tmem[(tmem_offs
								+((acc_s>>rdp_tile_shift_s)&rdp_tile_mask_s)
							)&0x3FF];

							// FIXME use colour combiners for once
							uint32_t *dst_data = &ram[(dram_offs+x)&(RAM_SIZE_WORDS-1)];
							uint32_t s0 = data&0x00FF00FF;
							uint32_t s1 = data&0xFF00FF00;
							uint32_t d0 = *dst_data&0x00FF00FF;
							uint32_t d1 = *dst_data&0xFF00FF00;
							uint32_t als = data&0xFF;
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

			rdp_cmd_len = 0;
		} break;

		case 0x27:
			rdp_debug_printf("RDP %016llX Sync Pipe\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x28:
			rdp_debug_printf("RDP %016llX Sync Tile\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x29:
			rdp_debug_printf("RDP %016llX Sync Full\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x2D:
			rdp_debug_printf("RDP %016llX Set Scissor\n", cmd);
			rdp_scissor_yl = (cmd>>0)&0xFFF;
			rdp_scissor_xl = (cmd>>12)&0xFFF;
			rdp_scissor_yh = (cmd>>32)&0xFFF;
			rdp_scissor_xh = (cmd>>44)&0xFFF;
			rdp_scissor_o = (((cmd>>24)&0x1) != 0);
			rdp_scissor_f = (((cmd>>25)&0x1) != 0);
			rdp_cmd_len = 0;
			break;

		case 0x2E:
			rdp_debug_printf("RDP %016llX Set Prim Depth\n", cmd);
			rdp_debug_printf(" --- TODO! --- \n");
			rdp_cmd_len = 0;
			break;

		case 0x2F:
			rdp_debug_printf("RDP %016llX Set Other Modes\n", cmd);
			rdp_other_modes = (cmd<<8)>>8;
			rdp_cmd_len = 0;
			break;

		case 0x30:
			rdp_debug_printf("RDP %016llX Load Tlut\n", cmd);
			rdp_debug_printf(" --- TODO! --- \n");
			rdp_cmd_len = 0;
			break;

		case 0x34:
			rdp_debug_printf("RDP %016llX Load Tile\n", cmd);
			{
				uint32_t sl = (cmd>>44)&0xFFF;
				uint32_t tl = (cmd>>32)&0xFFF;
				uint32_t tile = (cmd>>24)&0x7;
				uint32_t sh = (cmd>>12)&0xFFF;
				uint32_t th = (cmd>>0)&0xFFF;
				rdp_debug_printf("Load %d,%d -> %d,%d\n", sl, tl, sh, th);
				sl >>= 2;
				sh >>= 2;
				tl >>= 2;
				th >>= 2;
				switch(rdp_tile_size)
				{
					case 0: sl >>= 3; sh >>= 3; break;
					case 1: sl >>= 2; sh >>= 2; break;
					case 2: sl >>= 1; sh >>= 1; break;
					case 3: sl >>= 0; sh >>= 0; break;
				}
				sl <<= 1;
				sh += 1;
				sh <<= 1;
				tl <<= 1;
				th += 1;
				th <<= 1;
				int tmem_stride = (rdp_tile_line+0)<<3;
				int dram_stride = (rdp_texture_image_width+1);
				rdp_debug_printf("Load %d,%d -> %d,%d - w=%d\n", sl, tl, sh, th, tmem_stride);
				for(int y = tl; y < th; y++) {
					uint32_t tmem_offs = (rdp_tile_tmem_addr<<3);
					uint32_t dram_offs = (rdp_texture_image_addr>>2);
					tmem_offs += tmem_stride*y;
					dram_offs += dram_stride*y;
					for(int x = sl; x < sh; x++) {
						rdp_tmem[(tmem_offs+x)&0x3FF] = ram[(dram_offs+x)&RAM_SIZE_WORDS-1];
					}
				}
			}
			rdp_cmd_len = 0;
			break;

		case 0x35:
			rdp_debug_printf("RDP %016llX Set Tile\n", cmd);
			rdp_tile_format = (cmd>>53)&0x7;
			rdp_tile_size = (cmd>>51)&0x3;
			rdp_tile_line = (cmd>>41)&0x1FF;
			rdp_tile_tmem_addr = (cmd>>32)&0x1FF;
			rdp_tile_idx = (cmd>>24)&0x7;
			rdp_tile_palette = (cmd>>20)&0xF;
			rdp_tile_ct = ((cmd>>19)&0x1)!=0;
			rdp_tile_mt = ((cmd>>18)&0x1)!=0;
			rdp_tile_mask_t = (cmd>>14)&0xF;
			rdp_tile_shift_t = (cmd>>10)&0xF;
			rdp_tile_cs = ((cmd>>9)&0x1)!=0;
			rdp_tile_ms = ((cmd>>8)&0x1)!=0;
			rdp_tile_mask_s = (cmd>>4)&0xF;
			rdp_tile_shift_s = (cmd>>0)&0xF;

			rdp_tile_shift_t += 10;
			rdp_tile_shift_s += 10;

			// TODO: clamp
			if(rdp_tile_mask_s == 0) { rdp_tile_mask_s = 0xF; }
			if(rdp_tile_mask_t == 0) { rdp_tile_mask_t = 0xF; }
			rdp_tile_mask_s += 1;
			rdp_tile_mask_t += 1;
			rdp_tile_mask_s <<= 2;
			rdp_tile_mask_t <<= 2;
			rdp_tile_mask_s -= 1;
			rdp_tile_mask_t -= 1;

			rdp_cmd_len = 0;
			break;

		case 0x36:
			rdp_debug_printf("RDP %016llX Fill Rectangle\n", cmd);
			{
				int32_t yl = (cmd>>0)&0xFFF;
				int32_t xl = (cmd>>12)&0xFFF;
				int32_t yh = (cmd>>32)&0xFFF;
				int32_t xh = (cmd>>44)&0xFFF;

				if(xh < rdp_scissor_xh) { xh = rdp_scissor_xh; }
				if(yh < rdp_scissor_yh) { yh = rdp_scissor_yh; }
				if(xl > rdp_scissor_xl) { xl = rdp_scissor_xl; }
				if(yl > rdp_scissor_yl) { yl = rdp_scissor_yl; }

				xh >>= 2;
				yh >>= 2;
				xl >>= 2;
				yl >>= 2;
				rdp_debug_printf("Render %d,%d -> %d,%d\n", xl, yl, xh, yh);
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
			}
			rdp_cmd_len = 0;
			break;

		case 0x37:
			rdp_debug_printf("RDP %016llX Set Fill Color\n", cmd);
			rdp_fill_color = cmd&0xFFFFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x38:
			rdp_debug_printf("RDP %016llX Set Fog Color\n", cmd);
			rdp_fog_color = cmd&0xFFFFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x39:
			rdp_debug_printf("RDP %016llX Set Blend Color\n", cmd);
			rdp_blend_color = cmd&0xFFFFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x3C:
			rdp_debug_printf("RDP %016llX Set Combine Mode\n", cmd);
			rdp_debug_printf(" --- TODO! --- \n");
			rdp_cmd_len = 0;
			break;

		case 0x3D:
			rdp_debug_printf("RDP %016llX Set Texture Image\n", cmd);
			rdp_texture_image_format = (cmd>>53)&0x7;
			rdp_texture_image_size = (cmd>>51)&0x3;
			rdp_texture_image_width = (cmd>>32)&0x3FF;
			rdp_texture_image_addr = (cmd>>0)&0x3FFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x3E:
			rdp_debug_printf("RDP %016llX Set Z Image\n", cmd);
			rdp_texture_image_addr = (cmd>>0)&0x3FFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x3F:
			rdp_debug_printf("RDP %016llX Set Color Image\n", cmd);
			rdp_color_image_format = (cmd>>53)&0x7;
			rdp_color_image_size = (cmd>>51)&0x3;
			rdp_color_image_width = (cmd>>32)&0x3FF;
			rdp_color_image_addr = (cmd>>0)&0x3FFFFFF;
			rdp_cmd_len = 0;
			break;

		default:
			rdp_debug_printf("RDP unhandled command %016llX\n", cmd);
			rdp_cmd_len = 0;
			break;
	}

	dpc_current += 8;
}

void rdp_run_commands(void)
{
	// Do we have commands to run?
	if(dpc_current == dpc_end_saved) {
		// Clear busy
		dpc_status &= ~RDSR_CMD_BUSY;
		dpc_status &= ~RDSR_DMA_BUSY;
		dpc_status |= RDSR_CMD_READY;

		// Are start and end valid?
		if((dpc_status & (RDSR_CMD_START_VALID|RDSR_CMD_END_VALID)) == (RDSR_CMD_START_VALID|RDSR_CMD_END_VALID)) {
			dpc_current = dpc_start;
			dpc_end_saved = dpc_end;
			dpc_status &= ~(RDSR_CMD_START_VALID|RDSR_CMD_END_VALID);

			// Set busy
			dpc_status |= (1<<8);
		}
	} else {
		// Set busy
		dpc_status |= RDSR_CMD_BUSY;
		dpc_status |= RDSR_DMA_BUSY;
		dpc_status &= ~RDSR_CMD_READY;

		// Run commands
		rdp_run_one_command();
	}
}

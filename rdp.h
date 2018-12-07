
uint64_t rdp_cmd_buffer[22];
uint64_t rdp_cmd_len = 0;

uint32_t rdp_scissor_xh = 0;
uint32_t rdp_scissor_yh = 0;
uint32_t rdp_scissor_xl = 0;
uint32_t rdp_scissor_yl = 0;
bool rdp_scissor_f = false;
bool rdp_scissor_o = false;

uint32_t rdp_fill_color = 0;
uint32_t rdp_fog_color = 0;
uint32_t rdp_blend_color = 0;

uint32_t rdp_texture_image_format = 0;
uint32_t rdp_texture_image_size = 0;
uint32_t rdp_texture_image_width = 0;
uint32_t rdp_texture_image_addr = 0;

uint32_t rdp_z_image_addr = 0;

uint32_t rdp_color_image_format = 0;
uint32_t rdp_color_image_size = 0;
uint32_t rdp_color_image_width = 0;
uint32_t rdp_color_image_addr = 0;

void rdp_run_one_command(void) {
	uint64_t cmd;

	if((dpc_status & 0x0001) != 0) {
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
			if(rdp_cmd_len < 2) break;
			printf("RDP %016llX Texture Rectangle\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x27:
			printf("RDP %016llX Sync Pipe\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x28:
			printf("RDP %016llX Sync Tile\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x29:
			printf("RDP %016llX Sync Full\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x2D:
			printf("RDP %016llX Set Scissor\n", cmd);
			rdp_scissor_yl = (cmd>>0)&0xFFF;
			rdp_scissor_xl = (cmd>>12)&0xFFF;
			rdp_scissor_yh = (cmd>>32)&0xFFF;
			rdp_scissor_xh = (cmd>>44)&0xFFF;
			rdp_scissor_o = (((cmd>>24)&0x1) != 0);
			rdp_scissor_f = (((cmd>>25)&0x1) != 0);
			rdp_cmd_len = 0;
			break;

		case 0x2E:
			printf("RDP %016llX Set Prim Depth\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x2F:
			printf("RDP %016llX Set Other Modes\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x34:
			printf("RDP %016llX Load Tile\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x35:
			printf("RDP %016llX Set Tile\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x36:
			printf("RDP %016llX Fill Rectangle\n", cmd);
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
				printf("Render %d,%d -> %d,%d\n", xl, yl, xh, yh);
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
								ram[offs+x] = rdp_fill_color;
							}
						}
						break;
					case 3: // 32bpp
						for(int y = yl; y < yh; y++) {
							uint32_t offs = (rdp_color_image_addr>>2);
							offs += (rdp_color_image_width+1)*y;
							for(int x = xl; x < xh; x++) {
								ram[offs+x] = rdp_fill_color;
							}
						}
						break;
				}
			}
			rdp_cmd_len = 0;
			break;

		case 0x37:
			printf("RDP %016llX Set Fill Color\n", cmd);
			rdp_fill_color = cmd&0xFFFFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x38:
			printf("RDP %016llX Set Fog Color\n", cmd);
			rdp_fog_color = cmd&0xFFFFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x39:
			printf("RDP %016llX Set Blend Color\n", cmd);
			rdp_blend_color = cmd&0xFFFFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x3C:
			printf("RDP %016llX Set Combine Mode\n", cmd);
			rdp_cmd_len = 0;
			break;

		case 0x3D:
			printf("RDP %016llX Set Texture Image\n", cmd);
			rdp_texture_image_format = (cmd>>53)&0x7;
			rdp_texture_image_size = (cmd>>51)&0x3;
			rdp_texture_image_width = (cmd>>32)&0x3FF;
			rdp_texture_image_addr = (cmd>>0)&0x3FFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x3E:
			printf("RDP %016llX Set Z Image\n", cmd);
			rdp_texture_image_addr = (cmd>>0)&0x3FFFFFF;
			rdp_cmd_len = 0;
			break;

		case 0x3F:
			printf("RDP %016llX Set Color Image\n", cmd);
			rdp_color_image_format = (cmd>>53)&0x7;
			rdp_color_image_size = (cmd>>51)&0x3;
			rdp_color_image_width = (cmd>>32)&0x3FF;
			rdp_color_image_addr = (cmd>>0)&0x3FFFFFF;
			rdp_cmd_len = 0;
			break;

		default:
			printf("RDP unhandled command %016llX\n", cmd);
			rdp_cmd_len = 0;
			break;
	}

	dpc_current += 8;
}

void rdp_run_commands(void)
{
	if(dpc_current != dpc_end) {
		rdp_run_one_command();
	}
}

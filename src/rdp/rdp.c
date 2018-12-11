#include "common.h"

#include "rdp/rdp.h"

uint32_t rdp_tmem[0x400];

uint64_t rdp_cmd_buffer[22];
uint64_t rdp_cmd_len = 0;

// 0x2D Set Scissor
int32_t rdp_scissor_xh = 0;
int32_t rdp_scissor_yh = 0;
int32_t rdp_scissor_xl = 0;
int32_t rdp_scissor_yl = 0;
bool rdp_scissor_f = false;
bool rdp_scissor_o = false;

// 0x2F Set Other Modes
// I refuse to split this into multiple fields.
#define RDOM_ATOMIC_PRIM (1ULL<<55)
#define RDOM_CYCLE_TYPE_MASK (0x3ULL<<52)
#define RDOM_CYCLE_TYPE_1CYCLE (0x0ULL<<52)
#define RDOM_CYCLE_TYPE_2CYCLE (0x1ULL<<52)
#define RDOM_CYCLE_TYPE_COPY (0x2ULL<<52)
#define RDOM_CYCLE_TYPE_FILL (0x3ULL<<52)
#define RDOM_PERSP_TEX_EN (1ULL<<51)
#define RDOM_DETAIL_TEX_EN (1ULL<<50)
#define RDOM_SHARPEN_TEX_EN (1ULL<<49)
#define RDOM_TEX_LOD_EN (1ULL<<48)
#define RDOM_EN_TLUT (1ULL<<47)
#define RDOM_TLUT_TYPE_MASK (0x1ULL<<46)
#define RDOM_TLUT_TYPE_RGBA (0x0ULL<<46)
#define RDOM_TLUT_TYPE_IA (0x1ULL<<46)
#define RDOM_SAMPLE_TYPE_MASK (0x1ULL<<45)
#define RDOM_SAMPLE_TYPE_1 (0x0ULL<<45)
#define RDOM_SAMPLE_TYPE_2 (0x1ULL<<45)
#define RDOM_MID_TEXEL (1ULL<<44)
#define RDOM_BI_LERP_0 (1ULL<<43)
#define RDOM_BI_LERP_1 (1ULL<<42)
#define RDOM_CONVERT_ONE (1ULL<<41)
#define RDOM_KEY_EN (1ULL<<40)
#define RDOM_RGB_DITHER_SEL_MASK (0x3ULL<<38)
#define RDOM_RGB_DITHER_SEL_MAGIC (0x0ULL<<38)
#define RDOM_RGB_DITHER_SEL_STANDARD (0x1ULL<<38)
#define RDOM_RGB_DITHER_SEL_NOISE (0x2ULL<<38)
#define RDOM_RGB_DITHER_SEL_NONE (0x3ULL<<38)
#define RDOM_ALPHA_DITHER_SEL_MASK (0x3ULL<<36)
#define RDOM_ALPHA_DITHER_SEL_PATTERN (0x0ULL<<36)
#define RDOM_ALPHA_DITHER_SEL_NOTPATTERN (0x1ULL<<36)
#define RDOM_ALPHA_DITHER_SEL_NOISE (0x2ULL<<36)
#define RDOM_ALPHA_DITHER_SEL_NONE (0x3ULL<<36)
#define RDOM_BLEND_M1A_CYCLE0_SHIFT 30
#define RDOM_BLEND_M1A_CYCLE0_MASK (0x3ULL<<RDOM_BLEND_M1A_CYCLE0_SHIFT)
#define RDOM_BLEND_M1A_CYCLE1_SHIFT 28
#define RDOM_BLEND_M1A_CYCLE1_MASK (0x3ULL<<RDOM_BLEND_M1A_CYCLE1_SHIFT)
#define RDOM_BLEND_M1B_CYCLE0_SHIFT 26
#define RDOM_BLEND_M1B_CYCLE0_MASK (0x3ULL<<RDOM_BLEND_M1B_CYCLE0_SHIFT)
#define RDOM_BLEND_M1B_CYCLE1_SHIFT 24
#define RDOM_BLEND_M1B_CYCLE1_MASK (0x3ULL<<RDOM_BLEND_M1B_CYCLE1_SHIFT)
#define RDOM_BLEND_M2A_CYCLE0_SHIFT 22
#define RDOM_BLEND_M2A_CYCLE0_MASK (0x3ULL<<RDOM_BLEND_M2A_CYCLE0_SHIFT)
#define RDOM_BLEND_M2A_CYCLE1_SHIFT 20
#define RDOM_BLEND_M2A_CYCLE1_MASK (0x3ULL<<RDOM_BLEND_M2A_CYCLE1_SHIFT)
#define RDOM_BLEND_M2B_CYCLE0_SHIFT 18
#define RDOM_BLEND_M2B_CYCLE0_MASK (0x3ULL<<RDOM_BLEND_M2B_CYCLE0_SHIFT)
#define RDOM_BLEND_M2B_CYCLE1_SHIFT 16
#define RDOM_BLEND_M2B_CYCLE1_MASK (0x3ULL<<RDOM_BLEND_M2B_CYCLE1_SHIFT)
#define RDOM_FORCE_BLEND (1ULL<<14)
#define RDOM_ALPHA_CVG_SELECT (1ULL<<13)
#define RDOM_CVG_TIMES_ALPHA (1ULL<<12)
#define RDOM_Z_MODE_MASK (0x3ULL<<10)
#define RDOM_Z_MODE_OPAQUE (0x0ULL<<10)
#define RDOM_Z_MODE_INTERPENETRATING (0x1ULL<<10)
#define RDOM_Z_MODE_TRANSPARENT (0x2ULL<<10)
#define RDOM_Z_MODE_DECAL (0x3ULL<<10)
#define RDOM_CVG_DEST_MASK (0x3ULL<<8)
#define RDOM_CVG_DEST_CLAMP (0x0ULL<<8)
#define RDOM_CVG_DEST_WRAP (0x1ULL<<8)
#define RDOM_CVG_DEST_ZAP (0x2ULL<<8)
#define RDOM_CVG_DEST_SAVE (0x3ULL<<8)
#define RDOM_COLOR_ON_CVG (1ULL<<7)
#define RDOM_IMAGE_READ_EN (1ULL<<6)
#define RDOM_Z_UPDATE_EN (1ULL<<5)
#define RDOM_Z_COMPARE_EN (1ULL<<4)
#define RDOM_ANTIALIAS_EN (1ULL<<3)
#define RDOM_Z_SOURCE_SEL (1ULL<<2)
#define RDOM_DITHER_ALPHA_EN (1ULL<<1)
#define RDOM_ALPHA_COMPARE_EN (1ULL<<0)
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

// 0x3C Set Combine Mode
uint32_t rdp_combine_AC0 = 0;
uint32_t rdp_combine_CC0 = 0;
uint32_t rdp_combine_AA0 = 0;
uint32_t rdp_combine_CA0 = 0;
uint32_t rdp_combine_AC1 = 0;
uint32_t rdp_combine_CC1 = 0;
uint32_t rdp_combine_BC0 = 0;
uint32_t rdp_combine_BC1 = 0;
uint32_t rdp_combine_AA1 = 0;
uint32_t rdp_combine_CA1 = 0;
uint32_t rdp_combine_DC0 = 0;
uint32_t rdp_combine_BA0 = 0;
uint32_t rdp_combine_DA0 = 0;
uint32_t rdp_combine_DC1 = 0;
uint32_t rdp_combine_BA1 = 0;
uint32_t rdp_combine_DA1 = 0;

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
		case 0x08: // Non-shaded triangle
		{
			if(rdp_cmd_len < 4) break;
			rdp_debug_printf("RDP %016llX Non-shaded triangle\n", cmd);
			bool left_major = (((cmd>>55)&0x1) != 0);
			int level = ((cmd>>51)&0x7);
			int tile = ((cmd>>48)&0x7);
			int16_t yl = ((int16_t)(((cmd>>32)&0x3FFF)<<2));
			int16_t ym = ((int16_t)(((cmd>>16)&0x3FFF)<<2));
			int16_t yh = ((int16_t)(((cmd)&0x3FFF)<<2));
			int32_t xl = ((int32_t)(rdp_cmd_buffer[1]>>32));
			int32_t dxldy = ((int32_t)(rdp_cmd_buffer[1]));
			int32_t xh = ((int32_t)(rdp_cmd_buffer[2]>>32));
			int32_t dxhdy = ((int32_t)(rdp_cmd_buffer[2]));
			int32_t xm = ((int32_t)(rdp_cmd_buffer[3]>>32));
			int32_t dxmdy = ((int32_t)(rdp_cmd_buffer[3]));

			int32_t x0 = xh;
			int32_t x1 = xm;

			int old_yh = yh;
			int old_ym = ym;
			if(yh < rdp_scissor_yh<<2) { yh = rdp_scissor_yh<<2; }
			if(ym < rdp_scissor_yh<<2) { ym = rdp_scissor_yh<<2; }
			if(yl < rdp_scissor_yh<<2) { yl = rdp_scissor_yh<<2; }
			if(yh > (rdp_scissor_yl-4)<<2) { yh = (rdp_scissor_yl-4)<<2; }
			if(ym > (rdp_scissor_yl-4)<<2) { ym = (rdp_scissor_yl-4)<<2; }
			if(yl > (rdp_scissor_yl-4)<<2) { yl = (rdp_scissor_yl-4)<<2; }

			int y_adjust = (yh-old_yh)>>4;
			x0 += y_adjust*dxmdy;
			x1 += y_adjust*dxhdy;

			switch((rdp_other_modes & RDOM_CYCLE_TYPE_MASK))
			{
				case RDOM_CYCLE_TYPE_FILL: {
#define FILL_COLOR rdp_fill_color
#include "rdp/fill-tri-switch.h"
#undef FILL_COLOR
				} break;

				default: {
#define FILL_COLOR rdp_blend_color
#include "rdp/fill-tri-switch.h"
#undef FILL_COLOR
				} break;
			}

			rdp_cmd_len = 0;
		} break;

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
			if(xl > rdp_scissor_xl-4) { xl = rdp_scissor_xl-4; }
			if(yl > rdp_scissor_yl-4) { yl = rdp_scissor_yl-4; }

			xh >>= 2;
			yh >>= 2;
			xl >>= 2;
			yl >>= 2;
			int tmem_stride = (rdp_tile_line+0)<<3;
			int dram_stride = (rdp_color_image_width+1);
			rdp_debug_printf("Render %d,%d -> %d,%d w=%d\n", xl, yl, xh, yh, tmem_stride);
			int16_t acc_t = t;
#include "rdp/tex-rect-switch.h"
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
#include "rdp/fill-rect-switch.h"
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
			rdp_combine_AC0 = ((cmd>>52)&0x0F);
			rdp_combine_CC0 = ((cmd>>47)&0x1F);
			rdp_combine_AA0 = ((cmd>>44)&0x07);
			rdp_combine_CA0 = ((cmd>>41)&0x07);
			rdp_combine_AC1 = ((cmd>>37)&0x0F);
			rdp_combine_CC1 = ((cmd>>32)&0x1F);
			rdp_combine_BC0 = ((cmd>>28)&0x0F);
			rdp_combine_BC1 = ((cmd>>24)&0x0F);
			rdp_combine_AA1 = ((cmd>>21)&0x07);
			rdp_combine_CA1 = ((cmd>>18)&0x07);
			rdp_combine_DC0 = ((cmd>>15)&0x07);
			rdp_combine_BA0 = ((cmd>>12)&0x07);
			rdp_combine_DA0 = ((cmd>>9)&0x07);
			rdp_combine_DC1 = ((cmd>>6)&0x07);
			rdp_combine_BA1 = ((cmd>>3)&0x07);
			rdp_combine_DA1 = ((cmd>>0)&0x07);

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
			rdp_debug_printf("RDP committed %08X -> %08X\n", dpc_current, dpc_end);
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

		// Are we done?
		if(dpc_current == dpc_end_saved) {
			rdp_debug_printf("RDP done\n");
		}
	}
}

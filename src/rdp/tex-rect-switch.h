#define CALL_TEXRECT_FUNCTION(func) func(xl, yl, xh, yh, dram_stride, s, t, dsdx, dtdy, tmem_stride);
switch(rdp_tile_size) {
	// 4bpp
	case 0: switch(rdp_tile_format) {

		// Color Index
		case 2: {
			CALL_TEXRECT_FUNCTION(tex_rect_rdp_4p);
		} break;

		// Intensity + Alpha
		case 3: {
			CALL_TEXRECT_FUNCTION(tex_rect_rdp_4ia);
		} break;

		// Intensity
		default:
		case 4: {
			CALL_TEXRECT_FUNCTION(tex_rect_rdp_4i);
		} break;
	} break;

	// 8bpp
	case 1: switch(rdp_tile_format) {
		// Color Index
		case 2: {
			CALL_TEXRECT_FUNCTION(tex_rect_rdp_8p);
		} break;

		// Intensity + Alpha
		case 3: {
			CALL_TEXRECT_FUNCTION(tex_rect_rdp_8ia);
		} break;

		// Intensity
		default:
		case 4: {
			CALL_TEXRECT_FUNCTION(tex_rect_rdp_8i);
		} break;

	} break;


	// 16bpp
	case 2: switch(rdp_tile_format) {
		// RGBA
		default:
		case 0: {
			CALL_TEXRECT_FUNCTION(tex_rect_rdp_16rgba);
		} break;

		// YUV
		case 1: {
			CALL_TEXRECT_FUNCTION(tex_rect_rdp_16yuv);
		} break;

		// Intensity + Alpha
		case 3: {
			CALL_TEXRECT_FUNCTION(tex_rect_rdp_16ia);
		} break;
	} break;

	default:
	// 32bpp
	case 3: {
		CALL_TEXRECT_FUNCTION(tex_rect_rdp_32rgba);
	} break;
}

#undef CALL_TEXRECT_FUNCTION

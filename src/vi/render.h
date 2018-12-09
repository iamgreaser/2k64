//printf("%d %d\n", vi_origin_reg, vi_width_reg);
switch(vi_status_reg & 0x00000003) {
	case 0: // blank
	case 1: // reserved - TODO: find info on this, otherwise for now treat as blank
		memset(surface->pixels, 0x55, surface->pitch*surface->h);
		break;
	case 2: // 15bpp
#define VI_BITS 16
#include "vi/case.h"
		break;
	case 3: // 32bpp
#define VI_BITS 32
#include "vi/case.h"
		break;
}


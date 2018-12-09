#define MIPS_IS_VR4300
#define MIPSNAME vr4300
#define MIPSXNAME(x) vr4300##x
#define MIPS_MEM_READ n64primary_mem_read
#define MIPS_MEM_WRITE n64primary_mem_write
#include "mips.h"
#undef MIPS_MEM_READ
#undef MIPS_MEM_WRITE
#undef MIPSNAME
#undef MIPSXNAME
#undef MIPS_IS_VR4300

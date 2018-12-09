#define MIPS_IS_RSP
#define MIPSNAME rsp
#define MIPSXNAME(x) rsp##x
#define MIPS_MEM_READ n64rsp_mem_read
#define MIPS_MEM_WRITE n64rsp_mem_write
#include "mips.h"
#undef MIPS_MEM_READ
#undef MIPS_MEM_WRITE
#undef MIPSNAME
#undef MIPSXNAME
#undef MIPS_IS_RSP

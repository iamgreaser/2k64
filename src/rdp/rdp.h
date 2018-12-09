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

void rdp_run_commands(void);

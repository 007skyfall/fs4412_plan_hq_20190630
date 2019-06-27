/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>

#include <asm/arch/clk.h>
#include "origen_setup.h"


DECLARE_GLOBAL_DATA_PTR;
struct exynos4_gpio_part1 *gpio1;
struct exynos4_gpio_part2 *gpio2;


#ifdef CONFIG_DRIVER_DM9000
#define EXYNOS4412_SROMC_BASE 0X12570000

#define DM9000_Tacs     (0x1)   // 0clk         address set-up
#define DM9000_Tcos     (0x1)   // 4clk         chip selection set-up
#define DM9000_Tacc     (0x5)   // 14clk        access cycle
#define DM9000_Tcoh     (0x1)   // 1clk         chip selection hold
#define DM9000_Tah      (0xC)   // 4clk         address holding time
#define DM9000_Tacp     (0x9)   // 6clk         page mode access cycle
#define DM9000_PMC      (0x1)   // normal(1data)page mode configuration


struct exynos_sromc {
	unsigned int bw;
	unsigned int bc[6];
};

/*
 *  s5p_config_sromc() - select the proper SROMC Bank and configure the
 *  band width control and bank control registers
 *  srom_bank    - SROM
 *  srom_bw_conf  - SMC Band witdh reg configuration value
 *  srom_bc_conf  - SMC Bank Control reg configuration value
 */

void exynos_config_sromc(u32 srom_bank, u32 srom_bw_conf, u32 srom_bc_conf)
{
	unsigned int tmp;
	struct exynos_sromc *srom = (struct exynos_sromc *)(EXYNOS4412_SROMC_BASE);

	/* Configure SMC_BW register to handle proper SROMC
	 * bank */
	tmp = srom->bw;
	tmp &= ~(0xF << (srom_bank * 4));
	tmp |= srom_bw_conf;
	srom->bw = tmp;

	/* Configure SMC_BC
	 * register */
	srom->bc[srom_bank] = srom_bc_conf;
}
static void dm9000aep_pre_init(void)
{
	unsigned int tmp;
	unsigned char smc_bank_num = 1;
	unsigned int     smc_bw_conf=0;
	unsigned int     smc_bc_conf=0;

	/* gpio configuration */
	writel(0x00220020, 0x11000000 + 0x120);
	writel(0x00002222, 0x11000000 + 0x140);
	/* 16 Bit bus width */
	writel(0x22222222, 0x11000000 + 0x180);
	writel(0x0000FFFF, 0x11000000 + 0x188);
	writel(0x22222222, 0x11000000 + 0x1C0);
	writel(0x0000FFFF, 0x11000000 + 0x1C8);
	writel(0x22222222, 0x11000000 + 0x1E0);
	writel(0x0000FFFF, 0x11000000 + 0x1E8);              
	smc_bw_conf &= ~(0xf<<4);
	smc_bw_conf |= (1<<7) | (1<<6) | (1<<5) | (1<<4);
	smc_bc_conf = ((DM9000_Tacs << 28)
			| (DM9000_Tcos << 24)
			| (DM9000_Tacc << 16)
			| (DM9000_Tcoh << 12)
			| (DM9000_Tah << 8)
			| (DM9000_Tacp << 4)
			| (DM9000_PMC));
	exynos_config_sromc(smc_bank_num,smc_bw_conf,smc_bc_conf);
}
#endif


int board_init(void)
{
	gpio1 = (struct exynos4_gpio_part1 *) EXYNOS4_GPIO_PART1_BASE;
	gpio2 = (struct exynos4_gpio_part2 *) EXYNOS4_GPIO_PART2_BASE;

	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100UL);

#ifdef CONFIG_DRIVER_DM9000
	dm9000aep_pre_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size	= get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE)
		+ get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE)
		+ get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE)
		+ get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE);

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((long *)PHYS_SDRAM_1, \
			PHYS_SDRAM_1_SIZE);
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((long *)PHYS_SDRAM_2, \
			PHYS_SDRAM_2_SIZE);
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = get_ram_size((long *)PHYS_SDRAM_3, \
			PHYS_SDRAM_3_SIZE);
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = get_ram_size((long *)PHYS_SDRAM_4, \
			PHYS_SDRAM_4_SIZE);
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("\nBoard: FS4412\n");
	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC

u32 sclk_mmc4;  /*clock source for emmc controller*/
#define __REGMY(x) (*((volatile u32 *)(x)))
#define CLK_SRC_FSYS  __REGMY(EXYNOS4_CLOCK_BASE + CLK_SRC_FSYS_OFFSET)
#define CLK_DIV_FSYS3 __REGMY(EXYNOS4_CLOCK_BASE + CLK_DIV_FSYS3_OFFSET)

int emmc_init()
{
	u32 tmp;
	u32 clock;
	u32 i;
	/* setup_hsmmc_clock */
	/* MMC4 clock src = SCLKMPLL */
	tmp = CLK_SRC_FSYS & ~(0x000f0000);
	CLK_SRC_FSYS = tmp | 0x00060000;
	/* MMC4 clock div */
	tmp = CLK_DIV_FSYS3 & ~(0x0000ff0f);
	clock = get_pll_clk(MPLL)/1000000;

	for(i=0 ; i<=0xf; i++)  {
		sclk_mmc4=(clock/(i+1));

		if(sclk_mmc4 <= 160) //200
		{
			CLK_DIV_FSYS3 = tmp | (i<<0);
			break;
		}
	}
	emmcdbg("[mjdbg] sclk_mmc4:%d MHZ; mmc_ratio: %d\n",sclk_mmc4,i);
	sclk_mmc4 *= 1000000;

	/*
	 * MMC4 EMMC GPIO CONFIG
	 *
	 * GPK0[0]	SD_4_CLK
	 * GPK0[1]	SD_4_CMD
	 * GPK0[2]	SD_4_CDn
	 * GPK0[3:6]	SD_4_DATA[0:3]
	 */
	writel(readl(0x11000048)&~(0xf),0x11000048); //SD_4_CLK/SD_4_CMD pull-down enable
	writel(readl(0x11000040)&~(0xff),0x11000040);//cdn set to be output

	writel(readl(0x11000048)&~(3<<4),0x11000048); //cdn pull-down disable
	writel(readl(0x11000044)&~(1<<2),0x11000044); //cdn output 0 to shutdown the emmc power
	writel(readl(0x11000040)&~(0xf<<8)|(1<<8),0x11000040);//cdn set to be output
	udelay(100*1000);
	writel(readl(0x11000044)|(1<<2),0x11000044); //cdn output 1


	writel(0x03333133, 0x11000040);

	writel(0x00003FF0, 0x11000048);
	writel(0x00002AAA, 0x1100004C);

#ifdef CONFIG_EMMC_8Bit
	writel(0x04444000, 0x11000060);
	writel(0x00003FC0, 0x11000068);
	writel(0x00002AAA, 0x1100006C);
#endif

#ifdef USE_MMC4
	smdk_s5p_mshc_init();
#endif 
}



int board_mmc_init(bd_t *bis)
{
	int i, err;

#ifdef CONFIG_EMMC
	err = emmc_init();
	return err;
#endif

	/*
	 * MMC2 SD card GPIO:
	 *
	 * GPK2[0]	SD_2_CLK(2)
	 * GPK2[1]	SD_2_CMD(2)
	 * GPK2[2]	SD_2_CDn
	 * GPK2[3:6]	SD_2_DATA[0:3](2)
	 */
	for (i = 0; i < 7; i++) {
		/* GPK2[0:6] special function 2 */
		s5p_gpio_cfg_pin(&gpio2->k2, i, GPIO_FUNC(0x2));

		/* GPK2[0:6] drv 4x */
		s5p_gpio_set_drv(&gpio2->k2, i, GPIO_DRV_4X);

		/* GPK2[0:1] pull disable */
		if (i == 0 || i == 1) {
			s5p_gpio_set_pull(&gpio2->k2, i, GPIO_PULL_NONE);
			continue;
		}

		/* GPK2[2:6] pull up */
		s5p_gpio_set_pull(&gpio2->k2, i, GPIO_PULL_UP);
	}

	err = s5p_mmc_init(2, 4);
	return err;
}
#endif

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)                                                  
{      

	int rc = 0;
#ifdef CONFIG_DRIVER_DM9000
	rc = dm9000_initialize(bis);                                            
#endif                                                                         
	return rc;                                                              
}  
#endif

#ifdef CONFIG_BOARD_LATE_INIT
#include <movi.h>
int  chk_bootdev(void)//mj for boot device check
{
	char run_cmd[100];
	struct mmc *mmc;
	int boot_dev = 0;
	int cmp_off = 0x10;
	ulong  start_blk, blkcnt;

	mmc = find_mmc_device(0);

	if (mmc == NULL)
	{
		printf("There is no eMMC card, Booting device is SD card\n");
		boot_dev = 1;
		return boot_dev;
	}
	start_blk = (24*1024/MOVI_BLKSIZE);
	blkcnt = 0x10;

	sprintf(run_cmd,"emmc open 0");
	run_command(run_cmd, 0);

	sprintf(run_cmd,"mmc read 0 %lx %lx %lx",CFG_PHY_KERNEL_BASE,start_blk,blkcnt);
	run_command(run_cmd, 0);

	sprintf(run_cmd,"emmc close 0");
	run_command(run_cmd, 0);

	return 0;
}

int board_late_init (void)
{
	int boot_dev =0 ;
	char boot_cmd[100];
	boot_dev = chk_bootdev();
	if(!boot_dev)
	{
		printf("\n\nChecking Boot Mode ... EMMC4.41\n");
	}

//	sprintf(boot_cmd, "movi read kernel 40008000;movi read rootfs 40d00000 100000;bootm 40008000 40d00000");
//	setenv("bootcmd", boot_cmd);

	return 0;
}
#endif

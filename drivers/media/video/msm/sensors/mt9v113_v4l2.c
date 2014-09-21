/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "msm_sensor.h"
#include "msm.h"
#include "msm_ispif.h"
#define SENSOR_NAME "mt9v113"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9v113"
#define mt9v113_obj mt9v113_##obj

DEFINE_MUTEX(mt9v113_mut);
static struct msm_sensor_ctrl_t mt9v113_s_ctrl;

/* LGE_CHANGE_S, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */
#ifdef CONFIG_MACH_LGE
static int prev_balance_mode;
static int prev_effect_mode;
static int prev_brightness_mode;
#endif
/* LGE_CHANGE_E, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */

static struct msm_camera_i2c_reg_conf mt9v113_start_settings[] = {
	{0x0018, 0x4028},	// john.park, 2011-05-09, To replace Polling check
//	{0xFFFF, 0x0064},	// Delay = 100ms
//	{0xFFFD, 0x4028},	// For polling
};

static struct msm_camera_i2c_reg_conf mt9v113_stop_settings[] = {
	{0x0018, 0x4029},	// john.park, 2011-05-09, To replace Polling check
//	{0xFFFF, 0x0064},	// Delay = 100ms
//	{0xFFFD, 0x4029},	// For polling
};

static struct msm_camera_i2c_reg_conf mt9v113_groupon_settings[] = {
//	{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf mt9v113_groupoff_settings[] = {
//	{0x104, 0x00},
};

static struct msm_camera_i2c_reg_conf mt9v113_prev_settings[] = {
};

static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings[] = {
	// init_tbl_sub
	{0x0018, 0x4028}, // STANDBY_CONTROL

	// init_tbl1_sub
	{0x001A, 0x0013}, // RESET_AND_MISC_CONTROL 																				 //DELAY=10
	{0xFFFF, 0x000A},
	{0x001A, 0x0010}, // RESET_AND_MISC_CONTROL
	{0xFFFF, 0x000A},
	{0x0018, 0x4028}, // STANDBY_CONTROL

	//POLL_REG=0x0018, 0x4000, !=0, DELAY=10, TIMEOUT=100  // Verify streaming bit is high
	//POLL_REG=0x301A, 0x0004, !=1, DELAY=50, TIMEOUT=20  // Verify streaming bit is high

	// init_tbl2_sub
	{0x098C, 0x02F0},	// MCU_ADDRESS
	{0x0990, 0x0000},	// MCU_DATA_0
	{0x098C, 0x02F2},	// MCU_ADDRESS
	{0x0990, 0x0210},	// MCU_DATA_0
	{0x098C, 0x02F4},	// MCU_ADDRESS
	{0x0990, 0x001A},	// MCU_DATA_0
	{0x098C, 0x2145},	// MCU_ADDRESS
	{0x0990, 0x02F4},	// MCU_DATA_0
	{0x098C, 0xA134},	// MCU_ADDRESS
	{0x0990, 0x0001},	// MCU_DATA_0
	{0x31E0, 0x0001},	// CORE_31E0
	{0xFFFF, 0x000A},
	{0x3400, 0x783C},	// MIPI_CONTROL
	{0x001A, 0x0210},	// RESET_AND_MISC_CONTROL
	{0x001E, 0x0400},	// PAD_SLEW
	{0x0016, 0x42DF},	// CLOCKS_CONTROL

	{0x0014, 0x2145},	// PLL_CONTROL
	{0x0014, 0x2145},	// PLL_CONTROL
	{0x0010, 0x021A},	// 21c PLL_DIVIDERS
	{0x0012, 0x0000},	// PLL_P_DIVIDERS
	{0x0014, 0x244B},	// PLL_CONTROL
	{0xFFFF, 0x000A},

	{0x0014, 0x304B},	// PLL_CONTROL
	{0xFFFF, 0x0032},

	// POLL_REG=0x0014, 0x8000, ==0, DELAY=50, TIMEOUT=20	// Verify PLL lock	//DELAY=10

	{0x0014, 0xB04A},	// PLL_CONTROL
	{0xFFFF, 0x0012},
	{0x3400, 0x7A3C},	// MIPI_CONTROL
	{0x001A, 0x0010},
	{0x001A, 0x0018},
	{0x321C, 0x0003},	// OFIFO_CONTROL_STATUS

	{0x098C, 0x2703},	// Output Width (A)
	{0x0990, 0x0280},	//		= 640
	{0x098C, 0x2705},	// Output Height (A)
	{0x0990, 0x01E0},	//		= 480
	{0x098C, 0x2707},	// Output Width (B)
	{0x0990, 0x0280},	//		= 640
	{0x098C, 0x2709},	// Output Height (B)
	{0x0990, 0x01E0},	//		= 480
	{0x098C, 0x270D},	// Row Start (A)
	{0x0990, 0x0000},	//		= 0
	{0x098C, 0x270F},	// Column Start (A)
	{0x0990, 0x0000},	//		= 0
	{0x098C, 0x2711},	// Row End (A)
	{0x0990, 0x01E7},	//		= 487
	{0x098C, 0x2713},	// Column End (A)
	{0x0990, 0x0287},	//		= 647
	{0x098C, 0x2715},	// Row Speed (A)
	{0x0990, 0x0001},	//		= 1
#ifdef CONFIG_MACH_MSM8960_L0
	{0x098C, 0x2717},	// Read Mode (A)
	{0x0990, 0x0026},	//		= 38
#else
	{0x098C, 0x2717},	// Read Mode (A)
	{0x0990, 0x0025},	//		= 38
#endif
	{0x098C, 0x2719},	// sensor_fine_correction (A)
	{0x0990, 0x001A},	//		= 26
	{0x098C, 0x271B},	// sensor_fine_IT_min (A)
	{0x0990, 0x006B},	//		= 107
	{0x098C, 0x271D},	// sensor_fine_IT_max_margin (A)
	{0x0990, 0x006B},	//		= 107
	{0x098C, 0x271F},	// Frame Lines (A)
	{0x0990, 0x01FF},	//      = 554
	{0x098C, 0x2721},	//Line Length (A)
	{0x0990, 0x0350},	//      = 842
	{0x098C, 0x2723},	// Row Start (B)
	{0x0990, 0x0000},	//		= 0
	{0x098C, 0x2725},	// Column Start (B)
	{0x0990, 0x0000},	//		= 0
	{0x098C, 0x2727},	// Row End (B)
	{0x0990, 0x01E7},	//		= 487
	{0x098C, 0x2729},	// Column End (B)
	{0x0990, 0x0287},	//		= 647
	{0x098C, 0x272B},	// Row Speed (B)
	{0x0990, 0x0001},	//		= 1
#ifdef CONFIG_MACH_MSM8960_L0
	{0x098C, 0x272D},	// Read Mode (B)
	{0x0990, 0x0026},	//		= 38
#else
	{0x098C, 0x272D},	// Read Mode (B)
	{0x0990, 0x0025},	//		= 38
#endif
	{0x098C, 0x272F},	// sensor_fine_correction (B)
	{0x0990, 0x001A},	//		= 26
	{0x098C, 0x2731},	// sensor_fine_IT_min (B)
	{0x0990, 0x006B},	//		= 107
	{0x098C, 0x2733},	// sensor_fine_IT_max_margin (B)
	{0x0990, 0x006B},	//		= 107
	{0x098C, 0x2735},	// Frame Lines (B)
	{0x0990, 0x01FF},	//      = 1108
	{0x098C, 0x2737},	//Line Length (B)
	{0x0990, 0x0350},	//      = 842
	{0x098C, 0x2739},	// Crop_X0 (A)
	{0x0990, 0x0000},	//		= 0
	{0x098C, 0x273B},	// Crop_X1 (A)
	{0x0990, 0x027F},	//		= 639
	{0x098C, 0x273D},	// Crop_Y0 (A)
	{0x0990, 0x0000},	//		= 0
	{0x098C, 0x273F},	// Crop_Y1 (A)
	{0x0990, 0x01DF},	//		= 479
	{0x098C, 0x2747},	// Crop_X0 (B)
	{0x0990, 0x0000},	//		= 0
	{0x098C, 0x2749},	// Crop_X1 (B)
	{0x0990, 0x027F},	//		= 639
	{0x098C, 0x274B},	// Crop_Y0 (B)
	{0x0990, 0x0000},	//		= 0
	{0x098C, 0x274D},	// Crop_Y1 (B)
	{0x0990, 0x01DF},	//		= 479
	{0x098C, 0x222D},	// R9 Step
	{0x0990, 0x0080},	//      = 139
	{0x098C, 0xA408},	//search_f1_50
	{0x0990, 0x001F},	//      = 33
	{0x098C, 0xA409},	//search_f2_50
	{0x0990, 0x0021},	//      = 35
	{0x098C, 0xA40A},	//search_f1_60
	{0x0990, 0x0025},	//      = 40
	{0x098C, 0xA40B},	//search_f2_60
	{0x0990, 0x0027},	//      = 42
	{0x098C, 0x2411},	//R9_Step_60 (A)
	{0x0990, 0x0080},	//      = 139
	{0x098C, 0x2413},	//R9_Step_50 (A)
	{0x0990, 0x0099},	//      = 166
	{0x098C, 0x2415},	//R9_Step_60 (B)
	{0x0990, 0x0080},	//      = 139
	{0x098C, 0x2417},	//R9_Step_50 (B)
	{0x0990, 0x0099},	//      = 166
	{0x098C, 0xA404},	//FD Mode
	{0x0990, 0x0010},	//      = 16
	{0x098C, 0xA40D},	//Stat_min
	{0x0990, 0x0001},	//      = 2
	{0x098C, 0xA40E},	// Stat_max
	{0x0990, 0x0003},	//		= 3
	{0x098C, 0xA410},	// Min_amplitude
	{0x0990, 0x000A},	//		= 10

	// [Gamma]
	{0x098C, 0xA20C},	// MCU_ADDRESS
	{0x0990, 0x000A},	// AE_MAX_INDEX (12FPS ~:0x000A)(10FPS~:0x000C),(7.5FPS~:0x0010)


	// [Gamma]
	{0x098C, 0xAB37},	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
	{0x0990, 0x0001},	// MCU_DATA_0
	{0x098C, 0x2B38},	// MCU_ADDRESS [HG_GAMMASTARTMORPH]
	{0x0990, 0x1000},	// MCU_DATA_0
	{0x098C, 0x2B3A},	// MCU_ADDRESS [HG_GAMMASTOPMORPH]
	{0x0990, 0x2000},	// MCU_DATA_0
	{0x098C, 0xAB3C},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000},	// MCU_DATA_0
	{0x098C, 0xAB3D},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x0017},	// MCU_DATA_0
	{0x098C, 0xAB3E},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0028},	// MCU_DATA_0
	{0x098C, 0xAB3F},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x003D},	// MCU_DATA_0
	{0x098C, 0xAB40},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x005B},	// MCU_DATA_0
	{0x098C, 0xAB41},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x0074},	// MCU_DATA_0
	{0x098C, 0xAB42},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x0089},	// MCU_DATA_0
	{0x098C, 0xAB43},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x009B},	// MCU_DATA_0
	{0x098C, 0xAB44},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00AA},	// MCU_DATA_0
	{0x098C, 0xAB45},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00B7},	// MCU_DATA_0
	{0x098C, 0xAB46},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00C3},	// MCU_DATA_0
	{0x098C, 0xAB47},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00CD},	// MCU_DATA_0
	{0x098C, 0xAB48},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00D6},	// MCU_DATA_0
	{0x098C, 0xAB49},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00DE},	// MCU_DATA_0
	{0x098C, 0xAB4A},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00E6},	// MCU_DATA_0
	{0x098C, 0xAB4B},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00ED},	// MCU_DATA_0
	{0x098C, 0xAB4C},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F3},	// MCU_DATA_0
	{0x098C, 0xAB4D},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F9},	// MCU_DATA_0
	{0x098C, 0xAB4E},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF},	// MCU_DATA_0
	{0x098C, 0xAB3C},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000},	// MCU_DATA_0
	{0x098C, 0xAB3D},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x0017},	// MCU_DATA_0
	{0x098C, 0xAB3E},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0028},	// MCU_DATA_0
	{0x098C, 0xAB3F},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x003D},	// MCU_DATA_0
	{0x098C, 0xAB40},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x005B},	// MCU_DATA_0
	{0x098C, 0xAB41},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x0074},	// MCU_DATA_0
	{0x098C, 0xAB42},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x0089},	// MCU_DATA_0
	{0x098C, 0xAB43},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x009B},	// MCU_DATA_0
	{0x098C, 0xAB44},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00AA},	// MCU_DATA_0
	{0x098C, 0xAB45},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00B7},	// MCU_DATA_0
	{0x098C, 0xAB46},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00C3},	// MCU_DATA_0
	{0x098C, 0xAB47},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00CD},	// MCU_DATA_0
	{0x098C, 0xAB48},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00D6},	// MCU_DATA_0
	{0x098C, 0xAB49},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00DE},	// MCU_DATA_0
	{0x098C, 0xAB4A},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00E6},	// MCU_DATA_0
	{0x098C, 0xAB4B},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00ED},	// MCU_DATA_0
	{0x098C, 0xAB4C},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F3},	// MCU_DATA_0
	{0x098C, 0xAB4D},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F9},	// MCU_DATA_0
	{0x098C, 0xAB4E},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF},	// MCU_DATA_0

	// [Lens Correction 85]
	{0x3658, 0x0130},	// P_RD_P0Q0
	{0x365A, 0x030D},	// P_RD_P0Q1
	{0x365C, 0x6B92},	// P_RD_P0Q2
	{0x365E, 0xE62E},	// P_RD_P0Q3
	{0x3660, 0x53B4},	// P_RD_P0Q4
	{0x3680, 0x1AED},	// P_RD_P1Q0
	{0x3682, 0x2A6A},	// P_RD_P1Q1
	{0x3684, 0xBA2B},	// P_RD_P1Q2
	{0x3686, 0x392F},	// P_RD_P1Q3
	{0x3688, 0xAD53},	// P_RD_P1Q4
	{0x36A8, 0x1453},	// P_RD_P2Q0
	{0x36AA, 0xC78D},	// P_RD_P2Q1
	{0x36AC, 0x4C35},	// P_RD_P2Q2
	{0x36AE, 0x7791},	// P_RD_P2Q3
	{0x36B0, 0x6797},	// P_RD_P2Q4
	{0x36D0, 0xE3F0},	// P_RD_P3Q0
	{0x36D2, 0xF8F0},	// P_RD_P3Q1
	{0x36D4, 0x5934},	// P_RD_P3Q2
	{0x36D6, 0x6AF2},	// P_RD_P3Q3
	{0x36D8, 0xFC98},	// P_RD_P3Q4
	{0x36F8, 0x2EB4},	// P_RD_P4Q0
	{0x36FA, 0x9972},	// P_RD_P4Q1
	{0x36FC, 0x5E33},	// P_RD_P4Q2
	{0x36FE, 0x07D8},	// P_RD_P4Q3
	{0x3700, 0x51DC},	// P_RD_P4Q4
	{0x364E, 0x0170},	// P_GR_P0Q0
	{0x3650, 0x570C},	// P_GR_P0Q1
	{0x3652, 0x6652},	// P_GR_P0Q2
	{0x3654, 0xB5B0},	// P_GR_P0Q3
	{0x3656, 0x3274},	// P_GR_P0Q4
	{0x3676, 0x214D},	// P_GR_P1Q0
	{0x3678, 0x3D46},	// P_GR_P1Q1
	{0x367A, 0x218F},	// P_GR_P1Q2
	{0x367C, 0x13D0},	// P_GR_P1Q3
	{0x367E, 0x8C34},	// P_GR_P1Q4
	{0x369E, 0x1253},	// P_GR_P2Q0
	{0x36A0, 0x98B0},	// P_GR_P2Q1
	{0x36A2, 0x3275},	// P_GR_P2Q2
	{0x36A4, 0x4FD0},	// P_GR_P2Q3
	{0x36A6, 0x0896},	// P_GR_P2Q4
	{0x36C6, 0x82D0},	// P_GR_P3Q0
	{0x36C8, 0x8292},	// P_GR_P3Q1
	{0x36CA, 0x0CD5},	// P_GR_P3Q2
	{0x36CC, 0x2632},	// P_GR_P3Q3
	{0x36CE, 0xC659},	// P_GR_P3Q4
	{0x36EE, 0x2374},	// P_GR_P4Q0
	{0x36F0, 0xDC32},	// P_GR_P4Q1
	{0x36F2, 0xCBF7},	// P_GR_P4Q2
	{0x36F4, 0x1918},	// P_GR_P4Q3
	{0x36F6, 0x675C},	// P_GR_P4Q4
	{0x3662, 0x00D0},	// P_BL_P0Q0
	{0x3664, 0x67AC},	// P_BL_P0Q1
	{0x3666, 0x6CD2},	// P_BL_P0Q2
	{0x3668, 0xB350},	// P_BL_P0Q3
	{0x366A, 0x0CD4},	// P_BL_P0Q4
	{0x368A, 0x400C},	// P_BL_P1Q0
	{0x368C, 0x07ED},	// P_BL_P1Q1
	{0x368E, 0x57CF},	// P_BL_P1Q2
	{0x3690, 0x3C50},	// P_BL_P1Q3
	{0x3692, 0xD654},	// P_BL_P1Q4
	{0x36B2, 0x0213},	// P_BL_P2Q0
	{0x36B4, 0x2CCF},	// P_BL_P2Q1
	{0x36B6, 0x1636},	// P_BL_P2Q2
	{0x36B8, 0x8EB5},	// P_BL_P2Q3
	{0x36BA, 0xF338},	// P_BL_P2Q4
	{0x36DA, 0xD310},	// P_BL_P3Q0
	{0x36DC, 0x22B0},	// P_BL_P3Q1
	{0x36DE, 0x3273},	// P_BL_P3Q2
	{0x36E0, 0x9BF6},	// P_BL_P3Q3
	{0x36E2, 0x82D8},	// P_BL_P3Q4
	{0x3702, 0x4854},	// P_BL_P4Q0
	{0x3704, 0x9AD5},	// P_BL_P4Q1
	{0x3706, 0xD519},	// P_BL_P4Q2
	{0x3708, 0x5D99},	// P_BL_P4Q3
	{0x370A, 0x685D},	// P_BL_P4Q4
	{0x366C, 0x00D0},	// P_GB_P0Q0
	{0x366E, 0x498C},	// P_GB_P0Q1
	{0x3670, 0x6B32},	// P_GB_P0Q2
	{0x3672, 0xA910},	// P_GB_P0Q3
	{0x3674, 0x3614},	// P_GB_P0Q4
	{0x3694, 0x3FAC},	// P_GB_P1Q0
	{0x3696, 0xB68A},	// P_GB_P1Q1
	{0x3698, 0x0FB0},	// P_GB_P1Q2
	{0x369A, 0x5E70},	// P_GB_P1Q3
	{0x369C, 0xD6B4},	// P_GB_P1Q4
	{0x36BC, 0x09F3},	// P_GB_P2Q0
	{0x36BE, 0xB9F1},	// P_GB_P2Q1
	{0x36C0, 0x3C55},	// P_GB_P2Q2
	{0x36C2, 0x2035},	// P_GB_P2Q3
	{0x36C4, 0x5B55},	// P_GB_P2Q4
	{0x36E4, 0x9450},	// P_GB_P3Q0
	{0x36E6, 0xF171},	// P_GB_P3Q1
	{0x36E8, 0x16D4},	// P_GB_P3Q2
	{0x36EA, 0x8234},	// P_GB_P3Q3
	{0x36EC, 0x98F9},	// P_GB_P3Q4
	{0x370C, 0x3854},	// P_GB_P4Q0
	{0x370E, 0x3754},	// P_GB_P4Q1
	{0x3710, 0xBED7},	// P_GB_P4Q2
	{0x3712, 0xF557},	// P_GB_P4Q3
	{0x3714, 0x4B7C},	// P_GB_P4Q4
	{0x3644, 0x0148},	// POLY_ORIGIN_C
	{0x3642, 0x00F0},	// POLY_ORIGIN_R
	{0x3210, 0x09B8},	// COLOR_PIPELINE_CONTROL

	// [AWB and CCMs ..2]
	{0x098C, 0x2306},	// MCU_ADDRESS [AWB_CCM_L_0]
	{0x0990, 0x0133},	// MCU_DATA_0
	{0x098C, 0x2308},	// MCU_ADDRESS [AWB_CCM_L_1]
	{0x0990, 0xFFC4},	// MCU_DATA_0
	{0x098C, 0x230A},	// MCU_ADDRESS [AWB_CCM_L_2]
	{0x0990, 0x0014},	// MCU_DATA_0
	{0x098C, 0x230C},	// MCU_ADDRESS [AWB_CCM_L_3]
	{0x0990, 0xFF64},	// MCU_DATA_0
	{0x098C, 0x230E},	// MCU_ADDRESS [AWB_CCM_L_4]
	{0x0990, 0x01E3},	// MCU_DATA_0
	{0x098C, 0x2310},	// MCU_ADDRESS [AWB_CCM_L_5]
	{0x0990, 0xFFB2},	// MCU_DATA_0
	{0x098C, 0x2312},	// MCU_ADDRESS [AWB_CCM_L_6]
	{0x0990, 0xFF9A},	// MCU_DATA_0
	{0x098C, 0x2314},	// MCU_ADDRESS [AWB_CCM_L_7]
	{0x0990, 0xFEDB},	// MCU_DATA_0
	{0x098C, 0x2316},	// MCU_ADDRESS [AWB_CCM_L_8]
	{0x0990, 0x0213},	// MCU_DATA_0
	{0x098C, 0x2318},	// MCU_ADDRESS [AWB_CCM_L_9]
	{0x0990, 0x001C},	// MCU_DATA_0
	{0x098C, 0x231A},	// MCU_ADDRESS [AWB_CCM_L_10]
	{0x0990, 0x003A},	// MCU_DATA_0
	{0x098C, 0x231C},	// MCU_ADDRESS [AWB_CCM_RL_0]
	{0x0990, 0x0064},	// MCU_DATA_0
	{0x098C, 0x231E},	// MCU_ADDRESS [AWB_CCM_RL_1]
	{0x0990, 0xFF7D},	// MCU_DATA_0
	{0x098C, 0x2320},	// MCU_ADDRESS [AWB_CCM_RL_2]
	{0x0990, 0xFFFF},	// MCU_DATA_0
	{0x098C, 0x2322},	// MCU_ADDRESS [AWB_CCM_RL_3]
	{0x0990, 0x001A},	// MCU_DATA_0
	{0x098C, 0x2324},	// MCU_ADDRESS [AWB_CCM_RL_4]
	{0x0990, 0xFF94},	// MCU_DATA_0
	{0x098C, 0x2326},	// MCU_ADDRESS [AWB_CCM_RL_5]
	{0x0990, 0x0048},	// MCU_DATA_0
	{0x098C, 0x2328},	// MCU_ADDRESS [AWB_CCM_RL_6]
	{0x0990, 0x001B},	// MCU_DATA_0
	{0x098C, 0x232A},	// MCU_ADDRESS [AWB_CCM_RL_7]
	{0x0990, 0x0166},	// MCU_DATA_0
	{0x098C, 0x232C},	// MCU_ADDRESS [AWB_CCM_RL_8]
	{0x0990, 0xFEE3},	// MCU_DATA_0
	{0x098C, 0x232E},	// MCU_ADDRESS [AWB_CCM_RL_9]
	{0x0990, 0x0004},	// MCU_DATA_0
	{0x098C, 0x2330},	// MCU_ADDRESS [AWB_CCM_RL_10]
	{0x0990, 0xFFDC},	// MCU_DATA_0
	{0x098C, 0xA348},	// MCU_ADDRESS
	{0x0990, 0x0008},	// AWB_GAIN_BUFFER_SPEED
	{0x098C, 0xA349},	// MCU_ADDRESS
	{0x0990, 0x0002},	// AWB_JUMP_DIVISOR
	{0x098C, 0xA34A},	// MCU_ADDRESS
	{0x0990, 0x0059},	// AWB_GAINMIN_R
	{0x098C, 0xA34B},	// MCU_ADDRESS
	{0x0990, 0x00E6},	// AWB_GAINMAX_R
	{0x098C, 0xA34C},	// MCU_ADDRESS
	{0x0990, 0x0059},	// AWB_GAINMIN_B
	{0x098C, 0xA34D},	// MCU_ADDRESS
	{0x0990, 0x00A6},	// AWB_GAINMAX_B
	{0x098C, 0xA34E},	// MCU_ADDRESS
	{0x0990, 0x0080},	// AWB_GAIN_R
	{0x098C, 0xA34F},	// MCU_ADDRESS
	{0x0990, 0x0080},	// AWB_GAIN_G
	{0x098C, 0xA350},	// MCU_ADDRESS
	{0x0990, 0x0080},	// AWB_GAIN_B
	//[AWB MIN POS CHANGE BY DANIEL]
	{0x098C, 0xA351},	// MCU_ADDRESS [AWB_CCM_POSITION_MIN]
	{0x0990, 0x0020},	// MCU_DATA_0
	{0x098C, 0xA352},	// MCU_ADDRESS
	{0x0990, 0x007F},	// AWB_CCM_POSITION_MAX
	{0x098C, 0xA354},	// MCU_ADDRESS
	{0x0990, 0x0060},	// AWB_CCM_POSITION
	{0x098C, 0xA354},	// MCU_ADDRESS
	{0x0990, 0x0060},	// AWB_SATURATION
	{0x098C, 0xA355},	// MCU_ADDRESS
	{0x0990, 0x0001},	// AWB_MODE
	{0x098C, 0xA35D},	// MCU_ADDRESS
	{0x0990, 0x0078},	// AWB_STEADY_BGAIN_OUT_MIN
	{0x098C, 0xA35E},	// MCU_ADDRESS
	{0x0990, 0x0086},	// AWB_STEADY_BGAIN_OUT_MAX
	{0x098C, 0xA35F},	// MCU_ADDRESS
	{0x0990, 0x007E},	// AWB_STEADY_BGAIN_IN_MIN
	{0x098C, 0xA360},	// MCU_ADDRESS
	{0x0990, 0x0082},	// AWB_STEADY_BGAIN_IN_MAX
	{0x098C, 0xA302},	// MCU_ADDRESS
	{0x0990, 0x0000},	// AWB_WINDOW_POS
	{0x098C, 0xA303},	// MCU_ADDRESS
	{0x0990, 0x00EF},	// AWB_WINDOW_SIZE
	// .. . ..(RED higher)
	{0x098C, 0xA364},	// MCU_ADDRESS	// 20091217 add (110721 cayman newly updated)
	{0x0990, 0x00E4},	// MCU_DATA_0
	{0x098C, 0xA365},	// MCU_ADDRESS
	{0x0990, 0x0000},	// AWB_X0 <-0x0010
	{0x098C, 0xA366},	// MCU_ADDRESS
	{0x0990, 0x0080},	// AWB_KR_L
	{0x098C, 0xA367},	// MCU_ADDRESS
	{0x0990, 0x0080},	// AWB_KG_L
	{0x098C, 0xA368},	// MCU_ADDRESS
	{0x0990, 0x0080},	// AWB_KB_L
	{0x098C, 0xA369},	// MCU_ADDRESS
	{0x0990, 0x0083},	// 8a AWB_KR_R <-0x0082 (110721 cayman updated)
	{0x098C, 0xA36A},	// MCU_ADDRESS
	{0x0990, 0x0082},	// AWB_KG_R
	{0x098C, 0xA36B},	// MCU_ADDRESS
	{0x0990, 0x007c},	// 82 AWB_KB_R (110721 cayman updated)

	// [LL(Low Light) setting & NR(Noise Reduction)]
	{0x098C, 0xAB1F},	// MCU_ADDRESS
	{0x0990, 0x00C6},	// RESERVED_HG_1F
	{0x098C, 0xAB20},	// MCU_ADDRESS
	{0x0990, 0x0060},	// RESERVED_HG_20(maximum saturation)(080731) 80->43
	{0x098C, 0xAB21},	// MCU_ADDRESS
	{0x0990, 0x001F},	// RESERVED_HG_21
	{0x098C, 0xAB22},	// MCU_ADDRESS
	{0x0990, 0x0003},	// RESERVED_HG_22
	{0x098C, 0xAB23},	// MCU_ADDRESS
	{0x0990, 0x0005},	// RESERVED_HG_23
	{0x098C, 0xAB24},	// MCU_ADDRESS
	{0x0990, 0x0030},	// 20 RESERVED_HG_24(minimum saturation)<-0x0030 (080731) 10->00 (110721 cayman updated)
	{0x098C, 0xAB25},	// MCU_ADDRESS
	{0x0990, 0x0060},	// 35,	// RESERVED_HG_25(noise filter)<-0x0014
	{0x098C, 0xAB26},	// MCU_ADDRESS
	{0x0990, 0x0000},	// RESERVED_HG_26
	{0x098C, 0xAB27},	// MCU_ADDRESS
	{0x0990, 0x0006},	// RESERVED_HG_27
	{0x098C, 0x2B28},	// MCU_ADDRESS
	{0x0990, 0x1800},	// HG_LL_BRIGHTNESSSTART <-0x1388
	{0x098C, 0x2B2A},	// MCU_ADDRESS
	{0x0990, 0x3000},	// HG_LL_BRIGHTNESSSTOP <-0x4E20
	{0x098C, 0xAB2C},	// MCU_ADDRESS
	{0x0990, 0x0006},	// RESERVED_HG_2C
	{0x098C, 0xAB2D},	// MCU_ADDRESS
	{0x0990, 0x000A},	// RESERVED_HG_2D
	{0x098C, 0xAB2E},	// MCU_ADDRESS
	{0x0990, 0x0006},	// RESERVED_HG_2E
	{0x098C, 0xAB2F},	// MCU_ADDRESS
	{0x0990, 0x0006},	// RESERVED_HG_2F
	{0x098C, 0xAB30},	// MCU_ADDRESS
	{0x0990, 0x001E},	// RESERVED_HG_30
	{0x098C, 0xAB31},	// MCU_ADDRESS
	{0x0990, 0x000E},	// RESERVED_HG_31
	{0x098C, 0xAB32},	// MCU_ADDRESS
	{0x0990, 0x001E},	// RESERVED_HG_32
	{0x098C, 0xAB33},	// MCU_ADDRESS
	{0x0990, 0x001E},	// RESERVED_HG_33
	{0x098C, 0xAB34},	// MCU_ADDRESS
	{0x0990, 0x0008},	// RESERVED_HG_34
	{0x098C, 0xAB35},	// MCU_ADDRESS
	{0x0990, 0x0080},	// RESERVED_HG_35

	// [AE WINDOW SIZE POS CHANGE-CENTER] window wider 080827
	{0x098C, 0xA202},	// MCU_ADDRESS [AE_WINDOW_POS] (080731) AE window change
	{0x0990, 0x0021},	// 0x0043,	// MCU_DATA_0
	{0x098C, 0xA203},	// MCU_ADDRESS [AE_WINDOW_SIZE]
	{0x0990, 0x00DD},	// 0x00B9,	// MCU_DATA_0

	// [20070806 tuned]
	{0x098C, 0xA11D},	// MCU_ADDRESS
	{0x0990, 0x0002},	// SEQ_PREVIEW_1_AE
	{0x098C, 0xA208},
	{0x0990, 0x0003},	// 080723  AE speed ..	0x0004	(080731 speed ... 1->3)
	{0x098C, 0xA209},
	{0x0990, 0x0002},
	{0x098C, 0xA20A},
	{0x0990, 0x001F},
	{0x098C, 0xA216},
	{0x0990, 0x003A},
	{0x098C, 0xA244},	// MCU_ADDRESS
	{0x0990, 0x0008},	// RESERVED_AE_44
	{0x098C, 0xA24F},	// MCU_ADDRESS
	{0x0990, 0x0042},	// 080723 AE target 0x0045, // AE_BASETARGET <-0x004A
	{0x098C, 0xA207},	// MCU_ADDRESS
	{0x0990, 0x0005},	// AE_GATE	{0x0990, 0x000A},
	{0x098C, 0xA20D},	// MCU_ADDRESS
	{0x0990, 0x0020},	// AE_MinVirtGain(minimum allowed virtual gain)
	{0x098C, 0xA20E},	// MCU_ADDRESS	// 080723 ... Gain
	{0x0990, 0x0080},	// a0->80  AE_MaxVirtGain(maximum allowed virtual gain)
	{0x098C, 0xAB04},
	{0x0990, 0x0014},
	{0x098C, 0x2361},	// protect the WB hunting
	{0x0990, 0x0a00},	// <-0x00X0
	{0x3244, 0x0310},

	// 2008/01/16 Dgain higher
	{0x098C, 0x2212},	// MCU_ADDRESS
	{0x0990, 0x00F0},	// RESERVED_AE_12(default:0x0080)

	// [Edge]
	{0x326C, 0x1305},

	{0x098C, 0xA103},
	{0x0990, 0x0006},	// refresh mode
//	{0x098C, 0xA103},
//	{0x0990, 0x0005},	// refresh
};


/* LGE_CHANGE_S, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */
#ifdef CONFIG_MACH_LGE
/* White balance register settings */
static struct msm_camera_i2c_reg_conf wb_default_tbl_sub[]=
{
	{0x098C, 0xA102}, {0x0990, 0x000F},	// Mode(AWB/Flicker/AE driver enable)
	{0x098C, 0xA34A}, {0x0990, 0x0059},	// AWB_GAINMIN_R
	{0x098C, 0xA34B}, {0x0990, 0x00E6},	// AWB_GAINMAX_R
	{0x098C, 0xA34C}, {0x0990, 0x0059},	// AWB_GAINMIN_B
	{0x098C, 0xA34D}, {0x0990, 0x00A6},	// AWB_GAINMAX_B
	{0x098C, 0xA351}, {0x0990, 0x0020},	// AWB_CCM_POSITION_MIN
	{0x098C, 0xA352}, {0x0990, 0x007F},	// AWB_CCM_POSITION_MAX
	{0x098C, 0xA365}, {0x0990, 0x0000},	// AWB_X0
	{0x098C, 0xA369}, {0x0990, 0x0083},	// AWB_KR_R
	{0x098C, 0xA36A}, {0x0990, 0x0082},	// AWB_KG_R
	{0x098C, 0xA36B}, {0x0990, 0x0082},	// AWB_KB_R
};
static struct msm_camera_i2c_reg_conf wb_sunny_tbl_sub[]=
{
	{0x098C, 0xA102}, {0x0990, 0x000B},	// Mode(AWB disable)
	{0x098C, 0xA34A}, {0x0990, 0x00B0},	// AWB_GAIN_MIN
	{0x098C, 0xA34B}, {0x0990, 0x00B0},	// AWB_GAIN_MAX
	{0x098C, 0xA34C}, {0x0990, 0x0080},	// AWB_GAINMIN_B
	{0x098C, 0xA34D}, {0x0990, 0x0080},	// AWB_GAINMAX_B
	{0x098C, 0xA351}, {0x0990, 0x004B},	// AWB_CCM_POSITION_MIN
	{0x098C, 0xA352}, {0x0990, 0x004B},	// AWB_CCM_POSITION_MAX
	{0x098C, 0xA353}, {0x0990, 0x004B},	// AWB_CCM_POSITION
	{0x098C, 0xA369}, {0x0990, 0x0095},	// AWB_KR_R
	{0x098C, 0xA36A}, {0x0990, 0x0075},	// AWB_KG_R
	{0x098C, 0xA36B}, {0x0990, 0x008A},	// AWB_KB_R
};
static struct msm_camera_i2c_reg_conf wb_cloudy_tbl_sub[]=
{
	{0x098C, 0xA102}, {0x0990, 0x000B},	// Mode(AWB disable)
	{0x098C, 0xA34A}, {0x0990, 0x00B0},	// AWB_GAIN_MIN
	{0x098C, 0xA34B}, {0x0990, 0x00B0},	// AWB_GAIN_MAX
	{0x098C, 0xA34C}, {0x0990, 0x0080},	// AWB_GAINMIN_B
	{0x098C, 0xA34D}, {0x0990, 0x0080},	// AWB_GAINMAX_B
	{0x098C, 0xA351}, {0x0990, 0x004B},	// AWB_CCM_POSITION_MIN
	{0x098C, 0xA352}, {0x0990, 0x004B},	// AWB_CCM_POSITION_MAX
	{0x098C, 0xA353}, {0x0990, 0x004B},	// AWB_CCM_POSITION
	{0x098C, 0xA369}, {0x0990, 0x00A5},	// AWB_KR_R
	{0x098C, 0xA36A}, {0x0990, 0x0080},	// AWB_KG_R
	{0x098C, 0xA36B}, {0x0990, 0x0065},	// AWB_KB_R
};
static struct msm_camera_i2c_reg_conf wb_fluorescent_tbl_sub[]=
{
	{0x098C, 0xA102}, {0x0990, 0x000B},	// Mode(AWB disable)
	{0x098C, 0xA34A}, {0x0990, 0x0059},	// AWB_GAINMIN_R
	{0x098C, 0xA34B}, {0x0990, 0x00E6},	// AWB_GAINMAX_R
	{0x098C, 0xA34C}, {0x0990, 0x0059},	// AWB_GAINMIN_B
	{0x098C, 0xA34D}, {0x0990, 0x00A6},	// AWB_GAINMAX_B
	{0x098C, 0xA351}, {0x0990, 0x0020},	// AWB_CCM_POSITION_MIN
	{0x098C, 0xA352}, {0x0990, 0x007F},	// AWB_CCM_POSITION_MAX
	{0x098C, 0xA365}, {0x0990, 0x0000},	// AWB_X0
	{0x098C, 0xA369}, {0x0990, 0x0075},	// AWB_KR_R
	{0x098C, 0xA36A}, {0x0990, 0x0082},	// AWB_KG_R
	{0x098C, 0xA36B}, {0x0990, 0x00A5},	// AWB_KB_R
};
static struct msm_camera_i2c_reg_conf wb_incandescent_tbl_sub[]=
{
	{0x098C, 0xA102}, {0x0990, 0x000B},	// Mode(AWB disable)
	{0x098C, 0xA34A}, {0x0990, 0x0092},	// AWB_GAIN_MIN
	{0x098C, 0xA34B}, {0x0990, 0x0092},	// AWB_GAIN_MAX
	{0x098C, 0xA34C}, {0x0990, 0x0096},	// AWB_GAINMIN_B
	{0x098C, 0xA34D}, {0x0990, 0x0096},	// AWB_GAINMAX_B
	{0x098C, 0xA351}, {0x0990, 0x001F},	// AWB_CCM_POSITION_MIN
	{0x098C, 0xA352}, {0x0990, 0x001F},	// AWB_CCM_POSITION_MAX
	{0x098C, 0xA353}, {0x0990, 0x001F},	// AWB_CCM_POSITION
	{0x098C, 0xA369}, {0x0990, 0x0072},	// AWB_KR_R
	{0x098C, 0xA36A}, {0x0990, 0x0082},	// AWB_KG_R
	{0x098C, 0xA36B}, {0x0990, 0x007C},	// AWB_KB_R
};

/* Effect register settings */
static struct msm_camera_i2c_reg_conf effect_default_tbl_sub[]=
{
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6440},	// MODE_A_SPECIAL_EFFECT_OFF
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6440},	// MODE_B_SPECIAL_EFFECT_OFF
};
static struct msm_camera_i2c_reg_conf effect_mono_tbl_sub[]=
{
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6441},	// MODE_A_MONO_ON
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6441},	// MODE_B_MONO_ON
};
static struct msm_camera_i2c_reg_conf effect_sepia_tbl_sub[]=
{
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6442},	// MODE_A_SEPIA_ON
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6442},	// MODE_B_SEPIA_ON
//Start LGE_BSP_CAMERA::elin.lee@lge.com 2011-07-14  Fix the Sepia effect
	{0x098C, 0x2763},	// LOGICAL_ADDRESS_ACCESS [MODE_COMMONMODESETTINGS_FX_SEPIA_SETTINGS]
	{0x0990, 0xE817},	// MCU_DATA_0
//End LGE_BSP_CAMERA::elin.lee@lge.com 2011-07-14  Fix the Sepia effect
};
static struct msm_camera_i2c_reg_conf effect_negative_tbl_sub[]=
{
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6443},	// MODE_A_NEGATIVE_ON
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6443},	// MODE_B_NEGATIVE_ON
};

/* Change-config register settings only used for change brightness*/
static struct msm_camera_i2c_reg_conf change_config_tbl_sub[]=
{
	//	{0x098C, 0xA103},	// LOGICAL_ADDRESS_ACCESS			===== 8
	//	{0x0990, 0x0006},	// AE_TRACK_AE_TRACKING_DAMPENING_SPEED
	//	{0xFFFF, 0x012C},	// LGE_BSP_CAMERA   miracle.kim@lge.com    delete for reducing camera start time
	{0x098C, 0xA103},	// COMMAND_REGISTER
	{0x0990, 0x0005},
	//	{0xFFFF, 0x012C},	// LGE_BSP_CAMERA miralce.kim@lge.com delete for reducing camera start time
};
#endif
/* LGE_CHANGE_E, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */

static struct msm_camera_i2c_reg_conf brightness_0_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x0005}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x000C}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x0017}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x002A}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x003D}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x0050}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x0063}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x0077}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x008B}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x009D}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00AD}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00BC}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00CA}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00D6}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00E1}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00EC}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F6}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0025}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_1_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x0008}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0011}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x001F}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x0036}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x004B}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x005F}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x0073}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x0088}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x009A}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00AA}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00B8}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00C5}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00D1}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00DC}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00E5}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00EF}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F7}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0025}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_2_tbl_sub[]=
{
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x0008}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0011}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x001F}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x0036}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x004B}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x005F}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x0073}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x0088}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x009A}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00AA}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00B8}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00C5}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00D1}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00DC}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00E5}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00EF}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F7}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x002f}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_3_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x000E}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x001A}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x002C}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x0047}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x005D}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x0073}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x0087}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x0099}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00A8}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00B5}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00C2}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00CD}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00D7}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00E0}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00E9}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F1}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F8}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x002f}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_4_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x000E}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x001A}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x002C}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x0047}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x005D}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x0073}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x0087}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x0099}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00A8}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00B5}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00C2}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00CD}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00D7}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00E0}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00E9}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F1}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F8}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0037}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_5_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x0017}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0027}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x003B}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x005A}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x0073}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x008A}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x009C}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00AB}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00B8}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00C4}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00CE}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00D7}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00DF}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00E6}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00ED}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F4}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F9}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0037}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_6_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x0017}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0027}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x003B}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x005A}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x0073}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x008A}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x009C}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00AB}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00B8}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00C4}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00CE}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00D7}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00DF}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00E6}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00ED}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F4}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F9}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0042}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_7_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x0017}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0027}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x003B}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x005A}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x0073}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x008A}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x009C}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00AB}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00B8}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00C4}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00CE}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00D7}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00DF}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00E6}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00ED}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F4}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00F9}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0051}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_8_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x001E}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0031}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x0048}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x0068}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x0081}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x0095}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x00A6}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00B3}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00BF}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00C9}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00D2}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00DA}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00E2}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00E8}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00EF}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F4}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00FA}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0051}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_9_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x001E}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0031}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x0048}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x0068}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x0081}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x0095}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x00A6}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00B3}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00BF}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00C9}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00D2}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00DA}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00E2}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00E8}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00EF}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F4}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00FA}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0060}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_10_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x002E}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0040}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x0058}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x0078}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x0091}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x00A3}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x00B1}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00BD}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00C7}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00D0}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00D8}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00DF}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00E5}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00EB}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00F1}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F6}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00FB}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0060}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_11_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x002E}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x0040}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x0058}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x0078}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x0091}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x00A3}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x00B1}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00BD}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00C7}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00D0}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00D8}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00DF}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00E5}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00EB}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00F1}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F6}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00FB}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS [AE_BASETARGET]
	{0x0990, 0x0072}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS [AE_STATUS]
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf brightness_12_tbl_sub[]=
{
	{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
	{0x0990, 0x003B}, 	// MCU_DATA_0
	{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
	{0x0990, 0x004F}, 	// MCU_DATA_0
	{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
	{0x0990, 0x0068}, 	// MCU_DATA_0
	{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
	{0x0990, 0x008A}, 	// MCU_DATA_0
	{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
	{0x0990, 0x009F}, 	// MCU_DATA_0
	{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
	{0x0990, 0x00AF}, 	// MCU_DATA_0
	{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
	{0x0990, 0x00BC}, 	// MCU_DATA_0
	{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
	{0x0990, 0x00C6}, 	// MCU_DATA_0
	{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
	{0x0990, 0x00CF}, 	// MCU_DATA_0
	{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
	{0x0990, 0x00D7}, 	// MCU_DATA_0
	{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
	{0x0990, 0x00DD}, 	// MCU_DATA_0
	{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
	{0x0990, 0x00E3}, 	// MCU_DATA_0
	{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
	{0x0990, 0x00E9}, 	// MCU_DATA_0
	{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
	{0x0990, 0x00EE}, 	// MCU_DATA_0
	{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
	{0x0990, 0x00F3}, 	// MCU_DATA_0
	{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
	{0x0990, 0x00F7}, 	// MCU_DATA_0
	{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
	{0x0990, 0x00FB}, 	// MCU_DATA_0
	{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
	{0x0990, 0x00FF}, 	// MCU_DATA_0

	{0x098C, 0xA217}, 	// MCU_ADDRESS
	{0x0990, 0x0004}, 	// MCU_DATA_0
	{0x098C, 0xA24F}, 	// MCU_ADDRESS
	{0x0990, 0x0072}, 	// MCU_DATA_0
	{0x098C, 0xA217}, 	// MCU_ADDRESS
	{0x0990, 0x0004}, 	// MCU_DATA_0
};

static struct v4l2_subdev_info mt9v113_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8, /* For YUV type sensor (YUV422) */
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order  = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array mt9v113_init_conf[] = {
	{&mt9v113_recommend_settings[0],
	ARRAY_SIZE(mt9v113_recommend_settings), 0, MSM_CAMERA_I2C_WORD_DATA}
};

static struct msm_camera_i2c_conf_array mt9v113_confs[] = {
	{&mt9v113_prev_settings[0],
	ARRAY_SIZE(mt9v113_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_sensor_output_info_t mt9v113_dimensions[] = {
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		.line_length_pclk = 0xD06,
		.frame_length_lines = 0x04ED,
		.vt_pixel_clk = 45600000,
		.op_pixel_clk = 45600000,
	},
};

static struct msm_camera_csid_vc_cfg mt9v113_cid_cfg[] = {
	{0, CSI_YUV422_8, CSI_DECODE_8BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params mt9v113_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 1,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = mt9v113_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 1,
		.settle_cnt = 0x14,
	},
};

static struct msm_camera_csi2_params *mt9v113_csi_params_array[] = {
	&mt9v113_csi_params,
};

static struct msm_sensor_output_reg_addr_t mt9v113_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t mt9v113_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x2280,
};

static struct msm_sensor_exp_gain_info_t mt9v113_exp_gain_info = {
	.coarse_int_time_addr = 0x3012,
	.global_gain_addr = 0x305E,
	.vert_offset = 0x00,
};

//static struct sensor_calib_data mt9v113_calib_data;
static const struct i2c_device_id mt9v113_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9v113_s_ctrl},
	{ }
};

static struct i2c_driver mt9v113_i2c_driver = {
	.id_table = mt9v113_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9v113_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&mt9v113_i2c_driver);
}

static struct v4l2_subdev_core_ops mt9v113_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

#define GPIO_VGACAM_LDO_EN      (64)
#ifdef CONFIG_MACH_MSM8960_L0
#define GPIO_CAM2_PWRDOWN       (54)
#endif
int mt9v113_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	pr_err("%s: %d\n", __func__, __LINE__);
	s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
			* data->sensor_platform_info->num_vreg, GFP_KERNEL);
	if (!s_ctrl->reg_ptr) {
		pr_err("%s: could not allocate mem for regulators\n",
			__func__);
		return -ENOMEM;
	}

	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}

	rc = msm_camera_request_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		goto request_gpio_failed;
	}
#ifdef CONFIG_MACH_MSM8960_L0
		/* CAMVDD */
	rc = gpio_request(GPIO_CAM2_PWRDOWN, "CAM_PWRDOWN");
	if (rc) {
		pr_err("gpio_request(CAM_PWRDOWN) error\n");
	} else {
		rc = gpio_direction_output(GPIO_VGACAM_LDO_EN, 0);
		if (rc) {
			pr_err("gpio_direction_output(CAM_PWRDOWN) error\n");
		}
	}
#endif

	rc = msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 1);
	if (rc < 0) {
		pr_err("%s: regulator on failed\n", __func__);
		goto config_vreg_failed;
	}

	rc = msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 1);
	if (rc < 0) {
		pr_err("%s: enable regulator failed\n", __func__);
		goto enable_vreg_failed;
	}
#ifndef CONFIG_MACH_MSM8960_L0
	/* CAMVDD */
	rc = gpio_request(GPIO_VGACAM_LDO_EN, "CAM_GPIO");
	if (rc) {
		pr_err("gpio_request(GPIO_VGACAM_LDO_EN) error\n");
	} else {
		rc = gpio_direction_output(GPIO_VGACAM_LDO_EN, 1);
		if (rc) {
			pr_err("gpio_direction_output(GPIO_VGACAM_LDO_EN) error\n");
		}
	}
#endif
	rc = msm_camera_config_gpio_table(data, 0);

	msleep(10);

	if (s_ctrl->clk_rate != 0)
		cam_clk_info->clk_rate = s_ctrl->clk_rate;

	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}

	/* Recommanded delay for mt9v113 */
	mdelay(10);
	rc = msm_camera_config_gpio_table(data, 1);
	mdelay(10);
	/* Recommanded delay for mt9v113 */

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(1);

	return rc;
enable_clk_failed:
	msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 0);

enable_vreg_failed:
	msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
		s_ctrl->sensordata->sensor_platform_info->cam_vreg,
		s_ctrl->sensordata->sensor_platform_info->num_vreg,
		s_ctrl->reg_ptr, 0);
config_vreg_failed:
	msm_camera_request_gpio_table(data, 0);
request_gpio_failed:
	msm_camera_config_gpio_table(data, 0);
config_gpio_failed:
	kfree(s_ctrl->reg_ptr);
	return rc;
}

int mt9v113_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	CDBG("%s\n", __func__);
	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(0);
	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
	msm_camera_config_gpio_table(data, 0);
	msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
		s_ctrl->sensordata->sensor_platform_info->cam_vreg,
		s_ctrl->sensordata->sensor_platform_info->num_vreg,
		s_ctrl->reg_ptr, 0);
	msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
		s_ctrl->sensordata->sensor_platform_info->cam_vreg,
		s_ctrl->sensordata->sensor_platform_info->num_vreg,
		s_ctrl->reg_ptr, 0);
#ifndef CONFIG_MACH_MSM8960_L0
	/* CAMVDD */
	if (gpio_direction_output(GPIO_VGACAM_LDO_EN, 0)) {
		pr_err("gpio_direction_output(GPIO_VGACAM_LDO_EN, 0) error\n");
	} else {
		gpio_free(GPIO_VGACAM_LDO_EN);
	}
#endif
	msm_camera_request_gpio_table(data, 0);
	kfree(s_ctrl->reg_ptr);
	return 0;
}

int32_t mt9v113_camera_i2c_write_tbl(struct msm_camera_i2c_client *client,
                                     struct msm_camera_i2c_reg_conf *reg_conf_tbl, uint16_t size,
                                     enum msm_camera_i2c_data_type data_type)
{
	int i;
	int32_t rc = -EIO;
#if 1 // seonghyon.cho@lge.com 2011.10.19
	printk(KERN_ERR "### %s %d %d size: %d \n",
		__func__,client->addr_type,data_type, size);
#endif

	for (i = 0; i < size; i++) {
		if (reg_conf_tbl->reg_addr == 0xFFFF) {
			msleep(reg_conf_tbl->reg_data);
			rc = 0;
		} else if (reg_conf_tbl->reg_addr == 0xFFFE) {
			unsigned short test_data = 0;
			for (i = 0; i < 50; i++) {
				/* max delay ==> 500 ms */
				rc  = msm_camera_i2c_read(client, 0x0080, &test_data, data_type);
				if (rc < 0)
					return rc;
				if((test_data & reg_conf_tbl->reg_data) == 0)
					break;
				else
					mdelay(10);

				/*	printk(KERN_ERR "### %s :  Polling set, 0x0080 Reg : 0x%x\n", __func__, test_data);  */
			}
		} else if (reg_conf_tbl->reg_addr == 0x301A) {
			unsigned short test_data = 0;
			rc  = msm_camera_i2c_read(client, 0x301A, &test_data, data_type);
			if (rc < 0)
				return rc;
			rc = msm_camera_i2c_write(client, 0x301A, test_data | 0x0200, data_type);
			if (rc < 0)
				return rc;

			/*	 printk(KERN_ERR "### %s : Reset reg check, 0x301A Reg : 0x%x\n", __func__, test_data|0x0200);  */
		} else if ((reg_conf_tbl->reg_addr == 0x0080) &&
		                ((reg_conf_tbl->reg_data == 0x8000) || (reg_conf_tbl->reg_data == 0x0001))) {
			unsigned short test_data = 0;
			rc  = msm_camera_i2c_read(client, 0x0080, &test_data, data_type);
			if (rc < 0)
				return rc;

			test_data = test_data | reg_conf_tbl->reg_data;
			rc = msm_camera_i2c_write(client, 0x0080, test_data, data_type);
			if (rc < 0)
				return rc;

			/*	printk(KERN_ERR "### %s : Patch check, 0x0080 Reg : 0x%x\n", __func__,test_data);  */
		} else {
			rc = msm_camera_i2c_write(client, reg_conf_tbl->reg_addr, reg_conf_tbl->reg_data, data_type);
			if (rc < 0)
				return rc;
			/*	rc = mt9v113_i2c_write(mt9v113_client->addr, reg_conf_tbl->waddr, reg_conf_tbl->wdata, reg_conf_tbl->width);  */
		}

		if (rc < 0)
			break;

		reg_conf_tbl++;
	}

	return rc;
}

int32_t mt9v113_sensor_write_init_settings(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0, i;
	for (i = 0; i < s_ctrl->msm_sensor_reg->init_size; i++) {
		rc = mt9v113_camera_i2c_write_tbl(
		             s_ctrl->sensor_i2c_client,
		             s_ctrl->msm_sensor_reg->init_settings[i].conf,
		             s_ctrl->msm_sensor_reg->init_settings[i].size,
		             s_ctrl->msm_sensor_reg->init_settings[i].data_type);

		if (rc < 0)
			break;
	}

	return rc;
}

int32_t mt9v113_sensor_write_res_settings(struct msm_sensor_ctrl_t *s_ctrl,
                uint16_t res)
{
	int32_t rc = 0;
/*
	rc = mt9v113_camera_i2c_write_tbl(
	             s_ctrl->sensor_i2c_client,
	             s_ctrl->msm_sensor_reg->mode_settings[res].conf,
	             s_ctrl->msm_sensor_reg->mode_settings[res].size,
	             s_ctrl->msm_sensor_reg->mode_settings[res].data_type);
	if (rc < 0)
		return rc;

	rc = msm_sensor_write_output_settings(s_ctrl, res);
*/
	return rc;
}


void mt9v113_sensor_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	mt9v113_camera_i2c_write_tbl(
	        s_ctrl->sensor_i2c_client,
	        s_ctrl->msm_sensor_reg->start_stream_conf,
	        s_ctrl->msm_sensor_reg->start_stream_conf_size,
	        s_ctrl->msm_sensor_reg->default_data_type);
}

void mt9v113_sensor_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	mt9v113_camera_i2c_write_tbl(
	        s_ctrl->sensor_i2c_client,
	        s_ctrl->msm_sensor_reg->stop_stream_conf,
	        s_ctrl->msm_sensor_reg->stop_stream_conf_size,
	        s_ctrl->msm_sensor_reg->default_data_type);
}

void mt9v113_sensor_group_hold_on(struct msm_sensor_ctrl_t *s_ctrl)
{
	return;
}

void mt9v113_sensor_group_hold_off(struct msm_sensor_ctrl_t *s_ctrl)
{
	return;
}


int32_t mt9v113_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
                               int update_type, int res)
{
	int32_t rc = 0;

	v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
		NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
		PIX_0, ISPIF_OFF_IMMEDIATELY));
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		mt9v113_sensor_write_init_settings(s_ctrl);
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		mt9v113_sensor_write_res_settings(s_ctrl, res);
		if (s_ctrl->curr_csi_params != s_ctrl->csi_params[res]) {
			s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSID_CFG,
				&s_ctrl->curr_csi_params->csid_params);
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
						NOTIFY_CID_CHANGE, NULL);
			mb();
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIPHY_CFG,
				&s_ctrl->curr_csi_params->csiphy_params);
			mb();
			msleep(20);
		}

		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
			output_settings[res].op_pixel_clk);
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
			PIX_0, ISPIF_ON_FRAME_BOUNDARY));
		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(30);
	}

/* LGE_CHANGE_S, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */
#ifdef CONFIG_MACH_LGE
	prev_balance_mode = -1;

	prev_effect_mode = -1;

	prev_brightness_mode = -1;
#endif
/* LGE_CHANGE_E, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */

	return rc;
}


/* ES4 ku.kwon@lge.com
static int __init msm_sensor_init_module(void)
{
	printk("yongjin1.kim: %s\n", __func__);
	return platform_driver_register(&mt9v113_driver);
}
*/

/* LGE_CHANGE_S, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */
#ifdef CONFIG_MACH_LGE
/*
static int mt9v113_polling_reg(unsigned short waddr, unsigned short wcondition, unsigned short result, int delay, int time_out)
{
	int rc = -EFAULT;
	int i=0;
	unsigned short wdata;
	printk("### yongjin1.kim, %s \n", __func__);

	for(i=0; i<time_out; i++){
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, waddr, &wdata, MSM_CAMERA_I2C_WORD_DATA);

		if (rc<0) {
			CDBG("polling reg failed to read\n");
			return rc;
		}

		CDBG("polling reg 0x%x ==> 0x%x\n", waddr, wdata);

		if ((wdata&wcondition)==result) {
			CDBG("polling register meets condition\n");
			break;
		}
		else
			mdelay(delay);
	}

	return rc;
}
//rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x0990, &mcu_data, MSM_CAMERA_I2C_WORD_DATA);
//rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0990, 0x0004, MSM_CAMERA_I2C_WORD_DATA);
*/
int mt9v113_check_sensor_mode(struct msm_sensor_ctrl_t *s_ctrl)
{
	unsigned short mcu_address_sts =0, mcu_data =0;
	int i, rc=-EFAULT;

	CDBG("mt9v113 : check_sensor_mode E\n");

	for(i=0; i<50; i++){

		/* MCU_ADDRESS : check mcu_address */
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x098C, &mcu_address_sts, MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0){
			CDBG("mt9v113: reading mcu_address_sts fail\n");
			return rc;
		}

		/* MCU_DATA_0 : check mcu_data */
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x0990, &mcu_data, MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0){
			CDBG("mt9v113: reading mcu_data fail\n");
			return rc;
		}


		if( ((mcu_address_sts & 0xA103) == 0xA103) && (mcu_data == 0x00)){
			CDBG("mt9v113: sensor refresh mode success!!\n");
			break;
		}
		msleep(10);
	}

	CDBG("mt9v113: check_sensor_mode X\n");

	return rc;
}

static int32_t mt9v113_set_wb(struct msm_sensor_ctrl_t *s_ctrl, uint8_t mode)
{
	int32_t rc = 0;
	// printk("ku.kwon: %s %d\n", __func__, mode);

	if(prev_balance_mode == mode)
	{
		CDBG("###  [CHECK]%s: skip this function, wb_mode -> %d\n", __func__, mode);
		return rc;
	}
       CDBG("###  [CHECK]%s: mode -> %d\n", __func__, mode);

	switch (mode) {
		case CAMERA_WB_AUTO:
			rc = mt9v113_camera_i2c_write_tbl(
						 s_ctrl->sensor_i2c_client,
						 wb_default_tbl_sub,
						 ARRAY_SIZE(wb_default_tbl_sub),
						 MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_WB_DAYLIGHT:	// sunny
			rc = mt9v113_camera_i2c_write_tbl(
						 s_ctrl->sensor_i2c_client,
						 wb_sunny_tbl_sub,
						 ARRAY_SIZE(wb_sunny_tbl_sub),
						 MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_WB_CLOUDY_DAYLIGHT:  // cloudy
			rc = mt9v113_camera_i2c_write_tbl(
						 s_ctrl->sensor_i2c_client,
						 wb_cloudy_tbl_sub,
						 ARRAY_SIZE(wb_cloudy_tbl_sub),
						 MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_WB_FLUORESCENT:
			rc = mt9v113_camera_i2c_write_tbl(
						 s_ctrl->sensor_i2c_client,
						 wb_fluorescent_tbl_sub,
						 ARRAY_SIZE(wb_fluorescent_tbl_sub),
						 MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_WB_INCANDESCENT:
			rc = mt9v113_camera_i2c_write_tbl(
						 s_ctrl->sensor_i2c_client,
						 wb_incandescent_tbl_sub,
						 ARRAY_SIZE(wb_incandescent_tbl_sub),
						 MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		default:
			return -EINVAL;
	}
 	rc = mt9v113_camera_i2c_write_tbl(
			s_ctrl->sensor_i2c_client,
			change_config_tbl_sub,
			ARRAY_SIZE(change_config_tbl_sub),
			MSM_CAMERA_I2C_WORD_DATA);
	if (rc < 0)
		return rc;

	rc = mt9v113_check_sensor_mode(s_ctrl);

	if (rc<0)
	{
		CDBG("###[ERROR]%s: failed to check sensor mode\n", __func__);
		return rc;
	}

	prev_balance_mode = mode;
	return rc;
}

static int32_t mt9v113_set_effect(struct msm_sensor_ctrl_t *s_ctrl, uint8_t mode)
{
	int32_t rc = 0;
	// printk("yongjin1.kim: %s %d\n", __func__, mode);

	if(prev_effect_mode == mode)
	{
		CDBG("###  [CHECK]%s: skip this function, effect_mode -> %d\n", __func__, mode);
		return rc;
	}
       CDBG("###  [CHECK]%s: mode -> %d\n", __func__, mode);

	switch (mode) {
		case CAMERA_EFFECT_OFF:
			rc = mt9v113_camera_i2c_write_tbl(
						 s_ctrl->sensor_i2c_client,
						 effect_default_tbl_sub,
						 ARRAY_SIZE(effect_default_tbl_sub),
						 MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_EFFECT_MONO:		// mono
			rc = mt9v113_camera_i2c_write_tbl(
						 s_ctrl->sensor_i2c_client,
						 effect_mono_tbl_sub,
						 ARRAY_SIZE(effect_mono_tbl_sub),
						 MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_EFFECT_SEPIA:		// sepia
			rc = mt9v113_camera_i2c_write_tbl(
						 s_ctrl->sensor_i2c_client,
						 effect_sepia_tbl_sub,
						 ARRAY_SIZE(effect_sepia_tbl_sub),
						 MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_EFFECT_NEGATIVE:	// negative
			rc = mt9v113_camera_i2c_write_tbl(
						 s_ctrl->sensor_i2c_client,
						 effect_negative_tbl_sub,
						 ARRAY_SIZE(effect_negative_tbl_sub),
						 MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		default:
			return -EINVAL;
	}
 	rc = mt9v113_camera_i2c_write_tbl(
			s_ctrl->sensor_i2c_client,
			change_config_tbl_sub,
			ARRAY_SIZE(change_config_tbl_sub),
			MSM_CAMERA_I2C_WORD_DATA);
	if (rc < 0)
		return rc;

	rc = mt9v113_check_sensor_mode(s_ctrl);

	if (rc<0)
	{
		CDBG("###[ERROR]%s: failed to check sensor mode\n", __func__);
		return rc;
	}

	prev_effect_mode = mode;
	return rc;
}

// static int brightness_table[] = {0x0024, 0x0029, 0x002E, 0x0033, 0x0038, 0x003D, 0x0042, 0x0047, 0x004C, 0x0051, 0x0056, 0x005B, 0x0060};
// {0x001E, 0x0021, 0x0024, 0x0027, 0x002A, 0x002D, 0x0030, 0x0033, 0x0036, 0x0039, 0x003C, 0x003F, 0x0042, 0x0045, 0x0048, 0x004B, 0x004E, 0x0051, 0x0054, 0x0057, 0x005A, 0x005D, 0x0060, 0x0063, 0x0066};

// {0x0015, 0x001C, 0x0022, 0x0029, 0x0032, 0x0039, 0x0042, 0x0049, 0x0052, 0x0059, 0x0062, 0x0069, 0x0072};

static int mt9v113_set_brightness(struct msm_sensor_ctrl_t *s_ctrl, uint8_t mode)
{
	int32_t rc = 0;
	printk("### yongjin1.kim, %s: mode = %d(2)\n", __func__, mode);

       // printk(KERN_ERR "###  [CHECK]%s: mode -> %d\n", __func__, mode);

	if (prev_brightness_mode==mode){
		CDBG("###%s: skip mt9v113_set_brightness\n", __func__);
		return rc;
	}
	if(mode < 0 || mode > 12){
		CDBG("###[ERROR]%s: Invalid Mode value\n", __func__);
		return -EINVAL;
	}

	switch (mode) {
		case 0:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_0_tbl_sub, ARRAY_SIZE(brightness_0_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 1:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_1_tbl_sub, ARRAY_SIZE(brightness_1_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 2:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_2_tbl_sub, ARRAY_SIZE(brightness_2_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 3:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_3_tbl_sub, ARRAY_SIZE(brightness_3_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 4:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_4_tbl_sub, ARRAY_SIZE(brightness_4_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 5:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_5_tbl_sub, ARRAY_SIZE(brightness_5_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 6:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_6_tbl_sub, ARRAY_SIZE(brightness_6_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 7:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_7_tbl_sub, ARRAY_SIZE(brightness_7_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 8:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_8_tbl_sub, ARRAY_SIZE(brightness_8_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 9:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_9_tbl_sub, ARRAY_SIZE(brightness_9_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 10:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_10_tbl_sub, ARRAY_SIZE(brightness_10_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 11:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_11_tbl_sub, ARRAY_SIZE(brightness_11_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		case 12:
			rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, brightness_12_tbl_sub, ARRAY_SIZE(brightness_12_tbl_sub), MSM_CAMERA_I2C_WORD_DATA);
			if (rc < 0)
				return rc;
			break;
		default:
			return -EINVAL;
	}

 	rc = mt9v113_camera_i2c_write_tbl(
			s_ctrl->sensor_i2c_client,
			change_config_tbl_sub,
			ARRAY_SIZE(change_config_tbl_sub),
			MSM_CAMERA_I2C_WORD_DATA);
	if (rc < 0)
		return rc;

	rc = mt9v113_check_sensor_mode(s_ctrl);

	if (rc<0)
	{
		CDBG("###[ERROR]%s: failed to check sensor mode\n", __func__);
		return rc;
	}
	prev_brightness_mode = mode;

	return rc;
}
#endif
/* LGE_CHANGE_E, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */

static struct v4l2_subdev_video_ops mt9v113_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9v113_subdev_ops = {
	.core = &mt9v113_subdev_core_ops,
	.video  = &mt9v113_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9v113_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
#if 0 /* removed at M8960AAAAANLYA1022 */
	.sensor_get_prev_lines_pf = msm_sensor_get_prev_lines_pf,
	.sensor_get_prev_pixels_pl = msm_sensor_get_prev_pixels_pl,
	.sensor_get_pict_lines_pf = msm_sensor_get_pict_lines_pf,
	.sensor_get_pict_pixels_pl = msm_sensor_get_pict_pixels_pl,
	.sensor_get_pict_max_exp_lc = msm_sensor_get_pict_max_exp_lc,
	.sensor_get_pict_fps = msm_sensor_get_pict_fps,
#endif
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = msm_sensor_write_exp_gain1,
	.sensor_setting = mt9v113_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = mt9v113_sensor_power_up,
	.sensor_power_down = mt9v113_sensor_power_down,
/* LGE_CHANGE_S, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */
#ifdef CONFIG_MACH_LGE
	.sensor_set_wb = mt9v113_set_wb,
	.sensor_set_effect = mt9v113_set_effect,
	.sensor_set_brightness = mt9v113_set_brightness,
#endif
/* LGE_CHANGE_E, Implementation of SoC Sensor features for v4l2 system, 2012.02.02, yongjin1.kim@lge.com */
};

static struct msm_sensor_reg_t mt9v113_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
	.start_stream_conf = mt9v113_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(mt9v113_start_settings),
	.stop_stream_conf = mt9v113_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(mt9v113_stop_settings),
	.group_hold_on_conf = mt9v113_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(mt9v113_groupon_settings),
	.group_hold_off_conf = mt9v113_groupoff_settings,
	.group_hold_off_conf_size =
	ARRAY_SIZE(mt9v113_groupoff_settings),
	.init_settings = &mt9v113_init_conf[0],
	.init_size = ARRAY_SIZE(mt9v113_init_conf),
	.mode_settings = &mt9v113_confs[0],
	.output_settings = &mt9v113_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9v113_confs),
};

static struct msm_sensor_ctrl_t mt9v113_s_ctrl = {
	.msm_sensor_reg = &mt9v113_regs,
	.sensor_i2c_client = &mt9v113_sensor_i2c_client,
	.sensor_i2c_addr = 0x7A,
	.sensor_output_reg_addr = &mt9v113_reg_addr,
	.sensor_id_info = &mt9v113_id_info,
	.sensor_exp_gain_info = &mt9v113_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csi_params = &mt9v113_csi_params_array[0],
	.msm_sensor_mutex = &mt9v113_mut,
	.sensor_i2c_driver = &mt9v113_i2c_driver,
	.sensor_v4l2_subdev_info = mt9v113_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9v113_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9v113_subdev_ops,
	.func_tbl = &mt9v113_func_tbl,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("mt9v113 VT sensor driver");
MODULE_LICENSE("GPL v2");


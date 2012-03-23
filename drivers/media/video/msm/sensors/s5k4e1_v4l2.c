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
#define SENSOR_NAME "s5k4e1"
#define PLATFORM_DRIVER_NAME "msm_camera_s5k4e1"
#define s5k4e1_obj s5k4e1_##obj

DEFINE_MUTEX(s5k4e1_mut);
static struct msm_sensor_ctrl_t s5k4e1_s_ctrl;

static struct msm_camera_i2c_reg_conf s5k4e1_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4e1_stop_settings[] = {
	{0x0100, 0x00},
};
static struct msm_camera_i2c_reg_conf s5k4e1_groupon_settings[] = {
	{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4e1_groupoff_settings[] = {
	{0x0104, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k4e1_prev_settings[] = {
	{0x301B, 0x83},/* CDS option setting */
	{0x30BC, 0xB0},/* LGE_CHANGE Full HD support, 2012.03.28 yt.kim@lge.com */
	/* PLL setting ... */
	/* (3) MIPI 2-lane Serial(TST = 0000b or TST = 0010b), 30 fps */
	{0x0305, 0x06},/* PLL P = 6 */
	{0x0306, 0x00},/* PLL M[8] = 0 */
	{0x0307, 0x32},
	{0x30B5, 0x00},
	{0x30E2, 0x02},
	{0x30F1, 0x70},
	/* output size (1304 x 980) */
	/* MIPI Size Setting ... */
	{0x30A9, 0x02},/* Horizontal Binning On */
	{0x300E, 0xEB},/* Vertical Binning On */
	{0x0387, 0x03},/* y_odd_inc 03(10b AVG) */
	{0x0344, 0x00},/* x_addr_start 0 */
	{0x0345, 0x00},
	{0x0348, 0x0A},/* x_addr_end 2607 */
	{0x0349, 0x2F},
	{0x0346, 0x00},/* y_addr_start 0 */
	{0x0347, 0x00},
	{0x034A, 0x07},/* y_addr_end 1959 */
	{0x034B, 0xA7},
	{0x0380, 0x00},/* x_even_inc 1 */
	{0x0381, 0x01},
	{0x0382, 0x00},/* x_odd_inc 1 */
	{0x0383, 0x01},
	{0x0384, 0x00},/* y_even_inc 1 */
	{0x0385, 0x01},
	{0x0386, 0x00},/* y_odd_inc 3 */
	{0x0387, 0x03},
	{0x034C, 0x05},/* x_output_size 1304 */
	{0x034D, 0x18},
	{0x034E, 0x03},/* y_output_size 980 */
	{0x034F, 0xd4},
	{0x30BF, 0xAB},/* outif_enable[7], data_type[5:0](2Bh = bayer 10bit} */
	{0x30C0, 0xA0},/* video_offset[7:4] 3260%12 */
	{0x30C8, 0x06},/* video_data_length 1600 = 1304 * 1.25 */
	{0x30C9, 0x5E},
	/* Integration setting */
	{0x0202, 0x03},		//coarse integration time
	{0x0203, 0xD4},
	{0x0204, 0x00},		//analog gain[msb] 0100 x8 0080 x4
	{0x0205, 0x80},		//analog gain[lsb] 0040 x2 0020 x1
};
/* LGE_CHANGE_S Full HD support, 2012.03.28 yt.kim@lge.com */
static struct msm_camera_i2c_reg_conf s5k4e1_video_settings[] = {
	{0x301B, 0x75 },/* CDS option setting */
	{0x30BC, 0x98 },
	/* PLL setting ... */
	/* (3) MIPI 2-lane Serial(TST = 0000b or TST = 0010b), 30 fps */
	{0x0305, 0x04},/* PLL P = 6 */
	{0x0306, 0x00},/* PLL M[8] = 0 */
	{0x0307, 0x66},
	{0x30B5, 0x01},
	{0x30E2, 0x02},
	{0x30F1, 0xA0},

	/* output size (1936 x 1096) */
	/* MIPI Size Setting ... */
	// MIPI Size Setting
	{0x30A9, 0x03},/* Horizontal Binning On */
	{0x300E, 0xE8},/* Vertical Binning On */
	{0x0387, 0x01},/* y_odd_inc */
	{0x034C, 0x07},/* x_output size */
	{0x034D, 0x90},
	{0x034E, 0x04},/* y_output size */
	{0x034F, 0x48},
	{0x0344, 0x01},/* x_addr_start 336 */
	{0x0345, 0x50},
	{0x0346, 0x01},/* y_addr_start 432 */
	{0x0347, 0xB0},
	{0x0348, 0x08},/* x_addr_end 2272 */
	{0x0349, 0xE0},
	{0x034A, 0x05},/* y_addr_end 1528 */
	{0x034B, 0xF8},
	{0x30BF, 0xAB},
	{0x30C0, 0x80},
	{0x30C8, 0x09},
	{0x30C9, 0x74},
	/* Integration setting */
	{0x0202, 0x04},	//coarse integration time
	{0x0203, 0x48},
	{0x0204, 0x00},	//analog gain[msb] 0100 x8 0080 x4
	{0x0205, 0x80},	//analog gain[lsb] 0040 x2 0020 x1
};
/* LGE_CHANGE_E Full HD support, 2012.03.28 yt.kim@lge.com */

static struct msm_camera_i2c_reg_conf s5k4e1_snap_settings[] = {
	{0x301B, 0x75},/* CDS option setting */
	{0x30BC, 0xB0},/* LGE_CHANGE Full HD support, 2012.03.28 yt.kim@lge.com */
	/* PLL setting ... */
	/* (3) MIPI 2-lane Serial(TST = 0000b or TST = 0010b), 30 fps */
	{0x0305, 0x06},/* PLL P = 6 */
	{0x0306, 0x00},/* PLL M[8] = 0 */
	{0x0307, 0x32},
	{0x30B5, 0x00},
	{0x30E2, 0x02},
	{0x30F1, 0x70},
	/* output size (2608 x 1960) */
	/* MIPI Size Setting ... */
	{0x30A9, 0x03},/*Horizontal Binning Off */
	{0x300E, 0xE8},/* Vertical Binning Off */
	{0x0387, 0x01},/* y_odd_inc */
	{0x0344, 0x00},/* x_addr_start 0 */
	{0x0345, 0x00},
	{0x0348, 0x0A},/* x_addr_end 2607 */
	{0x0349, 0x2F},
	{0x0346, 0x00},/* y_addr_start 0 */
	{0x0347, 0x00},
	{0x034A, 0x07},/* y_addr_end 1959 */
	{0x034B, 0xA7},
	{0x0380, 0x00},/* x_even_inc 1 */
	{0x0381, 0x01},
	{0x0382, 0x00},/* x_odd_inc 1 */
	{0x0383, 0x01},
	{0x0384, 0x00},/* y_even_inc 1 */
	{0x0385, 0x01},
	{0x0386, 0x00},/* y_odd_inc 3 */
	{0x0387, 0x01},
	{0x034C, 0x0A},/* x_output_size */
	{0x034D, 0x30},
	{0x034E, 0x07},/* y_output_size */
	{0x034F, 0xA8},
	{0x30BF, 0xAB},/* outif_enable[7], data_type[5:0](2Bh = bayer 10bit} */
	{0x30C0, 0x80},/* video_offset[7:4] 3260%12 */
	{0x30C8, 0x0C},/* video_data_length 3260 = 2608 * 1.25 */
	{0x30C9, 0xBC},
	/* Integration setting */
	{0x0202, 0x07},		//coarse integration time
	{0x0203, 0xA8},
	{0x0204, 0x00},		//analog gain[msb] 0100 x8 0080 x4
	{0x0205, 0x80},		//analog gain[lsb] 0040 x2 0020 x1
};

/* LGE_CHANGE_S Camera Zero shutter lag, 2012.03.12 yt.kim@lge.com */
static struct msm_camera_i2c_reg_conf s5k4e1_zsl_settings[] = {
	{0x301B, 0x75},/* CDS option setting */
	{0x30BC, 0xB0},/* LGE_CHANGE Full HD support, 2012.03.28 yt.kim@lge.com */
	// PLL setting ...
	//// input clock 24MHz
	////// (3) MIPI 2-lane Serial(TST = 0000b or TST = 0010b), 16.5 fps
	{ 0x0305, 0x06 },	//PLL P = 6
	{ 0x0306, 0x00 },	//PLL M[8] = 0
	{ 0x0307, 0x6B },	//PLL M = 107
	{ 0x30B5, 0x01 },	//PLL S = 1
	{ 0x30E2, 0x02 },	//num lanes[1:0] = 2
	{ 0x30F1, 0x70 },	//DPHY BANDCTRL 856MHz=85.6MHz /2
	/* output size (2560 x 1920) */
	/* MIPI Size Setting ... */
	{ 0x30A9, 0x03 },	//Horizontal Binning Off
	{ 0x300E, 0xE8 },	//Vertical Binning Off
	{ 0x0387, 0x01 },	//y_odd_inc
	{ 0x034C, 0x0A },	//x_output size
	{ 0x034D, 0x00 },
	{ 0x034E, 0x07 },	//y_output size
	{ 0x034F, 0x80 },
	{ 0x0344, 0x00 },	//x_addr_start
	{ 0x0345, 0x18 },
	{ 0x0346, 0x00 },	//y_addr_start
	{ 0x0347, 0x14 },
	{ 0x0348, 0x0A },	//x_addr_end
	{ 0x0349, 0x17 },
	{ 0x034A, 0x07 },	//y_addr_end
	{ 0x034B, 0x93 },
	{ 0x30BF, 0xAB },	//outif_enable[7], data_type[5:0](2Bh = bayer 10bit)
	{ 0x30C0, 0x80 },	//video_offset[7:4] 3200%12
	{ 0x30C8, 0x0C },	//video_data_length 3200 = 2560 * 1.25
	{ 0x30C9, 0x80 },
	/* Integration setting */
	{ 0x0202, 0x07 },	//coarse integration time
	{ 0x0203, 0x80 },
	{ 0x0204, 0x00 },	//analog gain[msb] 0100 x8 0080 x4
	{ 0x0205, 0x80 },	//analog gain[lsb] 0040 x2 0020 x1
};
/* LGE_CHANGE_E Camera Zero shutter lag, 2012.03.12 yt.kim@lge.com */

static struct msm_camera_i2c_reg_conf s5k4e1_recommend_settings[] = {
	/* Analog Setting */
	/* CDS timing setting ... */
	{0x3000, 0x05},
	{0x3001, 0x03},
	{0x3002, 0x08},
	{0x3003, 0x09},
	{0x3004, 0x2E},
	{0x3005, 0x06},
	{0x3006, 0x34},
	{0x3007, 0x00},
	{0x3008, 0x3C},
	{0x3009, 0x3C},
	{0x300A, 0x28},
	{0x300B, 0x04},
	{0x300C, 0x0A},
	{0x300D, 0x02},
	{0x300F, 0x82},
	/* CDS option setting ... */
	{0x3010, 0x00},
	{0x3011, 0x4C},
	{0x3012, 0x30},
	{0x3013, 0xC0},
	{0x3014, 0x00},
	{0x3015, 0x00},
	{0x3016, 0x2C},
	{0x3017, 0x94},
	{0x3018, 0x78},
	{0x301B, 0x83},
	{0x301D, 0xD4},
	{0x3021, 0x02},
	{0x3022, 0x24},
	{0x3024, 0x40},
	{0x3027, 0x08},
	{0x3029, 0xC6},
	{0x30BC, 0xB0},
	{0x302B, 0x01},
	/* Pixel option setting ... */
	{0x301C, 0x04},
	{0x30D8, 0x3F},
	/* ADLC setting ... */
	{0x3070, 0x5F},
	{0x3071, 0x00},
	{0x3080, 0x04},
	{0x3081, 0x38},
	/* Mask Corruptted Frame */
	{0x0105, 0x01},
	/* H-V flip*/
	{0x0101, 0x03},
	/* MIPI setting */
	{0x30BD, 0x00},
	{0x3084, 0x15},
	{0x30BE, 0x15},
	{0x30C1, 0x01},
	{0x30EE, 0x02},
	{0x3111, 0x86},
};


static struct v4l2_subdev_info s5k4e1_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order  = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array s5k4e1_init_conf[] = {
	{&s5k4e1_recommend_settings[0],
	ARRAY_SIZE(s5k4e1_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array s5k4e1_confs[] = {
	{&s5k4e1_snap_settings[0],
	ARRAY_SIZE(s5k4e1_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&s5k4e1_prev_settings[0],
	ARRAY_SIZE(s5k4e1_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
/* LGE_CHANGE_S Full HD support, 2012.03.28 yt.kim@lge.com */
	{&s5k4e1_video_settings[0],
	ARRAY_SIZE(s5k4e1_video_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
/* LGE_CHANGE_E Full HD support, 2012.03.28 yt.kim@lge.com */
/* LGE_CHANGE_S Camera Zero shutter lag, 2012.03.12 yt.kim@lge.com */
	{&s5k4e1_zsl_settings[0],
	ARRAY_SIZE(s5k4e1_zsl_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
/* LGE_CHANGE_E Camera Zero shutter lag, 2012.03.12 yt.kim@lge.com */
};

static struct msm_sensor_output_info_t s5k4e1_dimensions[] = {
	{
		/*snapshot */
		.x_output = 0xA30,/*2608 => 2560(48)*/
		.y_output = 0x7A8,/*1960 => 1920(40)*/
		.line_length_pclk = 0xAB2,
		.frame_length_lines = 0x7B4,
		.vt_pixel_clk = 80000000,
		.op_pixel_clk = 80000000,
		.binning_factor = 1,
	},
	{
		/* preview */
		.x_output = 0x518,/*1304, =>1280(24)=>1296(8)*/
		.y_output = 0x3D4,/*980, =>960(20)=>972(8)*/
		.line_length_pclk = 0xAB2,
		.frame_length_lines = 0x3E0,
		.vt_pixel_clk = 80000000,
		.op_pixel_clk = 80000000,
		.binning_factor = 1,
	},
/* LGE_CHANGE_S Full HD support, 2012.03.28 yt.kim@lge.com */
	{
		/* video */
		.x_output = 0x790,/*1936, =>1920(16)*/
		.y_output = 0x448,/*1096, =>1088(8)*/
		.line_length_pclk = 0xAB2,
		.frame_length_lines = 0x5C8,
		.vt_pixel_clk = 122400000,
		.op_pixel_clk = 122400000,
		.binning_factor = 1,
	},
/* LGE_CHANGE_E Full HD support, 2012.03.28 yt.kim@lge.com */
/* LGE_CHANGE_S Camera Zero shutter lag, 2012.03.12 yt.kim@lge.com */
	{
		.x_output = 0xA00,//2569  0xA30,/*2608 => 2560(48)*/
		.y_output = 0x780,//1920  0x7A8,/*1960 => 1920(40)*/
		.line_length_pclk = 0xAB2,
		.frame_length_lines = 0x78C,
		.vt_pixel_clk = 85600000,
		.op_pixel_clk = 85600000,
		.binning_factor = 1,
	},
/* LGE_CHANGE_E Camera Zero shutter lag, 2012.03.12 yt.kim@lge.com */
};

static struct msm_camera_csid_vc_cfg s5k4e1_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params s5k4e1_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = ARRAY_SIZE(s5k4e1_cid_cfg),
			.vc_cfg = s5k4e1_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 0x12,
	},
};

static struct msm_camera_csi2_params *s5k4e1_csi_params_array[] = {
	&s5k4e1_csi_params,
	&s5k4e1_csi_params,
	&s5k4e1_csi_params,/* LGE_CHANGE Full HD support, 2012.03.28 yt.kim@lge.com */
	&s5k4e1_csi_params,/* LGE_CHANGE_S Camera Zero shutter lag, 2012.03.12 yt.kim@lge.com */
};

static struct msm_sensor_output_reg_addr_t s5k4e1_reg_addr = {
	.x_output = 0x034C,
	.y_output = 0x034E,
	.line_length_pclk = 0x0342,
	.frame_length_lines = 0x0340,
};

static struct msm_sensor_id_info_t s5k4e1_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x4E10,
};

static struct msm_sensor_exp_gain_info_t s5k4e1_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x204,
	.vert_offset = 8,
};

int32_t s5k4e1_sensor_set_fps(struct msm_sensor_ctrl_t *s_ctrl, struct fps_cfg *fps)
{
	uint16_t total_lines_per_frame;
	int32_t rc = 0;
	s_ctrl->fps_divider = fps->fps_div;
	if (s_ctrl->curr_res != MSM_SENSOR_INVALID_RES) {
		uint16_t fl_read = 0;
		total_lines_per_frame = (uint16_t)
		((s_ctrl->curr_frame_length_lines) * s_ctrl->fps_divider/Q10);
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines,
		&fl_read, MSM_CAMERA_I2C_WORD_DATA);  

		CDBG("%s: before_fl = %d, new_fl = %d", __func__, fl_read, total_lines_per_frame);
		if(fl_read < total_lines_per_frame) {
			pr_err("%s: Write new_fl (before_fl = %d, new_fl = %d)", __func__, fl_read, total_lines_per_frame);
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			total_lines_per_frame, MSM_CAMERA_I2C_WORD_DATA);
		}
	}
	return rc;
}

static int32_t s5k4e1_write_exp_gain1(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{

	int32_t rc = 0;
	uint16_t max_legal_gain = 0x0200;
	static uint32_t fl_lines;
	uint16_t curr_fl_lines;
	uint8_t offset;

	curr_fl_lines = s_ctrl->curr_frame_length_lines;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;

	CDBG("#QCT s5k4e1_write_exp_gain1 = %d, %d \n", gain, line);

	if (gain > max_legal_gain) {
		pr_debug("Max legal gain Line:%d\n", __LINE__);
		gain = max_legal_gain;
	}
	/* Analogue Gain */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
			MSM_CAMERA_I2C_WORD_DATA);

	if (line > (curr_fl_lines - offset)) {
		fl_lines = line+offset;
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);

		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);
		CDBG("#QCT 11 s5k4e1_write_exp_gain1 = %d, %d \n", line, fl_lines);

		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	} else if (line < (fl_lines - offset)) {
		fl_lines = line+offset;
		if (fl_lines < curr_fl_lines)
			fl_lines = curr_fl_lines;

		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);

		CDBG("#QCT 22 s5k4e1_write_exp_gain1 = %d, %d \n", line, fl_lines);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	} else {
		fl_lines = line+offset;
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);
		CDBG("#QCT 33 s5k4e1_write_exp_gain1 = %d, %d \n", line, 0);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	}

	return rc;
}

static int32_t s5k4e1_write_exp_gain2(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint32_t fl_lines;
	int32_t rc = 0;
	uint16_t max_legal_gain = 0x0200;
	uint8_t offset;


	pr_debug("s5k4e1_write_exp_gain2 : gain = %d line = %d\n", gain, line);
	CDBG("#QCT s5k4e1_write_exp_gain2 = %d, %d \n", gain, line);

	if (gain > max_legal_gain) {
			pr_debug("Max legal gain Line:%d\n", __LINE__);
			gain = max_legal_gain;
	}


	fl_lines = s_ctrl->curr_frame_length_lines;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;

	if (fl_lines - offset < line)
		fl_lines = line + offset;

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
			MSM_CAMERA_I2C_WORD_DATA);

	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);

	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

	msleep(130);/* need to revisit */

	CDBG("#QCT s5k4e1_write_exp_gain2 after: gain,line,fl_lines, s_ctrl->curr_frame_length_lines"
		"0x%x, 0x%x, 0x%x, 0x%x ",gain,line,fl_lines, fl_lines);
	CDBG("#QCT s5k4e1_write_exp_gain2 after: gain,line,fl_lines, s_ctrl->curr_frame_length_lines"
		"%d, %d, %d, %d ",gain,line,fl_lines, fl_lines);

	return rc;


}

static const struct i2c_device_id s5k4e1_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&s5k4e1_s_ctrl},
	{ }
};

static struct i2c_driver s5k4e1_i2c_driver = {
	.id_table = s5k4e1_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k4e1_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	printk("msm_sensor_init_module s5k4e1 \n");
	return i2c_add_driver(&s5k4e1_i2c_driver);
}

static struct v4l2_subdev_core_ops s5k4e1_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};
static struct v4l2_subdev_video_ops s5k4e1_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops s5k4e1_subdev_ops = {
	.core = &s5k4e1_subdev_core_ops,
	.video  = &s5k4e1_subdev_video_ops,
};

static struct msm_sensor_fn_t s5k4e1_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = s5k4e1_sensor_set_fps,
	.sensor_write_exp_gain = s5k4e1_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = s5k4e1_write_exp_gain2,
	.sensor_csi_setting = msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
};

static struct msm_sensor_reg_t s5k4e1_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = s5k4e1_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(s5k4e1_start_settings),
	.stop_stream_conf = s5k4e1_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(s5k4e1_stop_settings),
	.group_hold_on_conf = s5k4e1_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(s5k4e1_groupon_settings),
	.group_hold_off_conf = s5k4e1_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(s5k4e1_groupoff_settings),
	.init_settings = &s5k4e1_init_conf[0],
	.init_size = ARRAY_SIZE(s5k4e1_init_conf),
	.mode_settings = &s5k4e1_confs[0],
	.output_settings = &s5k4e1_dimensions[0],
	.num_conf = ARRAY_SIZE(s5k4e1_confs),
};

static struct msm_sensor_ctrl_t s5k4e1_s_ctrl = {
	.msm_sensor_reg = &s5k4e1_regs,
	.sensor_i2c_client = &s5k4e1_sensor_i2c_client,
	.sensor_i2c_addr = 0x20,
	.sensor_output_reg_addr = &s5k4e1_reg_addr,
	.sensor_id_info = &s5k4e1_id_info,
	.sensor_exp_gain_info = &s5k4e1_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csi_params = &s5k4e1_csi_params_array[0],
	.msm_sensor_mutex = &s5k4e1_mut,
	.sensor_i2c_driver = &s5k4e1_i2c_driver,
	.sensor_v4l2_subdev_info = s5k4e1_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k4e1_subdev_info),
	.sensor_v4l2_subdev_ops = &s5k4e1_subdev_ops,
	.func_tbl = &s5k4e1_func_tbl,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Samsung 5 MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");

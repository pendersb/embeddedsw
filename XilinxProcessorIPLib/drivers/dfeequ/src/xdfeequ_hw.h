/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ_reg.h
* @addtogroup xdfeequ_v1_0
* @{
*
* Contains the register definitions for dfeequ. This is
* created to be used initialy while waiting for IP.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     09/03/20 Initial version
*       dc     02/22/21 align driver to current specification
*       dc     03/15/21 Add data latency api
* </pre>
*
******************************************************************************/
#ifndef XDFEEQU_HW_H_
#define XDFEEQU_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************** Definitions *******************************/
/* CONFIG */
#define XDFEEQU_VERSION_OFFSET 0x00U /* Register offset */
#define XDFEEQU_VERSION_PATCH_WIDTH 8U
#define XDFEEQU_VERSION_PATCH_OFFSET 0U
#define XDFEEQU_VERSION_REVISION_WIDTH 8U
#define XDFEEQU_VERSION_REVISION_OFFSET 8U
#define XDFEEQU_VERSION_MINOR_WIDTH 8U
#define XDFEEQU_VERSION_MINOR_OFFSET 16U
#define XDFEEQU_VERSION_MAJOR_WIDTH 8U
#define XDFEEQU_VERSION_MAJOR_OFFSET 24U

#define XDFEEQU_RESET_OFFSET 0x04U /* Register offset */
#define XDFEEQU_RESET_OFF 0x00U
#define XDFEEQU_RESET_ON 0x01U
#define XDFEEQU_MODEL_PARAM_OFFSET 0x08U /**< Register offset */
#define XDFEEQU_MODEL_PARAM_NUM_CHANNELS_WIDTH 4U
#define XDFEEQU_MODEL_PARAM_NUM_CHANNELS_OFFSET 0U
#define XDFEEQU_MODEL_PARAM_SAMPLE_WIDTH_WIDTH 5U
#define XDFEEQU_MODEL_PARAM_SAMPLE_WIDTH_OFFSET 4U
#define XDFEEQU_MODEL_PARAM_COMPLEX_MODE_WIDTH 1U
#define XDFEEQU_MODEL_PARAM_COMPLEX_MODE_OFFSET 16U
#define XDFEEQU_MODEL_PARAM_TUSER_WIDTH_WIDTH 7U
#define XDFEEQU_MODEL_PARAM_TUSER_WIDTH_OFFSET 24U

#define XDFEEQU_DELAY 0x0CU /**< Register offset */
#define XDFEEQU_DELAY_WIDTH 6U
#define XDFEEQU_DELAY_OFFSET 0U
#define XDFEEQU_DATA_LATENCY 0x10U /**< Register offset */
#define XDFEEQU_DATA_LATENCY_WIDTH 4U
#define XDFEEQU_DATA_LATENCY_OFFSET 0U

#define XDFEEQU_CURRENT_CONTROL_OFFSET 0x20U /* Register offset */
#define XDFEEQU_NEXT_CONTROL_OFFSET 0x24U /* Register offset */
#define XDFEEQU_POWERDOWN_MODE_WIDTH 1U
#define XDFEEQU_POWERDOWN_MODE_OFFSET 0U
#define XDFEEQU_POWERDOWN_MODE_POWERDOWN 0U
#define XDFEEQU_POWERDOWN_MODE_POWERUP 1U
#define XDFEEQU_FLUSH_BUFFERS_WIDTH 1U
#define XDFEEQU_FLUSH_BUFFERS_OFFSET 1U
#define XDFEEQU_COMPLEX_MODE_WIDTH 1U
#define XDFEEQU_COMPLEX_MODE_OFFSET 2U
#define XDFEEQU_REAL_MODE 0U
#define XDFEEQU_COMPLEX_MODE 1U
#define XDFEEQU_COEFF_SET_CONTROL_WIDTH 4U
#define XDFEEQU_COEFF_SET_CONTROL_OFFSET 4U
#define XDFEEQU_COEFF_SET_CONTROL_MASK 3U
#define XDFEEQU_COEFF_SET_CONTROL_IM_OFFSET 2U

/* Triggers */
#define XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET 0x28U /* Register offset */
#define XDFEEQU_DYNAMIC_POWER_DOWN_MODE_TRIGGER_OFFSET                         \
	0x34U /* Register offset */
/* Bit fields */
#define XDFEEQU_TRIGGERS_ENABLE_WIDTH 1U
#define XDFEEQU_TRIGGERS_ENABLE_OFFSET 0U
#define XDFEEQU_TRIGGERS_ENABLE_DISABLED 0U
#define XDFEEQU_TRIGGERS_ENABLE_ENABLED 1U
#define XDFEEQU_TRIGGERS_SOURCE_WIDTH 2U
#define XDFEEQU_TRIGGERS_SOURCE_OFFSET 4U
#define XDFEEQU_TRIGGERS_SOURCE_REGISTER 0U
#define XDFEEQU_TRIGGERS_SOURCE_TUSER 1U
#define XDFEEQU_TRIGGERS_SOURCE_TLAST 2U
#define XDFEEQU_TRIGGERS_SOURCE_RESERVED 3U
#define XDFEEQU_TRIGGERS_SIGNAL_EDGE_WIDTH 2U
#define XDFEEQU_TRIGGERS_SIGNAL_EDGE_OFFSET 8U
#define XDFEEQU_TRIGGERS_SIGNAL_EDGE_RISING 0U
#define XDFEEQU_TRIGGERS_SIGNAL_EDGE_FALING 1U
#define XDFEEQU_TRIGGERS_SIGNAL_EDGE_BOTH 2U
#define XDFEEQU_TRIGGERS_SIGNAL_EDGE_ACTIVE_HIGH 3U
#define XDFEEQU_TRIGGERS_ONE_SHOT_WIDTH 1U
#define XDFEEQU_TRIGGERS_ONE_SHOT_OFFSET 12U
#define XDFEEQU_TRIGGERS_ONE_SHOT_CONTINUOUS 0U
#define XDFEEQU_TRIGGERS_ONE_SHOT_ONESHOT 1U
#define XDFEEQU_TRIGGERS_TUSER_BIT_WIDTH 8U
#define XDFEEQU_TRIGGERS_TUSER_BIT_OFFSET 16U

#define XDFEEQU_CURRENT_DYNAMIC_POWERDOWN_OFFSET 0x2CU /* Register offset */
#define XDFEEQU_NEXT_DYNAMIC_POWERDOWN_OFFSET 0x30U /* Register offset */
#define XDFEEQU_DYNAMIC_POWERDOWN_MODE_WIDTH 1U
#define XDFEEQU_DYNAMIC_POWERDOWN_MODE_OFFSET 0U
#define XDFEEQU_DYNAMIC_POWERDOWN_MODE_ENABLED 0U
#define XDFEEQU_DYNAMIC_POWERDOWN_MODE_DISABLED 1U

/* Status */
#define XDFEEQU_CHANNEL_0_STATUS_OFFSET 0x40U /* Register offset */
#define XDFEEQU_CHANNEL_0_STATUS_MASK_OFFSET 0x44U /* Register offset */
#define XDFEEQU_CHANNEL_1_STATUS_OFFSET 0x50U /* Register offset */
#define XDFEEQU_CHANNEL_1_STATUS_MASK_OFFSET 0x54U /* Register offset */
#define XDFEEQU_CHANNEL_2_STATUS_OFFSET 0x60U /* Register offset */
#define XDFEEQU_CHANNEL_2_STATUS_MASK_OFFSET 0x64U /* Register offset */
#define XDFEEQU_CHANNEL_3_STATUS_OFFSET 0x70U /* Register offset */
#define XDFEEQU_CHANNEL_3_STATUS_MASK_OFFSET 0x74U /* Register offset */
#define XDFEEQU_CHANNEL_4_STATUS_OFFSET 0x80U /* Register offset */
#define XDFEEQU_CHANNEL_4_STATUS_MASK_OFFSET 0x84U /* Register offset */
#define XDFEEQU_CHANNEL_5_STATUS_OFFSET 0x90U /* Register offset */
#define XDFEEQU_CHANNEL_5_STATUS_MASK_OFFSET 0x94U /* Register offset */
#define XDFEEQU_CHANNEL_6_STATUS_OFFSET 0xA0U /* Register offset */
#define XDFEEQU_CHANNEL_6_STATUS_MASK_OFFSET 0xA4U /* Register offset */
#define XDFEEQU_CHANNEL_7_STATUS_OFFSET 0xB0U /* Register offset */
#define XDFEEQU_CHANNEL_7_STATUS_MASK_OFFSET 0xB4U /* Register offset */
#define XDFEEQU_CHANNEL_STATUS_OFFSET 0x10U
#define XDFEEQU_CHANNEL_STATUS_STATUS_WIDTH 2U
#define XDFEEQU_CHANNEL_STATUS_STATUS_OFFSET 0U
#define XDFEEQU_CHANNEL_I_STATUS_MASK 1U
#define XDFEEQU_CHANNEL_Q_STATUS_MASK 2U
#define XDFEEQU_CHANNEL_Q_STATUS_OFFSET 1U
#define XDFEEQU_CHANNEL_STATUS_MASK_ENABLE_WIDTH 2U
#define XDFEEQU_CHANNEL_STATUS_MASK_ENABLE_OFFSET 0U
/* Coefficients */
#define XDFEEQU_SET_TO_WRITE_OFFSET 0x0100U /* Register offset */
#define XDFEEQU_SET_TO_WRITE_SET_WIDTH 2U
#define XDFEEQU_SET_TO_WRITE_SET_OFFSET 0U
#define XDFEEQU_NUMBER_OF_UNITS_OFFSET 0x0104U /* Register offset */
#define XDFEEQU_NUMBER_OF_UNITS_SET_WIDTH 3U
#define XDFEEQU_NUMBER_OF_UNITS_SET_OFFSET 0U
#define XDFEEQU_SHIFT_VALUE_OFFSET 0x0108U /* Register offset */
#define XDFEEQU_SHIFT_VALUE_SET_WIDTH 3U
#define XDFEEQU_SHIFT_VALUE_SET_OFFSET 0U
#define XDFEEQU_CHANNEL_FIELD_OFFSET 0x010CU /* Register offset */
#define XDFEEQU_CHANNEL_FIELD_FIELD_WIDTH 8U
#define XDFEEQU_CHANNEL_FIELD_FIELD_OFFSET 0U
#define XDFEEQU_CHANNEL_FIELD_DONE_WIDTH 1U
#define XDFEEQU_CHANNEL_FIELD_DONE_OFFSET 8U
#define XDFEEQU_CHANNEL_FIELD_DONE_LOADING 0U
#define XDFEEQU_CHANNEL_FIELD_DONE_LOADING_DONE 1U

/* Coefficient sets */
#define XDFEEQU_COEFFICIENT_SET 0x0200U /* Register offset */
#define XDFEEQU_COEFFICIENT_SET_VALUE_WIDTH 16U
#define XDFEEQU_COEFFICIENT_SET_VALUE_OFFSET 0U
#define XDFEEQU_COEFFICIENT_SET_MAX 24U
#define XDFEEQU_RE_COEFFICIENT_SET_OFFSET 0U
#define XDFEEQU_IM_COEFFICIENT_SET_OFFSET 12U
#define XDFEEQU_TAP_UNIT_SIZE 4U

#ifdef __cplusplus
}
#endif

#endif
/** @} */

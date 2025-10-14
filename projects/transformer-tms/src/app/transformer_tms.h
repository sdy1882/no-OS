/***************************************************************************//**
 *   @file   transformer_tms.h
 *   @brief  Header file for Transformer Monitoring System (TMS)
 *   @author Analog Devices, Inc.
********************************************************************************
 * Copyright 2024(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#ifndef __TRANSFORMER_TMS_H__
#define __TRANSFORMER_TMS_H__

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "ade9430.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/* Transformer Monitoring Thresholds */
#define TMS_TEMP_WARNING_THRESHOLD      80      /* Temperature warning in °C */
#define TMS_TEMP_CRITICAL_THRESHOLD     100     /* Temperature critical in °C */
#define TMS_OVERLOAD_THRESHOLD          120     /* Overload threshold in % */
#define TMS_VOLTAGE_IMBALANCE_THRESHOLD 2       /* Voltage imbalance in % */
#define TMS_CURRENT_IMBALANCE_THRESHOLD 10      /* Current imbalance in % */
#define TMS_MIN_POWER_FACTOR            0.85    /* Minimum acceptable power factor */

/* Transformer Status Flags */
#define TMS_STATUS_NORMAL               0x00
#define TMS_STATUS_TEMP_WARNING         0x01
#define TMS_STATUS_TEMP_CRITICAL        0x02
#define TMS_STATUS_OVERLOAD             0x04
#define TMS_STATUS_VOLTAGE_IMBALANCE    0x08
#define TMS_STATUS_CURRENT_IMBALANCE    0x10
#define TMS_STATUS_LOW_POWER_FACTOR     0x20
#define TMS_STATUS_PHASE_LOSS           0x40

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/**
 * @struct tms_phase_data
 * @brief Structure to hold single phase measurements
 */
struct tms_phase_data {
	uint32_t voltage_rms;           /* RMS voltage in mV */
	uint32_t current_rms;           /* RMS current in mA */
	int32_t active_power;           /* Active power in mW */
	int32_t reactive_power;         /* Reactive power in mVAR */
	uint32_t apparent_power;        /* Apparent power in mVA */
	int16_t power_factor;           /* Power factor * 1000 */
	uint32_t energy_accumulated;    /* Accumulated energy in Wh */
};

/**
 * @struct tms_transformer_data
 * @brief Structure to hold complete transformer monitoring data
 */
struct tms_transformer_data {
	struct tms_phase_data phase_a;
	struct tms_phase_data phase_b;
	struct tms_phase_data phase_c;
	int32_t temperature;            /* Temperature in °C */
	uint32_t status_flags;          /* Status and alarm flags */
	uint32_t total_active_power;    /* Total 3-phase active power in mW */
	uint32_t total_apparent_power;  /* Total 3-phase apparent power in mVA */
	uint32_t neutral_current;       /* Neutral current in mA */
	uint16_t voltage_imbalance;     /* Voltage imbalance percentage * 10 */
	uint16_t current_imbalance;     /* Current imbalance percentage * 10 */
	uint32_t load_percentage;       /* Load as % of rated capacity * 10 */
};

/**
 * @struct tms_config
 * @brief Transformer monitoring system configuration
 */
struct tms_config {
	uint32_t rated_power;           /* Rated transformer power in VA */
	uint32_t rated_voltage;         /* Rated voltage in V */
	uint32_t rated_current;         /* Rated current in A */
	uint16_t monitoring_interval_ms; /* Monitoring interval in milliseconds */
	bool enable_cloud_logging;      /* Enable cloud data logging */
	bool enable_lcd_display;        /* Enable LCD display */
};

/**
 * @struct tms_dev
 * @brief Transformer monitoring system device structure
 */
struct tms_dev {
	struct ade9430_dev *ade9430;
	struct tms_config config;
	struct tms_transformer_data data;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/**
 * @brief Initialize transformer monitoring system
 * @param device - Pointer to TMS device structure
 * @param ade9430_dev - Pointer to initialized ADE9430 device
 * @param config - Pointer to TMS configuration
 * @return 0 in case of success, negative error code otherwise
 */
int tms_init(struct tms_dev **device, struct ade9430_dev *ade9430_dev,
	     struct tms_config *config);

/**
 * @brief Read all transformer parameters
 * @param dev - Pointer to TMS device structure
 * @return 0 in case of success, negative error code otherwise
 */
int tms_read_all_parameters(struct tms_dev *dev);

/**
 * @brief Check transformer status and update alarm flags
 * @param dev - Pointer to TMS device structure
 * @return 0 in case of success, negative error code otherwise
 */
int tms_check_status(struct tms_dev *dev);

/**
 * @brief Calculate voltage imbalance across three phases
 * @param dev - Pointer to TMS device structure
 * @return Voltage imbalance percentage * 10
 */
uint16_t tms_calculate_voltage_imbalance(struct tms_dev *dev);

/**
 * @brief Calculate current imbalance across three phases
 * @param dev - Pointer to TMS device structure
 * @return Current imbalance percentage * 10
 */
uint16_t tms_calculate_current_imbalance(struct tms_dev *dev);

/**
 * @brief Calculate transformer load percentage
 * @param dev - Pointer to TMS device structure
 * @return Load percentage * 10
 */
uint32_t tms_calculate_load_percentage(struct tms_dev *dev);

/**
 * @brief Format transformer data for display or transmission
 * @param dev - Pointer to TMS device structure
 * @param buffer - Output buffer for formatted string
 * @param buffer_size - Size of output buffer
 * @return Length of formatted string
 */
int tms_format_data(struct tms_dev *dev, char *buffer, uint32_t buffer_size);

/**
 * @brief Remove transformer monitoring system
 * @param dev - Pointer to TMS device structure
 * @return 0 in case of success, negative error code otherwise
 */
int tms_remove(struct tms_dev *dev);

#endif /* __TRANSFORMER_TMS_H__ */

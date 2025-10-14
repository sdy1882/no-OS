/***************************************************************************//**
 *   @file   transformer_tms.c
 *   @brief  Implementation of Transformer Monitoring System (TMS)
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "transformer_tms.h"
#include "no_os_alloc.h"
#include "no_os_error.h"
#include "no_os_print_log.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Read single phase data from ADE9430
 */
static int tms_read_phase_data(struct tms_dev *dev, enum ade9430_phase phase,
			       struct tms_phase_data *phase_data)
{
	int ret;
	uint32_t reg_val;
	uint16_t vrms_reg, irms_reg, watt_reg, var_reg, va_reg, pf_reg;

	/* Select register addresses based on phase */
	switch (phase) {
	case ADE9430_PHASE_A:
		vrms_reg = ADE9430_REG_AVRMS;
		irms_reg = ADE9430_REG_AIRMS;
		watt_reg = ADE9430_REG_AWATT;
		var_reg = ADE9430_REG_AVAR;
		va_reg = ADE9430_REG_AVA;
		pf_reg = ADE9430_REG_APF;
		break;
	case ADE9430_PHASE_B:
		vrms_reg = ADE9430_REG_BVRMS;
		irms_reg = ADE9430_REG_BIRMS;
		watt_reg = ADE9430_REG_BWATT;
		var_reg = ADE9430_REG_BVAR;
		va_reg = ADE9430_REG_BVA;
		pf_reg = ADE9430_REG_BPF;
		break;
	case ADE9430_PHASE_C:
		vrms_reg = ADE9430_REG_CVRMS;
		irms_reg = ADE9430_REG_CIRMS;
		watt_reg = ADE9430_REG_CWATT;
		var_reg = ADE9430_REG_CVAR;
		va_reg = ADE9430_REG_CVA;
		pf_reg = ADE9430_REG_CPF;
		break;
	default:
		return -EINVAL;
	}

	/* Read VRMS */
	ret = ade9430_read(dev->ade9430, vrms_reg, &reg_val);
	if (ret)
		return ret;
	phase_data->voltage_rms = reg_val;

	/* Read IRMS */
	ret = ade9430_read(dev->ade9430, irms_reg, &reg_val);
	if (ret)
		return ret;
	phase_data->current_rms = reg_val;

	/* Read Active Power (WATT) */
	ret = ade9430_read(dev->ade9430, watt_reg, &reg_val);
	if (ret)
		return ret;
	phase_data->active_power = (int32_t)reg_val;

	/* Read Reactive Power (VAR) */
	ret = ade9430_read(dev->ade9430, var_reg, &reg_val);
	if (ret)
		return ret;
	phase_data->reactive_power = (int32_t)reg_val;

	/* Read Apparent Power (VA) */
	ret = ade9430_read(dev->ade9430, va_reg, &reg_val);
	if (ret)
		return ret;
	phase_data->apparent_power = reg_val;

	/* Read Power Factor */
	ret = ade9430_read(dev->ade9430, pf_reg, &reg_val);
	if (ret)
		return ret;
	phase_data->power_factor = (int16_t)(reg_val & 0xFFFF);

	return 0;
}

/**
 * @brief Initialize transformer monitoring system
 */
int tms_init(struct tms_dev **device, struct ade9430_dev *ade9430_dev,
	     struct tms_config *config)
{
	struct tms_dev *dev;

	if (!ade9430_dev || !config)
		return -EINVAL;

	dev = (struct tms_dev *)no_os_calloc(1, sizeof(*dev));
	if (!dev)
		return -ENOMEM;

	dev->ade9430 = ade9430_dev;
	memcpy(&dev->config, config, sizeof(struct tms_config));
	dev->data.status_flags = TMS_STATUS_NORMAL;

	*device = dev;

	pr_info("Transformer Monitoring System initialized\n");
	pr_info("Rated Power: %u VA, Rated Voltage: %u V, Rated Current: %u A\n",
		config->rated_power, config->rated_voltage, config->rated_current);

	return 0;
}

/**
 * @brief Read all transformer parameters
 */
int tms_read_all_parameters(struct tms_dev *dev)
{
	int ret;
	uint32_t reg_val;

	if (!dev)
		return -EINVAL;

	/* Read Phase A data */
	ret = tms_read_phase_data(dev, ADE9430_PHASE_A, &dev->data.phase_a);
	if (ret) {
		pr_err("Failed to read Phase A data\n");
		return ret;
	}

	/* Read Phase B data */
	ret = tms_read_phase_data(dev, ADE9430_PHASE_B, &dev->data.phase_b);
	if (ret) {
		pr_err("Failed to read Phase B data\n");
		return ret;
	}

	/* Read Phase C data */
	ret = tms_read_phase_data(dev, ADE9430_PHASE_C, &dev->data.phase_c);
	if (ret) {
		pr_err("Failed to read Phase C data\n");
		return ret;
	}

	/* Read Temperature */
	ret = ade9430_read_temp(dev->ade9430);
	if (ret) {
		pr_err("Failed to read temperature\n");
		return ret;
	}
	dev->data.temperature = dev->ade9430->temp_deg;

	/* Read Neutral Current */
	ret = ade9430_read(dev->ade9430, ADE9430_REG_NIRMS, &reg_val);
	if (ret)
		return ret;
	dev->data.neutral_current = reg_val;

	/* Calculate total power */
	dev->data.total_active_power = dev->data.phase_a.active_power +
				       dev->data.phase_b.active_power +
				       dev->data.phase_c.active_power;

	dev->data.total_apparent_power = dev->data.phase_a.apparent_power +
					 dev->data.phase_b.apparent_power +
					 dev->data.phase_c.apparent_power;

	/* Calculate imbalances */
	dev->data.voltage_imbalance = tms_calculate_voltage_imbalance(dev);
	dev->data.current_imbalance = tms_calculate_current_imbalance(dev);

	/* Calculate load percentage */
	dev->data.load_percentage = tms_calculate_load_percentage(dev);

	return 0;
}

/**
 * @brief Calculate voltage imbalance
 */
uint16_t tms_calculate_voltage_imbalance(struct tms_dev *dev)
{
	uint32_t v_avg, v_max_dev;
	uint32_t va, vb, vc;

	va = dev->data.phase_a.voltage_rms;
	vb = dev->data.phase_b.voltage_rms;
	vc = dev->data.phase_c.voltage_rms;

	/* Calculate average voltage */
	v_avg = (va + vb + vc) / 3;

	if (v_avg == 0)
		return 0;

	/* Find maximum deviation */
	v_max_dev = 0;
	if ((va > v_avg ? va - v_avg : v_avg - va) > v_max_dev)
		v_max_dev = (va > v_avg ? va - v_avg : v_avg - va);
	if ((vb > v_avg ? vb - v_avg : v_avg - vb) > v_max_dev)
		v_max_dev = (vb > v_avg ? vb - v_avg : v_avg - vb);
	if ((vc > v_avg ? vc - v_avg : v_avg - vc) > v_max_dev)
		v_max_dev = (vc > v_avg ? vc - v_avg : v_avg - vc);

	/* Return percentage * 10 */
	return (uint16_t)((v_max_dev * 1000) / v_avg);
}

/**
 * @brief Calculate current imbalance
 */
uint16_t tms_calculate_current_imbalance(struct tms_dev *dev)
{
	uint32_t i_avg, i_max_dev;
	uint32_t ia, ib, ic;

	ia = dev->data.phase_a.current_rms;
	ib = dev->data.phase_b.current_rms;
	ic = dev->data.phase_c.current_rms;

	/* Calculate average current */
	i_avg = (ia + ib + ic) / 3;

	if (i_avg == 0)
		return 0;

	/* Find maximum deviation */
	i_max_dev = 0;
	if ((ia > i_avg ? ia - i_avg : i_avg - ia) > i_max_dev)
		i_max_dev = (ia > i_avg ? ia - i_avg : i_avg - ia);
	if ((ib > i_avg ? ib - i_avg : i_avg - ib) > i_max_dev)
		i_max_dev = (ib > i_avg ? ib - i_avg : i_avg - ib);
	if ((ic > i_avg ? ic - i_avg : i_avg - ic) > i_max_dev)
		i_max_dev = (ic > i_avg ? ic - i_avg : i_avg - ic);

	/* Return percentage * 10 */
	return (uint16_t)((i_max_dev * 1000) / i_avg);
}

/**
 * @brief Calculate transformer load percentage
 */
uint32_t tms_calculate_load_percentage(struct tms_dev *dev)
{
	uint32_t load_pct;

	if (dev->config.rated_power == 0)
		return 0;

	/* Calculate load as percentage of rated power * 10 */
	load_pct = (dev->data.total_apparent_power * 1000) / dev->config.rated_power;

	return load_pct;
}

/**
 * @brief Check transformer status and update alarm flags
 */
int tms_check_status(struct tms_dev *dev)
{
	if (!dev)
		return -EINVAL;

	/* Clear previous status flags */
	dev->data.status_flags = TMS_STATUS_NORMAL;

	/* Check temperature */
	if (dev->data.temperature >= TMS_TEMP_CRITICAL_THRESHOLD) {
		dev->data.status_flags |= TMS_STATUS_TEMP_CRITICAL;
		pr_warning("CRITICAL: Transformer temperature at %d°C\n",
			   dev->data.temperature);
	} else if (dev->data.temperature >= TMS_TEMP_WARNING_THRESHOLD) {
		dev->data.status_flags |= TMS_STATUS_TEMP_WARNING;
		pr_warning("WARNING: Transformer temperature at %d°C\n",
			   dev->data.temperature);
	}

	/* Check overload */
	if (dev->data.load_percentage > (TMS_OVERLOAD_THRESHOLD * 10)) {
		dev->data.status_flags |= TMS_STATUS_OVERLOAD;
		pr_warning("WARNING: Transformer overload at %u.%u%%\n",
			   dev->data.load_percentage / 10,
			   dev->data.load_percentage % 10);
	}

	/* Check voltage imbalance */
	if (dev->data.voltage_imbalance > (TMS_VOLTAGE_IMBALANCE_THRESHOLD * 10)) {
		dev->data.status_flags |= TMS_STATUS_VOLTAGE_IMBALANCE;
		pr_warning("WARNING: Voltage imbalance at %u.%u%%\n",
			   dev->data.voltage_imbalance / 10,
			   dev->data.voltage_imbalance % 10);
	}

	/* Check current imbalance */
	if (dev->data.current_imbalance > (TMS_CURRENT_IMBALANCE_THRESHOLD * 10)) {
		dev->data.status_flags |= TMS_STATUS_CURRENT_IMBALANCE;
		pr_warning("WARNING: Current imbalance at %u.%u%%\n",
			   dev->data.current_imbalance / 10,
			   dev->data.current_imbalance % 10);
	}

	/* Check for phase loss (very low voltage or current) */
	if (dev->data.phase_a.voltage_rms < 1000 ||
	    dev->data.phase_b.voltage_rms < 1000 ||
	    dev->data.phase_c.voltage_rms < 1000) {
		dev->data.status_flags |= TMS_STATUS_PHASE_LOSS;
		pr_warning("WARNING: Possible phase loss detected\n");
	}

	/* Check power factor */
	int16_t avg_pf = (dev->data.phase_a.power_factor +
			  dev->data.phase_b.power_factor +
			  dev->data.phase_c.power_factor) / 3;
	if (avg_pf < (TMS_MIN_POWER_FACTOR * 1000)) {
		dev->data.status_flags |= TMS_STATUS_LOW_POWER_FACTOR;
		pr_warning("WARNING: Low power factor detected\n");
	}

	return 0;
}

/**
 * @brief Format transformer data for display or transmission
 */
int tms_format_data(struct tms_dev *dev, char *buffer, uint32_t buffer_size)
{
	int len = 0;

	if (!dev || !buffer)
		return -EINVAL;

	len = snprintf(buffer, buffer_size,
		       "=== Transformer Monitoring System ===\n"
		       "Temperature: %d°C\n"
		       "Load: %u.%u%% of rated\n"
		       "Total Power: %d W (3-phase)\n"
		       "Voltage Imbalance: %u.%u%%\n"
		       "Current Imbalance: %u.%u%%\n"
		       "\nPhase A:\n"
		       "  V: %u mV, I: %u mA\n"
		       "  P: %d mW, Q: %d mVAR, S: %u mVA\n"
		       "  PF: %d.%03d\n"
		       "\nPhase B:\n"
		       "  V: %u mV, I: %u mA\n"
		       "  P: %d mW, Q: %d mVAR, S: %u mVA\n"
		       "  PF: %d.%03d\n"
		       "\nPhase C:\n"
		       "  V: %u mV, I: %u mA\n"
		       "  P: %d mW, Q: %d mVAR, S: %u mVA\n"
		       "  PF: %d.%03d\n"
		       "\nNeutral Current: %u mA\n"
		       "Status: 0x%02X\n",
		       dev->data.temperature,
		       dev->data.load_percentage / 10,
		       dev->data.load_percentage % 10,
		       dev->data.total_active_power,
		       dev->data.voltage_imbalance / 10,
		       dev->data.voltage_imbalance % 10,
		       dev->data.current_imbalance / 10,
		       dev->data.current_imbalance % 10,
		       /* Phase A */
		       dev->data.phase_a.voltage_rms,
		       dev->data.phase_a.current_rms,
		       dev->data.phase_a.active_power,
		       dev->data.phase_a.reactive_power,
		       dev->data.phase_a.apparent_power,
		       dev->data.phase_a.power_factor / 1000,
		       (dev->data.phase_a.power_factor % 1000),
		       /* Phase B */
		       dev->data.phase_b.voltage_rms,
		       dev->data.phase_b.current_rms,
		       dev->data.phase_b.active_power,
		       dev->data.phase_b.reactive_power,
		       dev->data.phase_b.apparent_power,
		       dev->data.phase_b.power_factor / 1000,
		       (dev->data.phase_b.power_factor % 1000),
		       /* Phase C */
		       dev->data.phase_c.voltage_rms,
		       dev->data.phase_c.current_rms,
		       dev->data.phase_c.active_power,
		       dev->data.phase_c.reactive_power,
		       dev->data.phase_c.apparent_power,
		       dev->data.phase_c.power_factor / 1000,
		       (dev->data.phase_c.power_factor % 1000),
		       /* Neutral and status */
		       dev->data.neutral_current,
		       dev->data.status_flags);

	return len;
}

/**
 * @brief Remove transformer monitoring system
 */
int tms_remove(struct tms_dev *dev)
{
	if (!dev)
		return -EINVAL;

	no_os_free(dev);
	pr_info("Transformer Monitoring System removed\n");

	return 0;
}

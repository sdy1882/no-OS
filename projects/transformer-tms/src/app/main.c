/***************************************************************************//**
 *   @file   main.c
 *   @brief  Main file for Transformer Monitoring System (TMS) project.
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
#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_uart.h"
#include "no_os_gpio.h"
#include "no_os_spi.h"
#include "no_os_print_log.h"
#include "ade9430.h"
#include "transformer_tms.h"
#include "parameters.h"

#include "maxim_gpio.h"
#include "maxim_uart.h"
#include "maxim_spi.h"
#include "maxim_uart_stdio.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Display transformer status
 */
static void display_transformer_status(struct tms_dev *tms)
{
	char buffer[512];
	int len;

	len = tms_format_data(tms, buffer, sizeof(buffer));
	if (len > 0) {
		pr_info("\n%s\n", buffer);
	}
}

/**
 * @brief Main function execution for maxim platform.
 *
 * @return ret - Result of the execution.
 */
int main()
{
	int ret;

	/* Initialize UART for console output */
	struct max_uart_init_param uart_extra_ip = {
		.flow = UART_FLOW_DIS
	};

	struct no_os_uart_init_param uart_ip = {
		.device_id = UART_DEVICE_ID,
		.irq_id = UART_IRQ_ID,
		.asynchronous_rx = false,
		.baud_rate = UART_BAUDRATE,
		.size = NO_OS_UART_CS_8,
		.parity = NO_OS_UART_PAR_NO,
		.stop = NO_OS_UART_STOP_1_BIT,
		.platform_ops = &max_uart_ops,
		.extra = &uart_extra_ip
	};
	struct no_os_uart_desc *uart_desc;

	ret = no_os_uart_init(&uart_desc, &uart_ip);
	if (ret) {
		return ret;
	}

	no_os_uart_stdio(uart_desc);

	pr_info("\n");
	pr_info("****************************************************\n");
	pr_info("*  Transformer Monitoring System (TMS)            *\n");
	pr_info("*  ADE9430 Power Quality Monitor                  *\n");
	pr_info("*  Analog Devices, Inc.                           *\n");
	pr_info("****************************************************\n\n");

	/* Platform SPI Initialization Parameters */
	struct max_spi_init_param spi_extra_ip = {
		.num_slaves = 1,
		.polarity = SPI_SS_POL_LOW,
		.vssel = MXC_GPIO_VSSEL_VDDIOH
	};

	/* Energy Meter SPI Initialization Parameters */
	struct no_os_spi_init_param spi_egy_ip = {
		.device_id = 1,
		.max_speed_hz = 1000000,
		.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
		.mode = NO_OS_SPI_MODE_0,
		.platform_ops = &max_spi_ops,
		.chip_select = 0,
		.extra = &spi_extra_ip,
	};

	/* ADE9430 Initialization Parameters */
	struct ade9430_init_param ade9430_ip = {
		.spi_init = &spi_egy_ip,
		.temp_en = true,
	};
	struct ade9430_dev *ade9430_device;

	pr_info("Initializing ADE9430 Energy Metering IC...\n");
	ret = ade9430_init(&ade9430_device, ade9430_ip);
	if (ret) {
		pr_err("Error: ADE9430 initialization failed! (ret = %d)\n", ret);
		goto error_uart;
	}
	pr_info("ADE9430 initialized successfully.\n");

	/* Set energy accumulation model */
	ret = ade9430_set_egy_model(ade9430_device, ADE9430_EGY_NR_SAMPLES,
				    ADE9430_SAMPLES_NR);
	if (ret) {
		pr_err("Error: Failed to set energy model! (ret = %d)\n", ret);
		goto error_ade9430;
	}
	pr_info("Energy accumulation model configured.\n");

	/* Initialize Transformer Monitoring System */
	struct tms_config tms_config = {
		.rated_power = TRANSFORMER_RATED_POWER,
		.rated_voltage = TRANSFORMER_RATED_VOLTAGE,
		.rated_current = TRANSFORMER_RATED_CURRENT,
		.monitoring_interval_ms = TMS_MONITORING_INTERVAL_MS,
		.enable_cloud_logging = ENABLE_CLOUD_LOGGING,
		.enable_lcd_display = ENABLE_LCD_DISPLAY,
	};

	struct tms_dev *tms_device;

	pr_info("Initializing Transformer Monitoring System...\n");
	ret = tms_init(&tms_device, ade9430_device, &tms_config);
	if (ret) {
		pr_err("Error: TMS initialization failed! (ret = %d)\n", ret);
		goto error_ade9430;
	}

	pr_info("\nTransformer TMS is now operational.\n");
	pr_info("Monitoring transformer health parameters...\n\n");

	/* Main monitoring loop */
	uint32_t iteration = 0;
	while (true) {
		iteration++;

		/* Read all transformer parameters */
		ret = tms_read_all_parameters(tms_device);
		if (ret) {
			pr_err("Error reading transformer parameters! (ret = %d)\n", ret);
			no_os_mdelay(TMS_MONITORING_INTERVAL_MS);
			continue;
		}

		/* Check status and update alarms */
		ret = tms_check_status(tms_device);
		if (ret) {
			pr_err("Error checking transformer status! (ret = %d)\n", ret);
		}

		/* Display data every iteration */
		if (ENABLE_SERIAL_OUTPUT) {
			pr_info("\n--- Monitoring Cycle %u ---\n", iteration);
			display_transformer_status(tms_device);

			/* Display alarm summary */
			if (tms_device->data.status_flags != TMS_STATUS_NORMAL) {
				pr_warning("\n!!! ALARMS ACTIVE !!!\n");
				if (tms_device->data.status_flags & TMS_STATUS_TEMP_CRITICAL)
					pr_warning("  - CRITICAL TEMPERATURE\n");
				if (tms_device->data.status_flags & TMS_STATUS_TEMP_WARNING)
					pr_warning("  - High Temperature\n");
				if (tms_device->data.status_flags & TMS_STATUS_OVERLOAD)
					pr_warning("  - Transformer Overload\n");
				if (tms_device->data.status_flags & TMS_STATUS_VOLTAGE_IMBALANCE)
					pr_warning("  - Voltage Imbalance\n");
				if (tms_device->data.status_flags & TMS_STATUS_CURRENT_IMBALANCE)
					pr_warning("  - Current Imbalance\n");
				if (tms_device->data.status_flags & TMS_STATUS_LOW_POWER_FACTOR)
					pr_warning("  - Low Power Factor\n");
				if (tms_device->data.status_flags & TMS_STATUS_PHASE_LOSS)
					pr_warning("  - Phase Loss\n");
				pr_warning("\n");
			} else {
				pr_info("Transformer Status: NORMAL\n");
			}
		}

		/* Wait for next monitoring cycle */
		no_os_mdelay(TMS_MONITORING_INTERVAL_MS);
	}

	/* Cleanup (unreachable in this implementation) */
	tms_remove(tms_device);
	ade9430_remove(ade9430_device);
	no_os_uart_remove(uart_desc);

	return 0;

error_ade9430:
	ade9430_remove(ade9430_device);
error_uart:
	no_os_uart_remove(uart_desc);

	return ret;
}

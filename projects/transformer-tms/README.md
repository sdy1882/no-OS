# Transformer Monitoring System (TMS) with ADE9430

## Overview

The Transformer Monitoring System (TMS) is a comprehensive power quality and transformer health monitoring solution based on the ADE9430 high-performance polyphase energy monitoring IC. This system is designed for transformer condition monitoring in industrial and utility applications.

## Features

### Core Monitoring Capabilities
- **3-Phase Power Measurement**
  - Voltage RMS (per phase)
  - Current RMS (per phase)
  - Active Power (P)
  - Reactive Power (Q)
  - Apparent Power (S)
  - Power Factor

### Transformer Health Monitoring
- **Temperature Monitoring**: Real-time transformer temperature with configurable warning and critical thresholds
- **Load Monitoring**: Calculates transformer load as percentage of rated capacity
- **Imbalance Detection**: 
  - Voltage imbalance calculation across three phases
  - Current imbalance detection
- **Neutral Current Monitoring**: Detects system imbalances and harmonics

### Alarm and Protection Features
- Temperature alarms (Warning at 80°C, Critical at 100°C)
- Overload detection (>120% of rated capacity)
- Voltage imbalance alert (>2%)
- Current imbalance alert (>10%)
- Phase loss detection
- Low power factor warning (<0.85)

## Hardware Requirements

### Supported Platforms
- **Primary Platform**: Maxim MAX32650 microcontroller
- **Energy Metering IC**: Analog Devices ADE9430
- **Development Board**: PQ-MON or compatible ADE9430 evaluation board

### Connections
- SPI interface for ADE9430 communication
- UART for console output and data logging
- GPIO for status LEDs (optional)

## Building the Project

### Prerequisites
```bash
export PLATFORM=maxim
export TARGET=max32650
```

### Build Commands
```bash
# Standard build
make

# Clean and rebuild
make reset
make

# Build with specific configuration
make PLATFORM=maxim TARGET=max32650
```

## Configuration

### Transformer Specifications
Edit `src/app/parameters.h` to configure transformer parameters:

```c
#define TRANSFORMER_RATED_POWER      10000  /* Rated power in VA */
#define TRANSFORMER_RATED_VOLTAGE    400    /* Rated voltage in V */
#define TRANSFORMER_RATED_CURRENT    14     /* Rated current per phase in A */
```

### Monitoring Parameters
```c
#define TMS_MONITORING_INTERVAL_MS   1000   /* Monitoring cycle time */
#define TMS_DATA_LOG_INTERVAL_MS     60000  /* Data logging interval */
```

### Alarm Thresholds
Modify thresholds in `src/app/transformer_tms.h`:

```c
#define TMS_TEMP_WARNING_THRESHOLD      80    /* Temperature warning (°C) */
#define TMS_TEMP_CRITICAL_THRESHOLD     100   /* Temperature critical (°C) */
#define TMS_OVERLOAD_THRESHOLD          120   /* Overload threshold (%) */
#define TMS_VOLTAGE_IMBALANCE_THRESHOLD 2     /* Voltage imbalance (%) */
#define TMS_CURRENT_IMBALANCE_THRESHOLD 10    /* Current imbalance (%) */
```

## Usage

### Serial Console Output
Connect to the device via UART at 115200 baud. The system will output:
- Transformer monitoring data every second
- Real-time alarm notifications
- Phase-by-phase voltage, current, and power measurements
- System status and health indicators

### Example Output
```
****************************************************
*  Transformer Monitoring System (TMS)            *
*  ADE9430 Power Quality Monitor                  *
*  Analog Devices, Inc.                           *
****************************************************

Initializing ADE9430 Energy Metering IC...
ADE9430 initialized successfully.
Transformer Monitoring System initialized
Rated Power: 10000 VA, Rated Voltage: 400 V, Rated Current: 14 A

--- Monitoring Cycle 1 ---
=== Transformer Monitoring System ===
Temperature: 45°C
Load: 75.5% of rated
Total Power: 7550 W (3-phase)
Voltage Imbalance: 1.2%
Current Imbalance: 5.3%

Phase A:
  V: 230000 mV, I: 11000 mA
  P: 2530 mW, Q: 450 mVAR, S: 2570 mVA
  PF: 0.984

Phase B:
  V: 228000 mV, I: 10500 mA
  P: 2394 mW, Q: 420 mVAR, S: 2430 mVA
  PF: 0.985

Phase C:
  V: 232000 mV, I: 11200 mA
  P: 2598 mW, Q: 465 mVAR, S: 2640 mVA
  PF: 0.983

Neutral Current: 150 mA
Status: 0x00
Transformer Status: NORMAL
```

## API Reference

### Core Functions

#### `tms_init()`
Initialize the transformer monitoring system.
```c
int tms_init(struct tms_dev **device, 
             struct ade9430_dev *ade9430_dev,
             struct tms_config *config);
```

#### `tms_read_all_parameters()`
Read all transformer electrical parameters.
```c
int tms_read_all_parameters(struct tms_dev *dev);
```

#### `tms_check_status()`
Check transformer health status and update alarm flags.
```c
int tms_check_status(struct tms_dev *dev);
```

#### `tms_format_data()`
Format transformer data for display or logging.
```c
int tms_format_data(struct tms_dev *dev, 
                    char *buffer, 
                    uint32_t buffer_size);
```

## Application Examples

### Power Quality Monitoring
The TMS continuously monitors:
- Voltage harmonics and distortion
- Current harmonics
- Phase relationships
- Power factor across all phases

### Transformer Protection
Real-time detection of:
- Overload conditions
- Thermal overload
- Phase imbalance
- Phase loss
- Abnormal neutral current

### Predictive Maintenance
Historical data enables:
- Load trending analysis
- Temperature history tracking
- Identification of degradation patterns
- Optimal maintenance scheduling

## Customization

### Adding Cloud Connectivity
To enable data logging to cloud services:
1. Set `ENABLE_CLOUD_LOGGING` to 1 in `parameters.h`
2. Integrate MQTT or HTTP client libraries
3. Implement data upload in the main monitoring loop

### LCD Display Integration
To add display support:
1. Include display driver (e.g., NHD-C12832A1Z)
2. Set `ENABLE_LCD_DISPLAY` to 1
3. Call display update functions in monitoring loop

### Additional Sensors
The architecture supports adding:
- Oil temperature sensors
- Humidity sensors
- Vibration monitoring
- Acoustic emission monitoring

## Troubleshooting

### ADE9430 Initialization Failure
- Check SPI connections (MOSI, MISO, SCK, CS)
- Verify power supply (3.3V)
- Ensure proper grounding

### Incorrect Readings
- Verify CT and PT calibration
- Check phase connections
- Review gain and offset settings in ADE9430 registers

### High Imbalance Readings
- Verify balanced load conditions
- Check CT installation and polarity
- Ensure proper phase sequence

## References

- [ADE9430 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ADE9430.pdf)
- [ADE9430 User Guide](https://www.analog.com/en/products/ade9430.html)
- [MAX32650 Reference Manual](https://www.analog.com/en/products/max32650.html)

## Support

For technical support and questions:
- Visit: https://ez.analog.com/
- Email: processor.support@analog.com

## License

Copyright 2024 Analog Devices, Inc.

This software is provided under the BSD-3-Clause license. See the LICENSE file for details.

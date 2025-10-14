# Transformer TMS Configuration Guide

## System Configuration

### 1. Transformer Specifications

The system must be configured to match your specific transformer ratings. Edit `src/app/parameters.h`:

```c
/* Example: 10 kVA, 400V, 3-phase transformer */
#define TRANSFORMER_RATED_POWER     10000   /* VA */
#define TRANSFORMER_RATED_VOLTAGE   400     /* V (line-to-line) */
#define TRANSFORMER_RATED_CURRENT   14      /* A (per phase) */
```

**Common Transformer Sizes:**

| Rating | Voltage | Current/Phase | Configuration |
|--------|---------|---------------|---------------|
| 10 kVA | 400V | 14.4 A | Small industrial |
| 25 kVA | 400V | 36 A | Industrial |
| 50 kVA | 400V | 72 A | Medium industrial |
| 100 kVA | 400V | 144 A | Large industrial |
| 500 kVA | 11kV | 26 A | Utility distribution |

### 2. Current and Voltage Transformers (CT/PT)

Configure the ADE9430 gain settings to match your CT and PT ratios:

**Current Transformer (CT) Configuration:**
```c
/* Example: 100:5 CT ratio */
CT_PRIMARY = 100;    /* Primary current in A */
CT_SECONDARY = 5;    /* Secondary current in A */
```

**Potential Transformer (PT) Configuration:**
```c
/* Example: 11000:110 PT ratio */
PT_PRIMARY = 11000;  /* Primary voltage in V */
PT_SECONDARY = 110;  /* Secondary voltage in V */
```

### 3. Monitoring Parameters

#### Sampling Rate
```c
#define TMS_MONITORING_INTERVAL_MS  1000    /* 1 second - real-time monitoring */
#define TMS_DATA_LOG_INTERVAL_MS    60000   /* 1 minute - data logging */
```

**Recommendations:**
- Real-time critical applications: 100-500 ms
- General monitoring: 1000 ms
- Data logging/trending: 60000 ms

#### Energy Accumulation
```c
#define ADE9430_SAMPLES_NR  7999    /* Samples per accumulation period */
```

### 4. Alarm Thresholds

Edit `src/app/transformer_tms.h` for alarm settings:

#### Temperature Thresholds
```c
#define TMS_TEMP_WARNING_THRESHOLD      80    /* °C */
#define TMS_TEMP_CRITICAL_THRESHOLD     100   /* °C */
```

**Industry Standards:**
- Oil-filled transformers: 65°C top oil, 110°C winding
- Dry-type transformers: 80°C winding rise
- Critical alarm: Rating + 20°C

#### Overload Protection
```c
#define TMS_OVERLOAD_THRESHOLD  120   /* % of rated power */
```

**Typical Settings:**
- Normal operation: 80-100%
- Warning: 100-120%
- Critical: >120%
- Short-term overload allowed: 120% for 2 hours (depends on transformer)

#### Voltage and Current Imbalance
```c
#define TMS_VOLTAGE_IMBALANCE_THRESHOLD  2    /* % */
#define TMS_CURRENT_IMBALANCE_THRESHOLD  10   /* % */
```

**IEEE Standards:**
- Voltage imbalance: <2% for motors/transformers (IEEE 1159)
- Current imbalance: <10% acceptable, >15% investigate

#### Power Factor
```c
#define TMS_MIN_POWER_FACTOR  0.85    /* Minimum acceptable */
```

**Power Factor Guidelines:**
- Excellent: >0.95
- Good: 0.85-0.95
- Poor: <0.85 (may incur utility penalties)

## Hardware Configuration

### 1. SPI Interface Setup

The ADE9430 communicates via SPI. Default settings:

```c
struct no_os_spi_init_param spi_egy_ip = {
    .device_id = 1,
    .max_speed_hz = 1000000,        /* 1 MHz */
    .bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
    .mode = NO_OS_SPI_MODE_0,
    .chip_select = 0,
};
```

**Pin Connections (MAX32650):**
- SPI1_MOSI → ADE9430 DIN
- SPI1_MISO → ADE9430 DOUT
- SPI1_SCK → ADE9430 SCLK
- SPI1_SS0 → ADE9430 CS

### 2. UART Console

```c
struct no_os_uart_init_param uart_ip = {
    .device_id = UART_DEVICE_ID,
    .baud_rate = 115200,            /* Standard rate */
    .size = NO_OS_UART_CS_8,
    .parity = NO_OS_UART_PAR_NO,
    .stop = NO_OS_UART_STOP_1_BIT,
};
```

### 3. ADE9430 Calibration

For accurate measurements, calibrate the ADE9430:

#### Voltage Calibration
```c
/* Phase A voltage gain */
ade9430_write(dev, ADE9430_REG_AVGAIN, voltage_gain_value);
/* Voltage offset */
ade9430_write(dev, ADE9430_REG_AVRMSOS, voltage_offset_value);
```

#### Current Calibration
```c
/* Phase A current gain */
ade9430_write(dev, ADE9430_REG_AIGAIN, current_gain_value);
/* Current offset */
ade9430_write(dev, ADE9430_REG_AIRMSOS, current_offset_value);
```

#### Phase Calibration
```c
/* Phase A power calibration */
ade9430_write(dev, ADE9430_REG_APHCAL0, phase_cal_value);
```

## Advanced Configuration

### 1. Harmonic Analysis

To enable harmonic monitoring, configure the ADE9430 waveform buffer:

```c
/* Enable waveform buffer */
ade9430_update_bits(dev, ADE9430_REG_WFB_CFG, 
                    ADE9430_WF_CAP_EN, 
                    no_os_field_prep(ADE9430_WF_CAP_EN, 1));
```

### 2. Multi-Transformer Monitoring

To monitor multiple transformers:

```c
/* Define multiple SPI chip selects */
#define TRANSFORMER_1_CS  0
#define TRANSFORMER_2_CS  1
#define TRANSFORMER_3_CS  2

/* Initialize multiple ADE9430 devices */
struct ade9430_dev *transformer[3];
for (int i = 0; i < 3; i++) {
    spi_ip.chip_select = i;
    ade9430_init(&transformer[i], ade9430_ip);
}
```

### 3. Data Logging and Trending

Enable long-term data storage:

```c
#define ENABLE_DATA_LOGGING     1
#define LOG_BUFFER_SIZE         1000
#define LOG_TO_SD_CARD         1
#define LOG_TO_CLOUD           0
```

### 4. Communication Interfaces

#### MQTT for Cloud Logging
```c
#define ENABLE_CLOUD_LOGGING   1
#define MQTT_BROKER_ADDR       "iot.example.com"
#define MQTT_BROKER_PORT       8883
#define MQTT_TOPIC_PREFIX      "transformer/site1/"
```

#### Modbus RTU for SCADA Integration
```c
#define ENABLE_MODBUS_RTU      1
#define MODBUS_SLAVE_ADDRESS   1
#define MODBUS_BAUD_RATE       9600
```

## Calibration Procedure

### Step 1: No-Load Test
1. Power transformer with no load
2. Measure no-load current and losses
3. Verify voltage accuracy across all phases

### Step 2: Load Test
1. Apply known load to transformer
2. Compare measured vs. actual values
3. Adjust gain settings as needed

### Step 3: Phase Calibration
1. Apply balanced 3-phase load
2. Verify phase angles (120° separation)
3. Adjust phase calibration registers

### Step 4: Validation
1. Test with various load conditions
2. Verify accuracy: ±1% for voltage, ±2% for current
3. Check power measurements: ±2% for active power

## Safety Considerations

⚠️ **WARNING:** This system monitors high voltage and current. Follow these safety guidelines:

1. **Electrical Safety:**
   - Use appropriate CT and PT for voltage/current levels
   - Ensure proper grounding
   - Never open CT secondary circuit when energized
   - Use isolated power supplies

2. **Installation:**
   - Follow local electrical codes
   - Use qualified electricians for installation
   - Proper CT/PT installation is critical for safety and accuracy

3. **Maintenance:**
   - Regular calibration checks (annually recommended)
   - Inspect all connections
   - Review alarm thresholds based on operating conditions

## Troubleshooting

### Issue: Incorrect Voltage Readings
**Possible Causes:**
- PT ratio not correctly configured
- Poor connections
- Incorrect phase sequence

**Solution:**
1. Verify PT connections and ratio
2. Check calibration settings
3. Use known voltage source to validate

### Issue: Current Imbalance Detected
**Possible Causes:**
- Unbalanced load (normal condition)
- CT installation issues
- Faulty CT

**Solution:**
1. Verify load balance
2. Check CT installation and polarity
3. Compare with clamp meter readings

### Issue: Temperature Reading Errors
**Possible Causes:**
- Sensor not enabled
- Sensor placement
- Calibration needed

**Solution:**
1. Verify `temp_en = true` in configuration
2. Check temperature sensor connections
3. Calibrate using known temperature reference

## Performance Optimization

### For High-Speed Monitoring
```c
#define TMS_MONITORING_INTERVAL_MS  100    /* 10 Hz update rate */
```

### For Low Power Applications
```c
#define TMS_MONITORING_INTERVAL_MS  5000   /* 0.2 Hz update rate */
#define ENABLE_SLEEP_MODE              1    /* Sleep between samples */
```

### For Maximum Accuracy
```c
#define ADE9430_SAMPLES_NR  15999          /* 2 second accumulation */
#define ENABLE_CALIBRATION_MODE    1       /* Periodic re-calibration */
```

## Support and Resources

- **ADE9430 Datasheet**: Contains detailed register descriptions
- **Application Notes**: Available from Analog Devices
- **Calibration Tools**: Use ADI's calibration software for initial setup
- **Technical Support**: Contact Analog Devices technical support for assistance

---

**Document Version:** 1.0  
**Last Updated:** 2024  
**Maintained by:** Analog Devices, Inc.

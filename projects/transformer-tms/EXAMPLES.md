# Transformer TMS - Usage Examples

## Basic Usage Examples

### Example 1: Simple Monitoring Application

```c
#include "transformer_tms.h"
#include "ade9430.h"

int main(void)
{
    struct ade9430_dev *ade9430;
    struct tms_dev *tms;
    
    /* Initialize ADE9430 */
    struct ade9430_init_param ade_init = {
        .spi_init = &spi_ip,
        .temp_en = true,
    };
    ade9430_init(&ade9430, ade_init);
    
    /* Initialize TMS */
    struct tms_config config = {
        .rated_power = 10000,      /* 10 kVA */
        .rated_voltage = 400,      /* 400V */
        .rated_current = 14,       /* 14A per phase */
        .monitoring_interval_ms = 1000,
    };
    tms_init(&tms, ade9430, &config);
    
    /* Main monitoring loop */
    while (1) {
        tms_read_all_parameters(tms);
        tms_check_status(tms);
        
        /* Display data */
        char buffer[512];
        tms_format_data(tms, buffer, sizeof(buffer));
        printf("%s\n", buffer);
        
        no_os_mdelay(config.monitoring_interval_ms);
    }
    
    return 0;
}
```

### Example 2: Alarm Handling

```c
void check_transformer_health(struct tms_dev *tms)
{
    tms_read_all_parameters(tms);
    tms_check_status(tms);
    
    /* Check for critical conditions */
    if (tms->data.status_flags & TMS_STATUS_TEMP_CRITICAL) {
        /* CRITICAL: Shutdown or reduce load */
        trigger_emergency_shutdown();
        send_alert_notification("CRITICAL: Transformer overtemperature!");
    }
    
    if (tms->data.status_flags & TMS_STATUS_OVERLOAD) {
        /* WARNING: Load shedding may be required */
        if (tms->data.load_percentage > 1500) {  /* >150% */
            trigger_load_shedding();
            send_alert_notification("WARNING: Severe overload detected");
        }
    }
    
    if (tms->data.status_flags & TMS_STATUS_PHASE_LOSS) {
        /* CRITICAL: Phase loss - immediate action required */
        trigger_phase_loss_protection();
        send_alert_notification("CRITICAL: Phase loss detected!");
    }
    
    if (tms->data.status_flags & TMS_STATUS_VOLTAGE_IMBALANCE) {
        /* WARNING: Check load balance */
        log_warning("Voltage imbalance: %d.%d%%", 
                   tms->data.voltage_imbalance / 10,
                   tms->data.voltage_imbalance % 10);
    }
}
```

### Example 3: Data Logging to SD Card

```c
#include "ff.h"  /* FatFs for SD card */

int log_transformer_data(struct tms_dev *tms)
{
    FIL file;
    UINT bytes_written;
    char log_entry[256];
    
    /* Open log file in append mode */
    if (f_open(&file, "transformer_log.csv", FA_OPEN_APPEND | FA_WRITE) != FR_OK) {
        return -1;
    }
    
    /* Format log entry */
    sprintf(log_entry, "%lu,%d,%d,%d,%d,%u,%u,%u\n",
            get_timestamp(),
            tms->data.temperature,
            tms->data.phase_a.voltage_rms,
            tms->data.phase_a.current_rms,
            tms->data.total_active_power,
            tms->data.load_percentage,
            tms->data.voltage_imbalance,
            tms->data.status_flags);
    
    /* Write to file */
    f_write(&file, log_entry, strlen(log_entry), &bytes_written);
    f_close(&file);
    
    return 0;
}
```

### Example 4: MQTT Cloud Integration

```c
#include "mqtt_client.h"

int publish_to_cloud(struct tms_dev *tms, struct mqtt_desc *mqtt)
{
    char json_payload[512];
    struct mqtt_message msg;
    
    /* Create JSON payload */
    sprintf(json_payload,
            "{"
            "\"transformer_id\":\"T1\","
            "\"timestamp\":%lu,"
            "\"temperature\":%d,"
            "\"load_pct\":%u.%u,"
            "\"total_power\":%d,"
            "\"phase_a\":{"
            "\"voltage\":%u,"
            "\"current\":%u,"
            "\"power\":%d,"
            "\"pf\":%d.%03d"
            "},"
            "\"phase_b\":{"
            "\"voltage\":%u,"
            "\"current\":%u,"
            "\"power\":%d,"
            "\"pf\":%d.%03d"
            "},"
            "\"phase_c\":{"
            "\"voltage\":%u,"
            "\"current\":%u,"
            "\"power\":%d,"
            "\"pf\":%d.%03d"
            "},"
            "\"alarms\":\"0x%02X\""
            "}",
            get_timestamp(),
            tms->data.temperature,
            tms->data.load_percentage / 10,
            tms->data.load_percentage % 10,
            tms->data.total_active_power,
            /* Phase A */
            tms->data.phase_a.voltage_rms,
            tms->data.phase_a.current_rms,
            tms->data.phase_a.active_power,
            tms->data.phase_a.power_factor / 1000,
            tms->data.phase_a.power_factor % 1000,
            /* Phase B */
            tms->data.phase_b.voltage_rms,
            tms->data.phase_b.current_rms,
            tms->data.phase_b.active_power,
            tms->data.phase_b.power_factor / 1000,
            tms->data.phase_b.power_factor % 1000,
            /* Phase C */
            tms->data.phase_c.voltage_rms,
            tms->data.phase_c.current_rms,
            tms->data.phase_c.active_power,
            tms->data.phase_c.power_factor / 1000,
            tms->data.phase_c.power_factor % 1000,
            tms->data.status_flags);
    
    /* Publish to MQTT */
    msg.payload = json_payload;
    msg.len = strlen(json_payload);
    msg.qos = MQTT_QOS1;
    msg.retained = false;
    
    return mqtt_publish(mqtt, "transformers/site1/t1/telemetry", &msg);
}
```

### Example 5: Historical Data Analysis

```c
#define HISTORY_SIZE 100

struct tms_history {
    uint32_t timestamp[HISTORY_SIZE];
    int32_t temperature[HISTORY_SIZE];
    uint32_t load_pct[HISTORY_SIZE];
    int head;
    int count;
};

void add_to_history(struct tms_history *hist, struct tms_dev *tms)
{
    hist->timestamp[hist->head] = get_timestamp();
    hist->temperature[hist->head] = tms->data.temperature;
    hist->load_pct[hist->head] = tms->data.load_percentage;
    
    hist->head = (hist->head + 1) % HISTORY_SIZE;
    if (hist->count < HISTORY_SIZE)
        hist->count++;
}

void analyze_trends(struct tms_history *hist)
{
    int32_t avg_temp = 0;
    int32_t max_temp = INT32_MIN;
    uint32_t avg_load = 0;
    
    for (int i = 0; i < hist->count; i++) {
        avg_temp += hist->temperature[i];
        avg_load += hist->load_pct[i];
        if (hist->temperature[i] > max_temp)
            max_temp = hist->temperature[i];
    }
    
    avg_temp /= hist->count;
    avg_load /= hist->count;
    
    pr_info("Temperature - Avg: %d°C, Max: %d°C\n", avg_temp, max_temp);
    pr_info("Load - Average: %u.%u%%\n", avg_load / 10, avg_load % 10);
    
    /* Detect rising temperature trend */
    if (hist->count >= 10) {
        int trend = hist->temperature[hist->head - 1] - 
                    hist->temperature[(hist->head - 10 + HISTORY_SIZE) % HISTORY_SIZE];
        if (trend > 10) {
            pr_warning("WARNING: Temperature rising rapidly (+%d°C in 10 samples)\n", 
                      trend);
        }
    }
}
```

### Example 6: Modbus RTU Interface

```c
#include "modbus_rtu.h"

/* Modbus register mapping */
#define REG_PHASE_A_VOLTAGE     0x0000
#define REG_PHASE_A_CURRENT     0x0001
#define REG_PHASE_A_POWER       0x0002
#define REG_PHASE_B_VOLTAGE     0x0010
#define REG_PHASE_B_CURRENT     0x0011
#define REG_PHASE_B_POWER       0x0012
#define REG_PHASE_C_VOLTAGE     0x0020
#define REG_PHASE_C_CURRENT     0x0021
#define REG_PHASE_C_POWER       0x0022
#define REG_TEMPERATURE         0x0030
#define REG_LOAD_PERCENT        0x0031
#define REG_STATUS_FLAGS        0x0032

void update_modbus_registers(struct tms_dev *tms, uint16_t *modbus_regs)
{
    /* Phase A */
    modbus_regs[REG_PHASE_A_VOLTAGE] = tms->data.phase_a.voltage_rms / 1000;
    modbus_regs[REG_PHASE_A_CURRENT] = tms->data.phase_a.current_rms;
    modbus_regs[REG_PHASE_A_POWER] = tms->data.phase_a.active_power / 1000;
    
    /* Phase B */
    modbus_regs[REG_PHASE_B_VOLTAGE] = tms->data.phase_b.voltage_rms / 1000;
    modbus_regs[REG_PHASE_B_CURRENT] = tms->data.phase_b.current_rms;
    modbus_regs[REG_PHASE_B_POWER] = tms->data.phase_b.active_power / 1000;
    
    /* Phase C */
    modbus_regs[REG_PHASE_C_VOLTAGE] = tms->data.phase_c.voltage_rms / 1000;
    modbus_regs[REG_PHASE_C_CURRENT] = tms->data.phase_c.current_rms;
    modbus_regs[REG_PHASE_C_POWER] = tms->data.phase_c.active_power / 1000;
    
    /* System values */
    modbus_regs[REG_TEMPERATURE] = tms->data.temperature;
    modbus_regs[REG_LOAD_PERCENT] = tms->data.load_percentage / 10;
    modbus_regs[REG_STATUS_FLAGS] = tms->data.status_flags;
}
```

### Example 7: LCD Display Integration

```c
#include "nhd_c12832a1z.h"  /* LCD driver */

void display_on_lcd(struct tms_dev *tms, struct nhd_c12832a1z_dev *lcd)
{
    char line[32];
    
    /* Clear display */
    nhd_c12832a1z_clear_screen(lcd);
    
    /* Line 1: Temperature and Load */
    sprintf(line, "T:%d C  L:%u%%", 
            tms->data.temperature,
            tms->data.load_percentage / 10);
    nhd_c12832a1z_print_string(lcd, line);
    
    /* Line 2: Phase A */
    sprintf(line, "A:%uV %umA %dW",
            tms->data.phase_a.voltage_rms / 1000,
            tms->data.phase_a.current_rms,
            tms->data.phase_a.active_power / 1000);
    nhd_c12832a1z_set_cursor(lcd, 0, 1);
    nhd_c12832a1z_print_string(lcd, line);
    
    /* Line 3: Phase B */
    sprintf(line, "B:%uV %umA %dW",
            tms->data.phase_b.voltage_rms / 1000,
            tms->data.phase_b.current_rms,
            tms->data.phase_b.active_power / 1000);
    nhd_c12832a1z_set_cursor(lcd, 0, 2);
    nhd_c12832a1z_print_string(lcd, line);
    
    /* Line 4: Phase C */
    sprintf(line, "C:%uV %umA %dW",
            tms->data.phase_c.voltage_rms / 1000,
            tms->data.phase_c.current_rms,
            tms->data.phase_c.active_power / 1000);
    nhd_c12832a1z_set_cursor(lcd, 0, 3);
    nhd_c12832a1z_print_string(lcd, line);
    
    /* Line 5: Status */
    if (tms->data.status_flags != TMS_STATUS_NORMAL) {
        nhd_c12832a1z_set_cursor(lcd, 0, 4);
        nhd_c12832a1z_print_string(lcd, "!!! ALARM !!!");
    }
}
```

### Example 8: Energy Consumption Tracking

```c
struct energy_tracker {
    uint64_t total_energy_wh;      /* Total energy in Wh */
    uint32_t last_sample_time;
    uint32_t peak_power_w;
    uint32_t daily_energy_wh;
    uint8_t current_hour;
};

void update_energy_tracker(struct energy_tracker *tracker, 
                           struct tms_dev *tms)
{
    uint32_t current_time = get_timestamp();
    uint32_t time_delta_s = current_time - tracker->last_sample_time;
    
    /* Calculate energy for this interval */
    uint32_t interval_energy_wh = 
        (tms->data.total_active_power / 1000) * time_delta_s / 3600;
    
    tracker->total_energy_wh += interval_energy_wh;
    tracker->daily_energy_wh += interval_energy_wh;
    
    /* Track peak power */
    if (tms->data.total_active_power / 1000 > tracker->peak_power_w) {
        tracker->peak_power_w = tms->data.total_active_power / 1000;
    }
    
    /* Reset daily counter at midnight */
    uint8_t hour = (current_time / 3600) % 24;
    if (hour == 0 && tracker->current_hour == 23) {
        pr_info("Daily energy consumption: %u kWh\n", 
               tracker->daily_energy_wh / 1000);
        tracker->daily_energy_wh = 0;
    }
    tracker->current_hour = hour;
    
    tracker->last_sample_time = current_time;
}

void print_energy_report(struct energy_tracker *tracker)
{
    pr_info("=== Energy Consumption Report ===\n");
    pr_info("Total Energy: %llu kWh\n", tracker->total_energy_wh / 1000);
    pr_info("Today's Energy: %u kWh\n", tracker->daily_energy_wh / 1000);
    pr_info("Peak Power: %u kW\n", tracker->peak_power_w / 1000);
}
```

### Example 9: Power Quality Analysis

```c
void analyze_power_quality(struct tms_dev *tms)
{
    /* Calculate system-wide power factor */
    int32_t total_p = tms->data.total_active_power;
    int32_t total_s = tms->data.total_apparent_power;
    int16_t system_pf = (total_s > 0) ? 
                        (int16_t)((total_p * 1000) / total_s) : 0;
    
    pr_info("=== Power Quality Analysis ===\n");
    
    /* Power Factor Analysis */
    pr_info("System Power Factor: %d.%03d\n", 
           system_pf / 1000, system_pf % 1000);
    
    if (system_pf < 850) {
        pr_warning("Low power factor detected! Consider:\n");
        pr_warning("  - Installing power factor correction capacitors\n");
        pr_warning("  - Checking for inductive loads\n");
        pr_warning("  - Reviewing motor efficiency\n");
    }
    
    /* Voltage Imbalance Analysis */
    if (tms->data.voltage_imbalance > 20) {
        pr_warning("Voltage imbalance: %u.%u%%\n",
                  tms->data.voltage_imbalance / 10,
                  tms->data.voltage_imbalance % 10);
        pr_warning("Potential causes:\n");
        pr_warning("  - Unbalanced single-phase loads\n");
        pr_warning("  - Faulty connections\n");
        pr_warning("  - Upstream supply issues\n");
    }
    
    /* Current Imbalance Analysis */
    if (tms->data.current_imbalance > 100) {
        pr_warning("Current imbalance: %u.%u%%\n",
                  tms->data.current_imbalance / 10,
                  tms->data.current_imbalance % 10);
        
        /* Check for phase loss */
        if (tms->data.phase_a.current_rms < 100 ||
            tms->data.phase_b.current_rms < 100 ||
            tms->data.phase_c.current_rms < 100) {
            pr_warning("Possible phase loss or very low load on one phase\n");
        }
    }
    
    /* Neutral Current Analysis */
    uint32_t avg_phase_current = (tms->data.phase_a.current_rms +
                                   tms->data.phase_b.current_rms +
                                   tms->data.phase_c.current_rms) / 3;
    
    if (tms->data.neutral_current > avg_phase_current / 2) {
        pr_warning("High neutral current: %u mA\n", 
                  tms->data.neutral_current);
        pr_warning("This may indicate:\n");
        pr_warning("  - Harmonic content (triplen harmonics)\n");
        pr_warning("  - Significant load imbalance\n");
        pr_warning("  - Non-linear loads present\n");
    }
}
```

### Example 10: Automatic Report Generation

```c
void generate_daily_report(struct tms_dev *tms, 
                          struct energy_tracker *energy,
                          struct tms_history *history)
{
    char report[2048];
    int len = 0;
    
    len += sprintf(report + len, 
                  "============================================\n"
                  "TRANSFORMER DAILY REPORT\n"
                  "Date: %s\n"
                  "Transformer ID: T1\n"
                  "============================================\n\n",
                  get_date_string());
    
    /* Current Status */
    len += sprintf(report + len,
                  "CURRENT STATUS:\n"
                  "  Temperature: %d°C\n"
                  "  Load: %u.%u%% of rated\n"
                  "  Total Power: %d kW\n"
                  "  Status: %s\n\n",
                  tms->data.temperature,
                  tms->data.load_percentage / 10,
                  tms->data.load_percentage % 10,
                  tms->data.total_active_power / 1000,
                  (tms->data.status_flags == TMS_STATUS_NORMAL) ? 
                  "NORMAL" : "ALARM ACTIVE");
    
    /* Energy Statistics */
    len += sprintf(report + len,
                  "ENERGY STATISTICS:\n"
                  "  Daily Consumption: %u kWh\n"
                  "  Total Consumption: %llu kWh\n"
                  "  Peak Power: %u kW\n\n",
                  energy->daily_energy_wh / 1000,
                  energy->total_energy_wh / 1000,
                  energy->peak_power_w / 1000);
    
    /* Temperature Statistics */
    int32_t avg_temp = 0, max_temp = INT32_MIN;
    for (int i = 0; i < history->count; i++) {
        avg_temp += history->temperature[i];
        if (history->temperature[i] > max_temp)
            max_temp = history->temperature[i];
    }
    avg_temp /= history->count;
    
    len += sprintf(report + len,
                  "TEMPERATURE STATISTICS:\n"
                  "  Average: %d°C\n"
                  "  Maximum: %d°C\n\n",
                  avg_temp, max_temp);
    
    /* Save report */
    save_report_to_file(report);
    
    /* Email report */
    email_report(report);
}
```

## Integration Examples

### Example: Complete System with All Features

See `src/app/main.c` for the complete implementation combining:
- ADE9430 initialization
- TMS configuration
- Main monitoring loop
- Status checking
- Data display

### Example: Multi-Transformer Substation

For monitoring multiple transformers in a substation, extend the basic example with arrays of TMS devices and coordinate monitoring across all units.

---

For more examples and detailed API documentation, refer to:
- `README.md` - Project overview
- `CONFIGURATION.md` - Configuration guide
- Source code in `src/app/` directory

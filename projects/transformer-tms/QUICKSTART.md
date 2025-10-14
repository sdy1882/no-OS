# Transformer TMS - Quick Start Guide

## 5-Minute Setup

### 1. Hardware Setup
```
ADE9430 (PQ-MON Board) → MAX32650 Microcontroller
    SPI1: MOSI, MISO, SCK, CS
    Power: 3.3V
    Temperature sensor: Enabled
```

### 2. Build the Project
```bash
cd projects/transformer-tms
export PLATFORM=maxim
export TARGET=max32650
make
```

### 3. Configure Your Transformer
Edit `src/app/parameters.h`:
```c
#define TRANSFORMER_RATED_POWER    10000  // Your transformer VA rating
#define TRANSFORMER_RATED_VOLTAGE  400    // Your voltage (V)
#define TRANSFORMER_RATED_CURRENT  14     // Your current per phase (A)
```

### 4. Flash and Run
```bash
make run
```

### 5. View Output
Connect UART at 115200 baud:
```
--- Monitoring Cycle 1 ---
Temperature: 45°C
Load: 75.5% of rated
Phase A: 230V, 11A, 2.53kW
Phase B: 228V, 10.5A, 2.39kW
Phase C: 232V, 11.2A, 2.60kW
Status: NORMAL
```

## Default Alarm Thresholds

| Parameter | Warning | Critical |
|-----------|---------|----------|
| Temperature | 80°C | 100°C |
| Overload | 100% | 120% |
| Voltage Imbalance | - | 2% |
| Current Imbalance | - | 10% |

## Common Tasks

### Change Monitoring Rate
In `parameters.h`:
```c
#define TMS_MONITORING_INTERVAL_MS  500  // 500ms = 2 Hz
```

### Adjust Temperature Threshold
In `transformer_tms.h`:
```c
#define TMS_TEMP_CRITICAL_THRESHOLD  90  // Lower to 90°C
```

### Enable Cloud Logging
In `parameters.h`:
```c
#define ENABLE_CLOUD_LOGGING  1
```
Then add MQTT client code (see EXAMPLES.md).

## What You Get

✓ Real-time 3-phase monitoring  
✓ Temperature tracking  
✓ Overload detection  
✓ Voltage/current imbalance alerts  
✓ Power factor monitoring  
✓ Load percentage calculation  
✓ Alarm system with status flags  

## Project Structure
```
transformer-tms/
├── src/app/
│   ├── main.c                    # Main application
│   ├── transformer_tms.c/.h      # TMS implementation
│   └── parameters.h              # Configuration
├── README.md                     # Full documentation
├── CONFIGURATION.md              # Detailed config guide
├── EXAMPLES.md                   # Code examples
└── QUICKSTART.md                 # This file
```

## Need Help?

1. **Build Issues**: Check MAXIM_LIBRARIES path
2. **Wrong Readings**: Verify CT/PT ratios in configuration
3. **No Data**: Check SPI connections
4. **Alarms Always On**: Adjust thresholds in transformer_tms.h

## Next Steps

- Read `CONFIGURATION.md` for detailed setup
- Check `EXAMPLES.md` for integration code
- See `README.md` for complete documentation

---

**Ready to monitor your transformer in 5 minutes!**

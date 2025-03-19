# Documentation

[https://milkv.io/docs/duo/getting-started/8051core](https://milkv.io/docs/duo/getting-started/8051core)

## Firmware compilation
Go to `sdcc/mars/project/base_project` and run `make`.
Copy `mars_mcu_fw.bin` from output/ as well as `8051_up` and `8051_boot_cfg.ini` (from `firmware/`) into `/mnt/data` on the milkv.
Run `sudo ./8051_up` to flash the firmware.

## Notes
- The .ini file contains an address to which the firmware will be flashed. I don't think the current value is correct for the one we put in memmap.py.
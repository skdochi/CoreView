# CoreView

**CoreView** is a lightweight Windows system information utility written in C. It retrieves hardware, system, and operating system information using native Windows interfaces, then displays the results in a custom color-coded terminal interface.

The project is inspired by system monitoring utilities such as CrystalDiskInfo, with the long-term goal of bringing as much hardware and system information as possible into one application.

> **Development Note:** CoreView is an early-stage project. Versions 0.x will remain terminal-based. Development of a graphical user interface (GUI) is planned to begin with version 1.0.

## Features

The current implementation retrieves and displays the following information:

### System Information
- Computer name
- System architecture (x86, x64, ARM64)
- PC manufacturer and model
- System uptime 

### Processor (CPU)
- CPU model name
- CPU manufacturer
- Reported clock speed
- Number of physical CPU cores
- Number of logical processors

### Memory (RAM)
- Total installed physical memory
- Currently used memory
- Available memory
- Current memory usage percentage

### Operating System
- Windows edition (Windows 10 or Windows 11)
- Windows display version
- Windows build number

### Graphics (GPU)
- Detected graphics adapters
- Dedicated video memory
- Shared system memory

### Disk Storage
- Detected fixed drives
- Total storage capacity
- Used storage
- Available storage

### Battery
- Current charge percentage
- Charging, plugged-in, or discharging status
- Battery manufacturer
- Battery model
- Battery serial number
- Designed capacity
- Full charge capacity
- Estimated battery health percentage

### Network
- Active network adapters
- IPv4 addresses
- Default gateways
- MAC addresses

### Motherboard & BIOS
- Motherboard manufacturer
- Motherboard model
- Motherboard version
- BIOS manufacturer
- BIOS version

If Windows cannot retrieve a particular value, CoreView reports it as unavailable instead of interrupting the rest of the report.

## Technologies & Windows APIs

CoreView uses several native Windows interfaces to collect system information.

| Technology | Purpose |
|---|---|
| Windows API | System information, uptime, memory, drives, and battery status |
| Windows Registry | CPU specifications and Windows version information |
| DXGI | Graphics adapter and GPU memory information |
| WMI | Motherboard, BIOS, PC, and battery hardware information |
| IP Helper API | Network adapters, IPv4 addresses, gateways, and MAC addresses |
| ANSI Escape Sequences | Custom 24-bit RGB terminal colors |

## Terminal Interface

CoreView uses custom 24-bit RGB ANSI escape sequences to color its terminal output.

The current color palette:

- Cyan: `#66D9EF` — Titles
- Green: `#A6E22E` — Information labels
- Pink: `#F92672` — Retrieved values
- Gray: `#A9A9A9` — Separators and normal text

Windows Virtual Terminal Processing is enabled to support ANSI colors.

## Requirements

- Windows 10 or Windows 11
- MinGW-w64 GCC or another compatible C compiler
- Windows SDK headers and libraries
- A Windows terminal supporting ANSI escape sequences for the intended colors

CoreView is Windows-specific and will not compile on Linux or macOS without significant modifications.

## Build

Open a terminal in the project directory and compile using MinGW-w64 GCC:

```bash
gcc main.c -o CoreView.exe -ladvapi32 -ldxgi -ldxguid -lole32 -loleaut32 -lwbemuuid -liphlpapi -lws2_32
```

The linked libraries provide access to the Windows Registry, DXGI, COM/WMI, networking, and other Windows system interfaces.

## Run

From PowerShell, execute:

```powershell
.\CoreView.exe
```

CoreView will collect the available information, display the system report, and wait for Enter before exiting.

Some information may be unavailable depending on the hardware, drivers, permissions, or Windows configuration.

## Project Structure

```text
CoreView/
├── main.c              # Main application source code
├── README.md           # Project documentation
├── LICENSE             # MIT License
├── Coreview.rc         # Windows resource script
├── coreview_res.o      # Compiled resource object file
├── assets/             # Project assets
│   ├── CoreView.ico    # Application icon (ICO)
│   └── CoreView.png    # Application icon (PNG)
└── output/             # Generated output files
```

## Roadmap

- [ ] Add live-updating hardware statistics
- [ ] Improve error handling and information retrieval
- [ ] Expand hardware and system information detection
- [ ] Add additional hardware monitoring capabilities
- [ ] Begin GUI development in version 1.0
- [ ] Improve the overall user experience and interface

## License

CoreView is released under the MIT License. See the [LICENSE](LICENSE) file for details.

## Author

Created by **[@skdochi](https://github.com/skdochi)**.

Thanks for checking out CoreView!
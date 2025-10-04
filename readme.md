# SysMon — Linux System Monitor

**SysMon** is a real-time console application for monitoring key Linux system metrics including CPU, memory, disk I/O, and network activity. Built with performance and clarity in mind, it leverages native `/proc` filesystem interfaces and runs efficiently using a thread pool.

## Features

- **Real-time monitoring** of:
  - CPU usage (overall and per-core)
  - Memory usage (including swap)
  - Disk I/O (read/write throughput, IOPS, utilization)
  - Network activity (per-interface throughput)
- **Configurable update interval** (e.g., `500ms`, `2s`)
- **File-based logging** with periodic system summaries
- **Multi-threaded architecture** using a custom thread pool
- **Zero external dependencies** — only standard C++17 and Linux system interfaces

## Build

### Requirements
- A C++17-compliant compiler (e.g., `g++` ≥ 7)
- GNU Make
- Linux system with `/proc` filesystem

### Compile
```bash
make
```
This produces the executable `sysmon` in the project root.

## Usage

```bash
./sysmon [OPTIONS]
```

### Examples
```bash
# Update every 1 second, log to default log.txt
./sysmon

# Update every 300ms, log to custom file
./sysmon -i=300ms -l=monitor.log

# Enable per-core CPU stats
./sysmon --per-core

# Show help or version
./sysmon help
./sysmon version
```

### Command-Line Options

| Option | Description |
|--------|-------------|
| `-i=<dur>` / `--interval=<dur>` | Update interval (e.g., `1s`, `200ms`) |
| `-l=<file>` / `--log-file=<file>` | Log output file (default: `log.txt`) |
| `--log-interval=<dur>` | Interval between full log summaries (default: `120s`) |
| `--per-core` | Show CPU usage per core |
| `help` | Display help message |
| `version` | Show version info |

> Supported duration formats: `<N>s` (seconds) or `<N>ms` (milliseconds).

## Metrics Collected

- **CPU**: Total usage %; optionally per-core breakdown.
- **Memory**: Used vs. total (GiB), % usage, and swap utilization (if enabled).
- **Disk**: Read/write speed (MiB/s), IOPS, and disk utilization %.
- **Network**: Receive/transmit speed (MiB/s) per interface (excluding `lo`).

> All disk and network values are **averaged over the collection interval**.

## Logging

- Logs are written to `log.txt` by default (configurable).
- Includes:
  - Startup messages and errors
  - Full system summary every 2 minutes (adjustable via `--log-interval`)
- Log entries are timestamped with millisecond precision.

## Architecture

- Each metric type is handled by a dedicated collector class (`CpuCollector`, `MemoryCollector`, etc.)
- All collectors implement the `IMetricCollector` interface.
- Metrics are gathered **in parallel** using a thread pool (`ThreadPool.hpp`).
- No third-party libraries — pure C++17 and Linux `/proc` interfaces.

## License

This project was created as a technical assignment. Free to use, modify, and distribute.
# 🚀 Quick Start Guide

## Essential Commands

### 🎯 Most Important Commands

```bash
# Start complete load balancer setup (RECOMMENDED)
make start-lb

# Run single webserver
make run

# Stop all services
make stop
```

### 🧪 Testing Commands

```bash
# Run benchmark tests
make test

# Run Python load tests
make load-test
```

### 🔧 Build Commands

```bash
# Build everything
make all

# Clean build artifacts
make clean

# Build with debug info
make debug

# Build optimized version
make release
```

## 🌐 Access Points

- **Single Server**: http://localhost:8080
- **Load Balanced**: http://localhost:8085 (requires `make start-lb`)

## 📊 What Each Command Does

| Command | Description |
|---------|-------------|
| `make start-lb` | Starts 4 backend servers + load balancer (BEST OPTION) |
| `make run` | Starts single webserver on port 8080 |
| `make stop` | Stops all running services |
| `make test` | Runs automated benchmark tests |
| `make load-test` | Runs Python-based load testing |
| `make help` | Shows all available commands |

## 🎯 Recommended Workflow

1. **For Development**: `make run`
2. **For Load Balancing**: `make start-lb`
3. **For Testing**: `make test` or `make load-test`
4. **To Stop Everything**: `make stop`

## 🆘 Troubleshooting

If you get "Address already in use" errors:
```bash
make stop
# Then try your command again
```

## 📁 Project Structure

- `webserver` - Main server executable
- `load_balancer` - Load balancer executable
- `start_lb.sh` - Automated setup script
- `benchmark.sh` - Performance testing
- `load_test.py` - Python load testing

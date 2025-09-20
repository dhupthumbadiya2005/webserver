
##  **CORRECT Commands** (Use These)

```bash
# Most Important Commands
make start-lb    # Start complete load balancer setup (RECOMMENDED)
make run         # Run single webserver on port 8080
make stop        # Stop all running services

# Testing
make test        # Run benchmark tests
make load-test   # Run Python load tests

# Build
make all         # Build webserver and load balancer
make clean       # Remove build artifacts
make help        # Show all commands
```

##  **WRONG Commands** (Don't Use These)

```bash
make start -lb   # ❌ WRONG - has space
make start_lb    # ❌ WRONG - uses underscore
make run-lb      # ❌ WRONG - removed command
make run-backends # ❌ WRONG - removed command
make stop-backends # ❌ WRONG - removed command
```

##  **Most Common Workflow**

1. **Start everything**: `make start-lb`
2. **Test it works**: Visit http://localhost:8085
3. **Stop everything**: `make stop`

##  **If Something Goes Wrong**

```bash
make stop        # Stop all services
make clean       # Clean build files
make all         # Rebuild everything
make start-lb    # Start fresh
```

## 📊 **What Each Command Does**

| Command | What It Does | When To Use |
|---------|--------------|-------------|
| `make start-lb` | Starts 4 backend servers + load balancer | **Most common** |
| `make run` | Starts single webserver | For simple testing |
| `make stop` | Stops all running services | When done or troubleshooting |
| `make test` | Tests server performance | To check if everything works |
| `make all` | Builds the code | After changing code |
| `make help` | Shows all commands | When you forget |

## 🌐 **Access Points**

- **Single Server**: http://localhost:8080
- **Load Balanced**: http://localhost:8085 (requires `make start-lb`)

## 💡 **Pro Tips**

- Always use `make stop` before starting new services
- Use `make start-lb` for the full experience
- Use `make test` to verify everything works
- Use `make help` if you forget commands

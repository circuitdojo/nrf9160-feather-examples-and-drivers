# HTTPS Sample - Caddy Web Server Setup

This guide explains how to set up a basic Caddy web server to receive JSON payloads from the nRF9160 HTTPS sample.

## Understanding the Sample

The nRF9160 HTTPS sample sends a JSON payload to a configurable endpoint via HTTPS POST request:

- **JSON Payload Format**: `{"do_something":true}`
- **Request Method**: HTTP POST
- **Content-Type**: `application/json`
- **TLS Version**: TLS 1.2
- **Certificate**: Uses ISRG Root X1 (Let's Encrypt root CA)

The sample is configured via Kconfig options in `src/cloud/Kconfig`:
- `CONFIG_CLOUD_HOSTNAME`: Target server hostname (default: "www.circuitdojo.com")
- `CONFIG_CLOUD_PORT`: Server port (default: 443)
- `CONFIG_CLOUD_PUBLISH_PATH`: Endpoint path (default: "/some/path")

## Prerequisites

- A server with a public IP address or domain name
- A valid domain name pointing to your server (required for Let's Encrypt SSL)
- Caddy v2 installed on your server
- Node.js (v14 or later) and npm installed

## Installing Caddy

### Ubuntu/Debian
```bash
sudo apt install -y debian-keyring debian-archive-keyring apt-transport-https curl
curl -1sLf 'https://dl.cloudsmith.io/public/caddy/stable/gpg.key' | sudo gpg --dearmor -o /usr/share/keyrings/caddy-stable-archive-keyring.gpg
curl -1sLf 'https://dl.cloudsmith.io/public/caddy/stable/debian.deb.txt' | sudo tee /etc/apt/sources.list.d/caddy-stable.list
sudo apt update
sudo apt install caddy
```

### Other Systems
See the official Caddy installation guide: https://caddyserver.com/docs/install

## Caddy Configuration

Create a Caddyfile at `/etc/caddy/Caddyfile`:

```caddy
# Replace with your actual domain name
your-domain.com {
    # Enable automatic HTTPS with Let's Encrypt
    # Caddy will automatically obtain and renew SSL certificates

    # Log requests to a file
    log {
        output file /var/log/caddy/nrf9160.log
        format json
    }

    # Endpoint to receive nRF9160 data
    route /api/device/data {
        # Log the request body
        reverse_proxy localhost:8080
    }
}
```

## Backend Server Setup

### Install Dependencies

First, install Node.js if not already installed:

```bash
# Ubuntu/Debian
curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -
sudo apt-get install -y nodejs

# Verify installation
node --version
npm --version
```

### Create the Node.js Server

Create the project directory and initialize:

```bash
sudo mkdir -p /opt/nrf9160-receiver
cd /opt/nrf9160-receiver
sudo npm init -y
sudo npm install express winston
```

Create the server at `/opt/nrf9160-receiver/server.js`:

```javascript
const express = require('express');
const winston = require('winston');

// Configure logger
const logger = winston.createLogger({
  level: 'info',
  format: winston.format.combine(
    winston.format.timestamp(),
    winston.format.json()
  ),
  transports: [
    new winston.transports.File({ filename: '/var/log/nrf9160-receiver.log' }),
    new winston.transports.Console({
      format: winston.format.combine(
        winston.format.colorize(),
        winston.format.simple()
      )
    })
  ]
});

const app = express();
const PORT = 8080;

// Middleware to parse JSON
app.use(express.json());

// Request logging middleware
app.use((req, res, next) => {
  logger.info({
    method: req.method,
    path: req.path,
    ip: req.ip
  });
  next();
});

// Health check endpoint
app.get('/health', (req, res) => {
  res.json({ status: 'healthy', timestamp: new Date().toISOString() });
});

// Main endpoint to receive nRF9160 data
app.post('/api/device/data', (req, res) => {
  try {
    const data = req.body;

    logger.info('Received data from nRF9160', { payload: data });

    // Validate expected fields
    if ('do_something' in data) {
      logger.info(`do_something flag: ${data.do_something}`);
    }

    // Send success response
    res.status(200).json({
      status: 'success',
      timestamp: new Date().toISOString()
    });

  } catch (error) {
    logger.error('Error processing request', { error: error.message });
    res.status(500).json({
      status: 'error',
      message: 'Internal server error'
    });
  }
});

// Error handling middleware
app.use((err, req, res, next) => {
  logger.error('Unhandled error', { error: err.message, stack: err.stack });
  res.status(500).json({
    status: 'error',
    message: 'Internal server error'
  });
});

// Start server
app.listen(PORT, 'localhost', () => {
  logger.info(`nRF9160 receiver listening on localhost:${PORT}`);
});

// Graceful shutdown
process.on('SIGTERM', () => {
  logger.info('SIGTERM received, shutting down gracefully');
  process.exit(0);
});

process.on('SIGINT', () => {
  logger.info('SIGINT received, shutting down gracefully');
  process.exit(0);
});
```

## Setting Up the Backend as a Service

Create a systemd service file at `/etc/systemd/system/nrf9160-receiver.service`:

```ini
[Unit]
Description=nRF9160 Data Receiver
After=network.target

[Service]
Type=simple
User=caddy
Group=caddy
WorkingDirectory=/opt/nrf9160-receiver
ExecStart=/usr/bin/node /opt/nrf9160-receiver/server.js
Restart=always
RestartSec=10
Environment=NODE_ENV=production

[Install]
WantedBy=multi-user.target
```

Enable and start the service:

```bash
# Set proper ownership
sudo chown -R caddy:caddy /opt/nrf9160-receiver

# Create log directory and set permissions
sudo touch /var/log/nrf9160-receiver.log
sudo chown caddy:caddy /var/log/nrf9160-receiver.log

# Enable and start the service
sudo systemctl daemon-reload
sudo systemctl enable nrf9160-receiver
sudo systemctl start nrf9160-receiver

# Check service status
sudo systemctl status nrf9160-receiver
```

## Starting Caddy

```bash
# Reload Caddy configuration
sudo systemctl reload caddy

# Check Caddy status
sudo systemctl status caddy

# View Caddy logs
sudo journalctl -u caddy -f
```

## Configuring the nRF9160 Sample

Update the configuration in your project's `prj.conf` or via menuconfig:

```conf
CONFIG_CLOUD_HOSTNAME="your-domain.com"
CONFIG_CLOUD_PORT=443
CONFIG_CLOUD_PUBLISH_PATH="/api/device/data"
```

Build and flash the sample to your nRF9160 device.

## Testing the Setup

### Test with curl
```bash
curl -X POST https://your-domain.com/api/device/data \
  -H "Content-Type: application/json" \
  -d '{"do_something":true}'
```

### Monitor Logs

```bash
# Watch backend receiver logs
sudo tail -f /var/log/nrf9160-receiver.log

# Watch Caddy logs
sudo tail -f /var/log/caddy/nrf9160.log

# Watch systemd service logs
sudo journalctl -u nrf9160-receiver -f
```

## Troubleshooting

### Certificate Issues
The sample uses the ISRG Root X1 certificate (Let's Encrypt). Ensure your domain has a valid Let's Encrypt certificate. Caddy handles this automatically if your domain is properly configured.

### Connection Failures
- Verify firewall allows HTTPS traffic on port 443
- Check DNS records point to your server
- Ensure Caddy and backend services are running
- Review logs for specific error messages

### nRF9160 Not Connecting
- Verify `CONFIG_CLOUD_HOSTNAME` matches your domain exactly
- Check modem has LTE connectivity
- Review device logs for TLS handshake errors
- Ensure security tag (default: 42) is properly provisioned

## Security Considerations

- The backend server runs on localhost only and is proxied by Caddy
- Caddy automatically handles SSL/TLS termination
- Consider adding authentication if deploying to production
- Review and rotate certificates regularly (Caddy handles this automatically)
- Monitor logs for suspicious activity

## Advanced Configuration

### Adding Request Authentication

Add a bearer token check in the Express middleware:

```javascript
// Authentication middleware
const authenticate = (req, res, next) => {
  const authHeader = req.headers.authorization;

  if (authHeader !== 'Bearer YOUR_SECRET_TOKEN') {
    logger.warn('Unauthorized request attempt', { ip: req.ip });
    return res.status(401).json({
      status: 'error',
      message: 'Unauthorized'
    });
  }

  next();
};

// Apply to the endpoint
app.post('/api/device/data', authenticate, (req, res) => {
  // ... rest of handler
});
```

Update Caddyfile to pass auth header or add rate limiting:

```caddy
your-domain.com {
    route /api/device/data {
        # Rate limiting
        rate_limit {
            zone device_data {
                key {remote_host}
                events 10
                window 1m
            }
        }
        reverse_proxy localhost:8080
    }
}
```

## Code References

- Main application: `src/main.c:45` - Entry point and main loop
- Cloud publish: `src/cloud/cloud.c:219` - JSON payload creation and HTTP POST
- HTTP request setup: `src/cloud/cloud.c:258` - HTTP request configuration
- TLS setup: `src/cloud/cloud.c:34` - TLS socket configuration
- Configuration: `src/cloud/Kconfig:1` - Kconfig options

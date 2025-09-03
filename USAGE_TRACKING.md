# Usage Tracking Guide

This document explains how to use the new usage tracking functionality in the socks library to monitor your chat and premium request usage.

## Overview

The socks library now includes built-in usage tracking that allows applications to monitor:
- Number of chat requests made
- Number of premium requests made  
- Total connections established
- Bytes sent and received

## API Reference

### Tracking Functions

#### `trackChatRequest()`
Increments the chat request counter.
- **Returns**: Current chat request count
- **Usage**: Call this each time your application makes a chat request

```javascript
let count = sock.trackChatRequest();
console.log(`Chat requests: ${count}`);
```

#### `trackPremiumRequest()`
Increments the premium request counter.
- **Returns**: Current premium request count
- **Usage**: Call this each time your application makes a premium request

```javascript
let count = sock.trackPremiumRequest();
console.log(`Premium requests: ${count}`);
```

#### `trackConnection()`
Increments the connection counter.
- **Returns**: Current connection count
- **Usage**: Call this each time a new connection is established

```javascript
let count = sock.trackConnection();
console.log(`Connections: ${count}`);
```

#### `trackBytes(sent, received)`
Tracks bytes transferred.
- **Parameters**: 
  - `sent` (number): Bytes sent
  - `received` (number): Bytes received
- **Returns**: undefined
- **Usage**: Call this to manually track data transfer

```javascript
sock.trackBytes(1024, 512); // 1KB sent, 512 bytes received
```

### Viewing Usage

#### `getUsageStats()`
Returns current usage statistics.
- **Returns**: Object with usage data
- **Usage**: Call this to get a complete overview of usage

```javascript
let stats = sock.getUsageStats();
console.log('Current usage:', stats);
// Output:
// {
//   chatRequests: 10,
//   premiumRequests: 5,
//   totalConnections: 15,
//   bytesSent: 2048,
//   bytesReceived: 1024
// }
```

### Utility Functions

#### `resetUsageStats()`
Resets all usage counters to zero.
- **Returns**: undefined
- **Usage**: Call this to clear usage statistics

```javascript
sock.resetUsageStats();
```

## Automatic Tracking

The library automatically tracks certain operations:

- **Connections**: Automatically incremented when `connect()` or `accept()` succeed
- **Data Transfer**: Automatically tracked when using `send()` and `recv()` functions

## Example Usage Patterns

### Basic Usage Monitoring
```javascript
import * as sock from "./sock.so";

sock.init();

// Your application logic
function makeChatRequest() {
    // ... make request ...
    sock.trackChatRequest();
}

function makePremiumRequest() {
    // ... make premium request ...
    sock.trackPremiumRequest();
}

// Check usage
function showUsage() {
    let stats = sock.getUsageStats();
    console.log(`You have made ${stats.chatRequests} chat requests`);
    console.log(`You have made ${stats.premiumRequests} premium requests`);
    console.log(`Total data: ${stats.bytesSent} bytes sent, ${stats.bytesReceived} bytes received`);
}

sock.cleanup();
```

### Periodic Usage Reports
```javascript
// Check usage every hour
setInterval(() => {
    let stats = sock.getUsageStats();
    console.log("=== Hourly Usage Report ===");
    console.log(`Chat requests: ${stats.chatRequests}`);
    console.log(`Premium requests: ${stats.premiumRequests}`);
    console.log(`Connections: ${stats.totalConnections}`);
    console.log(`Bandwidth: ${stats.bytesSent + stats.bytesReceived} bytes total`);
}, 3600000); // 1 hour
```

### Usage Limits
```javascript
function checkLimits() {
    let stats = sock.getUsageStats();
    
    if (stats.premiumRequests >= 100) {
        console.log("Warning: Premium request limit approaching");
    }
    
    if (stats.chatRequests >= 1000) {
        console.log("Daily chat limit reached");
        return false;
    }
    
    return true;
}

// Before making requests
if (checkLimits()) {
    sock.trackChatRequest();
    // ... proceed with request ...
}
```

## Integration with Chat Applications

For chat applications using this networking library:

```javascript
class ChatClient {
    constructor() {
        this.sock = sock;
        this.sock.init();
    }
    
    async sendMessage(message, isPremium = false) {
        // Track the request type
        if (isPremium) {
            this.sock.trackPremiumRequest();
        } else {
            this.sock.trackChatRequest();
        }
        
        // Send message (automatic byte tracking)
        await this.sock.send(message);
    }
    
    getUsageSummary() {
        let stats = this.sock.getUsageStats();
        return {
            totalMessages: stats.chatRequests + stats.premiumRequests,
            premiumMessages: stats.premiumRequests,
            regularMessages: stats.chatRequests,
            dataUsage: `${stats.bytesSent + stats.bytesReceived} bytes`
        };
    }
}
```

## Troubleshooting

### Common Issues

**Q: Usage stats are not incrementing**
A: Make sure you're calling the tracking functions after successful operations, not before.

**Q: Automatic byte tracking seems incorrect**
A: Automatic tracking only works with the library's `send()` and `recv()` functions. If you're using direct socket operations, use `trackBytes()` manually.

**Q: How to handle usage across multiple sessions?**
A: The current implementation tracks usage per process session. For persistent tracking across restarts, save/load the stats to a file or database.

### Example: Persistent Usage Tracking
```javascript
// Save stats before shutdown
function saveUsageStats() {
    let stats = sock.getUsageStats();
    fs.writeFileSync('usage_stats.json', JSON.stringify(stats));
}

// Load stats on startup
function loadUsageStats() {
    try {
        let stats = JSON.parse(fs.readFileSync('usage_stats.json'));
        // Manually set the internal counters (would need additional API)
        // For now, you could track the difference
    } catch (e) {
        // No saved stats, start fresh
    }
}
```

## See Also

- `usage_example.js` - Complete working example
- `test_usage_tracking.js` - Test suite demonstrating all features
- `README.md` - Library overview and setup instructions
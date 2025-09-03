# socks v 0.03
socks library for c / quickjs / others

I keep needing and writing the same code! This is a small adaptable library in C with bindings for QuickJs and WSH
I've initially started with the simplest HTTP server and client in QuickJS. Now I want to support functionality for SCGI.
There's a problem in scripting environments with FIFOs, in that they block when there's no data. That's the desired 
behavior of a slave, to wait for commands. But to do anything, especially with multiple FIFOs, the nonblocking options
are designed around select() loops, and that might be a little heavy for a cross compatible script or small chunk of code.
I've used the wouldblock. In fact, a 'wouldblock' object that acts like a FIFO nonblocking endpoint is really what
I keep recreating. I just have to write support for it in all of these environments.

## Usage Tracking

This library now includes built-in usage tracking functionality to help you monitor your application's use of chats and premium requests. The following functions are available:

### Tracking Functions
- `sock.trackChatRequest()` - Increments the chat request counter and returns the new count
- `sock.trackPremiumRequest()` - Increments the premium request counter and returns the new count
- `sock.trackConnection()` - Increments the connection counter and returns the new count
- `sock.trackBytes(sent, received)` - Tracks bytes sent and received

### Viewing Usage
- `sock.getUsageStats()` - Returns an object with current usage statistics:
  ```javascript
  {
    chatRequests: 10,
    premiumRequests: 5,
    totalConnections: 15,
    bytesSent: 2048,
    bytesReceived: 1024
  }
  ```

### Utility Functions
- `sock.resetUsageStats()` - Resets all usage counters to zero

### Automatic Tracking
The library automatically tracks:
- Connections when using `connect()` or `accept()` 
- Bytes transferred when using `send()` and `recv()` functions

### Example Usage
```javascript
import * as sock from "./sock.so";

// Initialize
sock.init();

// Track a chat request
sock.trackChatRequest();

// Track a premium request
sock.trackPremiumRequest();

// View your usage
let stats = sock.getUsageStats();
console.log(`You have made ${stats.chatRequests} chat requests`);
console.log(`You have made ${stats.premiumRequests} premium requests`);

// Cleanup
sock.cleanup();
```

See `usage_example.js` for a complete working example.


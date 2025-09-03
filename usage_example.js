import * as os from "qjs:os";
import * as std from "qjs:std";
const isWin = os.platform === 'win32';
const sock = await import(`./sock.${isWin ? 'dll' : 'so'}`);

console.log("=== Socket Library Usage Tracking Example ===");

// Initialize the socket library
sock.init();

// Example: Track a chat request
console.log("\n1. Tracking a chat request...");
let chatCount = sock.trackChatRequest();
console.log("Chat requests tracked:", chatCount);

// Example: Track multiple premium requests
console.log("\n2. Tracking premium requests...");
for (let i = 0; i < 3; i++) {
    let premiumCount = sock.trackPremiumRequest();
    console.log(`Premium request ${i + 1} tracked. Total:`, premiumCount);
}

// Example: Manually track some bytes transferred
console.log("\n3. Manually tracking data transfer...");
sock.trackBytes(1024, 512); // 1024 bytes sent, 512 bytes received
console.log("Tracked 1024 bytes sent, 512 bytes received");

// Example: Simulate a connection
console.log("\n4. Tracking a connection...");
let connectionCount = sock.trackConnection();
console.log("Connections tracked:", connectionCount);

// Get current usage statistics
console.log("\n5. Current usage statistics:");
let stats = sock.getUsageStats();
console.log("Chat requests:", stats.chatRequests);
console.log("Premium requests:", stats.premiumRequests);
console.log("Total connections:", stats.totalConnections);
console.log("Bytes sent:", stats.bytesSent);
console.log("Bytes received:", stats.bytesReceived);

// Example: Show how to see usage in a formatted way
console.log("\n=== Usage Summary ===");
console.log(`You have made ${stats.chatRequests} chat requests`);
console.log(`You have made ${stats.premiumRequests} premium requests`);
console.log(`Total connections established: ${stats.totalConnections}`);
console.log(`Data transferred: ${stats.bytesSent} bytes sent, ${stats.bytesReceived} bytes received`);

// Reset usage statistics for demonstration
console.log("\n6. Resetting usage statistics...");
sock.resetUsageStats();
let resetStats = sock.getUsageStats();
console.log("After reset - Chat requests:", resetStats.chatRequests);
console.log("After reset - Premium requests:", resetStats.premiumRequests);

// Cleanup
sock.cleanup();
console.log("\nUsage tracking example completed!");
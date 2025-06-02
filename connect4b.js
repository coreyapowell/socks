import * as os from "qjs:os";
import * as std from "qjs:std";
const isWin = os.platform === 'win32';
const sock = await import(`./sock.${isWin ? 'dll' : 'so'}`);
//import * as sock from "qjs:sock";
//import * as sock from "./sock.dll";

console.log("sock.init();");
sock.init();

var sas = {
    // google
    //address: "2607:f8b0:4002:c06::6a",
    address: "74.125.21.104",
    // comcast
    //address: "96.17.45.142",
    port: 80,
    family: sock.AF_INET
};

console.log("sa = parseSocketAddress({ address:'" + sas.address + "', port: " + sas.port + 
    "family: sock.AF_INET,});");
var sa = sock.parseSocketAddress(sas);

console.log("socket = new sock.Socket(sock.AF_INET, sock.SOCK_STREAM, 0);");
var socket = new sock.Socket(sock.AF_INET, sock.SOCK_STREAM, 0);

console.log("i = socket.connect(sa);");
socket.connect(sa);

// send header for request
var req = ["GET http://www.google.com/index.html HTTP/1.1", "Accept: */*", 
  "Host: www.google.com",
 "User-Agent: myself/0.01", "Accept-Language: en-US, en","\r\n"];
var reqbuff = req.join("\r\n");
console.log("### Sending Header: ###\r\n" + reqbuff);
socket.sendstring(reqbuff);

console.log("### Response ####");

var ln = socket.recvline();
    console.log(ln);
var lna = ln.split(" ");
var resp = {};
if (lna.length >= 3) { 
    resp["Protocol-version"] =  lna[0];
    resp["Status-code"] =  lna[1];
    resp["Status-message"] =  lna[2];
};

while (ln.length) {
    ln = socket.recvline();
    console.log(ln);
    lna = ln.split(": ");
    if (lna.length > 1) { resp[ lna[0] ] = lna[1]; };
};
 
console.log("\r\n"+ JSON.stringify(resp) + "\r\n");

if (resp["Transfer-Encoding"] == "chunked") {
    
    console.log("\r\n### Transfer-Encoding: chunked ###\r\n");
    ln = socket.recvline();
    var len = parseInt(ln, 16);
    console.log(" - recvline() = [ " + ln + " ] parsed to " + len);
    while (len) {
        var b = new Uint8Array(len);
        
        console.log(" - recv(buffer, " + len + ")  to buffer.");
        var len2;
        while (len) {
            len2 = socket.recv(b.buffer, len, 0);
            //len2 = sock.recv(socket.handle, b.buffer, 0, len);
            
            if (len2 <= 0) { 
                console.log("something went wrong! abort!");
                len = 0; 
            };
            console.log(" - read " + len2 + " of " + len + " bytes. sample of data: #############");
            std.out.write(b.buffer, 0, 48);
            console.log(" ############\r\n");
            len -= len2;
        };       
        
        socket.recvline();
        ln = socket.recvline();
        console.log("\r\n - recvline() = [ " + ln + " ]");
        len = parseInt(ln, 16);
    };
};

console.log("socket.shutdown()");
socket.shutdown();

console.log("sock.cleanup();");
sock.cleanup();

if(false) {
};

socket=0;

console.log("done!");
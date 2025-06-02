import * as os from "qjs:os";
import * as std from "qjs:std";
const isWin = os.platform === 'win32';
const sock = await import(`./sock.${isWin ? 'dll' : 'so'}`);
//import * as sock from "./sock.dll";


console.log(sock);

const JSHDRSTR = " ************************ ";

var tmpfilename;
function getTmpFile() {
    var _tmpfile = std.tmpfile();
    if (_tmpfile) return _tmpfile;
    var tmpdir = std.getenv("TMP");
    if (!tmpdir) tmpdir = std.getenv("TEMP");
    if (!tmpdir) tmpdir = "";
    var randtime = os.cputime ?? os.now;
    var fn = "tmp." + randtime() + ".tmp";
    tmpfilename = tmpdir + "/" + fn;
    console.log(tmpfilename);
    return std.open(tmpfilename,  "a+");   
};

var tmpfile = getTmpFile();
console.log(tmpfile); 
if (tmpfile) {
    tmpfile.close();
    os.remove(tmpfilename);
};   

var JSLOG_OUT_FILE = std.out;
//var JSLOG_TMP_FILE = std.tmpfile();
var JSLOG_OUT = JSLOG_OUT_FILE.fileno();
//var JSLOG_TMP = JSLOG_TMP_FILE.fileno();

//sock.clogger( JSLOG_OUT );

var JSLOG_RESULTS = [];

function JSLOG(s) { if (JSLOGGING > 0) JSLOG_OUT_FILE.puts("JS> " + s + "\r\n"); };
function JSERR(s) { JSLOG_OUT_FILE.puts("JS:ERR> " + s + "\r\n"); };
function JSABORT() { os.Exit(0); };
function JSHDR(s) {  if (JSLOGGING > 0)JSLOG_OUT_FILE.puts("\r\nJS> " + JSHDRSTR + s + JSHDRSTR + "\r\n"); };

function getSocketAddress(p2) {
    JSHDR("SocketAddress");
    JSLOG("verifying IPV6 creation, change to IPV4, returning SocketAddress to use.");
    
    var addr = "1:2:3:4:5:6:7:8";
    var p = 80;
    var f = sock.AF_INET6;
    var sa_opts = { address: addr, port: p, family: f };

    JSLOG("sock.SocketAddress (family: sock.AF_INET6) ");
    JSLOG("creating saddr with args: " + JSON.stringify(sa_opts));

    var saddr = new sock.SocketAddress(sa_opts);
    JSLOG(" - reading from saddr, family: " + saddr.family + ", address: '" +
        saddr.address + "', port: " + saddr.port + "\r\n");

    var addr2 = "192.168.1.1";
    //var p2 = 8083;

    JSLOG(" - setting family to AF_INET, addr to '" + addr2 + "', port to "+p2);
    saddr.family = sock.AF_INET;
    saddr.address = addr2;
    saddr.port = p2;
    JSLOG(" - reading from saddr, family: " + saddr.family + ", address: '" +
        saddr.address + "', port: " + saddr.port + "\r\n");

    JSLOG(" - setting saddr to address 0 for listening later");
        saddr.address = "0.0.0.0"; 
    JSLOG(" - reading from saddr, address: '" + saddr.address + "'");

    return saddr;
};

// **********************************************************
function createListenSocket(saddr) {
    JSHDR("Create Socket and listen()");
    JSLOG("creating new new sock.Socket(sock.AF_INET, sock.SOCK_STREAM, 0);");
    var socket = new sock.Socket(sock.AF_INET, sock.SOCK_STREAM, 0);

/*
console.log(" - manually setting family: sock.AF_INET6, addr: '" + 
    addr + "', port: " + p);
socket.family = sock.AF_INET6; socket.port = p; socket.address = addr;
console.log(" - reading from socket, family: " + socket.family + ", address: '" +
    socket.address + "', port: " + socket.port + "\r\n");

*/

//******** setsockopt
    var optsize, optval, result;
    JSLOG("getsockopt(sock.SOL_SOCKET, sock.SO_REUSEADDR);");
    optval = socket.getsockopt(sock.SOL_SOCKET, sock.SO_REUSEADDR);
    JSLOG("getsockopt returned " + optval + ",  socket.error = " + socket.error);

    JSLOG(" getsockoptlen(sock.SOL_SOCKET, sock.SO_REUSEADDR);");
    optsize = socket.getsockoptlen(sock.SOL_SOCKET, sock.SO_REUSEADDR);
    JSLOG("getsockoptlen returned " + optsize + ",  socket.error = " + socket.error);

var reuseaddr = 1;
if (reuseaddr) {
    JSLOG("setsockopt(sock.SOL_SOCKET, sock.SO_REUSEADDR, 1, sizeof(int));");
    result = socket.setsockopt(sock.SOL_SOCKET, sock.SO_REUSEADDR, reuseaddr, optsize);
    JSLOG("setsockopt result = " + result + ", socket.error = " + socket.error);
};

/*
console.log(" getsockopt(sock.SOL_SOCKET, sock.SO_SNDTIMEO);");
optval = socket.getsockopt(sock.SOL_SOCKET, sock.SO_RCVTIMEO);
console.log("getsockopt returned " + optval + ",  socket.error = " + socket.error);

console.log(" getsockoptlen(sock.SOL_SOCKET, sock.SO_SNDTIMEO);");
optsize = socket.getsockoptlen(sock.SOL_SOCKET, sock.SO_RCVTIMEO);
console.log("getsockoptlen returned " + optsize + ",  socket.error = " + socket.error);

console.log("setsockopt(sock.SOL_SOCKET, sock.SO_SNDTIMEO, 100, sizeof(int));");
result = socket.setsockopt(sock.SOL_SOCKET, sock.SO_RCVTIMEO, 100, optsize);
console.log("getsockoptlen result = " + result + ", socket.error = " + socket.error);

console.log(" getsockopt(sock.SOL_SOCKET, sock.SO_RCVTIMEO);");
optval = socket.getsockopt(sock.SOL_SOCKET, sock.SO_RCVTIMEO);
console.log("getsockopt returned " + optval + ",  socket.error = " + socket.error);

console.log(" getsockoptlen(sock.SOL_SOCKET, sock.SO_RCVTIMEO);");
optsize = socket.getsockoptlen(sock.SOL_SOCKET, sock.SO_RCVTIMEO);
console.log("getsockoptlen returned " + optsize + ",  socket.error = " + socket.error);

console.log("setsockopt(sock.SOL_SOCKET, sock.SO_RCVTIMEO, 100, sizeof(int));");
result = socket.setsockopt(sock.SOL_SOCKET, sock.SO_RCVTIMEO, 100, optsize);
console.log("getsockoptlen result = " + result + ", socket.error = " + socket.error);

//*/

// *********************
    
    JSLOG("socket.bind(saddr) on port " + saddr.port);
    socket.bind(saddr);
    JSLOG("socket.listen");
    socket.listen();
    return socket;
};

function acceptClient(socket, saddr) {

    JSHDR("Socket.accept()");
    console.log("socket.accept() will block until you browse to http://localhost:" + 
        saddr.port + "\r\n........................................................");
    var socket2;
    var addr = new sock.SocketAddress();
    var handle = socket.accept(addr);  
    if (handle) {
        JSLOG("received socket handle from accept:[" + handle +
            "], cloning socket to Socket object.");
        socket2 = sock.cloneSocket(handle);
    } else {
        JSERR("Socket.accept failed to return a client socket handle.");
        JSABORT();
    };
    JSLOG('client Socket created successfully from address: "' + addr.address +
        '", port: ' + addr.port);
    return socket2;
    
};

function readRequest(socket2) {
    JSHDR("HTTP Request");
    JSLOG(" reading header lines (until empty line). initial line:");
    var s = socket2.recvline();
    if (!s) {
        JSERR("Socket failed to return any data. closing socket.");
        socket2.closesocket();
        socket2 = 0;
        return null;
    };
    JSLOG('  "' + s + '"');
    
    var a = s.split(" ");
    var hdr = {};
    if (a.length >= 3) {
        JSLOG("found the 3 parts of the protocol. continuing to read request.")
        hdr.Method = a[0];
        hdr.Path = a[1];
        hdr["Protocol-version"] = a[2];
    } else {
        JSERR("incompatible protocol for HTTP. parsed " + a.length + " parts.");
        JSABORT();
    };

    while (s.length > 0) {
        a = s.split(": ");
  //if (a.length >=2) hdr[a[0].split("-").join("_")] = a[1];
        if (a.length >=2) hdr[a[0]]= a[1];
        s = socket2.recvline(); JSLOG('  "' + s + '"');
    };

    JSLOG("\r\n### headers read! ### :");
    JSLOG(JSON.stringify(hdr));
    return hdr;
};

function readPost(socket2, request) {
    if (request.Method != "POST") { 
        JSLOG("\r\nMethod is not POST. skipping post operation.");
        return null;
    };
    
    JSHDR("POST Method");
    var clengthstring = request["Content-Length"];
    if (!clengthstring) {
        JSLOG("unsupported post method");
        return null;
    };
    var clength = parseInt(clengthstring);
    
    console.log("\r\n### POST data: " + clength + " bytes. ###\r\n");
    if (clength > 0) {
        var len = 4096;
        if (clength > 4096) len = clength;
        var barray = new Uint8Array(len);
        console.log(" - recv(Uint8Array barray.buffer, len) should clean buffer.");
        socket2.recv(barray.buffer, clength);
        console.log(" - std.out.write(barray.buffer, 0, " + clength + ");");
        std.out.write(barray.buffer,  0, clength);
    };
    
};

function generateHTML(getpath, i) {
    JSHDR("HTML FILE (to send)")
    var html = ["<!doctype html>"];
    html.push('<html lang="en">');
    html.push('<head>');
    html.push('<meta charset="utf-8">');
    html.push('<title>HTTP Test Webpage</title>');
    html.push('</head>');
    html.push('<body>');
    html.push('<h1>Basic HTML webpage #' + i +'</h1>');
    html.push('<p>Hello, world! from the lovely path of <b>' + getpath + '</b></p></br>');
    html.push('We are testing Socket.recvline and Socket.sendline in sock library.</br>');
    html.push('Once we get to this point, javascript makes server management easy!</br>');
    html.push('This is a blocking socket. We need to put a thread on each request.</br>');
    html.push('The problem I am facing is that the browsers are greedily connecting again.<br>');
    html.push('<br><br>');
    html.push('<form action="/up" method="post" enctype="multipart/form-data">');
    html.push('<input type="file" id="myfile" name="filename"><input type="submit">');
    html.push('</form>')
    html.push('</body>');
    html.push('</html>');
    html.push('');
    var htmlfile = html.join("\r\n");
    JSLOG("generated the following file: \r\n" + htmlfile);
    return htmlfile;
};

function sendResponse(socket2, request, i) {

    JSHDR("HTTP Response");
    if (!request) {
        JSERR("request is null. something went wrong.");
        JSABORT();
    };    
    var resp; var respbuff;
    var requestPath = request.Path;    
    if ((requestPath == "favicon.ico") || (requestPath == "/favicon.ico")) {
        var resp = ["HTTP/1.0 204 NO CONTENT"];
        if (!keepalive) resp.push('Connection: close');
        resp.push('\r\n');
        respbuff = resp.join("\r\n");
        JSLOG("requested favicon. generated dummy header:\r\n" + respbuff);
        
    } else {
        var htmlfile = generateHTML(request.Path, i);
        resp = ['HTTP/1.1 200 OK'];
        resp.push('Content-Type: text/html; charset=utf-8');
        resp.push('Content-Length: ' + htmlfile.length);
        if (!keepalive) resp.push('Connection: close');
        resp.push('\r\n');
        resp.push(htmlfile);

        var respbuff = resp.join("\r\n");
        JSLOG("generated headers for HTTP response:\r\n" + respbuff);
        
    };

    JSLOG("sending response headers.");
    socket2.sendstring(respbuff);
  if (htmlfile) {
    JSLOG("sending html file.");
    socket2.sendstring(htmlfile);
  };
    //JSLOG("sending final line terminator.");
   // socket2.sendstring("\r\n");
};

function acceptLoop(i, keepalive) {
    //JSLOGGING = 1;  // turn logging on and off
    JSHDR("Accept Loop - iteration: " + i);
    JSLOG("interation of accept loop:" + i + ":\r\n" + JSLOG_RESULTS.join("\r\n"));
   // sock.clogger(1);
    //if (i == 0)
    var acceptReason = 0;
    if (!keepalive) acceptReason = "not using keepalive.";
    else if (i==0) acceptReason = "first iteration always requires accept.";
    else if (socket2 == 0) acceptReason = "socket2 == 0";
    else if (socket2.handle == 0) acceptReason = "socket2.handle == 0";
    if (acceptReason) {
        "socket.accept must request another client socket because: " + acceptReason;
        socket2 = acceptClient(socket, saddr, addr);
        if (socket2) JSLOG_RESULTS.push("iteration " + i + " successfully received socket from accept().");
        else JSLOG_RESULTS.push("iteration " + i + " failed to receive socket from accept().");
    };    
    if (!socket2) {
        return JSERR("Socket was not returned. we can try again....");
    } else {
    };
    request = readRequest(socket2);
    if (!request) {
        JSLOG_RESULTS.push("iteration " + i + " failed to read request.");
        JSERR("Request was not returned. socket must be done or in err.");
        return null;
    };
    JSLOG_RESULTS.push("iteration " + i + " successfully read request: " + 
        request.Path);
    //JSLOGGING = 1;  // turn logging on and off
  //  sock.clogger(1);
    var postbuffer = readPost(socket2, request);
    sendResponse(socket2, request, i);
    
    if (!keepalive) {
        JSLOG("\r\n### shutdown socket ###\r\n");
         socket2.closesocket();
         socket2 = 0;
    };
};

var saddr, addr, socket, socket2, request, postbuffer;

JSLOGGING = 0;
const keepalive = 0;

JSLOG("sock.init();");
sock.init();
JSLOG("sock module with its flags: " + JSON.stringify(sock));
saddr = getSocketAddress("8085"); // listen on this port
//JSLOGGING = 1;  // turn logging on and off
socket = createListenSocket(saddr);
//sock.clogger(1);  // stdout

for (var i = 0; i < 20; i++) acceptLoop(i, keepalive);


console.log(" - shutdown the initial socket we created.");
socket.closesocket();
console.log(" - sock module cleanup().");

sock.cleanup();

//std.exit(0);

console.log("done.");

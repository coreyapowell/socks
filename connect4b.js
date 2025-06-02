import * as os from "qjs:os";
import * as std from "qjs:std";
const isWin = os.platform === 'win32';
const sock = await import(`./sock.${isWin ? 'dll' : 'so'}`);
//import * as sock from "qjs:sock";
//import * as sock from "./sock.dll";

const JSHDRSTR = " ************************ ";
var JSLOGGING = 1;

function JSLOG(s) { if (JSLOGGING > 0) console.log("JS> " + s + ""); };
function JSHDR(s) {  if (JSLOGGING > 0) console.log("\r\nJS> " + JSHDRSTR + s + JSHDRSTR + ""); };

/* testSocketAddress verifies SocketAddress.new, inheritance, set, get 
    testSocketAddress also verifies sock.parseSocketAddress
*/
function testSocketAddress() {

    JSHDR("SocketAddress");
    var sas = {
        address: "74.125.21.104",
        port: 80,
        family: sock.AF_INET,
    };
    JSLOG("var sa = sock.parseSocketAddress( { address:'" + sas.address + "', port: "
        + sas.port + "family: sock.AF_INET, } );");
    var sa = sock.parseSocketAddress(sas);
    JSLOG('sa == { address: "' + sa.address + '", port: ' + sa.port + 
        ', family: ' + sa.family + ' }');

    JSLOG('sa.family = sock.AF_INET6; sa.address = "2607:f8b0:4002:c09::6a";')
    sa.family = sock.AF_INET6; sa.address = "2607:f8b0:4002:c09::6a";
    JSLOG('sa == { address: "' + sa.address + '", port: ' + sa.port + 
        ', family: ' + sa.family + ' }');   
    
    class SocketAddressInherit extends sock.SocketAddress {
        constructor(o, m) { super( o ); this.mess = m; }
        jslog() { return " --- logging this.mess .... " + this.mess; }
    };
    
    JSLOG('class SocketAddressInherit extends sock.SocketAddress {');
    JSLOG('    constructor(o, m) { super( o ); this.mess = m; }');
    JSLOG('    jslog() { return " --- logging this.mess .... " + this.mess; } };');
    JSLOG('sai = new SocketAddressInherit(' + JSON.stringify(sas) + ', "@@ from class");');   
    var sai =  new SocketAddressInherit( sas , "@@ from class");
    
    JSLOG('sai == { address: "' + sai.address + '", port: ' + sai.port + 
        ', family: ' + sai.family + ', mess: "' + sai.mess + '" }');   
    JSLOG("calling sai.jslog() ...."+ sai.jslog());    
    
};

// testNsList verifies: new, first, next, shift, index
function testNsList() {
    JSHDR("NsList");
    var nsi = {
        name: "www.google.com", port: 80,
        flags: 0
    };
    
    JSLOG("nsi = " + JSON.stringify(nsi));
    JSLOG("nsl = new sock.NsList(nsi);");
    var nsl = new sock.NsList(nsi);
    JSLOG("sa = nsl.current()");
    var sa = nsl.current(); 
    if (!sa) JSLOG("!!! there were no records!");
    else {
        JSLOG("iterating each record with shift().... ");
        while (sa) {
            JSLOG("  sa.address[" + nsl.index + "]: " + sa.address);
            sa = nsl.next();
        };
        JSLOG(" end of list.");
        
        var i = nsl.index; 
        if (i > 3) {
            var i2 = i - 2;
            JSLOG("retrieved nsl.index==" + i + ", setting it to " + i2);
            nsl.index = i2;
            sa = nsl.current();
            while (sa) {
                JSLOG("sa=NSList.current(), sa.address[" + nsl.index + "]: " + sa.address);
                sa = nsl.next();
            };
            JSLOG("end of list. retrieving nsl.first()");
            sa = nsl.first();
            JSLOG("nsl.first() address[" +  nsl.index + "]: " + sa.address);
        };      
    };
    
    JSLOG("testing family filter. setting nsi.family to AF_INET6.");
    nsi.family = sock.AF_INET6;
    JSLOG("nsi = " + JSON.stringify(nsi));
    JSLOG("nsl = new sock.NsList(nsi);");
    var nsl = new sock.NsList(nsi);
    JSLOG("sa = nsl.current()");
    sa=nsl.current();
    JSLOG("  sa.address[" + nsl.index + "]: " + sa.address);
    

};

// testSocketConnecct verifies: constructor, inheritance
function testSocketNew() {
    JSHDR("new sock.Socket");

    // socket parameters are optional (in order)
    // test inheritance first
    JSLOG('class SocketInherit extends sock.Socket {');
    JSLOG('    constructor(m) { super( 2, 1, 0 ); this.mess = m; }');
    JSLOG('    jslog() { return " --- logging this.mess .... " + this.mess; }');
    JSLOG("};");
    class SocketInherit extends sock.Socket {
        constructor(m) { super( 2, 1, 0 ); this.mess = m; }
        jslog() { return " --- logging this.mess .... " + this.mess; }
    };
    JSLOG('socket = new SocketInherit("@@ from class");'); 
    var socket = new SocketInherit("@@ from class");  
    if (socket) {
        JSLOG("socket created successfully.");        
    } else { JSLOG("!!! error! no socket was returned."); return 1; };
     
};

// testSocketConnect verifies: Socket.constructor, read, recvline, sendstring
// testSocketConnect also verifies: sock.lookup
function testSocketConnect(ns) {
    JSHDR('Socket.connect("' + ns + '")');
    JSLOG("socket = new sock.Socket(sock.AF_INET, sock.SOCK_STREAM, 0);");
    var socket = new sock.Socket(sock.AF_INET, sock.SOCK_STREAM, 0);
    if (socket) {
    
        JSLOG("socket created successfully.");
        JSLOG('sa = sock.nslookup("' + ns + ', 80");' );
        var sa = sock.lookup(ns, 80);
        if (!sa) { JSLOG("!!! error! no address was returned."); return 1; }
        else { JSLOG('sa: { address: "' + sa.address + '", port: ' + sa.port + 
        ', family: ' + sa.family + ' }'); };
        JSLOG("i = socket.connect(sa);");
        var i = socket.connect(sa);      
              
    } else { JSLOG("!!! error! no socket was returned."); return 1; };
    if (i) { JSLOG("!!! error! socket returned " + i); return 1; };    
    JSLOG("socket connected successfully. generating request ....");
    
    JSHDR("HTTP Request");
    
    // send header for request
    var request = ["GET http://www.google.com/index.html HTTP/1.1", 
        "Accept: */*", "Host: www.google.com", "User-Agent: myself/0.01", 
        "Accept-Language: en-US, en","\r\n"];
    JSLOG("sending the following header lines:");
    for (var i = 0; i < request.length; i++) JSLOG(request[i]);
    var reqbuff = request.join("\r\n");
    socket.sendstring(reqbuff);

    JSHDR("HTTP Response");

    JSLOG(" - receiving the following header lines ( using socket.recvline() )");
    var ln = socket.recvline();
    JSLOG(ln);
    // prime the loop with the response protocol
    var lntuple = ln.split(" ");
    var response = {};
    if (lntuple.length >= 3) { 
        response["Protocol-version"] =  lntuple[0];
        response["Status-code"] =  lntuple[1];
        response["Status-message"] =  lntuple[2];
    };

    while (ln.length) {
        ln = socket.recvline();
        JSLOG(ln);
        lntuple = ln.split(": ");
        if (lntuple.length > 1) { response[ lntuple[0] ] = lntuple[1]; };
    };
 
    JSLOG("\r\n"+ JSON.stringify(response) + "\r\n");

    if (response["Transfer-Encoding"] == "chunked") {
    
        JSHDR("Transfer-Encoding: chunked");
        
        JSLOG("for each chunk, read the 4 byte chunksize string (plus terminator). parse chunklen in hex.");
        JSLOG("the buffered data comes in smaller segments (packets). so also track bytes read.");
        JSLOG("it would be better to verify the line endings, but I just use recvline for now.");
        JSLOG("if all goes according to plan, the final recvline should consume the last line terminator.");
        JSLOG("note: I would like to test a line feed only terminator with recvline. this script would fail.");
        JSLOG("");
        var chunksize = socket.recvline();
        var chunklen = parseInt(chunksize, 16);
        JSLOG(' ### recvline() returned chunksize: "' + chunksize + '", parsed to chunklen: ' + chunklen);
        //var b = new Uint8Array(chunklen); 
        while (chunklen > 0) {
            chunksize = chunklen;
            var b = new Uint8Array(chunklen); 
            var result; var readstart = 0;
            while (chunklen > 0) {
                JSLOG(" - socket.read(b.buffer, " + readstart + ", " + chunklen + ").");
                result = socket.read(b.buffer, readstart, chunklen);
                //result = sock.recv(socket.handle, b.buffer, 0, len);         
                if (result <= 0) { 
                    JSLOG('!!! something went wrong! recieved "' + result + '". abort!');
                    return 1;
                };
                chunklen -= result; readstart += result;
                //JSLOG(" - socket.read(b.buffer, " + readstart + ", " + chunklen + ").");
            };
            var samplesize = 48; if (chunksize < samplesize) samplesize = chunksize;
            JSLOG(" - - read " + chunksize + ' byte chunk. sample of chunk data: #############');
            if (JSLOGGING) std.out.write(b.buffer, 0, samplesize);
            JSLOG(" ############\r\n");
            
            var result = socket.recvline();;
            if (result.length == 0) JSLOG(" - - readline cleared (and verified) the next 2 bytes (line terminator).\r\n");
            else { JSLOG("!!! unexpected string size received! lucky your prompt didn't freeze."); return 1; };

            chunksize = socket.recvline();
            chunklen = parseInt(chunksize, 16);
            JSLOG(' ### recvline() returned chunksize: "' + chunksize + '", parsed to chunklen: ' + chunklen);            
        };       
        

        var result = socket.recvline();
        JSLOG("");
        JSLOG("chunk encoding loop is complete.");
        if (result.length == 0) JSLOG(" - - readline cleared (and verified) the next 2 bytes (line terminator).\r\n");
        else { JSLOG("!!! unexpected string size received! lucky your prompt didn't freeze."); return 1; };

    };

    JSLOG("socket.shutdown()");
    socket.shutdown();

};

/*

*/

//try {

    // set JSLOGGING to 1 to see results. set to 0 to just crash and output line.
    JSLOGGING = 1;

    JSHDR("sock.init();");
    sock.init();

    // testSocketAddress();
    testNsList();
    //testSocketNew();
    //testSocketConnect("www.google.com");

    JSLOG("sock.cleanup();");
    sock.cleanup();
    JSLOG("done");
    
//} catch (e) { JSLOG("!! error occurred: " + e.message); };


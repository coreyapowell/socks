# socks v 0.03
socks library for c / quickjs / others

I keep needing and writing the same code! This is a small adaptable library in C with bindings for QuickJs and WSH
I've initially started with the simplest HTTP server and client in QuickJS. Now I want to support functionality for SCGI.
There's a problem in scripting environments with FIFOs, in that they block when there's no data. That's the desired 
behavior of a slave, to wait for commands. But to do anything, especially with multiple FIFOs, the nonblocking options
are designed around select() loops, and that might be a little heavy for a cross compatible script or small chunk of code.
I've used the wouldblock. In fact, a 'wouldblock' object that acts like a FIFO nonblocking endpoint is really what
I keep recreating. I just have to write support for it in all of these environments.


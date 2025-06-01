/*  CLOG.h -
MIT License
Copyright (c) 2025 Corey Adam Powell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CLOG_MACROS
#define CLOG_MACROS

#ifndef CLOGGER
#define CLOGGER stderr
#endif

#define CLOGKNOT "\r\n"
#define CLOGGING 1
#define FLOGGING 1
#define BLOGGING 1

#else
#undef CLOG
#undef FLOG
#undef TLOG
#endif // CLOG_MACROS

#if defined(CLOGGING) && (CLOGGING == 1)

#define CLOG(STRANG,...)  if((CLOGGING != 0) { if (CLOGGER != 0)) {\
    fprintf(CLOGGER,STRANG CLOGKNOT, __VA_ARGS__); } }
#else
#define CLOG(...) {}

#endif // CLOGGING

#if defined(FLOGGING) && (FLOGGING == 1)
#define FLOG(STRANG,...) if((FLOGGING != 0) { if (FLOGGER != 0)) {\
    fprintf(FLOGGER," [%d] " STRANG CLOGKNOT,__LINE__,__VA_ARGS__); } }
#else
#define FLOG(...) {}
#endif // FLOGGING

#if defined(BLOGGING) && (BLOGGING == 1)
#define FLOG(STRANG,...) if((BLOGGING != 0) { if (BLOGGER != 0)) {\
    fprintf(BLOGGER," [%d] " STRANG CLOGKNOT,__LINE__,__VA_ARGS__); } }
#else
#define FLOG(...) {}

#endif // BLOGGING

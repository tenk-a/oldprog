#include <vcl.h>
#include <stdarg.h>
#include <stdio.h>
#include "subr.h"

AnsiString asprintf(const char *fmt, ...)
{
    va_list app;
    char buf[4100];

    va_start(app, fmt);
    vsprintf(buf, fmt, app);
    buf[4099] = 0;
    va_end(app);
    return (AnsiString)buf;
}

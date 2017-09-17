unsigned short debig5(short i)
    {
    unsigned short r = 0;
    if (i < 5401) r = i/157+0xa4;
    else if (i < 13094)
         { i -= 5401; r = i/157 + 0xc9; }
    else if (i < 13502)
         { i -= 13094; r = i/157 + 0xa1; }
    i %= 157;
    if (i < 63) r |= (i+0x40)<<8;
    else r |= (i - 63 + 0xa1)<<8;
    return(r);
    }

short big5(unsigned char a, unsigned char b)
     {
     short i;
     if (a < 0xa4) i = (a - 0xa1)*157 + 13094;
     else if (a < 0xc7) i = (a - 0xa4)*157;
     else if (a < 0xfa) i = (a - 0xc9)*157 + 5401;
     if (b > 0x7e) i += 63 + (b - 0xa1);
     else i += (b - 0x40);
     return(i);
     }

#include "jstr.h"
#include "tenkdefs.h"

byte *
jstruplow(jstr,mode)
     byte *jstr;
     word mode;
{
     byte up_f;
     byte low_f;
     byte jup_f;
     byte jlow_f;
     byte kata_f;
     byte hira_f;
     byte jspc_f;
     byte d;
     byte c;
     REGI byte *js;

     js = jstr;
     up_f = mode & A2UPPER;
     low_f = mode & A2LOWER;
     jup_f = mode & J2UPPER;
     jlow_f = mode & J2LOWER;
     kata_f = mode & J2KATA;
     hira_f = mode & J2HIRA;
     jspc_f = mode & JSPC2SPC;

     while ((c = *js++) != '\0') {
    	  if (c <= 0x80) {
    	       if (c < 'A') {
    	    	    ;
    	       } else if (c <= 'Z') {
    	    	    if (low_f)
    	    	    	 *(js - 1) = c + 0x20;
    	       } else if (c < 'a') {
    	    	    ;
    	       } else if (c <= 'z') {
    	    	    if (up_f)
    	    	    	 *(js - 1) = c - 0x20;
    	       }
    	  } else if (c <= 0x9f || (c >= 0xe0 && c <= 0xfc)) { /* ¼ÌÄJIS */
    	       d = *(js);
    	       if (d == 0)
    	    	    *(--js) = '\0';
    	       if (c == 0x81) {
    	    	    if (d == 0x40 && jspc_f) {
    	    	    	 *(js - 1) = 0x20;
    	    	    	 *(js) = 0x20;
    	    	    }
    	       } else if (c == 0x82) {
    	    	    if (d >= 0x60 && d <= 0x79) {
    	    	    	 if (jlow_f) {
    	    	    	      d += 0x81 - 0x60;
    	    	    	      *(js - 1) = 0x82;
    	    	    	      *(js) = d;
    	    	    	 }
    	    	    } else if (d >= 0x81 && d <= 0x9a) {
    	    	    	 if (jup_f) {
    	    	    	      d -= 0x81 - 0x60;
    	    	    	      *(js - 1) = 0x82;
    	    	    	      *(js) = d;
    	    	    	 }
    	    	    } else if (d >= 0x9f && d <= 0xf1) {
    	    	    	 if (kata_f) {
    	    	    	      d -= 0x9f - 0x40;
    	    	    	      if (d > 0x7e)
    	    	    	    	   ++d;
    	    	    	      *(js - 1) = 0x83;
    	    	    	      *(js) = d;
    	    	    	 }
    	    	    }
    	       } else if (c == 0x83 && d >= 0x40 && d <= 0x93) {
    	    	    if (hira_f) {
    	    	    	 if (d > 0x7e)
    	    	    	      --d;
    	    	    	 d += 0x9f - 0x40;
    	    	    	 *(js - 1) = 0x82;
    	    	    	 *(js) = d;
    	    	    }
    	       }
    	       js++;
    	  }
     }/* end of while */

     return jstr;
}

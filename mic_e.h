/*
 * mic_e.h: APRS Mic Encoder special case functions
 */

#ifndef MIC_E_H
#define MIC_E_H
typedef unsigned char u_char;
typedef unsigned int u_int;

extern int fmt_mic_e(const u_char *t,	/* tocall */
	  const u_char *i,	/* info */
	  const int l,		/* length of info */
	  u_char *buf1,		/* output buffer */
	  int *l1,		/* length of output buffer */
	  u_char *buf2,		/* 2nd output buffer */
	  int *l2);		/* length of 2nd output buffer */

extern int fmt_x1j4(const u_char *t,	/* tocall */
	  const u_char *i,	/* info */
	  const int l,		/* length of info */
	  u_char *buf1,		/* output buffer */
	  int *l1,		/* length of output buffer */
	  u_char *buf2,		/* 2nd output buffer */
	  int *l2);		/* length of 2nd output buffer */

#endif
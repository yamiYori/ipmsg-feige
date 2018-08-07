#ifndef _DEFINATIONS_H_
#define _DEFINATIONS_H_

#ifdef DEBUG
#define dpf(fmt,args...) printf(fmt,##args)
#else
#define dpf(fmt,args...)
#endif

#define dpl(x) printf("line:%d\n",x)
#define _DBG  

#define likely(x) __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

#define FUNCSUCCEED 0
#define FUNCFAILED -1

#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define s8 char
#define s16 short
#define s32 int
#define s64 long long
#define BIT bool
#define BYTE u8
#define WORD u16
#define WORD_LO(x) (BYTE)((x)&0xff)
#define WORD_HI(x) (BYTE)((x)>>8)
#define DWORD u32
#define QWORD u64

#define PORT 2425
#define BUFFLEN 1024*64

#define IPMSG_EX_CHATREADY 		0x0000008EUL
#define IPMSG_EX_CHATCHECKMAC	0x000000E7UL
#define IPMSG_EX_REQSHAREFILE	0x00000090UL
#define IPMSG_EX_REFUSEFILE     0x000000E6UL
#define IPMSG_EX_CANCELFILE     0x000000E8UL
#define IPMSG_EX_PREGETFILE     0x0000008BUL

const char vermagic[]="5.1.180210";
const char fontmagic[]="[rich]0A0000000000860008AE5F6F8FC596D19E12000000000000000000000000000000000000[/rich]";

#define MSGLENGTH 250
#define PACKRINGLEN 3
#endif

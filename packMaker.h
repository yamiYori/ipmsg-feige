#ifndef __PACKMAKER_H_
#define __PACKMAKER_H_

int mkHeartPack(char dst[],int magic);
int mkHeader(char dst[],int magic,unsigned *rtnPackNum=0);

#endif


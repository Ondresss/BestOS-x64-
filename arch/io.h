#pragma once
unsigned char portByteIn(unsigned short port);
unsigned char portByteOut(unsigned short port,unsigned char data);
unsigned short portWordIn(unsigned short  port);
void portWordOut(unsigned short  port, unsigned short  data);
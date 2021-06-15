#ifndef __PRINTF_H__
#define __PRINTF_H__

extern void myputchar(int c);

int myprintf(const char *format, ...);
int mysprintf(char *out, const char *format, ...);
void printfloat(float val);

#endif

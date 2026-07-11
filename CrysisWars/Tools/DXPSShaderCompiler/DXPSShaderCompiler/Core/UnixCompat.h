#ifndef _UNIX_COMPAT_
#define _UNIX_COMPAT_ 1

#define _fileno fileno

extern "C"
{
  uint32_t _filelength(int fd);
  void _mkdir(const char *);
};

#define __stdcall

#endif


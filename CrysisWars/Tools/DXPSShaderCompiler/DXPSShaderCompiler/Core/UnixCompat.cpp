#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Core/UnixCompat.h"

uint32_t _filelength(int fd)
{
  struct stat buf;
  if (fstat(fd, &buf) == -1)
  {
    perror("fstat()");
    exit(EXIT_FAILURE);
  }
  return static_cast<uint32_t>(buf.st_size);
}

void _mkdir(const char *path)
{
  mkdir(path, 0777);
}


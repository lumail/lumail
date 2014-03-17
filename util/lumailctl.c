/**
 * Pass a message to lumail.
 *
 * This script is designed as a simple alternative to merely executing
 * socat:
 *
 *     echo "alert('ok');" | socat - UNIX-CLIENT:/tmp/foo.sock
 *
 * Steve
 * --
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/**
 * Entry point.
 */
int main(int argc, char *argv[])
{
  struct sockaddr_un addr;
  int fd,rc;

  if ( argc != 3 )
  {
      fprintf(stderr,"Usage: %s /path/to/socket 'cmd'\n", argv[0]);
      exit(0);
  }

  char *socket_path = argv[1];
  char *lua         = argv[2];

  if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
      perror("socket error");
      exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
  {
      perror("connect error");
      exit(-1);
  }

  if ((rc = write(fd, lua, strlen(lua))) != strlen(lua) )
  {
      if (rc > 0)
      {
          fprintf(stderr,"partial write");
      }
  }
  close( fd );

  return 0;
}

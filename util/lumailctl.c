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

#include <sys/stat.h>
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

  char tmp[1024];
  char *soc;
  char *lua;

  if ( argc == 2 )
  {
      memset( tmp, '\0', sizeof(tmp));
      snprintf( tmp, sizeof(tmp)-1, "%s/.lumail.sock", getenv( "HOME" ) );

      soc =  tmp;
      lua = argv[1];
  }
  else if ( argc == 3 )
  {
      soc = argv[1];
      lua = argv[2];
  }
  else
  {
      fprintf(stderr,"Usage: %s [socket/path] 'lua'\n", argv[0]);
      exit(0);
  }


  /**
   * Ensure the socket exists.
   */
  struct stat sb;
  if ( -1 == stat(soc, &sb) )
  {
      perror( "Socket not found.");
      exit(-1);
  }


  if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
      perror("socket error");
      exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, soc, sizeof(addr.sun_path)-1);

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

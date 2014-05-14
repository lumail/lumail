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
#include <string.h>
#include <getopt.h>


#include "version.h"

/**
 * Entry point.
 */
int main(int argc, char *argv[])
{
  struct sockaddr_un addr;
  struct stat sb;
  int fd;
  size_t rc;

  char tmp[1024];
  char *soc;
  char *lua;
  int show_version = 0;

  /**
   * Minimal command-line processing.
   */
  while (1)
  {
      static struct option long_options[] =
          {
              {"version", no_argument, 0, 'v'},
              {0, 0, 0, 0}
          };

      /* getopt_long stores the option index here. */
      int option_index = 0;

      char c = getopt_long(argc, argv, "v", long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
          break;

      switch (c)
      {
        case 'v':
            show_version = 1;
            break;
        case '?':
            /* getopt_long already printed an error message. */
            exit(1);
            break;
        default:
            fprintf( stderr, "Unknown argument\n" );
            exit (1);
      }
  }

  if ( show_version )
  {
      printf("lumailctl version %s\n", LUMAIL_VERSION );
      exit(1);
  }


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
      fprintf(stderr,"Usage: %s [socket/path] 'lua code'\n", argv[0]);
      exit(0);
  }


  /**
   * Ensure the socket exists.
   */
  if ( -1 == stat(soc, &sb) )
  {
      fprintf( stderr, "The domain-socket was not found: %s\n", soc);
      exit(-1);
  }


  if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
      perror("Error opening the domain-socket");
      exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, soc, sizeof(addr.sun_path)-1);

  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
  {
      perror("Error connecting to the domain-socket");
      exit(-1);
  }

  if ((rc = (size_t)write(fd, lua, strlen(lua))) != strlen(lua) )
  {
      if (rc > 0)
      {
          fprintf(stderr,"Partial write performed on the domain-socket");
      }
  }
  close( fd );

  return 0;
}

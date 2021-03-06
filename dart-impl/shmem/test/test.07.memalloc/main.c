
#include <unistd.h>
#include <stdio.h>
#include <dart.h>

#include "../utils.h"

#define REPEAT 5

int main(int argc, char* argv[])
{
  int i;
  char buf[200];
  dart_ret_t ret;
  dart_unit_t myid;
  size_t size;

  dart_init(&argc, &argv);

  dart_myid(&myid);
  dart_size(&size);

  fprintf(stderr, "Hello World, I'm %d of %d\n",
	  myid, size);

  dart_gptr_t gptr = DART_GPTR_NULL;

  for( i=0; i<REPEAT; i++ ) {
    ret = dart_memalloc( 100, &gptr );
    
    if( ret==DART_OK ) {
      GPTR_SPRINTF(buf, gptr);
      fprintf(stderr, "dart_memalloc worked: %s\n", buf);
    }
    if( ret!=DART_OK ) {
      fprintf(stderr, "dart_memalloc did not work!\n");
    }
  }

  dart_exit();
}

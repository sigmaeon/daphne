#include <stddef.h>
#include <stdint.h>

int file_read(void *buffer, int size, char *filename);

int readCsv(void *mem, size_t mem_size) {
  /* Return 0 if read more than 0 bytes */
  return file_read(mem, mem_size, (char *) mem) > 0 ? 0 : 1;
}

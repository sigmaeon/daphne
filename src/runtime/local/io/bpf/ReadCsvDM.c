#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ReadCsvDM.h"

int file_read(void *buffer, int size, char *filename);
int atoi(const char *str);
long int atol(const char *str);
void *copy(void *dest, const void * src, size_t n);

int readCsv(void *mem, size_t mem_size) {
  int offset = sizeof(struct bpf_read_csv_dm);
  struct bpf_read_csv_dm *meta = (struct bpf_read_csv_dm *) mem;
  int row_space_needed = 0;

  for(int i = 0; i < meta->cols; i++) {
    if (meta->types[i] == BPF_COL_INT32) row_space_needed += sizeof(int);
    if (meta->types[i] == BPF_COL_INT64) row_space_needed += sizeof(long);
  }

  int total_space_needed = meta->rows * row_space_needed;
  char line_break = '\n';

  char *data = mem + mem_size - total_space_needed;

  int row = 0, col = 0;
  file_read(mem + offset, mem_size - offset, meta->filename);

  char *line_ctx = NULL;
  char *line = NULL, *segment = NULL;

  line = strtok_r(mem + offset, &line_break, &line_ctx);

  while(line != NULL) {
    segment = strtok(line, &meta->delim);
    while (segment != NULL) {
      int space_needed = 0;
      for(int i = 0; i < col; i++) {
        if (meta->types[i] == BPF_COL_INT32) space_needed += sizeof(int);
        if (meta->types[i] == BPF_COL_INT64) space_needed += sizeof(long);
      }

      int offz = row * row_space_needed + space_needed;
      char *ptr = data + offz;

      if(meta->types[col] == BPF_COL_INT32) {
        int res = atoi(segment);
        copy(ptr, &res, sizeof(int));
      }

      if(meta->types[col] == BPF_COL_INT64) {
        long res = atol(segment);
        copy(ptr, &res, sizeof(long));
      }

      if (++col >= meta->cols) {
        break;
      }

      segment = strtok(NULL, &meta->delim);
    }

    if (++row >= meta->rows) {
      break;
    }
    col = 0;

    line = strtok_r(NULL, &line_break, &line_ctx);
  }

  return row;
}

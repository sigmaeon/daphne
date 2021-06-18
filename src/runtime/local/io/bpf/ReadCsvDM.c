#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ReadCsvDM.h"

/* Interface available on the BPF device */
int file_read(void *buffer, int size, char *filename);
int atoi(const char *str);
long int atol(const char *str);
void *copy(void *dest, const void * src, size_t n);


int readCsv(void *mem, size_t mem_size) {
  int meta_offz = sizeof(struct bpf_read_csv_dm);
  struct bpf_read_csv_dm *meta = (struct bpf_read_csv_dm *) mem;

  int matrix_size = 0;
  int matrix_row_size = 0;
  for(int i = 0; i < meta->cols; i++) {
    if (meta->types[i] == BPF_COL_INT32) matrix_row_size += sizeof(int);
    if (meta->types[i] == BPF_COL_INT64) matrix_row_size += sizeof(long);
  }

  matrix_size = meta->rows * matrix_row_size;

  char line_break = '\n';
  char *matrix = mem + meta_offz;

  int row = 0, col = 0;
  file_read(mem + meta_offz + matrix_size, mem_size - meta_offz - matrix_size, meta->filename);

  char *line_ctx = NULL;
  char *line = NULL, *segment = NULL;

  line = strtok_r(mem + meta_offz + matrix_size, &line_break, &line_ctx);

  while(line != NULL) {
    segment = strtok(line, &meta->delim);
    while (segment != NULL) {
      int last_row_offz = 0;
      for(int i = 0; i < col; i++) {
        if (meta->types[i] == BPF_COL_INT32) last_row_offz += sizeof(int);
        if (meta->types[i] == BPF_COL_INT64) last_row_offz += sizeof(long);
      }

      int offz = row * matrix_row_size + last_row_offz;
      char *ptr = matrix + offz;

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

#ifndef BPF_READ_CSV_DM
#define BPF_READ_CSV_DM

#include <linux/limits.h>

enum coltype { BPF_COL_INT64, BPF_COL_INT32 };

struct bpf_read_csv_dm {
  char filename[PATH_MAX];
  char delim;
  size_t rows;
  size_t cols;
  uint8_t types[128];
};

#endif

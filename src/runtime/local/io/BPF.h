/*
 * Copyright 2021 The DAPHNE Consortium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_RUNTIME_LOCAL_IO_BPF_H
#define SRC_RUNTIME_LOCAL_IO_BPF_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "File.h"

#define PROG_BUF 1024 * 1024
#define DATA_BUF 1024 * 1024

struct BPF {
  File *file;
  int slot;

  int pos;
  int read;

  void *prog;
  void *data;
};

struct BPF *loadBpf(File *file, void *mem) {
  struct BPF *b = (struct BPF *)malloc(sizeof(struct BPF));

  b->slot = 0; // For now
  b->prog = malloc(PROG_BUF);
  b->data = mem == NULL ? malloc(DATA_BUF) : mem;
  b->file = file;

  char file_prog[64];
  char file_data[64];
  char file_load[64];

  sprintf(file_prog, "/tmp/bpf/%i/program", b->slot);
  sprintf(file_data, "/tmp/bpf/%i/data", b->slot);
  sprintf(file_load, "/tmp/bpf/%i/load", b->slot);

  int fd_prog = open(file_prog, O_RDWR, (mode_t)0777);
  int fd_data = open(file_data, O_RDWR, (mode_t)0777);
  int fd_load = open(file_load, O_RDWR, (mode_t)0777);

  b->prog = getAll(b->file);
  b->pos = -1;
  b->read = 0;

  char one = '1';
  write(fd_prog, b->prog, PROG_BUF);
  write(fd_data, b->data, DATA_BUF);
  write(fd_load, &one, sizeof(char));

  printf("%s\n", (char *) b->data);

  close(fd_prog);
  close(fd_data);
  close(fd_load);

  return b;
}

inline char *getLine(BPF *b) {
  char line[1024];
  char *data = (char *) b->data;

  if(b->pos == -1) {
      b->pos = 0;
      char file_trig[64];
      sprintf(file_trig, "/tmp/bpf/%i/trigger", b->slot);

      int fd_trig = open(file_trig, O_RDWR, (mode_t)0777);

      char one = '1';
      write(fd_trig, &one, sizeof(char));
      close(fd_trig);
      usleep(1000 * 25);

      char file_data[64];
      sprintf(file_data, "/tmp/bpf/%i/data", b->slot);
      int fd_data = open(file_data, O_RDWR, (mode_t)0777);
      read(fd_data, b->data, DATA_BUF);
      close(fd_data);

      b->pos = 0; // File opened, buffer (probably) filled
  }

  sscanf(data + b->pos, "%s\n", line);

  printf("%s\n", line);

  b->read = strlen(line);
  b->pos += b->read;

  return &data[b->pos - b->read];
}

#endif

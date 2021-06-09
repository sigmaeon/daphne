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

  int fd_prog;
  int fd_data;
  int fd_trigger;
  int fd_status;
  int fd_load;

  void *prog;
  void *data;
};

inline void triggerBPF(BPF *b) {
  char one = '1';
  write(b->fd_trigger, &one, sizeof(char));

  lseek(b->fd_data, 0, SEEK_SET);

  char status = '\0', trigger = '\0';
  while(1) {
    read(b->fd_status, &status, sizeof(char));
    read(b->fd_trigger, &trigger, sizeof(char));
    lseek(b->fd_status, 0, SEEK_SET);
    lseek(b->fd_trigger, 0, SEEK_SET);

    if(status == '0' && trigger == '0') break;

    usleep(1000);
  }

  read(b->fd_data, b->data, DATA_BUF);
}

struct BPF *loadBpf(File *file, void *mem) {
  struct BPF *b = (struct BPF *)malloc(sizeof(struct BPF));

  b->slot = 0; // For now
  b->prog = malloc(PROG_BUF);
  b->data = mem == NULL ? malloc(DATA_BUF) : mem;
  b->file = file;

  char file_prog[64];
  char file_data[64];
  char file_load[64];
  char file_trigger[64];
  char file_status[64];

  sprintf(file_prog, "/tmp/bpf/%i/program", b->slot);
  sprintf(file_data, "/tmp/bpf/%i/data", b->slot);
  sprintf(file_load, "/tmp/bpf/%i/load", b->slot);
  sprintf(file_trigger, "/tmp/bpf/%i/trigger", b->slot);
  sprintf(file_status, "/tmp/bpf/%i/status", b->slot);

  b->fd_prog = open(file_prog, O_RDWR, (mode_t)0777);
  b->fd_data = open(file_data, O_RDWR, (mode_t)0777);
  b->fd_load = open(file_load, O_RDWR, (mode_t)0777);
  b->fd_trigger = open(file_trigger, O_RDWR, (mode_t)0777);
  b->fd_status = open(file_status, O_RDWR, (mode_t)0777);

  b->prog = getAll(b->file); // Read ELF file to buffer
  b->pos = -1;
  b->read = 0;

  char one = '1';
  write(b->fd_prog, b->prog, PROG_BUF);
  write(b->fd_data, b->data, DATA_BUF);
  write(b->fd_load, &one, sizeof(char));

  return b;
}

inline char *getLine(BPF *b) {
  char line[1024 * 1024]; // Assumption: CSV lines does not go beyond 1 MB
  char *data = (char *) b->data;

  if(b->pos == -1) {
      b->pos = 0;
      triggerBPF(b);
  }

  sscanf(data + b->pos, "%s\n", line);

  b->read = strlen(line);
  b->pos += b->read;

  return &data[b->pos - b->read];
}

#endif

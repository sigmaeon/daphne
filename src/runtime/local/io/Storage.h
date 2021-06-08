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

#ifndef SRC_RUNTIME_LOCAL_IO_STORAGE_H
#define SRC_RUNTIME_LOCAL_IO_STORAGE_H

#include <runtime/local/io/File.h>
#include <runtime/local/io/BPF.h>

enum storageType { STORAGE_TYPE_BPF, STORAGE_TYPE_FILE };

struct Storage {
    int type;
    union {
      struct File file;
      struct BPF bpf;
    };
};

inline char *getLine(Storage *s) {
  if (s->type == STORAGE_TYPE_FILE) return getLine(&s->file);
  return getLine(&s->bpf);
}

#endif

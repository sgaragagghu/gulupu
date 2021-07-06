/*
 * Copyright (C) 2021 sgaragagghu marco.scarlino@students-live.uniroma2.it
 *
 * This file is part of Gulupu
 * Gulupu is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef __EXAMPLEAPP_PARSER_THREAD_H
#define __EXAMPLEAPP_PARSER_THREAD_H

#define BUFF_SIZE 1024
#define REFRESH_TIME 3

#define PARSER_GET_WARN(ADDRESS) ((gboolean)((uintptr_t)ADDRESS & 1))
#define PARSER_GET_ADDRESS(ADDRESS) ((gpointer)((uintptr_t)ADDRESS) - ((uintptr_t)ADDRESS % 2))

enum info_types {
  INFO_VERSION,
  INFO_NAME,
  INFO_TARGET,
  INFO_PEERS,
  INFO_BEST_BLOCK,
  INFO_LOCAL_HASHRATE,
  INFO_NETWORK_HASHRATE,
  INFO_DATABASE,
  INFO_IDENTITY,
  INFO_ADDRESS,
  INFO_MINING,
  ERR_DB,
  ERR_KEY_NOT_FOUND,
  ERR_INVALID_ADDR,
  INFO_COUNT
};

struct information {
  const gchar * pattern;
  enum info_types type;
  enum info_types propagate;
  enum info_types next;
  gchar token[BUFF_SIZE];
};

void        *thread_loop         (GSubprocess *process);

#endif /* __EXAMPLEAPP_PARSER_THREAD_H */
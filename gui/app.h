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

#ifndef __GULUPUAPP_H
#define __GULUPUAPP_H

#include <gtk/gtk.h>
#include <pthread.h>

#define GULUPU_APP_TYPE (gulupu_app_get_type ())

#define GULUPU_EXIT() \
  {                   \
  gulupu_quit();      \
  return;             \
  }

G_DECLARE_FINAL_TYPE (GulupuApp, gulupu_app, GULUPU, APP, GtkApplication)

enum action_thread {
  TH_START,
  TH_STOP
};

struct manage_node_arg {
  enum action_thread action;
  pthread_t tid;
  pthread_t *current_tid;
  GtkSwitch *switcher;
  gboolean quit;
};

GulupuApp       *gulupu_app_new         (void);
void            manage_node_thread      (struct manage_node_arg);
void            gulupu_quit             (void);


#endif /* __GULUPUAPP_H */

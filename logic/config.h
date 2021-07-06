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
 
#include <glib.h>
 
#ifndef __EXAMPLEAPP_CONFIG_H
#define __EXAMPLEAPP_CONFIG_H

#define YES_NO_TO_BOOL(STR) (strcmp(STR, "no") ? TRUE : FALSE )

struct setting {
  const gchar *group;
  const gchar *key;
  const gchar *value;
};

gboolean          initialize_config       (void);
gboolean          save_config             (const struct setting *,
                                           gint,
                                           gboolean);
struct setting    load_config             (const struct setting *);

#endif /* __EXAMPLEAPP_CONFIG_H */
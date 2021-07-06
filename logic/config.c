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

#include "config.h"

#define LOAD_FILE(KEY_FILE, ERROR)                                                   \
  g_autoptr(GKeyFile) KEY_FILE = g_key_file_new ();                                  \
  g_autoptr(GError) ERROR = NULL;                                                    \
                                                                                     \
  if (!g_key_file_load_from_file (KEY_FILE, "config.ini", G_KEY_FILE_NONE, &ERROR))  \
    {                                                                                \
      if (ERROR != NULL)                                                             \
        g_warning ("Error loading key file: %s", ERROR->message);                    \
      ERROR = NULL;                                                                  \
    }

struct setting 
load_config (const struct setting *set)
{
  LOAD_FILE(key_file, error);

  struct setting ret = *set;
  ret.value = g_key_file_get_string (key_file, set->group, set->key, &error);
  return ret;
}

gboolean
save_config (const struct setting *settings,
             gint settings_amount,
             gboolean overwrite)
{
  LOAD_FILE(key_file, error);

  gboolean config_modified = FALSE;

  for (int i = 0; i < settings_amount; ++i)
    {
    g_autofree gchar *val = g_key_file_get_string (key_file, settings[i].group, settings[i].key, NULL);
    if (overwrite || val == NULL)
      {
      g_key_file_set_string (key_file, settings[i].group, settings[i].key, settings[i].value);
      }
    }

  if (!g_key_file_save_to_file (key_file, "config.ini", &error))
    {
    g_warning ("Error saving config file: %s", error->message);
    return FALSE;
    }
  return TRUE;
}

gboolean
initialize_config ()
{
#define default_settings_amount 4
  const struct setting default_settings[default_settings_amount] =
  {
    {"Kulupu", "threads", "2"},
    {"Kulupu", "exe", "./kulupu.exe"},
    {"Kulupu", "address", ""},
    {"Gulupu", "mine_at_start", "no"}
  };
  
  return save_config (default_settings, default_settings_amount, FALSE);
#undef default_settings_amount
}

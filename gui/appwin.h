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

#ifndef __EXAMPLEAPPWIN_H
#define __EXAMPLEAPPWIN_H

#include <gtk/gtk.h>

#include "app.h"
#include "../logic/parser_thread.h"

#define EXAMPLE_APP_WINDOW_TYPE (example_app_window_get_type ())
G_DECLARE_FINAL_TYPE (ExampleAppWindow, example_app_window, EXAMPLE, APP_WINDOW, GtkApplicationWindow)

ExampleAppWindow *      example_app_window_new          (ExampleApp *);
void                    example_app_window_open         (ExampleAppWindow *,
                                                         GFile *);
gboolean                update_informations             (struct information *);
gboolean                save_address                    (struct information *);
gboolean                err_key_not_found               (void *);
gboolean                update_messages                 (void *);
void                    clear_address                   (void);
void                    node_switch_wrap_cb             (GObject *, GAsyncResult *, gpointer);
GtkWindow * const       get_window                      (void);

							
#endif /* __EXAMPLEAPPWIN_H */

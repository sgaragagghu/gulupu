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

#include <gtk/gtk.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "app.h"

int
main (int argc, char *argv[])
{
#ifdef WIN32
#include <windows.h>
  HWND console = GetConsoleWindow();
  ShowWindow (console, SW_HIDE);
#endif
  return g_application_run (G_APPLICATION (gulupu_app_new ()), argc, argv);
}

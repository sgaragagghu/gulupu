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
#include "appwin.h"
#include "../logic/parser_thread.h"
#include "../logic/config.h"

struct _ExampleApp
{
  GtkApplication parent;
};

G_DEFINE_TYPE(ExampleApp, example_app, GTK_TYPE_APPLICATION);

static ExampleApp *g_app = NULL;

static void
example_app_init (ExampleApp *app)
{
  initialize_config ();
  g_app = app;
}

static void
manage_node_error (uintptr_t error)
{
  if (error == ERR_DB)
    {
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *dialog = gtk_message_dialog_new (get_window (),
                                    flags,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_OK_CANCEL,
                                    "Do we proceed ?");
   GtkWidget *headerbar = gtk_header_bar_new ();
   gtk_widget_set_visible (headerbar, TRUE);
   gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (headerbar),
                                         TRUE);
   gtk_header_bar_set_title (GTK_HEADER_BAR (headerbar),
                             "The chain needs to be purged");
   gtk_window_set_titlebar (GTK_WINDOW (dialog), headerbar);
   if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog)))
     {
     g_autofree const gchar *exe = load_config (&(struct setting){"Kulupu", "exe", NULL}).value;
     g_autofree const gchar *threads = load_config (&(struct setting){"Kulupu", "threads", NULL}).value;
     const gchar * const base_argv[] = {
                                         exe,
                                         "purge-chain",
                                         "-y",
                                         NULL
                                       };

     GSubprocess *process = g_subprocess_newv(base_argv,
                                              G_SUBPROCESS_FLAGS_STDOUT_SILENCE,
                                              NULL);
     g_subprocess_wait(process, NULL, NULL);

     if (process == NULL)
       g_application_quit (G_APPLICATION (g_app));
     }
    gtk_widget_destroy (dialog);
    }

  else if (error == ERR_INVALID_ADDR)
    {
    clear_address ();
    }
}

void
manage_node_thread (struct manage_node_arg arg)
{
  enum action_thread action = arg.action;
  static GSubprocess *process = NULL;
  pthread_t *thread_id = arg.current_tid;

  if(action == TH_START && process == NULL)
    {
#ifdef WIN32
  SetConsoleCtrlHandler (NULL, FALSE);
#endif

  g_autofree const gchar *exe = load_config (&(struct setting){"Kulupu", "exe", NULL}).value;
  g_autofree const gchar *threads = load_config (&(struct setting){"Kulupu", "threads", NULL}).value;
  g_autofree const gchar *address = load_config (&(struct setting){"Kulupu", "address", NULL}).value;

  const gchar * const * base_argv;

  const gchar * const base_argv_addr[] = {
                                           exe,
                                           "--validator",
                                           "--author",
                                           address,
                                           "--threads",
                                           threads,
                                           NULL
                                         };

  const gchar * const base_argv_no_addr[] = {
                                              exe,
                                              "--validator",
                                              "--threads",
                                              threads,
                                              NULL
                                            };

  if (strlen (address))
    base_argv = base_argv_addr;
  else
    base_argv = base_argv_no_addr;

  process = g_subprocess_newv(base_argv,
                              G_SUBPROCESS_FLAGS_STDERR_PIPE,
                              NULL);
  if (process == NULL)
    g_application_quit (G_APPLICATION (g_app));
			 
  if (pthread_create(thread_id, NULL, (void * (*)(void *))thread_loop, process))
    g_application_quit (G_APPLICATION (g_app));

  struct manage_node_arg *arg_cb = g_malloc (sizeof(struct manage_node_arg));
  *arg_cb = (struct manage_node_arg){TH_STOP, *thread_id, arg.current_tid, arg.switcher};
  g_subprocess_wait_async (process,
                           NULL,
                           node_switch_wrap_cb,
                           arg_cb);
						   
  } else if(action == TH_STOP && process != NULL)
    {
#ifndef WIN32
    g_subprocess_send_signal (process, SIGINT);
#else
    SetConsoleCtrlHandler (NULL, TRUE);
    GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
#endif
    g_subprocess_wait(process, NULL, NULL);
    uintptr_t retval;
    pthread_join (*thread_id, (gpointer *)&retval);
    manage_node_error (retval);
    process = NULL;
  }
}

static void
example_app_activate (GApplication *app)
{
  ExampleAppWindow *win;

  win = example_app_window_new (EXAMPLE_APP (app));
  
  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(cssProvider, "/org/gtk/app/styles.css");
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(cssProvider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_window_present (GTK_WINDOW (win));
}

static void
example_app_class_init (ExampleAppClass *class)
{
  G_APPLICATION_CLASS (class)->activate = example_app_activate;
}

ExampleApp *
example_app_new (void)
{

  return g_object_new (EXAMPLE_APP_TYPE,
                       "application-id", "org.gtk.exampleapp",
                       "flags", G_APPLICATION_FLAGS_NONE,
                       NULL);
}

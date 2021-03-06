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

struct _GulupuApp
{
  GtkApplication parent;
};

G_DEFINE_TYPE(GulupuApp, gulupu_app, GTK_TYPE_APPLICATION);

static GulupuApp *g_app = NULL;

static void
gulupu_app_init (GulupuApp *app)
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
       {
       gulupu_quit();
       return; // shouldn't be necessary.
       }
     }
    gtk_widget_destroy (dialog);
    }

  else if (error == ERR_INVALID_ADDR)
    {
    clear_address ();
    }
}

static void
process_init_failed_dialog ()
{
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *dialog = gtk_message_dialog_new (get_window (),
                                    flags,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "Are you sure to have choosen the kulupu executable ?");
   GtkWidget *headerbar = gtk_header_bar_new ();
   gtk_widget_set_visible (headerbar, TRUE);
   gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (headerbar),
                                         TRUE);
   gtk_header_bar_set_title (GTK_HEADER_BAR (headerbar),
                             "Kulupu failed to start");
   gtk_window_set_titlebar (GTK_WINDOW (dialog), headerbar);
   gtk_dialog_run (GTK_DIALOG (dialog));
   gtk_widget_destroy (dialog);
}

static void
debug_dialog (char *string)
{
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *dialog = gtk_message_dialog_new (get_window (),
                                    flags,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    string);
   GtkWidget *headerbar = gtk_header_bar_new ();
   gtk_widget_set_visible (headerbar, TRUE);
   gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (headerbar),
                                         TRUE);
   gtk_header_bar_set_title (GTK_HEADER_BAR (headerbar),
                             "Debug");
   gtk_window_set_titlebar (GTK_WINDOW (dialog), headerbar);
   gtk_dialog_run (GTK_DIALOG (dialog));
   gtk_widget_destroy (dialog);
}

gboolean
manage_node_thread (struct manage_node_arg arg)
{
  if (FALSE) debug_dialog ("");
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
//                                           "2>&1",
                                           NULL
                                         };

  const gchar * const base_argv_no_addr[] = {
"cmd", "/c",
                                              exe,
                                              "--validator",
                                              "--threads",
                                              threads,
//                                              "2>&1",
                                              NULL
                                            };

  if (strlen (address))
    base_argv = base_argv_addr;
  else
    base_argv = base_argv_no_addr;

  GError *error = NULL;
  process = g_subprocess_newv(base_argv,
                              G_SUBPROCESS_FLAGS_STDERR_PIPE,
                              &error);
  if (process == NULL)
    {
		debug_dialog (error->message);
    process_init_failed_dialog();
    return FALSE;
    }
			 
  if (pthread_create(thread_id, NULL, (void * (*)(void *))thread_loop, process))
    GULUPU_EXIT(FALSE);

  struct manage_node_arg *arg_cb = g_malloc (sizeof(struct manage_node_arg));
  *arg_cb = (struct manage_node_arg){TH_STOP, *thread_id, arg.current_tid, arg.switcher, FALSE};
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
  return TRUE;
}

static void
gulupu_app_activate (GApplication *app)
{
  GulupuAppWindow *win;

  win = gulupu_app_window_new (GULUPU_APP (app));
  
  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(cssProvider, "/org/gtk/app/styles.css");
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(cssProvider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_window_present (GTK_WINDOW (win));
}

static void
gulupu_app_class_init (GulupuAppClass *class)
{
  G_APPLICATION_CLASS (class)->activate = gulupu_app_activate;
}

GulupuApp *
gulupu_app_new (void)
{

  return g_object_new (GULUPU_APP_TYPE,
                       "application-id", "org.gtk.gulupuapp",
                       "flags", G_APPLICATION_FLAGS_NONE,
                       NULL);
}

void
gulupu_quit (void)
{
  if (g_app != NULL) g_application_quit (G_APPLICATION (g_app));
}

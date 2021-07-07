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

#include "app.h"
#include "appwin.h"
#include "../logic/config.h"

struct _GulupuAppWindow
{
  GtkApplicationWindow parent;

  GtkWidget *stack;
  GtkWidget *version_entry;
  GtkWidget *name_entry;
  GtkWidget *peers_entry;
  GtkWidget *best_block_entry;
  GtkWidget *local_hashrate_entry;
  GtkWidget *network_hashrate_entry;
  GtkWidget *database_entry;
  GtkWidget *identity_entry;
  GtkWidget *threads_entry;
  GtkWidget *address_entry;
  GtkWidget *target_entry;
  GtkWidget *status_entry;

  GtkWidget *node_switch;
  
  GtkWidget *mine_at_start_switch;

  GtkWidget *kulupu_exe_entry;
  GtkWidget *kulupu_exe_button;

  GtkWidget *header_image;



  GtkWidget *node_warnings_frame_content;
  GtkWidget *warnings_frame_content;
  GtkWidget *logs_frame_content;
};

G_DEFINE_TYPE(GulupuAppWindow, gulupu_app_window, GTK_TYPE_APPLICATION_WINDOW);

GulupuAppWindow *g_window = NULL;

GtkWindow * const get_window ()
{
  return GTK_WINDOW (g_window);
}

static gboolean
node_switch_cb (GtkSwitch  *switcher,
                gboolean    state,
                gpointer    user_data)
{
  static pthread_t thread_id;
  if(state)
    {
    manage_node_thread ((struct manage_node_arg){TH_START, 0, &thread_id, switcher});
    gtk_switch_set_state (switcher, TRUE);
    } 
  else
    {
    manage_node_thread ((struct manage_node_arg){TH_STOP, 0, &thread_id, switcher});
    gtk_switch_set_state (switcher, FALSE);
    }
  return TRUE;
}

void
node_switch_wrap_cb (GObject *source_object,
                     GAsyncResult *res,
                     gpointer user_data)
{
  struct manage_node_arg *arg = user_data;

  if (*(arg->current_tid) == arg->tid)
    node_switch_cb (GTK_SWITCH (arg->switcher), FALSE, NULL);

  g_free(user_data);
}

/*Signal handler for the "state-set" signal of the Switch*/
static gboolean
mine_at_start_switch_cb (GtkSwitch  *switcher,
                         gboolean    state,
                         gpointer    user_data)
{
  if(state)
    {
    save_config (&(struct setting){"Gulupu", "mine_at_start", "yes"},
                 1,
                 TRUE);
    gtk_switch_set_state (switcher, TRUE);
    } 
  else 
    {
    save_config (&(struct setting){"Gulupu", "mine_at_start", "no"},
                 1,
                 TRUE);	
    gtk_switch_set_state (switcher, FALSE);
    }
  return TRUE;
}

static void 
kulupu_exe_select_cb (GtkButton *button, GulupuAppWindow *win)
{
  GtkWindow *parent_window = GTK_WINDOW(win);
  GtkWidget *dialog;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;

  dialog = gtk_file_chooser_dialog_new ("Select kulupu executable file",
                                        parent_window,
                                        action,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Open",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);

  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT)
    {
    gchar *filename;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    filename = gtk_file_chooser_get_filename (chooser);
    save_config (&(struct setting){"Kulupu", "exe", filename},
                 1,
                 TRUE);
    gtk_entry_set_text(GTK_ENTRY(win->kulupu_exe_entry), filename);
    g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

static void
thread_changed_cb (GtkEditable *editable,
               gpointer     user_data)
  {
  g_autofree gchar *threads = gtk_editable_get_chars (editable, 0, -1);
  save_config (&(struct setting){"Kulupu", "threads", threads},
               1,
               TRUE);
   return;  
  }

static void
thread_deleted_cb (GtkEditable *editable,
                   int          start_pos,
                   int          end_pos,
                   gpointer     user_data)
{
#define MAXPASTESIZE 1000 // to hide the warning
  g_autofree gchar *threads = gtk_editable_get_chars (editable, 0, -1);
  threads[start_pos] = '\0';
  strncat (threads, threads + end_pos, MAXPASTESIZE);
  
  for (gint read = 0, accept_zero = FALSE;
       threads[read] != '\0';
       ++read)
    {
    if ((threads[read] >= '1' && threads[read] <= '9') || (accept_zero && (threads[read] == '0')))
      {
      if (!accept_zero) accept_zero = TRUE;
      }
    else
      {
      g_signal_stop_emission_by_name (editable, "delete-text");
      return;
      }
    }

  return;
#undef MAXPASTESIZE
}

static void
thread_inserted_cb (GtkEditable *editable,
                    char        *new_text,
                    int          new_text_length,
                    gpointer     position,
                    gpointer     user_data)
{	
  g_autofree gchar *old_text = gtk_editable_get_chars (editable, 0, -1);

  for (gint write = 0, read = 0, accept_zero = (*old_text >= '1' && *old_text <= '9') && *(char *)position;
       new_text[read] != '\0' || ((new_text[write] = new_text[read]) && FALSE);
       ++read)
    {
    if ((new_text[read] >= '1' && new_text[read] <= '9') || (accept_zero && (new_text[read] == '0')))
      {
      new_text[write++] = new_text[read];
      if (!accept_zero) accept_zero = TRUE;
      }
    }

  return;
}

static void
gulupu_app_window_init (GulupuAppWindow *win)
{

  gtk_widget_init_template (GTK_WIDGET (win));
  
  g_window = win;
  
  g_signal_connect (GTK_SWITCH (win->node_switch), 
                    "state-set", 
                    G_CALLBACK (node_switch_cb), 
                    NULL);
					
  g_signal_connect (GTK_SWITCH (win->mine_at_start_switch), 
                    "state-set", 
                    G_CALLBACK (mine_at_start_switch_cb), 
                    win);

  g_signal_connect (GTK_BUTTON (win->kulupu_exe_button), 
                    "clicked",
                    G_CALLBACK (kulupu_exe_select_cb),
                    win);

  g_signal_connect (GTK_ENTRY (win->threads_entry), 
                    "insert-text",
                    G_CALLBACK (thread_inserted_cb),
                    win);

  g_signal_connect (GTK_ENTRY (win->threads_entry), 
                    "changed",
                    G_CALLBACK (thread_changed_cb),
                    win);
					
  g_signal_connect (GTK_ENTRY (win->threads_entry), 
                    "delete-text",
                    G_CALLBACK (thread_deleted_cb),
                    win);

  gtk_image_set_from_resource (GTK_IMAGE (win->header_image),
                               "/org/gtk/app/icon.png");
						   
  g_autofree const gchar *executable = load_config (&(struct setting){"Kulupu", "exe", NULL}).value;
  gtk_entry_set_text (GTK_ENTRY(win->kulupu_exe_entry), executable);

  g_autofree const gchar *threads = load_config (&(struct setting){"Kulupu", "threads", NULL}).value;
  gtk_entry_set_text (GTK_ENTRY(win->threads_entry), threads);

  g_autofree const gchar *address = load_config (&(struct setting){"Kulupu", "address", NULL}).value;
  gtk_entry_set_text (GTK_ENTRY(win->address_entry), address);
  
  g_autofree const gchar *mine_at_start_active = load_config (&(struct setting){"Gulupu", "mine_at_start", NULL}).value;
  const gboolean mine_at_start_maybe = YES_NO_TO_BOOL(mine_at_start_active);
  gtk_switch_set_active (GTK_SWITCH (win->mine_at_start_switch),
                         mine_at_start_maybe);
  if (mine_at_start_maybe)
    {
    node_switch_cb (GTK_SWITCH(win->node_switch), TRUE, win);
    }
}

static void
gulupu_app_window_class_init (GulupuAppWindowClass *class)
{
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               "/org/gtk/app/window.ui");

  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, stack);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, version_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, name_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, peers_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, best_block_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, local_hashrate_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, network_hashrate_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, database_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, identity_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, threads_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, address_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, target_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, status_entry);

  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, node_switch);
  
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, mine_at_start_switch);
  
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, kulupu_exe_entry);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, kulupu_exe_button);

  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, header_image);

  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, node_warnings_frame_content);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, warnings_frame_content);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), GulupuAppWindow, logs_frame_content);
}

GulupuAppWindow *
gulupu_app_window_new (GulupuApp *app)
{
  return g_object_new (GULUPU_APP_WINDOW_TYPE, "application", app, NULL);
}

void
gulupu_app_window_open (GulupuAppWindow *win,
                         GFile *file)
{
  gchar *basename;
  GtkWidget *scrolled, *view;
  gchar *contents;
  gsize length;

//  priv = gulupu_app_window_get_instance_private (win);
  basename = g_file_get_basename (file);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolled);
  gtk_widget_set_hexpand (scrolled, TRUE);
  gtk_widget_set_vexpand (scrolled, TRUE);
  view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
  gtk_widget_show (view);
  gtk_container_add (GTK_CONTAINER (scrolled), view);
  gtk_stack_add_titled (GTK_STACK (win->stack), scrolled, basename, basename);

  if (g_file_load_contents (file, NULL, &contents, &length, NULL, NULL))
    {
      GtkTextBuffer *buffer;

      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
      gtk_text_buffer_set_text (buffer, contents, length);
      g_free (contents);
    }

  g_free (basename);
}

gboolean
update_informations (struct information *informations)
{
  if ((gchar)*informations[INFO_VERSION].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->version_entry), (gchar*)informations[INFO_VERSION].token);
  if ((gchar)*informations[INFO_NAME].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->name_entry), (gchar*)informations[INFO_NAME].token);
  if ((gchar)*informations[INFO_PEERS].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->peers_entry), (gchar*)informations[INFO_PEERS].token);
  if ((gchar)*informations[INFO_BEST_BLOCK].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->best_block_entry), (gchar*)informations[INFO_BEST_BLOCK].token);
  if ((gchar)*informations[INFO_LOCAL_HASHRATE].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->local_hashrate_entry), (gchar*)informations[INFO_LOCAL_HASHRATE].token);
  if ((gchar)*informations[INFO_NETWORK_HASHRATE].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->network_hashrate_entry), (gchar*)informations[INFO_NETWORK_HASHRATE].token);
  if ((gchar)*informations[INFO_DATABASE].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->database_entry), (gchar*)informations[INFO_DATABASE].token);
  if ((gchar)*informations[INFO_IDENTITY].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->identity_entry), (gchar*)informations[INFO_IDENTITY].token);
  if ((gchar)*informations[INFO_ADDRESS].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->address_entry), (gchar*)informations[INFO_ADDRESS].token);
  if ((gchar)*informations[INFO_TARGET].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->target_entry), (gchar*)informations[INFO_TARGET].token);
  if ((gchar)*informations[INFO_MINING].token != '\0') gtk_entry_set_text(GTK_ENTRY(g_window->status_entry), (gchar*)informations[INFO_MINING].token);
  //g_free (informations);
  return FALSE;
}

gboolean
save_address (struct information *informations)
{

  save_config (&(struct setting){"Kulupu", "address", informations[INFO_ADDRESS].token},
               1,
               TRUE);

  return FALSE;
}

void
clear_address ()
{
  save_config (&(struct setting){"Kulupu", "address", ""},
               1,
               TRUE);
			   
  gtk_entry_set_text(GTK_ENTRY(g_window->address_entry), "");
}

gboolean
err_key_not_found (void *argv)
{

  clear_address ();

  node_switch_cb (GTK_SWITCH (g_window->node_switch), FALSE, NULL);

  return FALSE;
}

static void
add_text_bottom (GtkTextView *textview,
                 gchar *line,
                 gboolean free,
                 gboolean scroll_down)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
  
  GtkTextIter end;
  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_insert (buffer, &end, line, -1);
  //gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_view_scroll_to_iter (textview,
                                &end,
                                0,
                                TRUE,
                                0,
                                0);
  if (free) g_free (line);
}

gboolean
update_messages (void *line)
{
  if PARSER_GET_WARN(line)
  {
    add_text_bottom (GTK_TEXT_VIEW(g_window->node_warnings_frame_content),
                     PARSER_GET_ADDRESS(line),
                     FALSE,
                     FALSE);


    add_text_bottom (GTK_TEXT_VIEW(g_window->warnings_frame_content),
                     PARSER_GET_ADDRESS(line),
                     FALSE,
                     FALSE);

  }

  add_text_bottom (GTK_TEXT_VIEW(g_window->logs_frame_content),
                   PARSER_GET_ADDRESS(line),
                   TRUE,
                   FALSE);

  return FALSE;
}

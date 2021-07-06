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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "parser_thread.h"
#include "../gui/appwin.h"

#define DEBUG_PARSING 1

#define PARSER_SET_WARN(ADDRESS, BOOL) ((gpointer)(((uintptr_t)ADDRESS)|(!!BOOL)))

struct normal_message_patterns {
#define DIM_STRUCT_NORMAL_MESSAGE ((sizeof(struct normal_message_patterns) - sizeof(gint))/sizeof(gchar *))
  const gint dim;
  const gchar * pattern1;
  const gchar * pattern2;
  const gchar * pattern3;
  const gchar * pattern4;
  const gchar * pattern5;
  const gchar * pattern6;
  const gchar * pattern7;
};

static struct information *
get_informations ()
{
  struct information * informations = g_malloc (INFO_COUNT * sizeof(struct information));
  informations[INFO_VERSION] = (struct information) {"%*[^v]version %[^-]", INFO_VERSION, INFO_COUNT, INFO_COUNT, {'\0'}};
  informations[INFO_NAME] = (struct information) {"%*[^N]Node name:%s", INFO_NAME, INFO_COUNT, INFO_COUNT, {'\0'}};
  informations[INFO_TARGET] = (struct information) {"%*[^t]target=%s", INFO_TARGET, INFO_COUNT, INFO_PEERS, {'\0'}};
  informations[INFO_PEERS] = (struct information) {"%*[^(](%s", INFO_PEERS, INFO_COUNT, INFO_BEST_BLOCK, {'\0'}};
  informations[INFO_BEST_BLOCK] = (struct information) {"%*[^)]), best:%s", INFO_BEST_BLOCK, INFO_TARGET, INFO_COUNT, {'\0'}};
  informations[INFO_LOCAL_HASHRATE] = (struct information) {"%*[^L]Local hashrate:%[^,]", INFO_LOCAL_HASHRATE, INFO_COUNT, INFO_NETWORK_HASHRATE, {'\0'}};
  informations[INFO_NETWORK_HASHRATE] = (struct information) {"%*[^n]network hashrate:%[^,]", INFO_NETWORK_HASHRATE, INFO_COUNT, INFO_MINING, {'\0'}};
  informations[INFO_MINING] = (struct information) {"%*[^x]xpected %[^(]", INFO_MINING, INFO_COUNT, INFO_COUNT, {'\0'}};
  informations[INFO_DATABASE] = (struct information) {"%*[^D]Database: %*s %*s %[^,]", INFO_DATABASE, INFO_COUNT, INFO_COUNT, {'\0'}};
  informations[INFO_IDENTITY] = (struct information) {"%*[^L]Local node identity is: %[^,]", INFO_IDENTITY, INFO_COUNT, INFO_COUNT, {'\0'}};
  informations[INFO_ADDRESS] = (struct information) {"%*[^G]Generated a mining key with address:%s", INFO_ADDRESS, INFO_COUNT, INFO_COUNT, {'\0'}};
  informations[ERR_DB] = (struct information) {"%*[^e]ervice(Client(StateDatabase(\"Expected pruning mode: constrained\"))%s", ERR_DB, INFO_COUNT, INFO_COUNT, {'\0'}};
  informations[ERR_KEY_NOT_FOUND] = (struct information) {"%*[^M]Mining failed: Consensus(Other(\"Unable to mine: key not found in keystore\")%s", ERR_KEY_NOT_FOUND, INFO_COUNT, INFO_COUNT, {'\0'}};
  informations[ERR_INVALID_ADDR] = (struct information) {"%*[^e]ervice(Other(\"Invalid author address\")%s", ERR_INVALID_ADDR, INFO_COUNT, INFO_COUNT, {'\0'}};
  return informations;
}

static enum info_types
get_token (struct information * informations, const char * string)
{
  enum info_types ret = INFO_COUNT;
  for(gint i = 0; i < INFO_COUNT; ++i)
    {
    if(sscanf (string, informations[i].pattern, &informations[i].token) == 1)
      {
      enum info_types next = informations[i].next;
      if(ret == INFO_COUNT) 
        ret = informations[i].type;
      if (informations[i].propagate < ret && 
          informations[i].propagate != INFO_COUNT)
        strcpy (informations[informations[i].propagate].token, informations[i].token);
      if(next == INFO_COUNT)
        return ret;
      else
        i = next - 1;
      } 
	else if(ret != INFO_COUNT)
      for (enum info_types temp = ret;
           temp < INFO_COUNT ||
		   ((ret = INFO_COUNT) && FALSE);
           temp = informations[temp].next)
         {
           informations[temp].token[0] = '\0';
           if (informations[temp].propagate != INFO_COUNT)
             informations[informations[temp].propagate].token[0] = '\0';
         }
  }
  return INFO_COUNT;
}

static const struct normal_message_patterns *
get_normal_message_patterns ()
{
  static const struct normal_message_patterns patterns =
    { DIM_STRUCT_NORMAL_MESSAGE,
      "%*[^b]best: %*s %*s finalized %s",
      "%*[^S]Starting consensus session on top of parent %s",
      "%*[^P]Prepared block for proposing %s",
      "%*[^S]Successfully mined block on top %s",
      "%*[^✨]✨ Imported %s",
      "%*[^L]Local hashrate:%s",
      "%*[^⚙️]⚙️ Syncing %s" };
  return &patterns;

}

static gboolean
is_warn (const char * string, const struct normal_message_patterns *patterns)
{
  static gchar buff[BUFF_SIZE];
  for(gint i = 0; i < patterns->dim; ++i)
    {
    if(sscanf (string, *(&(patterns->pattern1) + i) , buff) == 1)
      {
#if DEBUG_PARSING
        g_print("bool:TRUE pattern:%s token:%s /", *(&(patterns->pattern1) + i), buff);  
#endif
        return FALSE;
      } 
    }
#if DEBUG_PARSING
  g_print ("bool:FALSE /");
#endif
  return TRUE;
}

static void
print_parser_debug (gchar * line, enum info_types type, struct information * informations)
{
  g_print ("%s", line);
    for(enum info_types type_t = type;
      type_t != INFO_COUNT;
      type_t = informations[type_t].next)
      {
        g_print (" type: %d, value: %s ", type_t, informations[type_t].token);
      }
  g_print("\n");
}

static inline gchar *
trailing_new_line (gchar * line)
{
  size_t offset = strlen(line) - 1;
  if (offset)
	  line[offset] = '\n';
  return line;
}

// 0: success, INFO_COUNT: generic error, ERR_xxxx: particular error
void *
thread_loop (GSubprocess *process)
{    
  GInputStream * stream = g_subprocess_get_stderr_pipe (process);
  if (stream == NULL) return (gpointer)(uintptr_t)INFO_COUNT;

  struct information * informations = get_informations ();

  gsize buffer_size = BUFF_SIZE;
  gssize read = 0;
  gchar * buffer = g_malloc (BUFF_SIZE * sizeof(gchar));
  gchar * available_buffer = buffer;
  time_t last_refresh = {0}, now;
			
  while(TRUE)
    {
    read = g_input_stream_read(stream, available_buffer, buffer_size - (buffer - available_buffer), NULL, NULL);
    if (!(read > 0)) return (gpointer)(uintptr_t)INFO_COUNT;

    gchar * end_of_record = available_buffer + read;
    *end_of_record = '\0';

	gchar * last_line = NULL;
    
    for(gchar * line = strtok (buffer, "\r\n");
         line != NULL && line <= end_of_record;
         last_line = line, line = strtok (NULL, "\r\n"))
      {

	  enum info_types type = get_token(informations, line);

      g_idle_add ((GSourceFunc) update_messages, PARSER_SET_WARN(trailing_new_line (strdup (line)), is_warn (line, get_normal_message_patterns ())));

#if DEBUG_PARSING
      print_parser_debug (line, type, informations);
#endif

      if (type == ERR_DB) return (gpointer)(uintptr_t)ERR_DB;
      if (type == ERR_INVALID_ADDR) return (gpointer)(uintptr_t)ERR_INVALID_ADDR;
      if (type == ERR_KEY_NOT_FOUND)
        {
        g_idle_add ((GSourceFunc) err_key_not_found, NULL);
        return (gpointer)(uintptr_t)ERR_KEY_NOT_FOUND;
        }

      if (type == INFO_ADDRESS) g_idle_add ((GSourceFunc) save_address, informations);


      time (&now);
      if(difftime (now, last_refresh) > REFRESH_TIME)
        {
        g_idle_add ((GSourceFunc) update_informations, informations);
        last_refresh = now;
        }
      }
	
    if(last_line)
      {
      available_buffer = buffer;
      strcpy(buffer, last_line + strlen (last_line));
      } 
	else
      available_buffer += read;
    }

  return (gpointer)(uintptr_t)0;
}
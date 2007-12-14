/*  xfce4-places-plugin
 *
 *  This file provides support wrappers for the structs defined in model.h.
 *
 *  Copyright (c) 2007 Diego Ongaro <ongardie@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "model.h"

#include <libxfce4util/libxfce4util.h>

/********** PlacesBookmarkAction **********/

inline PlacesBookmarkAction*
places_bookmark_action_new(gchar *label)
{
    PlacesBookmarkAction *action;

    action = g_new0(PlacesBookmarkAction, 1);
    action->label = label;

    return action;
}

inline void
places_bookmark_action_free(PlacesBookmarkAction *act)
{
    g_assert(act != NULL);

    if(act->free != NULL)
        act->free(act);
    else
        g_free(act);
}

inline void
places_bookmark_action_call(PlacesBookmarkAction *act)
{
    g_assert(act != NULL);

    if(act->action != NULL)
        act->action(act);
}

/********** PlacesBookmark **********/

#if defined(DEBUG) && (DEBUG > 0)
static int bookmarks = 0;
#endif

inline PlacesBookmark*
places_bookmark_create(gchar *label)
{
    PlacesBookmark *bookmark;

    g_assert(label != NULL);

    bookmark = g_new0(PlacesBookmark, 1);
    bookmark->label = label;

    DBG("bookmarks: %02d %p %s", bookmarks++, bookmark, label);

    return bookmark;
}

static inline void
places_bookmark_actions_free(GList *actions)
{
    while(actions != NULL){
        if(actions->data != NULL)
            places_bookmark_action_free((PlacesBookmarkAction*) actions->data);
        actions = actions->next;
    }
    g_list_free(actions);
}

inline void
places_bookmark_free(PlacesBookmark *bookmark)
{
    g_assert(bookmark != NULL);

    DBG("bookmarks: %02d %p", --bookmarks, bookmark);

    if(bookmark->primary_action != NULL){

        /* don't double-free */
        if(g_list_find(bookmark->actions, bookmark->primary_action) == NULL)
            places_bookmark_action_free(bookmark->primary_action);

        bookmark->primary_action = NULL;
    }

    if(bookmark->actions != NULL){
        places_bookmark_actions_free(bookmark->actions);
        bookmark->actions = NULL;
    }

    if(bookmark->free != NULL)
        bookmark->free(bookmark);
    else
        g_free(bookmark);
}

/********** PlacesBookmarkGroup **********/

inline GList*
places_bookmark_group_get_bookmarks(PlacesBookmarkGroup *pbg)
{
    return pbg->get_bookmarks(pbg);
}

inline gboolean
places_bookmark_group_changed(PlacesBookmarkGroup *pbg)
{
    return pbg->changed(pbg);
}

inline void
places_bookmark_group_finalize(PlacesBookmarkGroup *pbg)
{
    pbg->finalize(pbg);
}

/* vim: set ai et tabstop=4: */

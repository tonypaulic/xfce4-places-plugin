/*  xfce4-places-plugin
 *
 *  Provides the widget that sits on the panel
 *
 *  Note that, while this extends GtkToggleButton, much of the gtk_button_*()
 *  functions shouldn't be used.
 *
 *  Copyright (c) 2007-2008 Diego Ongaro <ongardie@gmail.com>
 *  
 *  Some code adapted from libxfce4panel for the togglebutton configuration
 *    (xfce-panel-convenience.c)
 *    Copyright (c) 2006-2007 Jasper Huijsmans <jasper@xfce.org>
 *
 *  Some code adapted from gtk+ for the properties
 *    (gtkbutton.c)
 *    Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *    Modified by the GTK+ Team and others 1997-2001
 *
 *  May also contain code adapted from:
 *   - notes plugin
 *     panel-plugin.c - (xfce4-panel plugin for temporary notes)
 *     Copyright (c) 2006 Mike Massonnet <mmassonnet@gmail.com>
 *
 *   - xfdesktop menu plugin
 *     desktop-menu-plugin.c - xfce4-panel plugin that displays the desktop menu
 *     Copyright (C) 2004 Brian Tarricone, <bjt23@cornell.edu>
 *  
 *   - launcher plugin
 *     launcher.c - (xfce4-panel plugin that opens programs)
 *     Copyright (c) 2005-2007 Jasper Huijsmans <jasper@xfce.org>
 *     Copyright (c) 2006-2007 Nick Schermer <nick@xfce.org>
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

#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-hvbox.h>
#include <string.h>

#define EXO_API_SUBJECT_TO_CHANGE
#include <exo/exo.h>

#include "button.h"

#define BOX_SPACING 4

enum
{
    PROP_0,
    PROP_PIXBUF_FACTORY,
    PROP_LABEL
};

static void
places_button_dispose(GObject*);

static void
places_button_resize(PlacesButton*);

static void
places_button_orientation_changed(XfcePanelPlugin*, GtkOrientation, PlacesButton*);

static gboolean
places_button_size_changed(XfcePanelPlugin*, gint size, PlacesButton*);

static void
places_button_theme_changed(PlacesButton*);

G_DEFINE_TYPE(PlacesButton, places_button, GTK_TYPE_TOGGLE_BUTTON);

void
places_button_set_label(PlacesButton *self, const gchar *label)
{
    g_assert(PLACES_IS_BUTTON(self));

    if (label == NULL && self->label_text == NULL)
        return;

    if (label != NULL && self->label_text != NULL &&
        strcmp(label, self->label_text) == 0) {
        return;
    }

    DBG("new label text: %s", label);

    if (self->label_text != NULL)
        g_free(self->label_text);
    
    self->label_text = g_strdup(label);

    places_button_resize(self);
}

void
places_button_set_pixbuf_factory(PlacesButton *self,
                                 places_button_image_pixbuf_factory *factory)
{
    g_assert(PLACES_IS_BUTTON(self));

    if (self->pixbuf_factory == factory)
        return;

    DBG("new pixbuf factory: %p", factory);
    self->pixbuf_factory = factory;

    places_button_resize(self);
}

static void
places_button_set_property(GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    PlacesButton *self;

    self = PLACES_BUTTON(object);

    switch (property_id) {

        case PROP_PIXBUF_FACTORY:
            places_button_set_pixbuf_factory(self, g_value_get_pointer(value));
            break;
        case PROP_LABEL:
            places_button_set_label(self, g_value_get_string(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

places_button_image_pixbuf_factory*
places_button_get_pixbuf_factory(PlacesButton *self)
{
    g_assert(PLACES_IS_BUTTON(self));

    DBG("returning %p", self->pixbuf_factory);
    return self->pixbuf_factory;
}


const gchar*
places_button_get_label(PlacesButton *self)
{
    g_assert(PLACES_IS_BUTTON(self));

    DBG("returning %s", self->label_text);
    return self->label_text;
}

static void
places_button_get_property(GObject      *object,
                           guint         property_id,
                           GValue       *value,
                           GParamSpec   *pspec)
{
    PlacesButton *self;

    self = PLACES_BUTTON(object);

    switch (property_id) {

        case PROP_PIXBUF_FACTORY:
            g_value_set_pointer(value, places_button_get_pixbuf_factory(self));
            break;

        case PROP_LABEL:
            g_value_set_string(value, places_button_get_label(self));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}



static void
places_button_class_init(PlacesButtonClass *klass)
{
    GObjectClass *gobject_class;
    
    gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = places_button_dispose;
  
    gobject_class->set_property = places_button_set_property;
    gobject_class->get_property = places_button_get_property;

    g_object_class_install_property(gobject_class,
        PROP_LABEL,
        g_param_spec_string("label",
            "Label",
            "Button text",
            NULL,
            EXO_PARAM_READWRITE));

    g_object_class_install_property(gobject_class,
        PROP_PIXBUF_FACTORY,
        g_param_spec_object("pixbuf-factory",
            "Pixbuf factory",
            "Factory to create icons for image to appear next to button text",
            GTK_TYPE_WIDGET,
            EXO_PARAM_READWRITE));
}

static void
places_button_init(PlacesButton *self)
{
    self->plugin = NULL;
    self->box = NULL;
    self->label = NULL;
    self->image = NULL;
    self->plugin_size = -1;
}

static void
places_button_construct(PlacesButton *self, XfcePanelPlugin *plugin)
{
    GtkOrientation orientation;

    g_assert(XFCE_IS_PANEL_PLUGIN(plugin));

    g_object_ref(plugin);
    self->plugin = plugin;

    /* from libxfce4panel */
    GTK_WIDGET_UNSET_FLAGS(self, GTK_CAN_DEFAULT|GTK_CAN_FOCUS);
    gtk_button_set_relief(GTK_BUTTON(self), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click(GTK_BUTTON(self), FALSE);

    orientation = xfce_panel_plugin_get_orientation(self->plugin);
    self->box = xfce_hvbox_new(orientation, FALSE, BOX_SPACING);
    gtk_container_set_border_width(GTK_CONTAINER(self->box), 0);
    gtk_container_add(GTK_CONTAINER(self), self->box);
    gtk_widget_show(self->box);

    places_button_resize(self);

    g_signal_connect(G_OBJECT(plugin), "orientation-changed",
                     G_CALLBACK(places_button_orientation_changed), self);
    g_signal_connect(G_OBJECT(plugin), "size-changed",
                     G_CALLBACK(places_button_size_changed), self);

    self->style_set_id = g_signal_connect(G_OBJECT(self), "style-set",
                     G_CALLBACK(places_button_theme_changed), NULL);
    self->screen_changed_id = g_signal_connect(G_OBJECT(self), "screen-changed",
                     G_CALLBACK(places_button_theme_changed), NULL);

}


GtkWidget*
places_button_new(XfcePanelPlugin *plugin)
{
    PlacesButton *button;

    g_assert(XFCE_IS_PANEL_PLUGIN(plugin));

    button = (PlacesButton*) g_object_new(PLACES_TYPE_BUTTON, NULL);
    places_button_construct(button, plugin);

    return (GtkWidget*) button;
}

static void
places_button_dispose(GObject *object)
{
    PlacesButton *self = PLACES_BUTTON(object);

    if (self->style_set_id != 0) {
        g_signal_handler_disconnect(self, self->style_set_id);
        self->style_set_id = 0;
    }

    if (self->screen_changed_id != 0) {
        g_signal_handler_disconnect(self, self->screen_changed_id);
        self->screen_changed_id = 0;
    }

    if (self->plugin != NULL) {
        g_object_unref(self->plugin);
        self->plugin = NULL;
    }

    (*G_OBJECT_CLASS(places_button_parent_class)->dispose) (object);
}

static void
places_button_destroy_image(PlacesButton *self)
{
    if (self->image != NULL) {
        gtk_widget_destroy(self->image);
        g_object_unref(self->image);
        self->image = NULL;
    }
}
static void
places_button_resize_image(PlacesButton *self, gint new_size, gint *width, gint *height)
{
    GdkPixbuf *icon;

    *width = 0;
    *height = 0;

    if (self->pixbuf_factory == NULL) {
        places_button_destroy_image(self);
        return;
    }

    icon = self->pixbuf_factory(new_size);
    
    if (G_UNLIKELY(icon == NULL)) {
        DBG("Could not load icon for button");
        places_button_destroy_image(self);
        return;
    }

    *width  = gdk_pixbuf_get_width(icon);
    *height = gdk_pixbuf_get_height(icon);
     
    if (self->image == NULL) {
            self->image = g_object_ref(gtk_image_new_from_pixbuf(icon));
            gtk_box_pack_start(GTK_BOX(self->box), self->image, TRUE, TRUE, 0);
            gtk_widget_show(self->image);
    }
    else
            gtk_image_set_from_pixbuf(GTK_IMAGE(self->image), icon);

    g_object_unref(G_OBJECT(icon));
}

static void
places_button_destroy_label(PlacesButton *self)
{
    if (self->label != NULL) {
        gtk_widget_destroy(self->label);
        g_object_unref(self->label);
        self->label = NULL;
    }
}

static void
places_button_resize_label(PlacesButton *self, gint *width, gint *height)
{
    GtkRequisition req;

    *width = 0;
    *height = 0;

    if (self->label_text == NULL) {
        places_button_destroy_label(self);
        return;
    }

    if (self->label == NULL) {
        self->label = g_object_ref(gtk_label_new(self->label_text));
        gtk_box_pack_end(GTK_BOX(self->box), self->label, TRUE, TRUE, 0);
        gtk_widget_show(self->label);
    }
    else
        gtk_label_set_text(GTK_LABEL(self->label), self->label_text);


    gtk_widget_size_request(self->label, &req);
    *width = req.width;
    *height = req.height;

}


static void
places_button_resize(PlacesButton *self)
{
    gboolean show_image, show_label;
    gint new_size, image_size;
    GtkOrientation orientation;

    gint total_width,  total_height;
    gint image_width,  image_height;
    gint label_width,  label_height;
    gint button_width, button_height;
    gint box_width,    box_height;

    if (self->plugin == NULL)
        return;

    new_size = xfce_panel_plugin_get_size(self->plugin);
    self->plugin_size = new_size;
    DBG("Panel size: %d", new_size);
    
    orientation = xfce_panel_plugin_get_orientation(self->plugin);

    show_image = self->pixbuf_factory != NULL;
    show_label = self->label_text != NULL;

    total_width  = 0;
    total_height = 0;

    /* these will be added into totals later */
    button_width  = 2 + 2 * ((GtkWidget*) self)->style->xthickness;
    button_height = 2 + 2 * ((GtkWidget*) self)->style->ythickness;
    
    /* image */
    image_size = new_size - MAX(button_width, button_height);
    /* TODO: could check if anything changed
     * (though it's hard to know if the icon theme changed) */
    places_button_resize_image(self,
                               image_size,
                               &image_width, &image_height);
    show_image = self->image != NULL;
    if (show_image) {
        image_width  = MAX(image_width,  image_size);
        image_height = MAX(image_height, image_size);
        total_width  += image_width;
        total_height += image_height;
    }

    /* label */
    /* TODO: could check if anything changed */
    places_button_resize_label(self,
                               &label_width, &label_height);
    show_label = self->label != NULL;

    if (show_label) {
        if (orientation == GTK_ORIENTATION_HORIZONTAL) {
            total_width += label_width;
            total_height = MAX(total_height, label_height);
        }
        else {
            total_width = MAX(total_width, label_width);
            total_height += label_height;
        }
    }
    /* at this point, total width and height reflect just image and label */
    /* now, add on the button and box overhead */
    total_width  += button_width;
    total_height += button_height;

    box_width  = 0;
    box_height = 0;
    if (show_image && show_label) {
    
        if (orientation == GTK_ORIENTATION_HORIZONTAL)
            box_width  = BOX_SPACING;
        else
            box_height = BOX_SPACING;
            
        total_width  += box_width;
        total_height += box_height;
    }
    
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        total_height = MAX(total_height, new_size);
    else
        total_width  = MAX(total_width,  new_size);

    DBG("width=%d, height=%d", total_width, total_height);
    gtk_widget_set_size_request((GtkWidget*) self, total_width, total_height);
}

static void
places_button_orientation_changed(XfcePanelPlugin *plugin, GtkOrientation orientation, PlacesButton *self)
{
    DBG("orientation changed");
    xfce_hvbox_set_orientation(XFCE_HVBOX(self->box), orientation);
    places_button_resize(self);
}

static gboolean
places_button_size_changed(XfcePanelPlugin *plugin, gint size, PlacesButton *self)
{
    if (self->plugin_size == size)
        return TRUE;

    DBG("size changed");
    places_button_resize(self);
    return TRUE;
}

static void
places_button_theme_changed(PlacesButton *self)
{
    DBG("theme changed");
    places_button_resize(self);
}

/* vim: set ai et tabstop=4: */

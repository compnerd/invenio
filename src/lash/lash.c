/* vim: set et fdm=syntax sts=4 sw=4 ts=4 : */
/**
 * Copyright © 2010 Saleem Abdulrasool <compnerd@compnerd.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 **/

#include "lash/lash.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <X11/Xlib.h>

struct _LashKeyBinding
{
    const gchar     *string;
    LashCallback     callback;
    gpointer         user_data;

    guint            keycode;
    GdkModifierType  modifiers;
};


static GSList *bindings;

static guint ignored_modifiers[] =
{
    GDK_LOCK_MASK,
    GDK_MOD2_MASK,
    GDK_MOD3_MASK,
    GDK_LOCK_MASK | GDK_MOD2_MASK,
    GDK_LOCK_MASK | GDK_MOD3_MASK,
    GDK_MOD2_MASK | GDK_MOD3_MASK,
    GDK_LOCK_MASK | GDK_MOD2_MASK | GDK_MOD3_MASK,
};


LashKeyBinding *
lash_key_binding_new (const gchar   *string,
                      LashCallback   callback,
                      gpointer       user_data)
{
    LashKeyBinding *binding;

    binding = g_new0 (LashKeyBinding, 1);
    binding->string = g_strdup (string);
    binding->callback = callback;
    binding->user_data = user_data;

    return binding;
}

void
lash_key_binding_free (LashKeyBinding *binding)
{
    g_free ((gchar *) binding->string);
    g_free (binding);
}


static GdkFilterReturn
_handle_bindings (GdkXEvent *xevent,
                  GdkEvent  *event,
                  gpointer   data)
{
    XEvent *xev;
    GSList *iter;

    xev = (XEvent *) xevent;

    if (xev->type == KeyPress)
    {
        for (iter = bindings; iter; iter = iter->next)
        {
            LashKeyBinding *binding = (LashKeyBinding *) iter->data;

            if (xev->xkey.keycode == binding->keycode && xev->xkey.state == binding->modifiers)
            {
                (binding->callback)(binding->string, binding->user_data);
            }
        }
    }

    return GDK_FILTER_CONTINUE;
}

static void
_GrabKey(const guint            keycode,
         const GdkModifierType  modifiers)
{
    guint i;

    XGrabKey (GDK_WINDOW_XDISPLAY (gdk_get_default_root_window ()),
              keycode, modifiers,
              GDK_WINDOW_XWINDOW (gdk_get_default_root_window ()),
              True, GrabModeAsync, GrabModeAsync);

    for (i = 0; i < G_N_ELEMENTS (ignored_modifiers); i++)
        XGrabKey (GDK_WINDOW_XDISPLAY (gdk_get_default_root_window ()),
                  keycode, modifiers | ignored_modifiers[i],
                  GDK_WINDOW_XWINDOW (gdk_get_default_root_window ()),
                  True, GrabModeAsync, GrabModeAsync);
}

static void
_UngrabKey(const guint              keycode,
           const GdkModifierType    modifiers)
{
    guint i;

    XUngrabKey (GDK_WINDOW_XDISPLAY (gdk_get_default_root_window ()),
                keycode, modifiers,
                GDK_WINDOW_XWINDOW (gdk_get_default_root_window ()));

    for (i = 0; i < G_N_ELEMENTS (ignored_modifiers); i++)
        XUngrabKey (GDK_WINDOW_XDISPLAY (gdk_get_default_root_window ()),
                    keycode, modifiers | ignored_modifiers[i],
                    GDK_WINDOW_XWINDOW (gdk_get_default_root_window ()));
}

static gboolean
_map_binding (LashKeyBinding *binding,
              GdkKeymap      *keymap)
{
    GdkKeymapKey *key = NULL;
    GdkModifierType modifiers;
    guint keyval, keycode;
    gint n_keys;

    gtk_accelerator_parse (binding->string, &keyval, &modifiers);
    if (! keyval && ! modifiers)
    {
        g_printerr ("Unable to rebind '%s' with new keymap", binding->string);
        return FALSE;
    }

    if (! gtk_accelerator_valid (keyval, modifiers))
    {
        g_printerr ("Accelerator '%s' is invalid with new keymap", binding->string);
        return FALSE;
    }

    if (! gdk_keymap_get_entries_for_keyval (keymap, keyval, &key, &n_keys))
    {
        g_printerr ("Unable to get keycode for keyval %#x", keyval);
        return FALSE;
    }

    keycode = key->keycode;

    g_free (key);

    binding->keycode = keycode;
    binding->modifiers = modifiers;

    return TRUE;
}

static void
keymap_changed (GdkKeymap   *keymap,
                gpointer     user_data)
{
    GSList *binding;

    for (binding = bindings; binding; binding = binding->next)
    {
        LashKeyBinding *keybinding = (LashKeyBinding *) binding->data;

        _UngrabKey (keybinding->keycode, keybinding->modifiers);

        if (_map_binding (keybinding, keymap))
            _GrabKey(keybinding->keycode, keybinding->modifiers);
    }
}

void
lash_init (void)
{
    gdk_window_add_filter (gdk_get_default_root_window (), _handle_bindings, NULL);
    g_signal_connect (gdk_keymap_get_default (), "keys-changed", G_CALLBACK (keymap_changed), NULL);
}

void
lash_fini (void)
{
    GSList *binding;

    gdk_window_remove_filter (gdk_get_default_root_window (), _handle_bindings, NULL);

    for (binding = bindings; binding; binding = binding->next)
        lash_unbind ((LashKeyBinding *) binding->data);
}

LashKeyBinding *
lash_bind (const gchar  *string,
           LashCallback  callback,
           gpointer      user_data)
{
    LashKeyBinding *binding;

    binding = lash_key_binding_new (string, callback, user_data);

    if (! _map_binding (binding, NULL))
    {
        lash_key_binding_free (binding);
        return NULL;
    }

    _GrabKey (binding->keycode, binding->modifiers);

    bindings = g_slist_prepend (bindings, binding);

    return binding;
}

void
lash_unbind (LashKeyBinding *binding)
{
    _UngrabKey(binding->keycode, binding->modifiers);

    bindings = g_slist_remove_all (bindings, binding);

    lash_key_binding_free (binding);
}


#ifndef MYPAINTRESIZABLETILEDSURFACE_H
#define MYPAINTRESIZABLETILEDSURFACE_H

#include <mypaint-config.h>
#include <mypaint-glib-compat.h>
#include <mypaint-tiled-surface.h>

G_BEGIN_DECLS

/**
 * MyPaintResizableTiledSurface:
 *
 * 一个基于#MyPaintTiledSurface的，可以改变大小的#MyPaintSurface。
 */
typedef struct MyPaintResizableTiledSurface MyPaintResizableTiledSurface;

MyPaintResizableTiledSurface *
mypaint_resizable_tiled_surface_new (int width, int height);

int
mypaint_resizable_tiled_surface_get_width (MyPaintResizableTiledSurface *self);

int
mypaint_resizable_tiled_surface_get_height (MyPaintResizableTiledSurface *self);

int
mypaint_resizable_tiled_surface_tiles_per_rows (MyPaintResizableTiledSurface *self);

int
mypaint_resizable_tiled_surface_number_of_tile_rows (MyPaintResizableTiledSurface *self);

void
mypaint_resizable_tiled_surface_resize (MyPaintResizableTiledSurface *self, int width, int height);

void
mypaint_resizable_tiled_surface_clear (MyPaintResizableTiledSurface *self);

MyPaintSurface *
mypaint_resizable_tiled_surface_interface (MyPaintResizableTiledSurface *self);

G_END_DECLS

#endif // MYPAINTFIXEDTILEDSURFACE_H

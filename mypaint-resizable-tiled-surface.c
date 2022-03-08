#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <mypaint-config.h>

#if MYPAINT_CONFIG_USE_GLIB
#include <glib.h>
#endif

#include "mypaint-resizable-tiled-surface.h"

struct MyPaintResizableTiledSurface {
    MyPaintTiledSurface parent;

    size_t tile_size; // Size (in bytes) of single tile
    uint16_t *tile_buffer; // Stores tiles in a linear chunk of memory (16bpc RGBA)
    uint16_t *null_tile; // Single tile that we hand out and ignore writes to
    int tiles_width; // width in tiles
    int tiles_height; // height in tiles
    int width; // width in pixels
    int height; // height in pixels

};

void free_simple_tiledsurf(MyPaintSurface *surface);

void reset_null_tile(MyPaintResizableTiledSurface *self)
{
    memset(self->null_tile, 0, self->tile_size);
}

static void
tile_request_start(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request)
{
    MyPaintResizableTiledSurface *self = (MyPaintResizableTiledSurface *)tiled_surface;

    const int tx = request->tx;
    const int ty = request->ty;

    uint16_t *tile_pointer = NULL;

    if (tx >= self->tiles_width || ty >= self->tiles_height || tx < 0 || ty < 0) {
        // Give it a tile which we will ignore writes to
        tile_pointer = self->null_tile;

    } else {
        // Compute the offset for the tile into our linear memory buffer of tiles
        size_t rowstride = self->tiles_width * self->tile_size;
        size_t x_offset = tx * self->tile_size;
        size_t tile_offset = (rowstride * ty) + x_offset;

        tile_pointer = self->tile_buffer + tile_offset/sizeof(uint16_t);
    }

    request->buffer = tile_pointer;
}

static void
tile_request_end(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request)
{
    MyPaintResizableTiledSurface *self = (MyPaintResizableTiledSurface *)tiled_surface;

    const int tx = request->tx;
    const int ty = request->ty;

    if (tx >= self->tiles_width || ty >= self->tiles_height || tx < 0 || ty < 0) {
        // Wipe any changed done to the null tile
        reset_null_tile(self);
    } else {
        // We hand out direct pointers to our buffer, so for the normal case nothing needs to be done
    }
}

MyPaintSurface *
mypaint_resizable_tiled_surface_interface(MyPaintResizableTiledSurface *self)
{
    return (MyPaintSurface *)self;
}

int
mypaint_resizable_tiled_surface_get_width(MyPaintResizableTiledSurface *self)
{
    return self->width;
}

int
mypaint_resizable_tiled_surface_get_height(MyPaintResizableTiledSurface *self)
{
    return self->height;
}

int
mypaint_resizable_tiled_surface_tiles_per_rows(MyPaintResizableTiledSurface *self)
{
    return self->tiles_width;
}

int
mypaint_resizable_tiled_surface_number_of_tile_rows(MyPaintResizableTiledSurface *self)
{
    return self->tiles_height;
}

void
mypaint_resizable_tiled_surface_resize (MyPaintResizableTiledSurface *self, int width, int height)
{
}

void
mypaint_resizable_tiled_surface_clear(MyPaintResizableTiledSurface *self)
{
  for (int ty = 0; ty < mypaint_resizable_tiled_surface_number_of_tile_rows(self); ty++) {
    for (int tx = 0; tx < mypaint_resizable_tiled_surface_tiles_per_rows(self); tx++) {
      MyPaintTileRequest request;
      mypaint_tile_request_init(&request, 0, tx, ty, FALSE);
      mypaint_tiled_surface_tile_request_start((MyPaintTiledSurface *)self, &request);

      memset (request.buffer, 0xFF, MYPAINT_TILE_SIZE * MYPAINT_TILE_SIZE * 8);

      mypaint_tiled_surface_tile_request_end((MyPaintTiledSurface *)self, &request);
    }
  }
/*
  似乎不需要，不知道为什么
  int width = mypaint_resizable_tiled_surface_get_width (self);
  int height = mypaint_resizable_tiled_surface_get_height (self);
  self->parent.dirty_bbox.x = 0;
  self->parent.dirty_bbox.y = 0;
  self->parent.dirty_bbox.width = width;
  self->parent.dirty_bbox.height = height;
*/
}

MyPaintResizableTiledSurface *
mypaint_resizable_tiled_surface_new(int width, int height)
{
    assert(width > 0);
    assert(height > 0);

    MyPaintResizableTiledSurface *self = (MyPaintResizableTiledSurface *)malloc(sizeof(MyPaintResizableTiledSurface));

    mypaint_tiled_surface_init(&self->parent, tile_request_start, tile_request_end);

    const int tile_size_pixels = self->parent.tile_size;

    // MyPaintSurface vfuncs
    self->parent.parent.destroy = free_simple_tiledsurf;

    const int tiles_width = ceil((float)width / tile_size_pixels);
    const int tiles_height = ceil((float)height / tile_size_pixels);
    const size_t tile_size = tile_size_pixels * tile_size_pixels * 4 * sizeof(uint16_t);
    const size_t buffer_size = tiles_width * tiles_height * tile_size;

    assert(tile_size_pixels*tiles_width >= width);
    assert(tile_size_pixels*tiles_height >= height);
    assert(buffer_size >= width*height*4*sizeof(uint16_t));

    uint16_t * buffer = (uint16_t *)malloc(buffer_size);
    if (!buffer) {
        fprintf(stderr, "CRITICAL: unable to allocate enough memory: %zu bytes", buffer_size);
        free(self);
        return NULL;
    }
    memset(buffer, 255, buffer_size);

    self->tile_buffer = buffer;
    self->tile_size = tile_size;
    self->null_tile = (uint16_t *)malloc(tile_size);
    self->tiles_width = tiles_width;
    self->tiles_height = tiles_height;
    self->height = height;
    self->width = width;

    reset_null_tile(self);

    return self;
}

void free_simple_tiledsurf(MyPaintSurface *surface)
{
    MyPaintResizableTiledSurface *self = (MyPaintResizableTiledSurface *)surface;

    mypaint_tiled_surface_destroy(&self->parent);

    free(self->tile_buffer);
    free(self->null_tile);

    free(self);
}


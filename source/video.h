/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 * Copyright (C) 2007 takka <takka@tfact.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef VIDEO_H
#define VIDEO_H

#define GBA_SCREEN_WIDTH 240
#define GBA_SCREEN_HEIGHT 160

#define GBA_RED_MASK     0x1F
#define GBA_GREEN_MASK  0x3E0
#define GBA_BLUE_MASK  0x7C00

void update_scanline();

extern int32_t affine_reference_x[2];
extern int32_t affine_reference_y[2];

typedef void (* tile_render_function)(uint32_t layer_number, uint32_t start, uint32_t end,
 void *dest_ptr);
typedef void (* bitmap_render_function)(uint32_t start, uint32_t end, void *dest_ptr);

typedef struct
{
  tile_render_function normal_render_base;
  tile_render_function normal_render_transparent;
  tile_render_function alpha_render_base;
  tile_render_function alpha_render_transparent;
  tile_render_function color16_render_base;
  tile_render_function color16_render_transparent;
  tile_render_function color32_render_base;
  tile_render_function color32_render_transparent;
} tile_layer_render_struct;

typedef struct
{
  bitmap_render_function normal_render;
} bitmap_layer_render_struct;

void video_read_mem_savestate();
void video_write_mem_savestate();

#endif

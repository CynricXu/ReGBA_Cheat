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

#ifndef CHEATS_H
#define CHEATS_H

#define CHEAT_NAME_LENGTH (40*16)
#define MAX_CHEATS_PAGE 25
#define CHEATS_PER_PAGE 4
#define MAX_CHEATS (MAX_CHEATS_PAGE * CHEATS_PER_PAGE)

typedef enum
{
  CHEAT_TYPE_GAMESHARK_V1,
  CHEAT_TYPE_GAMESHARK_V3,
  CHEAT_TYPE_DIRECT_V1,
  CHEAT_TYPE_DIRECT_V3,
  CHEAT_TYPE_INVALID
} CHEAT_VARIANT_ENUM;

typedef struct
{
  char cheat_name[CHEAT_NAME_LENGTH];
  uint32_t cheat_active;
  uint32_t cheat_codes[2048];
  uint32_t num_cheat_lines;
  CHEAT_VARIANT_ENUM cheat_variant;
} CHEAT_TYPE;

// グローバル変数宣言

extern uint32_t g_num_cheats;

// 関数宣言

void process_cheats();
void add_cheats(char *cheats_filename);
int load_cheats_name(char* cheats_msg, char** cheats_msg_pt, unsigned int num);

#endif

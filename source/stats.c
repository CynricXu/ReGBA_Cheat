/* Statistics gathered during emulation in ReGBA
 *
 * Copyright (C) 2013 GBATemp user Nebuleon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licens e as
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
#include "common.h"
#include "stats.h"

struct ReGBA_Stats Stats;

void StatsStopFPS(void)
{
	Stats.RenderedFPS = -1;
	Stats.EmulatedFPS = -1;
	Stats.RenderedFrames = 0;
	Stats.EmulatedFrames = 0;
}

void StatsInit(void)
{
	StatsStopFPS();
}

void StatsInitGame(void)
{
	StatsInit();
#ifndef USE_C_CORE
	uint32_t reason;
	uint32_t area;
	for (area = 0; area < TRANSLATION_REGION_COUNT; area++)
	{
		Stats.TranslationBytesFlushed[area] = 0;
		Stats.TranslationBytesPeak[area] = 0;
		for (reason = 0; reason < CACHE_FLUSH_REASON_COUNT; reason++)
		{
			Stats.TranslationFlushCount[area][reason] = 0;
		}
	}
	for (area = 0; area < METADATA_AREA_COUNT; area++)
	{
		for (reason = 0; reason < METADATA_CLEAR_REASON_COUNT; reason++)
		{
			Stats.MetadataClearCount[area][reason] = 0;
		}
	}
	Stats.PartialFlushCount = 0;
#endif
	Stats.SoundBufferUnderrunCount = 0;
	Stats.InSoundBufferUnderrun = 0;
	Stats.TotalEmulatedFrames = 0;
	Stats.TotalRenderedFrames = 0;
#ifdef PERFORMANCE_IMPACTING_STATISTICS
	Stats.ARMOpcodesDecoded = 0;
	Stats.ThumbOpcodesDecoded = 0;
	Stats.ThumbROMConstants = 0;
	Stats.BlockReuseCount = 0;
	Stats.BlockRecompilationCount = 0;
	Stats.OpcodeReuseCount = 0;
	Stats.OpcodeRecompilationCount = 0;
	Stats.WrongAddressLineCount = 0;
#endif
}

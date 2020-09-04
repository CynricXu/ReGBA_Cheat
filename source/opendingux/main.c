/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
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

#include "common.h"
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <gui.h>


#ifdef RUMBLE
Shake_Device *device;
Shake_Effect effect_low;
Shake_Effect effect_medium;
Shake_Effect effect_strong;
int id_shake_low;
int id_shake_medium;
int id_shake_strong;
#endif


//TIMER_TYPE timer[4];

frameskip_type current_frameskip_type = auto_frameskip;
u32 frameskip_value = 4;
u32 random_skip = 0;
u32 skip_next_frame = 0;

u32 frameskip_counter = 0;

u32 cpu_ticks = 0;
u32 frame_ticks = 0;

u32 execute_cycles = 960;
s32 video_count = 960;
u32 ticks;

u32 arm_frame = 0;
u32 thumb_frame = 0;

u32 synchronize_flag = 1;

char executable_path[MAX_PATH + 1];

#define check_count(count_var)                                                \
  if(count_var < execute_cycles)                                              \
    execute_cycles = count_var;                                               \

#define check_timer(timer_number)                                             \
  if(timer[timer_number].status == TIMER_PRESCALE)                            \
    check_count(timer[timer_number].count);                                   \

#define update_timer(timer_number)                                            \
  if(timer[timer_number].status != TIMER_INACTIVE)                            \
  {                                                                           \
    if(timer[timer_number].status != TIMER_CASCADE)                           \
    {                                                                         \
      timer[timer_number].count -= execute_cycles;                            \
      io_registers[REG_TM##timer_number##D] =                                 \
       -(timer[timer_number].count >> timer[timer_number].prescale);          \
    }                                                                         \
                                                                              \
    if(timer[timer_number].count <= 0)                                        \
    {                                                                         \
      if(timer[timer_number].irq == TIMER_TRIGGER_IRQ)                        \
        irq_raised |= IRQ_TIMER##timer_number;                                \
                                                                              \
      if((timer_number != 3) &&                                               \
       (timer[timer_number + 1].status == TIMER_CASCADE))                     \
      {                                                                       \
        timer[timer_number + 1].count--;                                      \
        io_registers[REG_TM0D + (timer_number + 1) * 2] =                     \
         -(timer[timer_number + 1].count);                                    \
      }                                                                       \
                                                                              \
      if(timer_number < 2)                                                    \
      {                                                                       \
        if(timer[timer_number].direct_sound_channels & 0x01)                  \
          sound_timer(timer[timer_number].frequency_step, 0);                 \
                                                                              \
        if(timer[timer_number].direct_sound_channels & 0x02)                  \
          sound_timer(timer[timer_number].frequency_step, 1);                 \
      }                                                                       \
                                                                              \
      timer[timer_number].count +=                                            \
       (timer[timer_number].reload << timer[timer_number].prescale);          \
    }                                                                         \
  }                                                                           \

char *file_ext[] = { ".gba", ".bin", ".zip", NULL };

static bool caches_inited = false;

void init_main()
{
  u32 i;

  skip_next_frame = 0;

  for(i = 0; i < 4; i++)
  {
    dma[i].start_type = DMA_INACTIVE;
    dma[i].direct_sound_channel = DMA_NO_DIRECT_SOUND;
    timer[i].status = TIMER_INACTIVE;
    timer[i].reload = 0x10000;
    timer[i].stop_cpu_ticks = 0;
  }

  timer[0].direct_sound_channels = TIMER_DS_CHANNEL_BOTH;
  timer[1].direct_sound_channels = TIMER_DS_CHANNEL_NONE;

  cpu_ticks = 0;
  frame_ticks = 0;

  execute_cycles = 960;
  video_count = 960;

  if (!caches_inited)
  {
    flush_translation_cache(TRANSLATION_REGION_READONLY, FLUSH_REASON_INITIALIZING);
    flush_translation_cache(TRANSLATION_REGION_WRITABLE, FLUSH_REASON_INITIALIZING);
  }
  else
  {
    flush_translation_cache(TRANSLATION_REGION_READONLY, FLUSH_REASON_LOADING_ROM);
    clear_metadata_area(METADATA_AREA_EWRAM, CLEAR_REASON_LOADING_ROM);
    clear_metadata_area(METADATA_AREA_IWRAM, CLEAR_REASON_LOADING_ROM);
    clear_metadata_area(METADATA_AREA_VRAM, CLEAR_REASON_LOADING_ROM);
  }

  caches_inited = true;

  StatsInitGame();
}

int main(int argc, char *argv[])
{
  u32 i;
  u32 vcount = 0;
  u32 ticks;
  u32 dispstat;
  char load_filename[512];
  char file[MAX_PATH + 1];
  ssize_t count = readlink("/proc/self/exe", file, 256);
  
#ifdef RUMBLE
	Shake_Init();

	if (Shake_NumOfDevices() > 0) {
		device = Shake_Open(0);

		Shake_InitEffect(&effect_low, SHAKE_EFFECT_RUMBLE);
		effect_low.u.rumble.strongMagnitude = SHAKE_RUMBLE_STRONG_MAGNITUDE_MAX * 0.7f;
		effect_low.u.rumble.weakMagnitude = SHAKE_RUMBLE_WEAK_MAGNITUDE_MAX * 0.7f;
		effect_low.length = 100;
		effect_low.delay = 0;
		
		Shake_InitEffect(&effect_medium, SHAKE_EFFECT_RUMBLE);
		effect_medium.u.rumble.strongMagnitude = SHAKE_RUMBLE_STRONG_MAGNITUDE_MAX * 1.3f;
		effect_medium.u.rumble.weakMagnitude = SHAKE_RUMBLE_WEAK_MAGNITUDE_MAX * 1.3f;
		effect_medium.length = 100;
		effect_medium.delay = 0;
		

		Shake_InitEffect(&effect_strong, SHAKE_EFFECT_RUMBLE);
		effect_strong.u.rumble.strongMagnitude = SHAKE_RUMBLE_STRONG_MAGNITUDE_MAX * 2.0f;
		effect_strong.u.rumble.weakMagnitude = SHAKE_RUMBLE_WEAK_MAGNITUDE_MAX * 2.0f;
		effect_strong.length = 100;
		effect_strong.delay = 0;

		id_shake_low = Shake_UploadEffect(device, &effect_low);
		id_shake_medium = Shake_UploadEffect(device, &effect_medium);
		id_shake_strong = Shake_UploadEffect(device, &effect_strong);
	}
#endif

  // Copy the path of the executable into executable_path
  if (count == -1)
  {
    fprintf(stderr, "Failed to get the path to the ReGBA executable: %s\n", strerror(errno));
    fprintf(stderr, "Some bundled files will not be loaded correctly.\n");
    executable_path[0] = '\0';
  }
  else
  {
    const char *exedir = dirname(file);
    if (exedir != NULL)
    {
      strcpy(executable_path, exedir);
    }
    else
    {
      executable_path[0] = '\0';
    }
  }

  init_video();

  // Copy the user's .gpsp directory into main_path
  sprintf(main_path, "%s/.gpsp", getenv("HOME"));
  mkdir(main_path, 0755);

  ReGBA_LoadSettings("global_config", false);
#if 0
  load_config_file();
#endif

  // Try loading the user's .gpsp directory's GBA BIOS first.
  // Fall back on the bundled one.
  ReGBA_ProgressInitialise(FILE_ACTION_LOAD_BIOS);
  sprintf(file, "%s/gba_bios.bin", main_path);
  if(load_bios(file) == -1)
  {
    ReGBA_ProgressUpdate(1, 2);
    sprintf(file, "%s/gba_bios.bin", executable_path);
    if (load_bios(file) == -1)
    {
      ShowErrorScreen("The GBA BIOS was not found in any location. "
        "You can load one in your home directory's .gpsp "
        "subdirectory. On this platform, that's:\n%s\nThe file needs "
        "to be named gba_bios.bin.", main_path);

      error_quit();
    }
    else
      ReGBA_ProgressUpdate(2, 2);
  }
  else
    ReGBA_ProgressUpdate(2, 2);
  ReGBA_ProgressFinalise();

  init_main();
  init_sdlaudio();
  init_sound();

  // Try loading a border from the user's .gpsp directory first.
  // Fall back on the bundled one.
  sprintf(file, "%s/border.png", main_path);
  if (!ApplyBorder(file))
  {
    sprintf(file, "%s/regba-sp-border-silver.png", executable_path);
    if (!ApplyBorder(file))
    {
      fprintf(stderr, "Failed to load a GBA border. None will be shown in unscaled modes.\n");
    }
  }


#if 0
  video_resolution_large();
#endif

  if(argc > 1)
  {
    if(load_gamepak(argv[1]) == -1)
    {
      if (errno != 0)
        ShowErrorScreen("Loading ROM failed: %s", strerror(errno));
      else
        ShowErrorScreen("Loading ROM failed: File format invalid");
      error_quit();
    }

    if (IsGameLoaded)
    {
		char FileNameNoExt[MAX_PATH + 1];
		GetFileNameNoExtension(FileNameNoExt, CurrentGamePath);
		ReGBA_LoadSettings(FileNameNoExt, true);
		init_game_config();
		add_cheats(FileNameNoExt);
		process_cheats();
    }
#if 0
	init_input();

    set_gba_resolution(screen_scale);
    video_resolution_small();
#endif

    init_cpu(!ResolveSetting(BootFromBIOS, PerGameBootFromBIOS) /* in port.c */);

	

	
  }
  else
  {
      fprintf(stderr, "No ROM was specified was on the command line.\n");
      error_quit();
#if 0
  init_video();
  init_sound();
  init_input();

    if(load_file(file_ext, load_filename) == -1)
    {
    ReGBA_Menu(REGBA_MENU_ENTRY_REASON_NO_ROM);
    }
    else
    {
      if(load_gamepak(load_filename) == -1)
      {
        printf("Failed to load gamepak %s, exiting.\n", load_filename);
        delay_us(5000000);
        quit();
      }

      set_gba_resolution(screen_scale);
      video_resolution_small();
      clear_screen(0);
      flip_screen();
      init_cpu(!ResolveSetting(BootFromBIOS, PerGameBootFromBIOS) /* in port.c */);
    }
#endif
  }

  // We'll never actually return from here.

  SetGameResolution();

  execute_arm_translate(execute_cycles);
  return 0;
}

u32 update_gba()
{
  IRQ_TYPE irq_raised = IRQ_NONE;
  do
  {
    cpu_ticks += execute_cycles;
    reg[CHANGED_PC_STATUS] = 0;

    if(gbc_sound_update)
    {
      update_gbc_sound(cpu_ticks);
      gbc_sound_update = 0;
    }

    update_timer(0);
    update_timer(1);
    update_timer(2);
    update_timer(3);

    video_count -= execute_cycles;

    if(video_count <= 0)
    {
      u32 vcount = io_registers[REG_VCOUNT];
      u32 dispstat = io_registers[REG_DISPSTAT];

      if((dispstat & 0x02) == 0)
      {
        // Transition from hrefresh to hblank
        video_count += (272);
        dispstat |= 0x02;

        if((dispstat & 0x01) == 0)
        {
          u32 i;

          update_scanline();

          // If in visible area also fire HDMA
          for(i = 0; i < 4; i++)
          {
            if(dma[i].start_type == DMA_START_HBLANK)
              dma_transfer(dma + i);
          }
        }

        if(dispstat & 0x10)
          irq_raised |= IRQ_HBLANK;
      }
      else
      {
        // Transition from hblank to next line
        video_count += 960;
        dispstat &= ~0x02;

        vcount++;

        if(vcount == 160)
        {
          // Transition from vrefresh to vblank
          u32 i;

          dispstat |= 0x01;
          if(dispstat & 0x8)
          {
            irq_raised |= IRQ_VBLANK;
          }

          affine_reference_x[0] =
           (s32)(ADDRESS32(io_registers, 0x28) << 4) >> 4;
          affine_reference_y[0] =
           (s32)(ADDRESS32(io_registers, 0x2C) << 4) >> 4;
          affine_reference_x[1] =
           (s32)(ADDRESS32(io_registers, 0x38) << 4) >> 4;
          affine_reference_y[1] =
           (s32)(ADDRESS32(io_registers, 0x3C) << 4) >> 4;

          for(i = 0; i < 4; i++)
          {
            if(dma[i].start_type == DMA_START_VBLANK)
              dma_transfer(dma + i);
          }
        }
        else

        if(vcount == 228)
        {
          // Transition from vblank to next screen
          dispstat &= ~0x01;
          frame_ticks++;

          if(update_input())
            continue;

          update_gbc_sound(cpu_ticks);

          Stats.EmulatedFrames++;
          Stats.TotalEmulatedFrames++;
          ReGBA_RenderScreen();

          update_backup();
		  
          vcount = 0;
        }

        if(vcount == (dispstat >> 8))
        {
          // vcount trigger
          dispstat |= 0x04;
          if(dispstat & 0x20)
          {
            irq_raised |= IRQ_VCOUNT;
          }
        }
        else
        {
          dispstat &= ~0x04;
        }

        io_registers[REG_VCOUNT] = vcount;
      }
      io_registers[REG_DISPSTAT] = dispstat;
    }

    if(irq_raised)
      raise_interrupt(irq_raised);

    execute_cycles = video_count;

    check_timer(0);
    check_timer(1);
    check_timer(2);
    check_timer(3);
  } while(reg[CPU_HALT_STATE] != CPU_ACTIVE);
  return execute_cycles;
}

u64 last_screen_timestamp = 0;
u32 frame_speed = 15000;

u32 ticks_needed_total = 0;
float us_needed = 0.0f;
u32 frames = 0;
u32 skipped_num_frame = 60;
const u32 frame_interval = 60;
u32 skipped_num = 0;

#if 0
void synchronize()
{
  u64 new_ticks;
  char char_buffer[64];
  u64 time_delta = 16667;
  u32 fpsw = 60;

  get_ticks_us(&new_ticks);
  time_delta = new_ticks - last_screen_timestamp;
  last_screen_timestamp = new_ticks;
  ticks_needed_total += time_delta;

  skip_next_frame = 0;
  if((time_delta < frame_speed) && synchronize_flag)
  {
    delay_us(frame_speed - time_delta);
  }
  frames++;

  if(frames == 60)
  {
    if(status_display) {
      us_needed = (float)ticks_needed_total / frame_interval;
      fpsw = (u32)(1000000.0f / us_needed);
      ticks_needed_total = 0;
      if(current_frameskip_type == manual_frameskip) {
        sprintf(char_buffer, "%s%3dfps %s:%d slot:%d ", synchronize_flag?"  ":">>", fpsw,
          current_frameskip_type==auto_frameskip?"Auto":current_frameskip_type==manual_frameskip?"Manu":"Off ",
          frameskip_value, savestate_slot);
      } else {
        sprintf(char_buffer, "%s%3dfps %s:%d slot:%d ", synchronize_flag?"  ":">>",
         (skipped_num_frame==60&&fpsw>60)?fpsw:skipped_num_frame,
          current_frameskip_type==auto_frameskip?"Auto":current_frameskip_type==manual_frameskip?"Manu":"Off ",
          frameskip_value, savestate_slot);
      }
    } else
      strcpy(char_buffer, "                             ");
    print_string(char_buffer, 0xFFFF, 0x000, 40, 30);
    print_string(ssmsg, 0xF000, 0x000, 180, 30);
    strcpy(ssmsg, "     ");
    frames = 0;
  skipped_num_frame = 60;
  }

  if(current_frameskip_type == manual_frameskip)
  {
    frameskip_counter = (frameskip_counter + 1) %
     (frameskip_value + 1);
    if(random_skip)
    {
      if(frameskip_counter != (rand() % (frameskip_value + 1)))
        skip_next_frame = 1;
    }
    else
    {
      if(frameskip_counter)
        skip_next_frame = 1;
    }
  } else if(current_frameskip_type == auto_frameskip) {
    static struct timeval next1 = {0, 0};
    static struct timeval now;

    gettimeofday(&now, NULL);
    if(next1.tv_sec == 0) {
      next1 = now;
      next1.tv_usec++;
    }
    if(timercmp(&next1, &now, >)) {
      //struct timeval tdiff;
     if(synchronize_flag)
       do {
          synchronize_sound();
        gettimeofday(&now, NULL);
       } while (timercmp(&next1, &now, >));
   else
      gettimeofday(&now, NULL);
      //timersub(&next1, &now, &tdiff);
    //usleep(tdiff.tv_usec/2);
    //gettimeofday(&now, NULL);
    skipped_num = 0;
    next1 = now;
    } else {
      if(skipped_num < frameskip_value) {
        skipped_num++;
      skipped_num_frame--;
        skip_next_frame = 1;
      } else {
        //synchronize_sound();
        skipped_num = 0;
      next1 = now;
      }
  }
    next1.tv_usec += 16667;
    if(next1.tv_usec >= 1000000) {
      next1.tv_sec++;
      next1.tv_usec -= 1000000;
    }
  }
}
#endif

static void quit_common()
{
  if(IsGameLoaded)
    update_backup_force();

#if defined(RUMBLE)
	Shake_Stop(device, id_shake_low);
	Shake_Stop(device, id_shake_medium);
	Shake_Stop(device, id_shake_strong);
	Shake_EraseEffect(device, id_shake_low);
	Shake_EraseEffect(device, id_shake_medium);
	Shake_EraseEffect(device, id_shake_strong);
	Shake_Close(device);
	Shake_Quit();
#endif

  SDL_Quit();
}

void quit()
{
  quit_common();
  exit(0);
}

void error_quit()
{
  quit_common();
  exit(1);
}

void reset_gba()
{
  init_main();
  init_memory();
  init_cpu(!ResolveSetting(BootFromBIOS, PerGameBootFromBIOS) /* in port.c */);
  reset_sound();
}

size_t FILE_LENGTH(FILE_TAG_TYPE fp)
{
  u32 length;

  fseek(fp, 0, SEEK_END);
  length = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  return length;
}

void delay_us(u32 us_count)
{
  SDL_Delay(us_count / 1000); // for dingux
  // sleep(0);
}

void get_ticks_us(u64 *ticks_return)
{
  *ticks_return = (SDL_GetTicks() * 1000);
}

// type = READ / WRITE_MEM
#define MAIN_SAVESTATE_BODY(type)                                             \
{                                                                             \
  FILE_##type##_VARIABLE(g_state_buffer_ptr, cpu_ticks);                      \
  FILE_##type##_VARIABLE(g_state_buffer_ptr, execute_cycles);                 \
  FILE_##type##_VARIABLE(g_state_buffer_ptr, video_count);                    \
  FILE_##type##_ARRAY(g_state_buffer_ptr, timer);                             \
}                                                                             \

void main_read_mem_savestate()
MAIN_SAVESTATE_BODY(READ_MEM);

void main_write_mem_savestate()
MAIN_SAVESTATE_BODY(WRITE_MEM);

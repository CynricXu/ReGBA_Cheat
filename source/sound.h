/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 * Copyright (C) 2007 takka <takka@tfact.net>
 * Copyright (C) 2007 ????? <?????>
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

/******************************************************************************
 * sound.h
 * サウンド周りの処理
 ******************************************************************************/
#ifndef SOUND_H
#define SOUND_H

/******************************************************************************
 * マクロ等の定義
 ******************************************************************************/
typedef enum
{
  DIRECT_SOUND_INACTIVE,
  DIRECT_SOUND_RIGHT,
  DIRECT_SOUND_LEFT,
  DIRECT_SOUND_LEFTRIGHT
} DIRECT_SOUND_STATUS_TYPE;

typedef enum
{
  DIRECT_SOUND_VOLUME_50,
  DIRECT_SOUND_VOLUME_100
} DIRECT_SOUND_VOLUME_TYPE;

typedef enum
{
  GBC_SOUND_INACTIVE,
  GBC_SOUND_RIGHT,
  GBC_SOUND_LEFT,
  GBC_SOUND_LEFTRIGHT
} GBC_SOUND_STATUS_TYPE;

typedef struct
{
  int8_t fifo[32];
  uint32_t fifo_base;
  uint32_t fifo_top;
  FIXED16_16 fifo_fractional;
  // The + 1 is to give some extra room for linear interpolation
  // when wrapping around.
  uint32_t buffer_index;
  DIRECT_SOUND_STATUS_TYPE status;
  DIRECT_SOUND_VOLUME_TYPE volume;
  uint32_t last_cpu_ticks;
} DIRECT_SOUND_STRUCT;

typedef struct
{
  uint32_t rate;
  FIXED16_16 frequency_step;
  FIXED16_16 sample_index;
  FIXED16_16 tick_counter;
  uint32_t total_volume;
  uint32_t envelope_initial_volume;
  uint32_t envelope_volume;
  uint32_t envelope_direction;
  uint32_t envelope_status;
  uint32_t envelope_step;
  uint32_t envelope_ticks;
  uint32_t envelope_initial_ticks;
  uint32_t sweep_status;
  uint32_t sweep_direction;
  uint32_t sweep_ticks;
  uint32_t sweep_initial_ticks;
  uint32_t sweep_shift;
  uint32_t length_status;
  uint32_t length_ticks;
  uint32_t noise_type;
  uint32_t wave_type;
  uint32_t wave_bank;
  uint32_t wave_volume;
  GBC_SOUND_STATUS_TYPE status;
  uint32_t active_flag;
  uint32_t master_enable;
  int8_t *sample_data;
} GBC_SOUND_STRUCT;

#define BUFFER_SIZE (0x10000)
#define BUFFER_SIZE_MASK (0xffff)                        // バッファのバイト数。変更しないこと

// This is the frequency of sound output by the GBA. It is stored in a
// ring buffer containing BUFFER_SIZE bytes.
// The value should technically be 32768, but at least the GBA Video ROMs and
// Golden Sun - The Lost Age require this to be 2 times that, and
// the Pokémon GBA games require it to be a multiple or divisor of 22050 Hz.
#define SOUND_FREQUENCY (88200.0f)

#define GBC_SOUND_TONE_CONTROL_LOW(channel, address)                          \
{                                                                             \
  uint32_t initial_volume = (value >> 12) & 0x0F;                             \
  uint32_t envelope_ticks = ((value >> 8) & 0x07) * 4;                        \
  gbc_sound_channel[channel].length_ticks = 64 - (value & 0x3F);              \
  gbc_sound_channel[channel].sample_data =                                    \
   square_pattern_duty[(value >> 6) & 0x03];                                  \
  gbc_sound_channel[channel].envelope_direction = (value >> 11) & 0x01;       \
  gbc_sound_channel[channel].envelope_initial_volume = initial_volume;        \
  gbc_sound_channel[channel].envelope_volume = initial_volume;                \
  gbc_sound_channel[channel].envelope_initial_ticks = envelope_ticks;         \
  gbc_sound_channel[channel].envelope_ticks = envelope_ticks;                 \
  gbc_sound_channel[channel].envelope_status = (envelope_ticks != 0);         \
  gbc_sound_channel[channel].envelope_volume = initial_volume;                \
  gbc_sound_update = 1;                                                       \
  ADDRESS16(io_registers, address) = value;                                   \
}                                                                             \

#define GBC_SOUND_TONE_CONTROL_HIGH(channel, address)                         \
{                                                                             \
  uint32_t rate = value & 0x7FF;                                              \
  gbc_sound_channel[channel].rate = rate;                                     \
  gbc_sound_channel[channel].frequency_step = FLOAT_TO_FP16_16((1048576.0f / SOUND_FREQUENCY) / (2048 - rate));  \
  gbc_sound_channel[channel].length_status = (value >> 14) & 0x01;            \
  if(value & 0x8000)                                                          \
  {                                                                           \
    gbc_sound_channel[channel].active_flag = 1;                               \
    gbc_sound_channel[channel].sample_index -= FLOAT_TO_FP16_16(1.0f / 12.0f);  \
    gbc_sound_channel[channel].envelope_ticks =                               \
     gbc_sound_channel[channel].envelope_initial_ticks;                       \
    gbc_sound_channel[channel].envelope_volume =                              \
     gbc_sound_channel[channel].envelope_initial_volume;                      \
    /*gbc_sound_channel[channel].sweep_ticks =                                  \
     gbc_sound_channel[channel].sweep_initial_ticks;*/                          \
  }                                                                           \
                                                                              \
  gbc_sound_update = 1;                                                       \
  ADDRESS16(io_registers, address) = value;                                   \
}                                                                             \

#define GBC_SOUND_TONE_CONTROL_SWEEP()                                        \
{                                                                             \
  uint32_t sweep_ticks = ((value >> 4) & 0x07) * 2;                           \
  gbc_sound_channel[0].sweep_shift = value & 0x07;                            \
  gbc_sound_channel[0].sweep_direction = (value >> 3) & 0x01;                 \
  gbc_sound_channel[0].sweep_status = (value != 8);                           \
  gbc_sound_channel[0].sweep_ticks = sweep_ticks;                             \
  gbc_sound_channel[0].sweep_initial_ticks = sweep_ticks;                     \
  gbc_sound_update = 1;                                                       \
  ADDRESS16(io_registers, 0x60) = value;                                      \
}                                                                             \

#define GBC_SOUND_WAVE_CONTROL()                                              \
{                                                                             \
  gbc_sound_channel[2].wave_type = (value >> 5) & 0x01;                       \
  gbc_sound_channel[2].wave_bank = (value >> 6) & 0x01;                       \
  if(value & 0x80)                                                            \
  {                                                                           \
    gbc_sound_channel[2].master_enable = 1;                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    gbc_sound_channel[2].master_enable = 0;                                   \
  }                                                                           \
                                                                              \
  gbc_sound_update = 1;                                                       \
  ADDRESS16(io_registers, 0x70) = value;                                      \
}                                                                             \

#define GBC_SOUND_TONE_CONTROL_LOW_WAVE()                                     \
{                                                                             \
  gbc_sound_channel[2].length_ticks = 256 - (value & 0xFF);                   \
  if((value >> 15) & 0x01)                                                    \
  {                                                                           \
    gbc_sound_channel[2].wave_volume = 12288;                                 \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    gbc_sound_channel[2].wave_volume =                                        \
     gbc_sound_wave_volume[(value >> 13) & 0x03];                             \
  }                                                                           \
  gbc_sound_update = 1;                                                       \
  ADDRESS16(io_registers, 0x72) = value;                                      \
}                                                                             \

#define GBC_SOUND_TONE_CONTROL_HIGH_WAVE()                                    \
{                                                                             \
  uint32_t rate = value & 0x7FF;                                              \
  gbc_sound_channel[2].rate = rate;                                           \
  gbc_sound_channel[2].frequency_step = FLOAT_TO_FP16_16((2097152.0f / SOUND_FREQUENCY) / (2048 - rate));         \
  gbc_sound_channel[2].length_status = (value >> 14) & 0x01;                  \
  if(value & 0x8000)                                                          \
  {                                                                           \
    gbc_sound_channel[2].sample_index = 0;                                    \
    gbc_sound_channel[2].active_flag = 1;                                     \
  }                                                                           \
  gbc_sound_update = 1;                                                       \
  ADDRESS16(io_registers, 0x74) = value;                                      \
}                                                                             \

#define GBC_SOUND_NOISE_CONTROL()                                             \
{                                                                             \
  uint32_t dividing_ratio = value & 0x07;                                     \
  uint32_t frequency_shift = (value >> 4) & 0x0F;                             \
  if(dividing_ratio == 0)                                                     \
  {                                                                           \
    gbc_sound_channel[3].frequency_step =                                     \
     FLOAT_TO_FP16_16((1048576.0f / SOUND_FREQUENCY) /                         \
       (2 << frequency_shift));                                               \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    gbc_sound_channel[3].frequency_step =                                     \
     FLOAT_TO_FP16_16((524288.0f / SOUND_FREQUENCY) / (dividing_ratio *        \
     (2 << frequency_shift)));                                                \
  }                                                                           \
  gbc_sound_channel[3].noise_type = (value >> 3) & 0x01;                      \
  gbc_sound_channel[3].length_status = (value >> 14) & 0x01;                  \
  if(value & 0x8000)                                                          \
  {                                                                           \
    gbc_sound_channel[3].sample_index = 0;                                    \
    gbc_sound_channel[3].active_flag = 1;                                     \
    gbc_sound_channel[3].envelope_ticks =                                     \
     gbc_sound_channel[3].envelope_initial_ticks;                             \
    gbc_sound_channel[3].envelope_volume =                                    \
     gbc_sound_channel[3].envelope_initial_volume;                            \
  }                                                                           \
  gbc_sound_update = 1;                                                       \
  ADDRESS16(io_registers, 0x7C) = value;                                      \
}                                                                             \

#define GBC_TRIGGER_SOUND_CHANNEL(channel)                                    \
  gbc_sound_channel[channel].status =                                         \
   ((value >> (channel + 11)) & 0x02) | ((value >> (channel + 8)) & 0x01)      \

#define GBC_TRIGGER_SOUND()                                                   \
{                                                                             \
  gbc_sound_master_volume_right = value & 0x07;                               \
  gbc_sound_master_volume_left = (value >> 4) & 0x07;                         \
                                                                              \
  GBC_TRIGGER_SOUND_CHANNEL(0);                                               \
  GBC_TRIGGER_SOUND_CHANNEL(1);                                               \
  GBC_TRIGGER_SOUND_CHANNEL(2);                                               \
  GBC_TRIGGER_SOUND_CHANNEL(3);                                               \
  ADDRESS16(io_registers, 0x80) = value;                                      \
}                                                                             \

#define TRIGGER_SOUND()                                                       \
{                                                                             \
  timer[0].direct_sound_channels =                                            \
   ((~value >> 13) & 0x02) | ((~value >> 10) & 0x01);                         \
  timer[1].direct_sound_channels =                                            \
   ((value >> 13) & 0x02) | ((value >> 10) & 0x01);                           \
                                                                              \
  direct_sound_channel[0].volume = (value >> 2) & 0x01;                       \
  direct_sound_channel[0].status = (value >> 8) & 0x03;                       \
  direct_sound_channel[1].volume = (value >> 3) & 0x01;                       \
  direct_sound_channel[1].status = (value >> 12) & 0x03;                      \
  gbc_sound_master_volume = value & 0x03;                                     \
                                                                              \
  if((value >> 11) & 0x01)                                                    \
    sound_reset_fifo(0);                                                      \
  if((value >> 15) & 0x01)                                                    \
    sound_reset_fifo(1);                                                      \
  ADDRESS16(io_registers, 0x82) = value;                                      \
}                                                                             \

#define SOUND_ON()                                                            \
  if(value & 0x80)                                                            \
    sound_on = 1;                                                           \
  else                                                                        \
  {                                                                           \
    uint32_t i;                                                               \
    for(i = 0; i < 4; i++)                                                    \
    {                                                                         \
      gbc_sound_channel[i].active_flag = 0;                                   \
    }                                                                         \
    sound_on = 0;                                                             \
  }                                                                           \
  ADDRESS16(io_registers, 0x84) =                                             \
    (ADDRESS16(io_registers, 0x84) & 0x000F) | (value & 0xFFF0);              \

// タイマーの値の調整
// TODO:調整必要
#define ADJUST_SOUND_BUFFER(timer_number, channel)                            \
  if(timer[timer_number].direct_sound_channels & (0x01 << (channel)))         \
  {                                                                           \
    direct_sound_channel[channel].buffer_index = gbc_sound_buffer_index;      \
  }                                                                           \

#define SOUND_UPDATE_FREQUENCY_STEP(timer_number)                             \
  timer[timer_number].frequency_step =                                        \
   FLOAT_TO_FP16_16((SYS_CLOCK / SOUND_FREQUENCY) / timer_reload)             \

/******************************************************************************
 * グローバル変数の宣言
 ******************************************************************************/
extern DIRECT_SOUND_STRUCT direct_sound_channel[2];
extern GBC_SOUND_STRUCT gbc_sound_channel[4];
extern int8_t square_pattern_duty[4][8];
extern uint32_t gbc_sound_master_volume_left;
extern uint32_t gbc_sound_master_volume_right;
extern uint32_t gbc_sound_master_volume;

extern uint32_t sound_on;

extern uint32_t gbc_sound_wave_volume[4];

extern uint32_t gbc_sound_buffer_index;
extern uint32_t gbc_sound_partial_ticks;

void sound_timer_queue32(uint8_t channel);
void sound_timer(FIXED16_16 frequency_step, uint32_t channel);
void sound_reset_fifo(uint32_t channel);
void update_gbc_sound(uint32_t cpu_ticks);
void init_sound();
void sound_read_mem_savestate();
void sound_write_mem_savestate();
void reset_sound();

// Services provided to ports

/*
 * Returns the number of samples available in the core's audio buffer.
 * This number of samples is always interpreted with a sample rate of
 * SOUND_FREQUENCY Hz, which is used by the core to render to its buffer.
 * 
 * Returns:
 *   The number of samples available for reading by ReGBA_LoadNextAudioSample
 *   at the time of the function call, counting 1 for each two stereo samples
 *   available; that is, the number of times ReGBA_LoadNextAudioSample can be
 *   called with Left and Right without it returning zero.
 */
uint32_t ReGBA_GetAudioSamplesAvailable(void);

/*
 * Loads and consumes the next audio sample from the core's audio buffer.
 * The sample rate is SOUND_FREQUENCY Hz.
 * Output:
 *   Left: A pointer to a signed 16-bit variable updated with the value for
 *   the next sample's left stereo channel.
 *   Right: A pointer to a signed 16-bit variable updated with the value for
 *   the next sample's right stereo channel.
 * Returns:
 *   Non-zero if a sample was available; zero if no samples were available.
 * Output assertions:
 *   If non-zero is returned, the loaded sample is consumed in the core's
 *   audio buffer.
 *   If zero is returned, neither variable is written to.
 */
uint32_t ReGBA_LoadNextAudioSample(int16_t* Left, int16_t* Right);

/*
 * Discards the requested number of samples from the core's audio buffer.
 * The sample rate is SOUND_FREQUENCY Hz.
 * Input:
 *   Count: The number of samples to be discarded by the core.
 * Returns:
 *   The number of samples that were actually discarded, which may be Count
 *   or fewer.
 */
uint32_t ReGBA_DiscardAudioSamples(uint32_t Count);

#endif /* SOUND_H */

/*
 * CATS Flight Software
 * Copyright (C) 2021 Control and Telemetry Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "lfs.h"
#include "drivers/w25q.h"
#include "cli/cli.h"

static int w25q_lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
static int w25q_lfs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer,
                         lfs_size_t size);
static int w25q_lfs_erase(const struct lfs_config *c, lfs_block_t block);
static int w25q_lfs_sync(const struct lfs_config *c);

#define LFS_CACHE_SIZE     512
#define LFS_LOOKAHEAD_SIZE 512

/* LFS Static Buffers */
static uint8_t read_buffer[LFS_CACHE_SIZE] = {};
static uint8_t prog_buffer[LFS_CACHE_SIZE] = {};
static uint8_t lookahead_buffer[LFS_LOOKAHEAD_SIZE] = {};

/* File System Handle  -- NOT THREAD-SAFE!!! */
lfs_t lfs;
const struct lfs_config lfs_cfg = {
    // block device operations
    .read = w25q_lfs_read,
    .prog = w25q_lfs_prog,
    .erase = w25q_lfs_erase,
    .sync = w25q_lfs_sync,

    // block device configuration
    .read_size = 256,
    .prog_size = 256,
    .block_size = 4096,
    // 8k pages for 256 Mbit flash
    .block_count = 8192,
    .cache_size = LFS_CACHE_SIZE,
    .lookahead_size = LFS_LOOKAHEAD_SIZE,
    .block_cycles = 500,
    .read_buffer = read_buffer,
    .prog_buffer = prog_buffer,
    .lookahead_buffer = lookahead_buffer};

char cwd[256] = {};

uint32_t flight_counter = 0;
lfs_file_t fc_file;

int lfs_ls(const char *path) {
  lfs_dir_t dir;
  int err = lfs_dir_open(&lfs, &dir, path);
  if (err) {
    return err;
  }

  struct lfs_info info;
  while (true) {
    int res = lfs_dir_read(&lfs, &dir, &info);
    if (res < 0) {
      return res;
    }

    if (res == 0) {
      break;
    }

    switch (info.type) {
      case LFS_TYPE_REG:
        cli_print("file ");
        break;
      case LFS_TYPE_DIR:
        cli_print(" dir ");
        break;
      default:
        cli_print("   ? ");
        break;
    }

    static const char *prefixes[] = {"", "K", "M", "G"};
    if (info.type == LFS_TYPE_REG) {
      for (int i = sizeof(prefixes) / sizeof(prefixes[0]) - 1; i >= 0; i--) {
        if (info.size >= (1 << 10 * i) - 1) {
          cli_printf("%*lu%sB ", 4 - (i != 0), info.size >> 10 * i, prefixes[i]);
          break;
        }
      }
    } else {
      cli_print("      ");
    }

    cli_printf("%s\n", info.name);
  }

  err = lfs_dir_close(&lfs, &dir);
  if (err) {
    return err;
  }

  return 0;
}

static int w25q_lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
  if (w25q_read_buffer((uint8_t *)buffer, block * (w25q.sector_size) + off, size) == W25Q_OK) {
    return 0;
  }
  return LFS_ERR_CORRUPT;
}
static int w25q_lfs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer,
                         lfs_size_t size) {
  static uint32_t sync_counter = 0;
  static uint32_t sync_counter_err = 0;
  if (w25q_write_buffer((uint8_t *)buffer, block * (w25q.sector_size) + off, size) == W25Q_OK) {
    if (sync_counter % 32 == 0) {
      /* Flash the LED at certain intervals */
      HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    }
    ++sync_counter;
    return 0;
  }
  if (sync_counter_err % 32 == 0) {
    /* Flash the LED at certain intervals */
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
  }
  ++sync_counter_err;
  return LFS_ERR_CORRUPT;
}
static int w25q_lfs_erase(const struct lfs_config *c, lfs_block_t block) {
  if (w25q_sector_erase(block) == W25Q_OK) {
    return 0;
  }
  return LFS_ERR_CORRUPT;
}
static int w25q_lfs_sync(const struct lfs_config *c) { return 0; }
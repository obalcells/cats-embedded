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
static lfs_t lfs;
static const struct lfs_config lfs_cfg = {
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

int file_open(lfs_file_t *file_handle, const char *path, int flags) {
  return lfs_file_open(&lfs, file_handle, path, flags);
}
int file_close(lfs_file_t *file_handle) { return lfs_file_close(&lfs, file_handle); }
int file_sync(lfs_file_t *file_handle) { return lfs_file_sync(&lfs, file_handle); }
lfs_ssize_t file_read(lfs_file_t *file_handle, void *buffer, lfs_size_t size) {
  return lfs_file_read(&lfs, file_handle, buffer, size);
}
lfs_ssize_t file_write(lfs_file_t *file_handle, const void *buffer, lfs_size_t size) {
  return lfs_file_write(&lfs, file_handle, buffer, size);
}
lfs_soff_t file_size(lfs_file_t *file) { return lfs_file_size(&lfs, file); }

int fs_format() { return lfs_format(&lfs, &lfs_cfg); }
int fs_mount() { return lfs_format(&lfs, &lfs_cfg); }

int fs_mkdir(const char *path) { return lfs_mkdir(&lfs, path); }

int file_rewind(lfs_file_t *file) { return lfs_file_rewind(&lfs, file); }

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

void lfs_rm(char *file_name) {
  if (strlen(file_name) > LFS_NAME_MAX) {
    cli_print_line("File name too long!");
    return;
  }
  /* first +1 for the path separator (/), second +1 for the null terminator */
  char *full_path = malloc(strlen(cwd) + 1 + strlen(file_name) + 1);
  strcpy(full_path, cwd);
  strcat(full_path, "/");
  strcat(full_path, file_name);
  struct lfs_info info;
  int32_t stat_err = lfs_stat(&lfs, full_path, &info);
  if (stat_err < 0) {
    cli_print_linef("lfs_stat failed with %ld", stat_err);
    free(full_path);
    return;
  }
  if (info.type != LFS_TYPE_REG) {
    cli_print_line("This is not a file!");
    free(full_path);
    return;
  }
  int32_t rm_err = lfs_remove(&lfs, full_path);
  if (rm_err < 0) {
    cli_print_linef("File removal failed with %ld", rm_err);
  }
  cli_printf("File %s removed!", file_name);
  free(full_path);
}

void fs_create_default_dirs() {
  fs_mkdir("flights");
  fs_mkdir("stats");
}

void init_lfs() {
  /* mount the filesystem */
  int err = fs_mount();
  if (err == 0) {
    log_raw("LFS mounted successfully!");
  } else {
    /* reformat if we can't mount the filesystem */
    /* this should only happen on the first boot */
    log_raw("LFS mounting failed with error %d!", err);
    log_raw("Trying LFS format");
    fs_format();
    int err2 = fs_mount();
    if (err2 != 0) {
      log_raw("LFS mounting failed again with error %d!", err2);
    }
  }

  file_open(&fc_file, "flight_counter", LFS_O_RDWR | LFS_O_CREAT);

  /* read how many flights we have */
  if (file_read(&fc_file, &flight_counter, sizeof(flight_counter)) > 0) {
    log_debug("Flights found: %lu", flight_counter);
  } else {
    log_debug("Flights found: %lu", flight_counter);
    file_rewind(&fc_file);
    file_write(&fc_file, &flight_counter, sizeof(flight_counter));
  }
  file_close(&fc_file);

  fs_create_default_dirs();

  strncpy(cwd, "/", sizeof(cwd));
}

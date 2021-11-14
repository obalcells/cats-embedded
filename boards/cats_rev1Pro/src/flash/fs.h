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

#pragma once

#include "lfs.h"

extern uint32_t flight_counter;
extern lfs_file_t fc_file;

extern char cwd[256];

/**
 * List the contents of the directory
 *
 * @param path - path to the directory
 * @return 0 if no error
 */
int lfs_ls(const char *path);
int lfs_rm(const char *path);

int fs_format();
int fs_mount();

int fs_mkdir(const char *path);

int file_open(lfs_file_t *file, const char *path, int flags);
int file_rewind(lfs_file_t *file);
int file_close(lfs_file_t *fh);
int file_sync(lfs_file_t *file);
int file_rewind(lfs_file_t *file);
lfs_ssize_t file_read(lfs_file_t *file, void *buffer, lfs_size_t size);
lfs_ssize_t file_write(lfs_file_t *file, const void *buffer, lfs_size_t size);
lfs_soff_t file_size(lfs_file_t *file);

void fs_create_default_dirs();

void init_lfs();
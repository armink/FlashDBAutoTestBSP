/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-24     armink       the first version
 */

#include <stdio.h>
#include <fal.h>
#include <string.h>

#define FLASH_FILE_PATH       "/"
#define FLASH_FILE_NAME       "flash_sim.bin"
#define FLASH_FILE_SIZE       (1*1024*1024)
#define FLASH_FILE_PAGE_SIZE  (4*1024)

#include <dfs_posix.h>

static int flash_fd = -1;

static int createdir(char *path, int len)
{
    int index = len -1;
    int res;

    if (access(path, 0) != 0)
    {
        if (strcmp("/", path) == 0)
            return -1;

        while (index >= 0)
        {
            if (path[index] == '/')
            {
                path[index] = '\0';
                break;
            }
            index--;
        }
        res = createdir(path, index);
        if (res < 0)
            return res;

        path[index] = '/';
        res = mkdir(path, 0);
        if (res < 0)
        {
            return -1;
        }
        return 0;
    }
    return 0;
}

static int sim_init(void)
{
    char path[32];
    long off;

    if (sizeof(FLASH_FILE_PATH) + sizeof(FLASH_FILE_NAME) > sizeof(path) - 2)
        RT_ASSERT(0);

    /* create dir */
    sprintf(path, "%s", FLASH_FILE_PATH);
    if (createdir(path, strlen(FLASH_FILE_PATH)) < 0)
        return -1;
    /* open file */
    sprintf(path, "%s/%s", FLASH_FILE_PATH, FLASH_FILE_NAME);
    flash_fd = open(path, O_RDWR | O_CREAT, 0);
    if (flash_fd < 0)
        goto err_exit;
    /* read data */
    off = lseek(flash_fd, 0, SEEK_END);
    if (off >= 0 && off != FLASH_FILE_SIZE)
    {
        /* write data */
        off = lseek(flash_fd, FLASH_FILE_SIZE - 1, SEEK_SET);
        if (off != FLASH_FILE_SIZE - 1)
            goto err_exit;
        if (write(flash_fd, &off, 1) != 1)
            goto err_exit;
    }
    fsync(flash_fd);
    return 0;

err_exit:
    if (flash_fd >= 0)
    {
        close(flash_fd);
        flash_fd = -1;
    }
    return -1;
}

static int sim_read(long offset, uint8_t *buf, size_t size)
{
    if (flash_fd >= 0)
    {
        lseek(flash_fd, offset, SEEK_SET);
        size = read(flash_fd, buf, size);
        return size;
    }
    else
    {
        return -1;
    }
}

static int sim_write(long offset, const uint8_t *buf, size_t size)
{
    if (flash_fd >= 0)
    {
        lseek(flash_fd, offset, SEEK_SET);
        size = write(flash_fd, buf, size);
        fsync(flash_fd);
        return size;
    }
    else
    {
        return -1;
    }
}

static int sim_erase(long offset, size_t size)
{
#define BUF_SIZE 32
    uint8_t buf[BUF_SIZE];
    size_t i;

    memset(buf, 0xFF, BUF_SIZE);
    if (flash_fd) {
        lseek(flash_fd, offset, SEEK_SET);
        for (i = 0; i * BUF_SIZE < size; i++)
        {
            write(flash_fd, buf, BUF_SIZE);
        }
        write(flash_fd, buf, size - i * BUF_SIZE);
        fsync(flash_fd);
        return size;
    }
    else
    {
        return -1;
    }
}
const struct fal_flash_dev flash_sim_dev = { "flash_sim", 0, FLASH_FILE_SIZE, FLASH_FILE_PAGE_SIZE, {sim_init, sim_read, sim_write, sim_erase} };

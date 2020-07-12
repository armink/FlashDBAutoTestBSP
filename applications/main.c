#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>
#include <fal.h>
#include <flashdb.h>

static struct fdb_default_kv_node default_kv_set[] = {
        {"iap_need_copy_app", "0"},
        {"iap_need_crc32_check", "0"},
        {"iap_copy_app_size", "0"},
        {"stop_in_bootloader", "0"},
};

static fdb_time_t get_time(void)
{
//    return time(NULL);
    static fdb_time_t time = 0;

//    log_d("time: %d", time + 1);
    return time++;
}

struct fdb_kvdb _global_kvdb;
struct fdb_tsdb _global_tsdb;
struct rt_mutex kv_locker, ts_locker;

static void lock(fdb_db_t db)
{
    rt_mutex_take((rt_mutex_t)db->user_data, RT_WAITING_FOREVER);
}

static void unlock(fdb_db_t db)
{
    rt_mutex_release((rt_mutex_t)db->user_data);
}

int main(void)
{
    struct fdb_default_kv default_kv;
    uint32_t sec_size = 4096;

    printf("hello rt-thread\n");

    fal_init();

    default_kv.kvs = default_kv_set;
    default_kv.num = sizeof(default_kv_set) / sizeof(default_kv_set[0]);
    rt_mutex_init(&ts_locker, "fdb_kvdb1", RT_IPC_FLAG_FIFO);
    fdb_kvdb_control(&_global_kvdb, FDB_KVDB_CTRL_SET_LOCK, lock);
    fdb_kvdb_control(&_global_kvdb, FDB_KVDB_CTRL_SET_UNLOCK, unlock);
    fdb_kvdb_init(&_global_kvdb, "env", "fdb_kvdb1", &default_kv, &ts_locker);

    rt_mutex_init(&kv_locker, "fdb_tsdb1", RT_IPC_FLAG_FIFO);
    fdb_tsdb_control(&_global_tsdb, FDB_TSDB_CTRL_SET_LOCK, lock);
    fdb_tsdb_control(&_global_tsdb, FDB_TSDB_CTRL_SET_UNLOCK, unlock);
    fdb_tsdb_control(&_global_tsdb, FDB_TSDB_CTRL_SET_SEC_SIZE, &sec_size);
    fdb_tsdb_init(&_global_tsdb, "log", "fdb_tsdb1", get_time, 128, &kv_locker);

//    kvdb_type_string_sample();
//    kvdb_type_blob_sample();
//    tsdb_sample();

    return 0;
}


void file_test(void)
{
    FILE *flash_fp = fopen("/flash_sim.bin", "rb+");

    uint8_t buf[] = {1,2,3,4 };

    fseek(flash_fp, 0, SEEK_SET);
    fwrite(buf, sizeof(buf), 1, flash_fp);
    fflush(flash_fp);
    fclose(flash_fp);
}
MSH_CMD_EXPORT(file_test, file test)

void big_env_blob_test()
{
#define ENV_SIZE  512

    uint8_t value[ENV_SIZE];
    uint8_t read_value[ENV_SIZE];
    size_t return_len;
    fdb_err_t result;
    struct fdb_blob blob;

    for (size_t i = 0; i < ENV_SIZE; i++)
    {
        value[i] = i % 256;
    }

    result =  fdb_kv_set_blob(&_global_kvdb, "big_env", fdb_blob_make(&blob, value, ENV_SIZE));
    rt_kprintf("fdb_set_env_blob return: %d, size: %d\n", result, ENV_SIZE);

    return_len = fdb_kv_get_blob(&_global_kvdb, "big_env", fdb_blob_make(&blob, read_value, ENV_SIZE));
    rt_kprintf("fdb_get_env_blob return: %d, saved_value_len: %d\n", return_len, blob.saved.len);

    rt_kprintf("memcmp result: %d\n", memcmp(value, read_value, ENV_SIZE));
}
MSH_CMD_EXPORT(big_env_blob_test, big env blob test)

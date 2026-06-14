/*
 *
 * FocalTech ftxxxx TouchScreen driver.
 *
 * Copyright (c) 2012-2020, Focaltech Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*****************************************************************************
*
* File Name: focaltech_ex_mode.c
*
* Author: Focaltech Driver Team
*
* Created: 2016-08-31
*
* Abstract:
*
* Reference:
*
*****************************************************************************/

/*****************************************************************************
* 1.Included header files
*****************************************************************************/
#include "focaltech_core.h"

/*****************************************************************************
* 2.Private constant and macro definitions using #define
*****************************************************************************/

/*****************************************************************************
* 3.Private enumerations, structures and unions using typedef
*****************************************************************************/
enum _ex_mode {
    MODE_GLOVE = 0,
    MODE_COVER,
    MODE_CHARGER,
    MODE_EARPHONE,
    MODE_EDGEPALM,
    MODE_GAME,
    MODE_FOD,
    MODE_POCKET,
    MODE_GESTURE,
    MODE_PALM_TO_SLEEP,
};

/*****************************************************************************
* 4.Static variables
*****************************************************************************/
struct proc_dir_entry *proc_dir_touchpanel;

/*****************************************************************************
* 5.Global variable or extern global variabls/functions
*****************************************************************************/
extern struct fts_fwdbg *fts_fwdbg_data;
extern int fts_fwdbg_disable(struct fts_fwdbg *dbg);
extern int fts_fwdbg_enable(struct fts_fwdbg *dbg, int value);

/*****************************************************************************
* 6.Static function prototypes
*******************************************************************************/
static int fts_ex_mode_set_reg(u8 mode_regaddr, u8 mode_regval)
{
    int i = 0;
    u8 val = 0xFF;

    for (i = 0; i < FTS_MAX_RETRIES_WRITEREG; i++) {
        fts_read_reg(mode_regaddr, &val);
        if (val == mode_regval)
            break;
        fts_write_reg(mode_regaddr, mode_regval);
        fts_msleep(1);
    }

    if (i >= FTS_MAX_RETRIES_WRITEREG) {
        FTS_ERROR("set mode(%x) to %x failed,read val:%x", mode_regaddr, mode_regval, val);
        return -EIO;
    } else if (i > 0) {
        FTS_INFO("set mode(%x) to %x successfully", mode_regaddr, mode_regval);
    }
    return 0;
}

static int fts_ex_mode_switch(enum _ex_mode mode, int value)
{
    int ret = 0;
    u8 state = 0;
    switch (mode) {
    case MODE_GLOVE:
        ret = fts_ex_mode_set_reg(FTS_REG_GLOVE_MODE_EN, (value ? 0x01 : 0x00));
        if (ret) FTS_ERROR("Set MODE_GLOVE to %d failed", value);
        break;
    case MODE_COVER:
        ret = fts_ex_mode_set_reg(FTS_REG_COVER_MODE_EN, (value ? 0x01 : 0x00));
        if (ret) FTS_ERROR("Set MODE_COVER to %d failed", value);
        break;
    case MODE_CHARGER:
        switch (value) {
            case 0: FTS_INFO("Charger Mode: charger offline\n"); break;
            case 2: FTS_INFO("Charger Mode: wired_charger\n"); break;
            case 3: FTS_INFO("Charger Mode: wireless charger\n"); break;
            default: FTS_INFO("Charger Mode: Unknown mode\n"); break;
        }
        ret = fts_ex_mode_set_reg(FTS_REG_CHARGER_MODE_EN, (u8)value);
        if (ret) FTS_ERROR("Set MODE_CHARGER to %d failed", value);
        break;
    case MODE_EARPHONE:
        FTS_INFO("Earphone Mode %s\n", (value ? "Enable" : "Disable"));
        ret = fts_ex_mode_set_reg(FTS_REG_EARPHONE_MODE_EN, (value ? 0x01 : 0x00));
        if (ret) FTS_ERROR("Set MODE_EARPHONE to %d failed", value);
        break;
    case MODE_EDGEPALM:
        /* FW defines the following values: 0:vertical, 1:horizontal, USB on the right,
         *                                  2:horizontal, USB on the left
         * If host set the value not defined above, you should have a transition.
         */
        switch (value) {
            case 0: FTS_INFO("Edge Mode: 0 - vertical\n"); break;
            case 1: FTS_INFO("Edge Mode: 1 - horizontal, USB on the right\n"); break;
            case 2: FTS_INFO("Edge Mode: 2 - horizontal, USB on the left\n"); break;
            case 3: FTS_INFO("Edge Mode: 3 - Game mode horizontal, USB on the right\n"); break;
            case 4: FTS_INFO("Edge Mode: 4 - Game mode horizontal, USB on the left\n"); break;
            default: FTS_INFO("Edge Mode: Unknown mode\n"); break;
        }
        ret = fts_ex_mode_set_reg(FTS_REG_EDGEPALM_MODE_EN, (u8)value);
        if (ret) FTS_ERROR("Set MODE_EDGEPALM to %d failed", value);
        break;
    case MODE_GAME:
        FTS_INFO("Game Mode %s\n", (value ? "Enable" : "Disable"));
        ret = fts_ex_mode_set_reg(FTS_REG_GAME_MODE_EN, (value ? 0x01 : 0x00));
        if (ret) FTS_ERROR("Set MODE_GAME to %d failed", value);
        break;
    #if FTS_FOD_EN
    case MODE_FOD:
        fts_fod_enable(value);
        break;
    #endif
    case MODE_POCKET:
        ret = fts_ex_mode_set_reg(FTS_REG_POCKET_MODE, (u8)value);
        if (ret) FTS_ERROR("Set MODE_POCKET to %d failed", value);
        break;
    case MODE_GESTURE:
        FTS_INFO("Gesture Mode %s\n", (value ? "Enable" : "Disable"));
        break;
    case MODE_PALM_TO_SLEEP:
        FTS_INFO("PalmToSleep Mode %s\n", (value ? "Enable" : "Disable"));
        ret = fts_ex_mode_set_reg(FTS_REG_PALM_TO_SLEEP_EN, (u8)value);
        if (ret) FTS_ERROR("Set PalmToSleep Mode to %d failed", value);
        msleep(10);
        fts_read_reg(FTS_REG_PALM_TO_SLEEP_EN, &state);
        if (state != (u8)value) {
            ret = fts_ex_mode_set_reg(FTS_REG_PALM_TO_SLEEP_EN, (u8)value);
            if (ret) FTS_ERROR("Set PalmToSleep Mode to %d failed", value);
        }
        break;
    default:
        FTS_ERROR("mode(%d) unsupport", mode);
        ret = -EINVAL;
        break;
    }

    return ret;
}

int fts_ex_mode_recovery(struct fts_ts_data *ts_data)
{
    if (ts_data->glove_mode) {
        fts_ex_mode_switch(MODE_GLOVE, ENABLE);
    }

    if (ts_data->cover_mode) {
        fts_ex_mode_switch(MODE_COVER, ENABLE);
    }

    if (ts_data->charger_mode) {
        fts_ex_mode_switch(MODE_CHARGER, ts_data->charger_value);
    }

    if (ts_data->earphone_mode) {
        fts_ex_mode_switch(MODE_EARPHONE, ENABLE);
    }

    if (ts_data->edgepalm_mode) {
        fts_ex_mode_switch(MODE_EDGEPALM, ts_data->edgepalm_value);
    }
    if (ts_data->palm_to_sleep_support) {
        fts_ex_mode_switch(MODE_PALM_TO_SLEEP, ts_data->palm_to_sleep_support);
    }
    return 0;
}

static ssize_t fts_proc_mode_read(u8 reg_addr, char *mode_name, char __user *buff,
        size_t count, loff_t *pos)
{
    char local_buf[PROC_BUF_SIZE] = {0};
    ssize_t len = 0;
    u8 reg_val = 0;

    mutex_lock(&fts_data->input_dev->mutex);
    if ((reg_addr != 0) && fts_read_reg(reg_addr, &reg_val)) {
        FTS_ERROR("Failed to read register 0x%02x\n", reg_addr);
        mutex_unlock(&fts_data->input_dev->mutex);
        return -EIO;
    }

    if (strcmp(mode_name, "Game") == 0) {
        len += snprintf(local_buf + len, sizeof(local_buf) - len,
                        "Game Mode:%s\n", fts_data->game_mode ? "On" : "Off");
    } else if (strcmp(mode_name, "Earphone") == 0) {
        len += snprintf(local_buf + len, sizeof(local_buf) - len,
                        "Earphone Mode:%s\n", fts_data->earphone_mode ? "On" : "Off");
    } else if (strcmp(mode_name, "Pocket") == 0) {
        len += snprintf(local_buf + len, sizeof(local_buf) - len,
                        "Pocket Mode:%s\n", fts_data->pocket_mode ? "On" : "Off");
    } else if (strcmp(mode_name, "Gesture") == 0) {
        len += snprintf(local_buf + len, sizeof(local_buf) - len,
                        "Gesture Mode:%s\n", fts_data->gesture_support ? "On" : "Off");
    } else if (strcmp(mode_name, "Charger") == 0) {
        len += snprintf(local_buf + len, sizeof(local_buf) - len,
                        "Charger Mode:%s\n", fts_data->charger_mode ? "On" : "Off");
    } else if (strcmp(mode_name, "Edge") == 0) {
        len += snprintf(local_buf + len, sizeof(local_buf) - len,
                        "Edge Mode:%s\n", fts_data->edgepalm_mode ? "On" : "Off");
    } else if (strcmp(mode_name, "FOD") == 0) {
        len += snprintf(local_buf + len, sizeof(local_buf) - len,
                        "FOD Mode:%d\n", fts_data->fod_mode);
    } else if (strcmp(mode_name, "Diff") == 0) {
        len += snprintf(local_buf + len, sizeof(local_buf) - len,
                        "Diff Mode:%d value:%d\n",
                        fts_data->fwdbg_support, fts_data->fwdbg_value);
    } else if (strcmp(mode_name, "PalmToSleep") == 0) {
        len += snprintf(local_buf + len, sizeof(local_buf) - len,
                        "PalmToSleep Mode:%d\n", fts_data->palm_to_sleep_support);
    }
    else {
        FTS_ERROR("Unknown mode name: %s\n", mode_name);
    }
    if (reg_addr != 0)
        len += snprintf(local_buf + len, sizeof(local_buf) - len, "%s Reg:0x%02x,val:%d\n",
                        mode_name, reg_addr, reg_val);

    mutex_unlock(&fts_data->input_dev->mutex);

    if (*pos >= len) {
        return 0;
    }
    if (len > *pos && len - *pos > count) {
        len = count;
    }
    if (copy_to_user(buff, local_buf + *pos, len)) {
        FTS_ERROR("Failed to copy data to user space\n");
        return -EFAULT;
    }

    *pos += len;

    return len;
}
static int parse_and_validate_input(const char __user *buff, size_t count, int *value, int max_val)
{
    char local_buf[PROC_BUF_SIZE] = {0};

    if (count < 1 || count > sizeof(local_buf) - 1) {
        FTS_ERROR("Invalid input length: %zu\n", count);
        return -EINVAL;
    }

    if (copy_from_user(local_buf, buff, count)) {
        FTS_ERROR("Failed to copy from user space\n");
        return -EFAULT;
    }

    if (sscanf(local_buf, "%d", value) != 1) {
        FTS_ERROR("Failed to parse input value\n");
        return -EINVAL;
    }

    if (*value < 0 || *value > max_val) {
        FTS_ERROR("Invalid mode value: %d\n", *value);
        return -EINVAL;
    }

    return 0;
}
static ssize_t fts_proc_mode_write(enum _ex_mode mode, bool *mode_val, int *mode_data_val,
        int max_val, const char __user *buff, size_t count)
{
    int value = 0;

    if (parse_and_validate_input(buff, count, &value, max_val))
        return -EINVAL;

    mutex_lock(&fts_data->input_dev->mutex);

    if (mode_val != NULL)
        *mode_val = !!value;
    if (mode_data_val != NULL)
        *mode_data_val = value;

    fts_ex_mode_switch(mode, value);

    mutex_unlock(&fts_data->input_dev->mutex);

    return count;
}
static ssize_t fts_proc_diff_mode_read(struct file *filp, char __user *buff,
       size_t count, loff_t *pos)
{
    return fts_proc_mode_read(FTS_REG_DIFF_MODE_EN, "Diff", buff, count, pos);
}

static ssize_t fts_proc_diff_mode_write(struct file *filp, const char *buff,
        size_t count, loff_t *pos)
{
    char local_buf[PROC_BUF_SIZE] = {0};
    u32 tmp;
    struct fts_ts_data *ts_data = fts_data;
    struct fts_fwdbg *dbg = fts_fwdbg_data;
    if (!dbg || !ts_data) {
        FTS_ERROR("dbg/ts_data is null");
        return count;
    }

    memset(local_buf, 0x00, sizeof(local_buf));
    if (copy_from_user(&local_buf, buff, min_t(size_t, sizeof(local_buf) - 1, count)))
        return -EFAULT;
    if (kstrtouint(local_buf, 0, &tmp))
        return -EINVAL;

    mutex_lock(&ts_data->input_dev->mutex);
    if (!!tmp ^ ts_data->fwdbg_support) {
        if (tmp) {
            if (0 == fts_fwdbg_enable(dbg, tmp)) {
                ts_data->fwdbg_value = (u8)tmp;
                ts_data->fwdbg_support = ENABLE;
                FTS_INFO("diff fwdbg enable\n!");
            }
        } else {
            ts_data->fwdbg_support = DISABLE;
            fts_msleep(30);
            fts_fwdbg_disable(dbg);
            FTS_INFO("diff fwdbg disable!");
        }
    } else FTS_INFO("value(%d)==fwdbg_support(%d)", !!tmp, ts_data->fwdbg_support);
    mutex_unlock(&ts_data->input_dev->mutex);

    return count;
}
static struct proc_ops proc_diff_fops = {
        .proc_read = fts_proc_diff_mode_read,
        .proc_write = fts_proc_diff_mode_write,
        .proc_lseek = default_llseek,
};
#if FTS_FOD_EN
static ssize_t fts_proc_fod_mode_read(struct file *filp, char __user *buff,
       size_t count, loff_t *pos)
{
    return fts_proc_mode_read(FTS_REG_FOD_MODE_EN, "FOD", buff, count, pos);
}

static ssize_t fts_proc_fod_mode_write(struct file *filp, const char *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_write(MODE_FOD, NULL, NULL, 3, buff, count);
}
static struct proc_ops proc_fod_fops = {
        .proc_read = fts_proc_fod_mode_read,
        .proc_write = fts_proc_fod_mode_write,
        .proc_lseek = default_llseek,
};
#endif
static ssize_t fts_proc_earphone_mode_read(struct file *filp, char __user *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_read(FTS_REG_EARPHONE_MODE_EN, "Earphone", buff, count, pos);
}

static ssize_t fts_proc_earphone_mode_write(struct file *filp, const char *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_write(MODE_EARPHONE, &fts_data->earphone_mode, NULL, 1,
                               buff, count);
}
static struct proc_ops proc_earphone_fops = {
        .proc_read = fts_proc_earphone_mode_read,
        .proc_write = fts_proc_earphone_mode_write,
        .proc_lseek = default_llseek,
};
static ssize_t fts_proc_pocket_mode_read(struct file *filp, char __user *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_read(0, "Pocket", buff, count, pos);
}

static ssize_t fts_proc_pocket_mode_write(struct file *filp, const char __user *ubuf,
        size_t count, loff_t *pos)
{
    char buf[20];
    u32 tmp;
    int ret = 0;
    u8 state = 0xFF;
    struct fts_ts_data *ts_data = fts_data;

    memset(buf, 0x00, sizeof(buf));
    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;
    if (kstrtouint(buf, 0, &tmp))
        return -EINVAL;
    if ((!ts_data->gesture_support) && (ts_data->fod_mode == FTS_FOD_DISABLE)) {
        FTS_INFO("In sleep mode,not operation pocket mode!");
        return count;
    }
    mutex_lock(&ts_data->input_dev->mutex);

    if (ts_data->pm_suspend)
        __pm_stay_awake(ts_data->p_ws);

    if (FTS_SYSFS_ECHO_ON(buf)) {
        FTS_ERROR("Pre-pocket_mode = %d, Enter pocket mode", ts_data->pocket_mode);
        ret = fts_write_reg(FTS_REG_POCKET_MODE, ENABLE);
        ts_data->pocket_mode = ENABLE;
        fts_esdcheck_switch(ts_data, DISABLE);
        msleep(10); //Ensure that the firmware completes the pocket mode process.
    } else if (FTS_SYSFS_ECHO_OFF(buf)) {
        FTS_ERROR("Pre-pocket_mode = %d, Exit pocket mode", ts_data->pocket_mode);
        ret = fts_write_reg(FTS_REG_POCKET_MODE, DISABLE);
        if (!ts_data->suspended) {
            fts_esdcheck_switch(ts_data, ENABLE);
        }
        msleep(10);
        fts_read_reg(FTS_REG_POWER_MODE, &state);
        if (state == 2) {
            ret = fts_write_reg(FTS_REG_POWER_MODE, 0);
            if (!ts_data->suspended) {
                fts_esdcheck_switch(ts_data, ENABLE);
            }
        }
        ts_data->pocket_mode = DISABLE;
    }
    __pm_relax(ts_data->p_ws);
    mutex_unlock(&ts_data->input_dev->mutex);

    return count;
}
static struct proc_ops proc_pocket_fops = {
        .proc_read = fts_proc_pocket_mode_read,
        .proc_write = fts_proc_pocket_mode_write,
        .proc_lseek = default_llseek,
};
static ssize_t fts_proc_edge_mode_read(struct file *filp, char __user *buff,
        size_t count, loff_t *pos)
{
    FTS_INFO("edge_read:mode:%d, edge_value:%d\n",
              fts_data->edgepalm_mode, fts_data->edgepalm_value);
    return fts_proc_mode_read(FTS_REG_EDGEPALM_MODE_EN, "Edge", buff, count, pos);
}
/* 0:vertical
* 1:horizontal, USB on the left
* 2:horizontal, USB on the right
* 3:Game mode horizontal, USB on the left
* 4:Game mode horizontal, USB on the right
*/
static ssize_t fts_proc_edge_mode_write(struct file *filp, const char *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_write(MODE_EDGEPALM, &fts_data->edgepalm_mode,
                               &fts_data->edgepalm_value, 4, buff, count);
}
static struct proc_ops proc_edge_fops = {
        .proc_read = fts_proc_edge_mode_read,
        .proc_write = fts_proc_edge_mode_write,
        .proc_lseek = default_llseek,
};

static ssize_t fts_proc_gesture_point_read(struct file *filp, char __user *buff,
        size_t count, loff_t *pos)
{
    struct fts_gesture_st *gesture = &fts_gesture_data;
    char local_buf[PROC_BUF_SIZE] = {0};
    ssize_t len = 0;

    mutex_lock(&fts_data->input_dev->mutex);
    len += snprintf(local_buf + len, sizeof(local_buf) - len, "x:%x\ny:%x\n",
                    gesture->coordinate_x[0]/16, gesture->coordinate_y[0]/16);
    mutex_unlock(&fts_data->input_dev->mutex);
    if (*pos >= len) {
        return 0;
    }
    if (len > *pos && len - *pos > count) {
        len = count;
    }
    if (copy_to_user(buff, local_buf + *pos, len)) {
        FTS_ERROR("Failed to copy data to user space\n");
        return -EFAULT;
    }

    *pos += len;
    return len;
}

static struct proc_ops proc_gesture_point_fops = {
        .proc_read = fts_proc_gesture_point_read,
        .proc_lseek = default_llseek,
};

static ssize_t fts_proc_gesture_mode_write(struct file *filp, const char *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_write(MODE_GESTURE, &fts_data->gesture_support, NULL, 1,
                               buff, count);
}
static ssize_t fts_proc_gesture_mode_read(struct file *filp, char __user *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_read(FTS_REG_GESTURE_EN, "Gesture", buff, count, pos);
}

static struct proc_ops proc_gesture_fops = {
        .proc_read = fts_proc_gesture_mode_read,
        .proc_write = fts_proc_gesture_mode_write,
        .proc_lseek = default_llseek,
};

static ssize_t fts_proc_game_mode_write(struct file *filp, const char *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_write(MODE_GAME, &fts_data->game_mode, NULL, 1,
                               buff, count);
}

static ssize_t fts_proc_game_mode_read(struct file *filp, char __user *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_read(FTS_REG_GAME_MODE_EN, "Game", buff, count, pos);
}
static struct proc_ops proc_game_fops = {
        .proc_read = fts_proc_game_mode_read,
        .proc_write = fts_proc_game_mode_write,
        .proc_lseek = default_llseek,
};

/* 0：charger offline 2: wired_charger 3: wireless charger */
static ssize_t fts_proc_charger_mode_write(struct file *filp, const char __user *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_write(MODE_CHARGER, &fts_data->charger_mode, &fts_data->charger_value,
                               3, buff, count);
}
static ssize_t fts_proc_charger_mode_read(struct file *filp, char __user *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_read(FTS_REG_CHARGER_MODE_EN, "Charger", buff, count, pos);
}
static struct proc_ops proc_charger_fops = {
        .proc_read = fts_proc_charger_mode_read,
        .proc_write = fts_proc_charger_mode_write,
        .proc_lseek = default_llseek,
};

/* 0：Enable palm to sleep support; 2: Disable palm to sleep support */
static ssize_t fts_proc_palm_to_sleep_support_write(struct file *filp, const char __user *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_write(MODE_PALM_TO_SLEEP, &fts_data->palm_to_sleep_support, NULL,
                               1, buff, count);
}
static ssize_t fts_proc_palm_to_sleep_support_read(struct file *filp, char __user *buff,
        size_t count, loff_t *pos)
{
    return fts_proc_mode_read(FTS_REG_PALM_TO_SLEEP_EN, "PalmToSleep", buff, count, pos);
}
static struct proc_ops proc_palm_to_sleep_fops = {
        .proc_read = fts_proc_palm_to_sleep_support_read,
        .proc_write = fts_proc_palm_to_sleep_support_write,
        .proc_lseek = default_llseek,
};
struct proc_node ftsproc[] = {
        {"TP_charger_flags", NULL, &proc_charger_fops, false},
        {"game_mode", NULL, &proc_game_fops, false},
        {"gesture_mode", NULL, &proc_gesture_fops, false},
        {"gesture_code", NULL, &proc_gesture_point_fops, false},
        {"edge_mode", NULL, &proc_edge_fops, false},
        {"pocket_mode", NULL, &proc_pocket_fops, false},
        {"earphone_mode", NULL, &proc_earphone_fops, false},
        {"diff_mode", NULL, &proc_diff_fops, false},
#if FTS_FOD_EN
        {"fod_mode", NULL, &proc_fod_fops, false},
#endif
        {"palm_to_sleep_support", NULL, &proc_palm_to_sleep_fops, false},
};
int fts_ex_mode_init(struct fts_ts_data *ts_data)
{
    int i = 0;
    ts_data->charger_mode = DISABLE;
    ts_data->earphone_mode = DISABLE;
    ts_data->edgepalm_mode = DISABLE;
    ts_data->game_mode = DISABLE;
    ts_data->pocket_mode = DISABLE;
    ts_data->gesture_support = DISABLE;
    #if FTS_FOD_EN
    ts_data->fod_mode = DISABLE;
    #endif
    ts_data->palm_to_sleep_support = DISABLE;
    proc_dir_touchpanel = proc_mkdir("touchpanel", NULL);

    for (; i < ARRAY_SIZE(ftsproc); i++) {
        ftsproc[i].node = proc_create_data(ftsproc[i].name, 0664, proc_dir_touchpanel,
                                           ftsproc[i].fops, ts_data);
        if (ftsproc[i].node == NULL) {
            ftsproc[i].isCreated = false;
            FTS_ERROR("Failed to create %s under /proc/touchpanel\n", ftsproc[i].name);
        } else {
            ftsproc[i].isCreated = true;
            FTS_INFO("Succeed to create %s under /proc/touchpanel\n", ftsproc[i].name);
        }
    }

    return 0;
}

int fts_ex_mode_exit(struct fts_ts_data *ts_data)
{
    int i;

    if (!proc_dir_touchpanel) {
        return 0;
    }

    for (i = 0; i < ARRAY_SIZE(ftsproc); i++) {
        if (ftsproc[i].isCreated) {
            proc_remove(ftsproc[i].node);
            ftsproc[i].isCreated = false;
        }
    }

    proc_remove(proc_dir_touchpanel);
    proc_dir_touchpanel = NULL;

    return 0;
}

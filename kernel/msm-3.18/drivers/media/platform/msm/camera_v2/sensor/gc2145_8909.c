/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */
#include "msm_sensor.h"
#include "msm_cci.h"
#include "msm_camera_io_util.h"
#include "gc2145_8909.h"

#define CONFIG_MSMB_CAMERA_DEBUG

#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

#define GC2145_8909_SENSOR_NAME "gc2145_8909"
DEFINE_MSM_MUTEX(gc2145_8909_mut);

static struct msm_sensor_ctrl_t gc2145_8909_s_ctrl;

static struct msm_sensor_power_setting gc2145_8909_power_setting[] = {
{
	.seq_type = SENSOR_VREG,
	.seq_val = CAM_VDIG,
	.config_val = 1,
	.delay = 10,
},
{
	.seq_type = SENSOR_VREG,
	.seq_val = CAM_VIO,
	.config_val = 1,
	.delay = 10,
},
{
	.seq_type = SENSOR_VREG,
	.seq_val = CAM_VANA,
	.config_val = 1,
	.delay = 10,
},
{
	.seq_type = SENSOR_CLK,
	.seq_val = SENSOR_CAM_MCLK,
	.config_val = 24000000,
	.delay = 10,
},
{
	.seq_type = SENSOR_GPIO,
	.seq_val = SENSOR_GPIO_STANDBY,
	.config_val = GPIO_OUT_HIGH,
	.delay = 10,
},
{
	.seq_type = SENSOR_GPIO,
	.seq_val = SENSOR_GPIO_STANDBY,
	.config_val = GPIO_OUT_LOW,
	.delay = 10,
},
{
	.seq_type = SENSOR_GPIO,
	.seq_val = SENSOR_GPIO_RESET,
	.config_val = GPIO_OUT_LOW,
	.delay = 10,
},
{
	.seq_type = SENSOR_GPIO,
	.seq_val = SENSOR_GPIO_RESET,
	.config_val = GPIO_OUT_HIGH,
	.delay = 10,
},
{
	.seq_type = SENSOR_I2C_MUX,
	.seq_val = 0,
	.config_val = 0,
	.delay = 10,
},
};

static struct v4l2_subdev_info gc2145_8909_subdev_info[] = {
	{
		.code	= V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt	= 1,
		.order	= 0,
	},
};

static const struct i2c_device_id gc2145_8909_i2c_id[] = {
	{GC2145_8909_SENSOR_NAME, (kernel_ulong_t)&gc2145_8909_s_ctrl},
	{ }
};

static int32_t msm_gc2145_8909_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	CDBG("%s, E.", __func__);

	return msm_sensor_i2c_probe(client, id, &gc2145_8909_s_ctrl);
}

static struct i2c_driver gc2145_8909_i2c_driver = {
	.id_table = gc2145_8909_i2c_id,
	.probe	= msm_gc2145_8909_i2c_probe,
	.driver = {
		.name = GC2145_8909_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client gc2145_8909_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};

static const struct of_device_id gc2145_8909_dt_match[] = {
	{.compatible = "qcom,gc2145_8909", .data = &gc2145_8909_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, gc2145_8909_dt_match);

static struct platform_driver gc2145_8909_platform_driver = {
	.driver = {
		.name = "qcom,gc2145_8909",
		.owner = THIS_MODULE,
		.of_match_table = gc2145_8909_dt_match,
	},
};

static int32_t gc2145_8909_platform_probe(struct platform_device *pdev)
{
	int32_t rc;
	const struct of_device_id *match;
	CDBG("%s, E.", __func__);
	match = of_match_device(gc2145_8909_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init gc2145_8909_init_module(void)
{
	int32_t rc;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&gc2145_8909_platform_driver,
		gc2145_8909_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&gc2145_8909_i2c_driver);
}

static void __exit gc2145_8909_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (gc2145_8909_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&gc2145_8909_s_ctrl);
		platform_driver_unregister(&gc2145_8909_platform_driver);
	} else
		i2c_del_driver(&gc2145_8909_i2c_driver);
	return;
}

static void gc2145_8909_i2c_write_table(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_camera_i2c_reg_conf *table,
		int num)
{
	int i = 0;
	int rc = 0;
	pr_info("%s %d", __func__, num);
	for (i = 0; i < num; ++i) {
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write(
			s_ctrl->sensor_i2c_client, table->reg_addr,
			table->reg_data,
			MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) {
			msleep(100);
			rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write(
				s_ctrl->sensor_i2c_client, table->reg_addr,
				table->reg_data,
				MSM_CAMERA_I2C_BYTE_DATA);
		}
		table++;
	}

}

static void
	gc2145_8909_set_saturation(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_info("%s %d", __func__, value);

	gc2145_8909_i2c_write_table(s_ctrl,
		&gc2145_8909_reg_saturation[value][0],
	ARRAY_SIZE(gc2145_8909_reg_saturation[value]));
}

static void
	gc2145_8909_set_sharpness(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	int val = value/6;
	pr_info("%s %d", __func__, val);
	gc2145_8909_i2c_write_table(s_ctrl,
		&gc2145_8909_reg_sharpness[val][0],
	ARRAY_SIZE(gc2145_8909_reg_sharpness[value]));
}
static void
	gc2145_8909_set_contrast(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_info("%s %d", __func__, value);
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_contrast[value][0],
	ARRAY_SIZE(gc2145_8909_reg_contrast[value]));
}

static void gc2145_8909_set_iso(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_info("%s %d", __func__, value);
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_iso[value][0],
	ARRAY_SIZE(gc2145_8909_reg_iso[value]));
}
static void
	gc2145_8909_set_antibanding(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_info("%s %d", __func__, value);
	gc2145_8909_i2c_write_table(s_ctrl,
		&gc2145_8909_reg_antibanding[value][0],
	ARRAY_SIZE(gc2145_8909_reg_antibanding[value]));
}

static void
	gc2145_8909_set_exposure_compensation(struct msm_sensor_ctrl_t *s_ctrl,
	int value)
{
	int val = (value + 12) / 6;

	pr_info("%s val:%d value:%d\n", __func__, val, value);

	gc2145_8909_i2c_write_table(s_ctrl,
		&gc2145_8909_reg_exp_compensation[val][0],
	ARRAY_SIZE(gc2145_8909_reg_exp_compensation[val]));
}

static void gc2145_8909_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_debug("%s %d", __func__, value);
	switch (value) {
	case MSM_CAMERA_EFFECT_MODE_OFF: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_effect_normal[0],
		ARRAY_SIZE(gc2145_8909_reg_effect_normal));
	break;
	}
/*
	case MSM_CAMERA_EFFECT_MODE_NEON: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_effect_neon[0],
		ARRAY_SIZE(gc2145_8909_reg_effect_neon));
	break;
	}

	case MSM_CAMERA_EFFECT_MODE_SKETCH: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_effect_sketch[0],
		ARRAY_SIZE(gc2145_8909_reg_effect_sketch));
	break;
	}

	case MSM_CAMERA_EFFECT_MODE_EMBOSS: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_effect_emboss[0],
		ARRAY_SIZE(gc2145_8909_reg_effect_emboss));
	break;
	}
*/
	case MSM_CAMERA_EFFECT_MODE_MONO: {
	gc2145_8909_i2c_write_table(s_ctrl,
		&gc2145_8909_reg_effect_black_white[0],
		ARRAY_SIZE(gc2145_8909_reg_effect_black_white));
	break;
	}
	case MSM_CAMERA_EFFECT_MODE_NEGATIVE: {
	gc2145_8909_i2c_write_table(s_ctrl,
		&gc2145_8909_reg_effect_negative[0],
		ARRAY_SIZE(gc2145_8909_reg_effect_negative));
	break;
	}
/*
	case MSM_CAMERA_EFFECT_MODE_SEPIA: {
	gc2145_8909_i2c_write_table(s_ctrl,
		&gc2145_8909_reg_effect_sepiablue[0],
		ARRAY_SIZE(gc2145_8909_reg_effect_sepiablue));
	break;
	}
	case MSM_CAMERA_EFFECT_MODE_AQUA: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_effect_aqua[0],
		ARRAY_SIZE(gc2145_8909_reg_effect_aqua));
	break;
	}
*/
	default:
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_effect_normal[0],
		ARRAY_SIZE(gc2145_8909_reg_effect_normal));
	}
}


static void
	gc2145_8909_set_scene_mode(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_debug("%s %d", __func__, value);
	switch (value) {
	case MSM_CAMERA_SCENE_MODE_OFF: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_scene_auto[0],
		ARRAY_SIZE(gc2145_8909_reg_scene_auto));
	break;
	}
	case MSM_CAMERA_SCENE_MODE_NIGHT: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_scene_night[0],
		ARRAY_SIZE(gc2145_8909_reg_scene_night));
	break;
	}
	case MSM_CAMERA_SCENE_MODE_LANDSCAPE: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_scene_landscape[0],
		ARRAY_SIZE(gc2145_8909_reg_scene_landscape));
	break;
	}
	case MSM_CAMERA_SCENE_MODE_PORTRAIT: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_scene_portrait[0],
		ARRAY_SIZE(gc2145_8909_reg_scene_portrait));
	break;
	}
	default:
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_scene_auto[0],
		ARRAY_SIZE(gc2145_8909_reg_scene_auto));
	}
}


static void gc2145_8909_set_white_balance_mode(struct msm_sensor_ctrl_t *s_ctrl,
	int value)
{
	pr_info("%s %d\n", __func__, value);
	switch (value) {
	case MSM_CAMERA_WB_MODE_AUTO: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_wb_auto[0],
		ARRAY_SIZE(gc2145_8909_reg_wb_auto));
	break;
	}
	case MSM_CAMERA_WB_MODE_INCANDESCENT: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_wb_home[0],
		ARRAY_SIZE(gc2145_8909_reg_wb_home));
	break;
	}
	case MSM_CAMERA_WB_MODE_DAYLIGHT: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_wb_sunny[0],
		ARRAY_SIZE(gc2145_8909_reg_wb_sunny));
	break;
	}
	case MSM_CAMERA_WB_MODE_FLUORESCENT: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_wb_office[0],
		ARRAY_SIZE(gc2145_8909_reg_wb_office));
	break;
	}
	case MSM_CAMERA_WB_MODE_CLOUDY_DAYLIGHT: {
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_wb_cloudy[0],
		ARRAY_SIZE(gc2145_8909_reg_wb_cloudy));
	break;
	}
	default:
	{
	gc2145_8909_i2c_write_table(s_ctrl, &gc2145_8909_reg_wb_auto[0],
		ARRAY_SIZE(gc2145_8909_reg_wb_auto));
	}
	}
}
/*
static int setshutter = 0;
void gc2145_8909_Set_Shutter(uint16_t iShutter)
{
	CDBG(" gc2145_8909_Set_Shutter\r\n");
}

int gc2145_8909_Read_Shutter(void)
{
	int shutter = 0;
	CDBG(" gc2145_8909_Read_Shutter \r\n");
	return shutter;
}

void gc2145_8909_AfterSnapshot(void)
{
}

void gc2145_8909_BeforeSnapshot(void)
{
}
*/
int32_t gc2145_8909_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	long rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	CDBG("%s:%d %s cfgtype = %d\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++)
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
		cdata->cfg.sensor_info.is_mount_angle_valid =
			s_ctrl->sensordata->sensor_info->is_mount_angle_valid;
		cdata->cfg.sensor_info.sensor_mount_angle =
			s_ctrl->sensordata->sensor_info->sensor_mount_angle;
		CDBG("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		CDBG("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			CDBG("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);
		CDBG("%s:%d mount angle valid %d value %d\n", __func__,
			__LINE__, cdata->cfg.sensor_info.is_mount_angle_valid,
			cdata->cfg.sensor_info.sensor_mount_angle);

		break;
	case CFG_SET_INIT_SETTING:
		/* Write Recommend settings */
		pr_err("%s, sensor write init setting!!", __func__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			gc2145_8909_recommend_settings,
			ARRAY_SIZE(gc2145_8909_recommend_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_RESOLUTION:{
	/*copy from user the desired resoltuion*/
		enum msm_sensor_resolution_t res = MSM_SENSOR_INVALID_RES;
		if (copy_from_user(&res, (void *)cdata->cfg.setting,
			sizeof(enum msm_sensor_resolution_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		pr_err("%s:%d res =%d\n", __func__, __LINE__, res);

		if (res == MSM_SENSOR_RES_FULL) {
			rc =
			s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, gc2145_8909_uxga_settings,
			ARRAY_SIZE(gc2145_8909_uxga_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
			pr_err("%s:%d res =%d\n gc2145_8909_uxga_settings ",
			__func__, __LINE__, res);
		} else if (res == MSM_SENSOR_RES_QTR) {
			rc =
			s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, gc2145_8909_svga_settings,
			ARRAY_SIZE(gc2145_8909_svga_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
			pr_err("%s:%d res =%d gc2145_8909_svga_settings\n",
					 __func__, __LINE__, res);
		} else {
			pr_err("%s:%d failed resoultion set\n", __func__,
				__LINE__);
			rc = -EFAULT;
		}
		}
		break;
	case CFG_SET_STOP_STREAM:
		pr_err("%s, sensor stop stream!!", __func__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			gc2145_8909_stop_settings,
			ARRAY_SIZE(gc2145_8909_stop_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_START_STREAM:
		pr_err("%s, sensor start stream!!", __func__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			gc2145_8909_start_settings,
			ARRAY_SIZE(gc2145_8909_start_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_GET_SENSOR_INIT_PARAMS:
		cdata->cfg.sensor_init_params.modes_supported =
			s_ctrl->sensordata->sensor_info->modes_supported;
		cdata->cfg.sensor_init_params.position =
			s_ctrl->sensordata->sensor_info->position;
		cdata->cfg.sensor_init_params.sensor_mount_angle =
			s_ctrl->sensordata->sensor_info->sensor_mount_angle;
		CDBG("%s:%d init params mode %d pos %d mount %d\n", __func__,
			__LINE__,
			cdata->cfg.sensor_init_params.modes_supported,
			cdata->cfg.sensor_init_params.position,
			cdata->cfg.sensor_init_params.sensor_mount_angle);
		break;
	case CFG_SET_SLAVE_INFO: {
		struct msm_camera_sensor_slave_info *sensor_slave_info = NULL;
		struct msm_sensor_power_setting_array *power_setting_array;
		int slave_index = 0;
		sensor_slave_info =
			kmalloc(sizeof(struct msm_camera_sensor_slave_info)
			* 1, GFP_KERNEL);
			if (!sensor_slave_info) {
				rc = -ENOMEM;
			break;
		}

		if (copy_from_user(sensor_slave_info,
				(void *)cdata->cfg.setting,
				sizeof(struct msm_camera_sensor_slave_info))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		/* Update sensor slave address */
		if (sensor_slave_info->slave_addr) {
			s_ctrl->sensor_i2c_client->cci_client->sid =
				sensor_slave_info->slave_addr >> 1;
		}

		/* Update sensor address type */
		s_ctrl->sensor_i2c_client->addr_type =
			sensor_slave_info->addr_type;

		/* Update power up / down sequence */
		s_ctrl->power_setting_array =
			sensor_slave_info->power_setting_array;
		power_setting_array = &s_ctrl->power_setting_array;
		power_setting_array->power_setting = kzalloc(
			power_setting_array->size *
			sizeof(struct msm_sensor_power_setting), GFP_KERNEL);
		if (!power_setting_array->power_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(power_setting_array->power_setting,
		(void *)sensor_slave_info->power_setting_array.power_setting,
		power_setting_array->size *
		sizeof(struct msm_sensor_power_setting))) {
			kfree(power_setting_array->power_setting);
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info->slave_addr);
		CDBG("%s sensor addr type %d\n", __func__,
			sensor_slave_info->addr_type);
		CDBG("%s sensor reg %x\n", __func__,
			sensor_slave_info->sensor_id_info.sensor_id_reg_addr);
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info->sensor_id_info.sensor_id);
		for (slave_index = 0; slave_index <
			power_setting_array->size; slave_index++) {
			CDBG("%s i %d power setting %d %d %ld %d\n", __func__,
				slave_index,
				power_setting_array->power_setting[slave_index].
				seq_type,
				power_setting_array->power_setting[slave_index].
				seq_val,
				power_setting_array->power_setting[slave_index].
				config_val,
				power_setting_array->power_setting[slave_index].
				delay);
		}
		kfree(power_setting_array->power_setting);
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
			s_ctrl->sensor_i2c_client, &conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {
		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}

	case CFG_POWER_UP:
		if (s_ctrl->func_tbl->sensor_power_up)
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_POWER_DOWN:
		if (s_ctrl->func_tbl->sensor_power_down)
			rc = s_ctrl->func_tbl->sensor_power_down(
				s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_SET_STOP_STREAM_SETTING: {
		struct msm_camera_i2c_reg_setting *stop_setting =
			&s_ctrl->stop_setting;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		if (copy_from_user(stop_setting, (void *)cdata->cfg.setting,
				sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = stop_setting->reg_setting;
		stop_setting->reg_setting = kzalloc(stop_setting->size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!stop_setting->reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(stop_setting->reg_setting,
				(void *)reg_setting, stop_setting->size *
				sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(stop_setting->reg_setting);
			stop_setting->reg_setting = NULL;
			stop_setting->size = 0;
			rc = -EFAULT;
			break;
		}
		break;
	}
	case CFG_SET_SATURATION: {
		int32_t sat_lev;
		if (copy_from_user(&sat_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Saturation Value is %d", __func__, sat_lev);
		gc2145_8909_set_saturation(s_ctrl, sat_lev);
		break;
	}
	case CFG_SET_SHARPNESS: {
		int32_t shp_lev;
		if (copy_from_user(&shp_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_debug("%s: Sharpness Value is %d", __func__, shp_lev);
		gc2145_8909_set_sharpness(s_ctrl, shp_lev);
		break;
	}
	case CFG_SET_CONTRAST: {
		int32_t con_lev;
		if (copy_from_user(&con_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_debug("%s: Contrast Value is %d", __func__, con_lev);
		gc2145_8909_set_contrast(s_ctrl, con_lev);
		break;
	}
	case CFG_SET_ISO: {
		int32_t iso_lev;
		if (copy_from_user(&iso_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_debug("%s: ISO Value is %d", __func__, iso_lev);
		gc2145_8909_set_iso(s_ctrl, iso_lev);
		break;
	}
	case CFG_SET_EXPOSURE_COMPENSATION: {
		int32_t ec_lev;
		if (copy_from_user(&ec_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_debug("%s: Exposure compensation Value is %d",
			__func__, ec_lev);
		gc2145_8909_set_exposure_compensation(s_ctrl, ec_lev);
		break;
	}
	case CFG_SET_ANTIBANDING: {
		int32_t antibanding_mode;
		if (copy_from_user(&antibanding_mode,
			(void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_info("%s: anti-banding mode is %d", __func__,
			antibanding_mode);
		gc2145_8909_set_antibanding(s_ctrl, antibanding_mode);
		break;
	}
	case CFG_SET_EFFECT: {
		int32_t effect_mode;
		if (copy_from_user(&effect_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_debug("%s: Effect mode is %d", __func__, effect_mode);
		gc2145_8909_set_effect(s_ctrl, effect_mode);
		break;
	}

	case CFG_SET_BESTSHOT_MODE: {
		int32_t bs_mode;
		if (copy_from_user(&bs_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_debug("%s: best shot mode is %d", __func__, bs_mode);
		gc2145_8909_set_scene_mode(s_ctrl, bs_mode);
		break;
	}
	case CFG_SET_WHITE_BALANCE: {
		int32_t wb_mode;
		if (copy_from_user(&wb_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_debug("%s: white balance is %d", __func__, wb_mode);
		gc2145_8909_set_white_balance_mode(s_ctrl, wb_mode);
		break;
	}
	default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}

static struct msm_sensor_fn_t gc2145_8909_sensor_func_tbl = {
	.sensor_config = gc2145_8909_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = msm_sensor_match_id,
};

static struct msm_sensor_ctrl_t gc2145_8909_s_ctrl = {
	.sensor_i2c_client = &gc2145_8909_sensor_i2c_client,
	.power_setting_array.power_setting = gc2145_8909_power_setting,
	.power_setting_array.size = ARRAY_SIZE(gc2145_8909_power_setting),
	.msm_sensor_mutex = &gc2145_8909_mut,
	.sensor_v4l2_subdev_info = gc2145_8909_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(gc2145_8909_subdev_info),
	.func_tbl = &gc2145_8909_sensor_func_tbl,
};

module_init(gc2145_8909_init_module);
module_exit(gc2145_8909_exit_module);
MODULE_DESCRIPTION("gc2145_8909 2MP YUV sensor driver");
MODULE_LICENSE("GPL v2");

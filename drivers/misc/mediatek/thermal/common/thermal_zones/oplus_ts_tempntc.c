// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
#define pr_fmt(fmt)	"[OPLUS_TEMPNTC] %s: " fmt, __func__

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/acpi.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <mt-plat/aee.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "mt-plat/mtk_thermal_monitor.h"
#include "mach/mtk_thermal.h"
#include <linux/uidgid.h>
#include <linux/slab.h>
#include <soc/oplus/system/oplus_project.h>
#include <linux/power_supply.h>
#include <linux/iio/consumer.h>
#include <linux/kthread.h>	/* For Kthread_run */
#include <linux/of_gpio.h>

#define CHGNTC_ENABLE_LEVEL		     	(1)
#define CHGNTC_DISABLE_LEVEL			(0)
#define  BOARD_TEMP_GPIO_NUM   154
#define OPLUS_DALEY_MS 300
struct temp_data {
	struct platform_device *pdev;
	int batt_id_volt;
	int flash_ntc_volt;
	int charger_ntc_volt;
	int pa_ntc_volt;
	int bb_ntc_volt;
	int bat_con_ntc_volt;
	struct task_struct      *oplus_tempntc_kthread;
	struct iio_channel      *iio_channel_btb;
	struct iio_channel      *iio_channel_ftp;
	struct iio_channel      *iio_channel_btc;
	struct iio_channel      *iio_channel_bb;
	struct iio_channel      *iio_channel_pa;
	struct iio_channel      *iio_channel_flash;
	struct iio_channel      *iio_channel_batid;
	struct iio_channel      *iio_channel_charger;
	struct power_supply *ac_psy;
	struct pinctrl *pinctrl;
	struct pinctrl_state *ntc_switch1_ctrl_high;
	struct pinctrl_state *ntc_switch1_ctrl_low;
	struct pinctrl_state *ntc_switch2_ctrl_high;
	struct pinctrl_state *ntc_switch2_ctrl_low;
	struct delayed_work init_work;
	struct completion temp_det_start;
	struct mutex det_lock;
	bool is_kthread_get_adc;
	int ntcswitch1_pin;
	int ntcswitch2_pin;
	bool disable_ntc_switch;
};
static struct temp_data *pinfo;
extern void oplus_gpio_switch_lock(void);
extern void oplus_gpio_switch_unlock(void);
extern void oplus_gpio_value_switch(unsigned int pin, unsigned int val);

int get_flash_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->flash_ntc_volt;
}

int get_batt_id_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->batt_id_volt;
}

int get_charger_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->charger_ntc_volt;
}

int get_pa_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->pa_ntc_volt;
}

int get_bb_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->bb_ntc_volt;
}

int get_bat_con_ntc_volt(void)
{
    if (!pinfo)
        return -1;
    return pinfo->bat_con_ntc_volt;
}

bool is_kthread_get_adc(void)
{
    if (!pinfo)
        return 0;
    return pinfo->is_kthread_get_adc;
}

void oplus_tempntc_read_ntcswitch1_high(struct temp_data *info) {
	int ret;
        int iio_chan1_volt;
        int iio_chan3_volt;

	ret = iio_read_channel_processed(info->iio_channel_ftp, &iio_chan1_volt);
	if (ret < 0) {
		pr_err("PA_NTC read error!\n");
	} else {
		info->pa_ntc_volt = iio_chan1_volt;
	}

	ret = iio_read_channel_processed(info->iio_channel_btc, &iio_chan3_volt);
	if (ret < 0) {
		pr_err("CHARGE_NTC read error!\n");
	} else {
		info->charger_ntc_volt = iio_chan3_volt;
	}
}

void oplus_tempntc_read_ntcswitch1_low(struct temp_data *info) {
	int ret;
	int iio_chan1_volt;
	int iio_chan3_volt;

	ret = iio_read_channel_processed(info->iio_channel_ftp, &iio_chan1_volt);
	if (ret < 0) {
		pr_err("FLASH_NTC read error!\n");
	} else {
		info->flash_ntc_volt = iio_chan1_volt;
	}

	ret = iio_read_channel_processed(info->iio_channel_btc, &iio_chan3_volt);
	if (ret < 0) {
		pr_err("BAT_ID read error!\n");
	} else {
		info->batt_id_volt = iio_chan3_volt;
	}
}

void oplus_tempntc_read_ntcswitch2_high(struct temp_data *info) {
	int ret;
        int iio_chan0_volt;

	ret = iio_read_channel_processed(info->iio_channel_btb, &iio_chan0_volt);
	if (ret < 0) {
		pr_err("BAT_CON_NTC read error!\n");
	} else {
		info->bat_con_ntc_volt = iio_chan0_volt;
	}

}

void oplus_tempntc_read_ntcswitch2_low(struct temp_data *info) {
	int ret;
	int iio_chan0_volt;

	ret = iio_read_channel_processed(info->iio_channel_btb, &iio_chan0_volt);
	if (ret < 0) {
		pr_err("BB_NTC read error!\n");
	} else {
		info->bb_ntc_volt = iio_chan0_volt;
	}
}

int oplus_tempntc_get_volt(struct temp_data *info)
{
	int ret;
	int ntcswitch1_gpio_value = 0;
	int ntcswitch2_gpio_value = 0;
	int iio_chan_bb;
	int iio_chan_pa;
	int iio_chan_flash;
	int iio_chan_batid;
	int iio_chan_charger;
	if(pinfo->disable_ntc_switch){
		pr_err("Get tempntc This project will use ntc switch  \n");
		if ((!info) || (!info->iio_channel_bb) || (!info->iio_channel_pa)|| (!info->iio_channel_batid)
			|| (!info->iio_channel_flash) ||(!info->iio_channel_charger)){
			pr_err("conntinue\n");
			return 0;
		}

		ret = iio_read_channel_processed(info->iio_channel_bb, &iio_chan_bb);
		if (ret < 0) {
			pr_err("BB_NTC read error!\n");
		}
		else {
			info->bb_ntc_volt = iio_chan_bb;
		}
		ret = iio_read_channel_processed(info->iio_channel_pa, &iio_chan_pa);
		if (ret < 0) {
			pr_err("PA_NTC read error!\n");
		}
		else {
			info->pa_ntc_volt = iio_chan_pa;
		}
		ret = iio_read_channel_processed(info->iio_channel_flash, &iio_chan_flash);
		if (ret < 0) {
			pr_err("FLASH_NTC read error!\n");
		} else {
			info->flash_ntc_volt = iio_chan_flash;
		}
		ret = iio_read_channel_processed(info->iio_channel_batid, &iio_chan_batid);
		if (ret < 0) {
			pr_err("BATTERY_ID_NTC read error!\n");
		}
		else {
			info->batt_id_volt = iio_chan_batid;
		}
		ret = iio_read_channel_processed(info->iio_channel_charger, &iio_chan_charger);
		if (ret < 0) {
			pr_err("CHARGER_NTC read error!\n");
			}
		else {
			info->charger_ntc_volt = iio_chan_charger;
		}
	}

	else{
		pr_err("Get tempntc This project will not use ntc switch  \n");
	if ((!info->iio_channel_btb) || (!info->iio_channel_ftp)
		|| (!info->iio_channel_btc) || (!info)) {
		pr_err("conntinue\n");
		return 0;
	}

	ntcswitch1_gpio_value = gpio_get_value(info->ntcswitch1_pin);
	ntcswitch2_gpio_value = gpio_get_value(info->ntcswitch2_pin);
	pr_err("ntcswitch1_gpio_value = %d , ntcswitch2_gpio_value = %d.\n",
		 ntcswitch1_gpio_value, ntcswitch2_gpio_value);

	if (ntcswitch1_gpio_value == 0) {
		oplus_tempntc_read_ntcswitch1_low(info);
		/* -------------------------------------------------- */
		pinctrl_select_state(info->pinctrl, info->ntc_switch1_ctrl_high);
		msleep(OPLUS_DALEY_MS);

		ret = gpio_get_value(info->ntcswitch1_pin);
		if (ret < 0) {
			pr_err("ntcswitch1_gpio_value = %d\n", ret);
		}
		oplus_tempntc_read_ntcswitch1_high(info);
	} else {
		oplus_tempntc_read_ntcswitch1_high(info);
		/* -------------------------------------------------- */
		pinctrl_select_state(info->pinctrl, info->ntc_switch1_ctrl_low);
		msleep(OPLUS_DALEY_MS);

		ret = gpio_get_value(info->ntcswitch1_pin);
		if (ret < 0) {
			pr_err("ntcswitch1_gpio_value = %d\n", ret);
		}
		oplus_tempntc_read_ntcswitch1_low(info);
	}

	if (ntcswitch2_gpio_value == 0) {
		oplus_tempntc_read_ntcswitch2_low(info);
		/* -------------------------------------------------- */
		pinctrl_select_state(info->pinctrl, info->ntc_switch2_ctrl_high);
		msleep(OPLUS_DALEY_MS);

		ret = gpio_get_value(info->ntcswitch2_pin);
		if (ret < 0) {
			pr_err("ntcswitch2_gpio_value = %d\n", ret);
		}
		oplus_tempntc_read_ntcswitch2_high(info);
	} else {
		oplus_tempntc_read_ntcswitch2_high(info);
		/* -------------------------------------------------- */
		pinctrl_select_state(info->pinctrl, info->ntc_switch2_ctrl_low);
		msleep(OPLUS_DALEY_MS);

		ret = gpio_get_value(info->ntcswitch2_pin);
		if (ret < 0) {
			pr_err("ntcswitch2_gpio_value = %d\n", ret);
		}
		oplus_tempntc_read_ntcswitch2_low(info);
	}
	}

		pinfo->is_kthread_get_adc = true;
	pr_err("BAT_CON_NTC[%d], PA_NTC[%d], CHARGE_NTC[%d], BB_NTC[%d], FLASH_NTC[%d], BAT_ID[%d].\n",
		info->bat_con_ntc_volt, info->pa_ntc_volt, info->charger_ntc_volt,
		info->bb_ntc_volt, info->flash_ntc_volt, info->batt_id_volt);

	return 0;
}

static void oplus_tempntc_det_work(struct work_struct *work)
{
	mutex_lock(&pinfo->det_lock);
	oplus_tempntc_get_volt(pinfo);
	mutex_unlock(&pinfo->det_lock);
	schedule_delayed_work(&pinfo->init_work, msecs_to_jiffies(5000));
}

static int oplus_tempntc_parse_dt(struct temp_data *info,
				struct device *dev)
{
	struct device_node *np = dev->of_node;

	info->iio_channel_btb = iio_channel_get(dev, "auxadc0-bb_or_bat_con_v");
	if (IS_ERR(info->iio_channel_btb)) {
		pr_err("BAT_CON_NTC BB_NTC ERR \n");
	}

	info->iio_channel_ftp = iio_channel_get(dev, "auxadc1-flash_or_pa_v");
	if (IS_ERR(info->iio_channel_ftp)){
		pr_err("Flash PA CHANNEL ERR \n");
	}

	info->iio_channel_btc = iio_channel_get(dev, "auxadc3-bat_id_or_charge_v");
	if (IS_ERR(info->iio_channel_btc)){
		pr_err("BAT_ID CHARGE_NTC CHANNEL ERR \n");
	}

	info->iio_channel_bb = iio_channel_get(dev, "auxadc0-bb_v");
	if (IS_ERR(info->iio_channel_bb)){
		pr_err("BB NTC CHANNEL ERR \n");
	}

	info->iio_channel_pa = iio_channel_get(dev, "auxadc1-pa_v");
	if (IS_ERR(info->iio_channel_pa)){
		pr_err("PA NTC CHANNEL ERR \n");
	}

	info->iio_channel_flash = iio_channel_get(dev, "auxadc2-flash_v");
	if (IS_ERR(info->iio_channel_flash)){
		pr_err("FLASH NTC CHANNEL ERR \n");
	}

	info->iio_channel_batid = iio_channel_get(dev, "auxadc3-bat_id_v");
	if (IS_ERR(info->iio_channel_batid)){
		pr_err("BATTERY ID NTC CHANNEL ERR \n");
	}

	info->iio_channel_charger = iio_channel_get(dev, "auxadc4-charger_v");
	if (IS_ERR(info->iio_channel_charger)){
		pr_err("CHARGER NTC CHANNEL ERR \n");
	}

	info->disable_ntc_switch =
					of_property_read_bool(np, "disable_ntc_switch");

	if(!info->disable_ntc_switch)
		{
	pr_err("This project will use ntc switch  \n");
	info->ntcswitch1_pin = of_get_named_gpio(np, "ntc_switch1_gpio", 0);

	if(info->ntcswitch1_pin < 0)	{
		pr_err("ntc_switch1_gpio < 0 !!! \r\n");
	}

	if (gpio_request(info->ntcswitch1_pin, "NTC_SWITCH1_GPIO") < 0)
		pr_err("ntc_switch1_gpio gpio_request fail !!! \r\n");

	info->ntcswitch2_pin = of_get_named_gpio(np, "ntc_switch2_gpio", 0);
	if(info->ntcswitch2_pin < 0)	{
		pr_err("ntc_switch2_gpio < 0 !!! \r\n");
	}

	if (gpio_request(info->ntcswitch2_pin, "NTC_SWITCH2_GPIO") < 0)
		pr_err("ntc_switch2_gpio gpio_request fail !!! \r\n");
	}

	return 0;
}

static int oplus_tempntc_data_init(struct temp_data *info)
{
	info->flash_ntc_volt = -1;
	info->batt_id_volt = -1;
	info->pa_ntc_volt = -1;
	info->charger_ntc_volt = -1;
	info->bb_ntc_volt = -1;
	info->bat_con_ntc_volt = -1;
	mutex_init(&info->det_lock);

	return 0;
}

static int oplus_ntcctrl_gpio_init(struct temp_data *info,struct device *dev)
{
	if (!info) {
		pr_err("info  null !\n");
		return -EINVAL;
	}

  	info->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(info->pinctrl)) {
		pr_err("get temp ntc princtrl fail\n");
		return -EINVAL;
	}

	info->ntc_switch1_ctrl_high = pinctrl_lookup_state(info->pinctrl, "ntc_switch1_ctrl_high");
	if (IS_ERR_OR_NULL(info->ntc_switch1_ctrl_high)) {
		pr_err("get ntc_switch1_ctrl_high fail\n");
		return -EINVAL;
	}

	info->ntc_switch1_ctrl_low = pinctrl_lookup_state(info->pinctrl, "ntc_switch1_ctrl_low");
	if (IS_ERR_OR_NULL(info->ntc_switch1_ctrl_low)) {
		pr_err("get ntc_switch1_ctrl_low fail\n");
		return -EINVAL;
	}

	info->ntc_switch2_ctrl_high = pinctrl_lookup_state(info->pinctrl, "ntc_switch2_ctrl_high");
	if (IS_ERR_OR_NULL(info->ntc_switch2_ctrl_high)) {
		pr_err("get ntc_switch2_ctrl_high fail\n");
		return -EINVAL;
	}

	info->ntc_switch2_ctrl_low = pinctrl_lookup_state(info->pinctrl, "ntc_switch2_ctrl_low");
	if (IS_ERR_OR_NULL(info->ntc_switch2_ctrl_low)) {
		pr_err("get ntc_switch2_ctrl_low fail\n");
		return -EINVAL;
	}

	pinctrl_select_state(info->pinctrl, info->ntc_switch2_ctrl_low);
	msleep(100);
	pinctrl_select_state(info->pinctrl, info->ntc_switch1_ctrl_low);
	return 0;

}


static int oplus_tempntc_pdrv_probe(struct platform_device *pdev)
{
	struct temp_data *info;

	pr_err("starts\n");
	info = devm_kzalloc(&pdev->dev,sizeof(struct temp_data), GFP_KERNEL);
	if (!info) {
		pr_err(" kzalloc() failed\n");
		return -ENOMEM;
	}

	pinfo = info;
	platform_set_drvdata(pdev, info);
	info->pdev = pdev;

	oplus_tempntc_data_init(info);
	oplus_tempntc_parse_dt(info, &pdev->dev);

	if (!pinfo->disable_ntc_switch){
		pr_err("Probe This project will use ntc switch  \n");
		oplus_ntcctrl_gpio_init(info, &pdev->dev);
	}

	INIT_DELAYED_WORK(&info->init_work, oplus_tempntc_det_work);
	schedule_delayed_work(&info->init_work, msecs_to_jiffies(5000));

	return 0;
}

static int oplus_tempntc_pdrv_remove(struct platform_device *pdev)
{
	return 0;
}

static void oplus_tempntc_pdrv_shutdown(struct platform_device *dev)
{

}

static const struct of_device_id oplus_tempntc_of_match[] = {
	{.compatible = "oplus-tempntc",},
	{},
};
MODULE_DEVICE_TABLE(of, oplus_tempntc_of_match)

static int __maybe_unused tempntc_suspend(struct device *dev)
{
	cancel_delayed_work_sync(&pinfo->init_work);
	return 0;
}

static int __maybe_unused tempntc_resume(struct device *dev)
{
	/* Schedule timer to check current status */
	schedule_delayed_work(&pinfo->init_work,
			msecs_to_jiffies(5000));
	return 0;
}

static SIMPLE_DEV_PM_OPS(tempntc_pm_ops, tempntc_suspend, tempntc_resume);

static struct platform_driver oplus_tempntc_driver = {
	.probe = oplus_tempntc_pdrv_probe,
	.remove = oplus_tempntc_pdrv_remove,
	.shutdown = oplus_tempntc_pdrv_shutdown,
	.driver = {
		.name = "oplus_tempntc",
		.owner = THIS_MODULE,
		.of_match_table = oplus_tempntc_of_match,
		.pm = &tempntc_pm_ops,
	},
};

static int __init oplus_tempntc_init(void)
{
    int ret;
	ret = platform_driver_register(&oplus_tempntc_driver);
    if (ret) {
		pr_err("%s fail to tempntc device\n");
		return ret;
    }
    return ret;
}
late_initcall(oplus_tempntc_init);

static void __exit oplus_tempntc_exit(void)
{
    platform_driver_unregister(&oplus_tempntc_driver);
}
module_exit(oplus_tempntc_exit);

MODULE_AUTHOR("wy.chuang <wy.chuang@mediatek.com>");
MODULE_DESCRIPTION("MTK Gauge Device Driver");
MODULE_LICENSE("GPL");

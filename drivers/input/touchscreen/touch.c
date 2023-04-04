/***************************************************
 * File:touch.c
 * Copyright (c)  2008- 2030  oplus Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * TAG: BSP.TP.Init
*/

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include "oplus_touchscreen/Synaptics/S3508/synaptics_s3508.h"
#include "oplus_touchscreen/tp_devices.h"
#include "oplus_touchscreen/touchpanel_common.h"
#include <soc/oplus/system/oplus_project.h>
#include <soc/oplus/device_info.h>
#include "touch.h"

#define MAX_LIMIT_DATA_LENGTH         100
#define S3706_FW_NAME_18073 "tp/18073/18073_FW_S3706_SYNAPTICS.img"
#define S3706_BASELINE_TEST_LIMIT_NAME_18073 "tp/18073/18073_Limit_data.img"
#define S3706_FW_NAME_19301 "tp/19301/19301_FW_S3706_SYNAPTICS.img"
#define S3706_BASELINE_TEST_LIMIT_NAME_19301 "tp/19301/19301_Limit_data.img"
#define GT9886_FW_NAME_19551 "tp/19551/FW_GT9886_SAMSUNG.img"
#define GT9886_BASELINE_TEST_LIMIT_NAME_19551 "tp/19551/LIMIT_GT9886_SAMSUNG.img"
#define GT9886_FW_NAME_19357 "tp/19357/FW_GT9886_SAMSUNG.img"
#define GT9886_BASELINE_TEST_LIMIT_NAME_19357 "tp/19357/LIMIT_GT9886_SAMSUNG.img"
#define GT9886_FW_NAME_19537 "tp/19537/FW_GT9886_SAMSUNG.img"
#define GT9886_BASELINE_TEST_LIMIT_NAME_19537 "tp/19537/LIMIT_GT9886_SAMSUNG.img"
#define TD4330_NF_CHIP_NAME "TD4330_NF"

extern char *saved_command_line;
int g_tp_dev_vendor = TP_UNKNOWN;
int g_tp_prj_id = 0;
struct hw_resource *g_hw_res;
int tp_type = 0;
int ret = 0;
static bool is_tp_type_got_in_match = false;    /*indicate whether the tp type is specified in the dts*/


/*if can not compile success, please update vendor/oplus_touchsreen*/
struct tp_dev_name tp_dev_names[] =
{
	{TP_OFILM, "OFILM"},
	{TP_BIEL, "BIEL"},
	{TP_TRULY, "TRULY"},
	{TP_BOE, "BOE"},
	{TP_G2Y, "G2Y"},
	{TP_TPK, "TPK"},
	{TP_JDI, "JDI"},
	{TP_TIANMA, "TIANMA"},
	{TP_SAMSUNG, "SAMSUNG"},
	{TP_DSJM, "DSJM"},
	{TP_BOE_B8, "BOEB8"},
	{TP_INNOLUX, "INNOLUX"},
	{TP_HIMAX_DPT, "DPT"},
	{TP_AUO, "AUO"},
	{TP_DEPUTE, "DEPUTE"},
	{TP_HUAXING, "HUAXING"},
	{TP_HLT, "HLT"},
	{TP_DJN, "DJN"},
	{TP_BOE_B3, "BOE"},
	{TP_CDOT, "CDOT"},
	{TP_INX, "INX"},
	{TP_INX, "TXD"},
	{TP_UNKNOWN, "UNKNOWN"},
	{TP_HLT, "HLT"},
};

typedef enum {
	TP_INDEX_NULL,
	icnl9911c_txd,
	himax_83112a,
	himax_83112f,
	ili9881_auo,
	ili9881_tm,
	nt36525b_boe,
	nt36525b_inx,
	ili9882n_cdot,
	ili9882n_hlt,
	ili9882n_inx,
	ili9882n_inx_72,
	nt36525b_hlt,
	nt36672c,
	ili9881_inx,
	goodix_gt9886,
	focal_ft3518,
	td4330,
	himax_83112b,
	himax_83102d,
	ili7807s_tm,
	ili7807s_tianma,
	ili7807s_jdi,
	hx83102d_txd,
	ft8006s_truly,
	ili9882n_truly,
	ili7807s_boe,
	ili7807s_hlt,
	nt36672c_boe,
	focal_ft3681,
} TP_USED_INDEX;
TP_USED_INDEX tp_used_index  = TP_INDEX_NULL;

#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")

#ifndef CONFIG_MTK_FB
void primary_display_esd_check_enable(int enable)
{
	return;
}
EXPORT_SYMBOL(primary_display_esd_check_enable);
#endif /*CONFIG_MTK_FB*/

bool __init tp_judge_ic_match(char *tp_ic_name)
{
	int prj_id = 0;
	pr_err("[TP] tp_ic_name = %s \n", tp_ic_name);
	prj_id = get_project();

	switch(prj_id)
	{
	case 20131:
		if(strstr(saved_command_line, "20131_tianma_nt37701_1080p_dsi_cmd")) {
			pr_info("tp judge ic match 20131 20131_tianma_nt37701_1080p_dsi_cmd\n");
			return true;
		} else if (strstr(saved_command_line, "20131_tianma_nt37701_32_1080p_dsi_cmd")) {
			pr_info("tp judge ic match 20131 20131_tianma_nt37701_32_1080p_dsi_cmd\n");
			return true;
		} else if (strstr(saved_command_line, "20131_boe_nt37800_1080p_dsi_cmd")) {
			pr_info("tp judge ic match 20131 20131_boe_nt37800_1080p_dsi_cmd\n");
			return true;
		} else if (strstr(saved_command_line, "20131_samsung_ana6705_1080p_dsi_cmd")) {
			pr_info("tp judge ic match 20131 20131_samsung_ana6705_1080p_dsi_cmd\n");
			return true;
		}
		pr_info("tp judge ic match failed 20131\n");
		return false;
	case 22603:
	case 22604:
	case 22609:
	case 0x2260A:
	case 0x2260B:
	case 22669:
	case 0x2266A:
	case 0x2266B:
	case 0x2266C:
		pr_info("tp judge ic forward for blade\n");
		if (strstr(tp_ic_name, "himax,hx83102d_nf") && strstr(saved_command_line, "hx83102d")) {
			pr_info("tp judge ic hx83102d blade\n");
			return true;
		}

		if (strstr(tp_ic_name, "hx83112f_nf") && strstr(saved_command_line, "hx83112f")) {
			pr_info("tp judge ic hx83112f blade\n");
			return true;
		}

		if (strstr(tp_ic_name, "chipone,icnl9911c") && strstr(saved_command_line, "icnl9911c_txd")) {
			pr_info("[TP] blade 22603 %s matched\n", tp_ic_name);
                        tp_used_index = icnl9911c_txd;
			return true;
		}
		pr_info("tp judge ic match failed blade\n");
	case 22083:
	case 22084:
	case 22291:
	case 22292:
		pr_info("tp judge ic forward for Lijing\n");
		if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili9883c")) {
			pr_info("tp judge ic ili9883c Lijing\n");
			return true;
		}

		if (strstr(tp_ic_name, "td4160") && strstr(saved_command_line, "td4160")) {
			pr_info("tp judge ic td4160 Lijing\n");
			return true;
		}
		pr_info("tp judge ic match failed Lijing\n");
		return false;
	case 0x210A0:
		pr_info("[TP] case 210A0\n");
		if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili7807s_tm")) {
			pr_info("[TP] touch judge ic = ilitek,ili7807s,TIANMA\n");
			return true;
		}
		if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili7807s_jdi")) {
			pr_info("[TP] touch judge ic = ilitek,ili7807s,JDI\n");
			return true;
		}
		pr_info("tp judge ic match failed 210A0\n");
		return false;
		return false;
	case 21639:
	case 0x2163B:
	case 0x2163C:
	case 0x2163D:
	case 0x216CD:
	case 0x216CE:
		if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "oplus2163b_ili9883a")) {
			return true;
		}

		if (strstr(tp_ic_name, "td4160") && strstr(saved_command_line, "oplus2163b_td4160_inx_hd_vdo_lcm_drv")) {
			return true;
		}

		if (strstr(tp_ic_name, "novatek,nf_nt36672c") && strstr(saved_command_line, "oplus21639_nt36672c_tm_cphy_dsi_vdo_lcm_drv")) {
			return true;
		}

		if (strstr(tp_ic_name, "td4160") && strstr(saved_command_line, "oplus2163b_td4160_truly_hd_vdo_lcm_drv")) {
			return true;
		}
		return false;
	case 20015:
	case 20013:
	case 20016:
	case 20108:
	case 20109:
	case 20307:
		pr_info("[TP] tp_judge_ic_match case 20015\n");
		if (strstr(tp_ic_name, "nf_nt36672c") && strstr(saved_command_line, "nt36672c")) {
			pr_info("tp judge ic match 20015 nf_nt36525b\n");
			return true;
		}
		if (strstr(tp_ic_name, "nf_nt36525b") && strstr(saved_command_line, "nt35625b_boe_hlt_b3_vdo_lcm_drv")) {
			pr_info("tp judge ic match 20015 nf_nt36525b\n");
			return true;
		}
		if (strstr(tp_ic_name, "ili7807s") && strstr(saved_command_line, "ili9882n")) {
			pr_info("tp judge ic match 20015 ili9882n\n");
			return true;
		}
		if (strstr(tp_ic_name, "hx83102") && strstr(saved_command_line, "hx83102_truly_vdo_hd_dsi_lcm_drv")) {
			pr_info("tp judge ic match 20015 hx83102\n");
			return true;
		}
		pr_info("tp judge ic match failed 20015\n");
		return false;
	case 20608:
	case 20609:
	case 0x2060A:
	case 0x2060B:
	case 0x206F0:
	case 0x206FF:
	case 20796:
	case 20795:
	case 0x2070B:
	case 0x2070C:
	case 0x2070E:
	case 21625:
	case 0x216A0:
	case 0x216A1:
	case 0x216B5:
	case 0x216B6:
	case 21747:
	case 21748:
	case 21749:
		pr_info("[TP] case 20609\n");
		if (strstr(tp_ic_name, "novatek,nf_nt36672c") && strstr(saved_command_line, "oplus20609_nt36672c_tm_cphy_dsi_vdo_lcm_drv")) {
			pr_info("[TP] touch judge ic = novatek,nf_nt36672c,TIANMA\n");
			return true;
		}
		if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "oplus20609_ili7807s_tm_cphy_dsi_vdo_lcm_drv")) {
			pr_info("[TP] touch judge ic = ilitek,ili9882n(ili7807s),TIANMA\n");
			return true;
		}
		return false;
	case 21251:
	case 21253:
	case 21254:
		if (strstr(tp_ic_name, "ili9882n")) {
			return true;
		}
		pr_err("[TP] ERROR! ic is not match driver\n");
		return false;
	case 132769:
		pr_info("[TP] tp judge ic forward for 206A1\n");
		if (strstr(tp_ic_name, "nf_nt36525b") && strstr(saved_command_line, "nt36525b_hlt"))
		{
			return true;
		}
		else if (strstr(tp_ic_name, "hx83102d_nf") && strstr(saved_command_line, "hx83102d"))
		{
			return true;
		}
		else if (strstr(tp_ic_name, "ili9881h") && strstr(saved_command_line, "ilt9881h"))
		{
			return true;
		}
		pr_err("[TP] ERROR! ic is not match driver\n");
		return false;
	case 20375: /*jelly*/
	case 20376:
	case 20377:
	case 20378:
	case 20379:
	case 131962:/*2037A*/
		pr_info("[TP] tp judge ic forward for %d.\n", get_project());
		if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt7807s")) {
			return true;
		} else if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt9882n_innolux")) {
			return true;
		} else if (strstr(tp_ic_name, "ili9882n") && strstr(saved_command_line, "ilt9882q_innolux")) {
			return true;
		} else if (strstr(tp_ic_name, "hx83102d_nf") && strstr(saved_command_line, "hx83102d_txd")) {
			return true;
		}
		pr_err("[TP] ERROR! jelly tp ic is not match driver\n");
		return false;
	case 21331: /*zhaoyun*/
	case 21332:
	case 21333:
	case 21334:
	case 21335:
	case 21336:
	case 21337:
	case 21338:
	case 21339:
	case 21361: /*zhaoyun-lite*/
	case 21362:
	case 21363:
	case 21107:
	case 22875:
	case 22876:
	case 22261: /*limu*/
	case 22262:
	case 22263:
	case 22264:
	case 22265:
	case 22266:
	case 22081:
		is_tp_type_got_in_match = true;
		if (strstr(tp_ic_name, "td4160") && strstr(saved_command_line, "td4160_inx")) {
			g_tp_dev_vendor = TP_INNOLUX;
                        return true;
                }
		else if (strstr(tp_ic_name, "td4160") && strstr(saved_command_line, "td4160_truly")) {
			g_tp_dev_vendor = TP_TRULY;
                        return true;
                }
		else if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili9883c_hlt")) {
			g_tp_dev_vendor = TP_HLT;
                        return true;
                }
		else if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili9883c_yfhlt")) {
			g_tp_dev_vendor = TP_HLT;
                        return true;
                }
		else if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "ili9883c_boe")) {
			g_tp_dev_vendor = TP_BOE;
                        return true;
                }
		return false;
	case 21101:
	case 21102:
	case 21235:
	case 21236:
	case 21831:
		if (strstr(tp_ic_name, "ilitek,ili7807s") && strstr(saved_command_line, "oplus21101_ili9883a")) {
			return true;
		}

		if (strstr(tp_ic_name, "novatek,nf_nt36672c") && strstr(saved_command_line, "oplus21101_nt36672c_tm_cphy_dsi_vdo_lcm_drv")) {
			return true;
		}

		if (strstr(tp_ic_name, "td4160") && strstr(saved_command_line, "oplus21101_td4160")) {
			return true;
		}
		return false;
	case 22612:
	case 22693:
	case 22694:
	case 0x226B1:
		pr_info("[TP] tp judge ic forward for 22612\n");
		if (strstr(tp_ic_name, "focaltech-FT3681") && strstr(saved_command_line, "oplus22612_tianma_ili7838a_fhd_dsi_cmd_lcm_drv")) {
			return true;
		}
		if (strstr(tp_ic_name, "focaltech-FT3681") && strstr(saved_command_line, "oplus22612_tianma_ili7838a_t7_plus_fhd_dsi_cmd_lcm_drv")) {
			return true;
		}
		return false;
	default:
		break;
	}
	return true;
}
EXPORT_SYMBOL(tp_judge_ic_match);

bool  tp_judge_ic_match_commandline(struct panel_info *panel_data)
{
	int prj_id = 0;
	int i = 0;
	bool ic_matched = false;
	prj_id = get_project();

	pr_err("[TP] get_project() = %d \n", prj_id);
	pr_err("[TP] boot_command_line = %s \n", saved_command_line);
	for(i = 0; i < panel_data->project_num; i++)
	{
		if(prj_id == panel_data->platform_support_project[i])
		{
			g_tp_prj_id = panel_data->platform_support_project_dir[i];
			pr_err("[TP] platform_support_commandline = %s \n", panel_data->platform_support_commandline[i]);
			if(strstr(saved_command_line, panel_data->platform_support_commandline[i]) || strstr("default_commandline", panel_data->platform_support_commandline[i]) )
			{
				pr_err("[TP] Driver match the project\n");
				ic_matched = true;
			}
		}
	}

	if (!ic_matched) {
		pr_err("[TP] Driver does not match the project\n");
		pr_err("Lcd module not found\n");
		return false;
	}

	switch (prj_id) {
	case 20615:
	case 20662:
	case 20619:
	case 21609:
	case 21651:
		pr_info("[TP] case 20615/20662/20619/21609/21651\n");
		is_tp_type_got_in_match = true;
		pr_err("[TP] touch ic = focal_ft3518 \n");
		tp_used_index = focal_ft3518;
		g_tp_dev_vendor = TP_SAMSUNG;
		break;
	case 22603:
	case 22604:
	case 22609:
	case 0x2260A:
	case 0x2260B:
	case 22669:
	case 0x2266A:
	case 0x2266B:
	case 0x2266C:
		pr_info("[TP] case blade\n");
		is_tp_type_got_in_match = true;
		if (strstr(saved_command_line, "hx83102d")) {
			tp_used_index = himax_83102d;
			g_tp_dev_vendor = TP_TIANMA;
		}

		if (strstr(saved_command_line, "hx83112f")) {
			tp_used_index = himax_83112f;
			g_tp_dev_vendor = TP_TXD;
		}

		if (strstr(saved_command_line, "icnl9911c_txd")) {
			tp_used_index = icnl9911c_txd;
			g_tp_dev_vendor = TP_TXD;
	case 0x210A0:
		pr_info("[TP] enter case 210A0\n");
		is_tp_type_got_in_match = true;
		if (strstr(saved_command_line, "ili7807s_tm")) {
			pr_info("[TP] touch ic = novatek,ili9882n(ili7807s) tianma\n");
			g_tp_dev_vendor = TP_TIANMA;
			tp_used_index = ili7807s_tianma;
		}
		if (strstr(saved_command_line, "ili7807s_jdi")) {
			pr_info("[TP] touch ic = novatek,ili9882n(ili7807s) jdi\n");
			g_tp_dev_vendor = TP_JDI;
			tp_used_index = ili7807s_jdi;
		}
		break;
		}
		break;
	case 20391:
		pr_info("[TP] case %d,\n", prj_id);
		is_tp_type_got_in_match = true;
		pr_err("[TP] touch ic = focaltech3518 \n");
		tp_used_index = focal_ft3518;
		g_tp_dev_vendor = TP_SAMSUNG;
		break;
	case 20015:
	case 20013:
	case 20016:
	case 20108:
	case 20109:
	case 20307:
		pr_info("[TP] case 20015\n");

		is_tp_type_got_in_match = true;

		if (strstr(saved_command_line, "nt36672c")) {
			pr_err("[TP] touch ic = nt36672c_jdi\n");
			tp_used_index = nt36672c;
			g_tp_dev_vendor = TP_JDI;
		}
		if (strstr(saved_command_line, "20625_nt35625b_boe_hlt_b3_vdo_lcm_drv")) {
			pr_info("[TP] touch ic = novatek,nf_nt36525b\n");
			g_tp_dev_vendor = TP_INX;
			tp_used_index = nt36525b_boe;
		}
		if (strstr(saved_command_line, "20015_ili9882n_truly_hd_vdo_lcm_drv")) {
			pr_info("[TP] touch ic = tchip,ilitek\n");
			g_tp_dev_vendor = TP_CDOT;
			tp_used_index = ili9882n_cdot;
		}
		if (strstr(saved_command_line, "20015_ili9882n_boe_hd_vdo_lcm_drv")) {
			pr_info("[TP] touch ic = tchip,ilitek,HLT\n");
			g_tp_dev_vendor = TP_HLT;
			tp_used_index = ili9882n_hlt;
		}
		if (strstr(saved_command_line, "20015_ili9882n_innolux_hd_vdo_lcm_drv")) {
			pr_info("[TP] touch ic = tchip,ilitek,INX\n");
			g_tp_dev_vendor = TP_INX;
			tp_used_index = ili9882n_inx;
		}
		if (strstr(saved_command_line, "20015_hx83102_truly_vdo_hd_dsi_lcm_drv")) {
			pr_info("[TP] touch ic = himax,hx83102d_nf,TRULY\n");
			g_tp_dev_vendor = TP_TRULY;
			tp_used_index = himax_83102d;
		}
		break;
	case 21101:
	case 21102:
	case 21235:
	case 21236:
	case 21831:
                is_tp_type_got_in_match = true;

                if (strstr(saved_command_line, "oplus21101_ili9883a")) {
                        tp_used_index = ili9881_auo;
                        g_tp_dev_vendor = TP_BOE;
                }

                if (strstr(saved_command_line, "oplus21101_nt36672c_tm_cphy_dsi_vdo_lcm_drv")) {
                        pr_err("[TP] touch ic = nf_nt36672c \n");
                        tp_used_index = nt36672c;
                        g_tp_dev_vendor = TP_TIANMA;
                }

                if (strstr(saved_command_line, "oplus21101_td4160")) {
                        pr_err("[TP] touch ic = td4160 \n");
                        tp_used_index = ili9881_auo;
                        g_tp_dev_vendor = TP_INX;
                }
                break;

	case 21639:
	case 0x2163B:
	case 0x2163C:
	case 0x2163D:
	case 0x216CD:
	case 0x216CE:
                is_tp_type_got_in_match = true;

                if (strstr(saved_command_line, "oplus2163b_ili9883a")) {
                        tp_used_index = ili9881_auo;
                        g_tp_dev_vendor = TP_BOE;
                }

                if (strstr(saved_command_line, "oplus2163b_td4160_inx_hd_vdo_lcm_drv")) {
                        pr_err("[TP] touch ic = td4160 \n");
                        tp_used_index = ili9881_auo;
                        g_tp_dev_vendor = TP_INX;
                }

                if (strstr(saved_command_line, "oplus21639_nt36672c_tm_cphy_dsi_vdo_lcm_drv")) {
                        pr_err("[TP] touch ic = nf_nt36672c \n");
                        tp_used_index = nt36672c;
                        g_tp_dev_vendor = TP_TIANMA;
                }

                if (strstr(saved_command_line, "oplus2163b_td4160_truly_hd_vdo_lcm_drv")) {
                        pr_err("[TP] touch ic = td4160 \n");
                        tp_used_index = ili9881_auo;
                        g_tp_dev_vendor = TP_TRULY;
                }
                break;

	case 21061:
	case 20827:
	case 20831:
	case 20181:
	case 20355:
	case 21081:
	case 21851:
	case 0x212A1:
	case 21881:
	case 21882:
		pr_info("[TP] case %d,\n", prj_id);
		is_tp_type_got_in_match = true;
		pr_err("[TP] touch ic = focaltech3518 \n");
		tp_used_index = focal_ft3518;
		g_tp_dev_vendor = TP_SAMSUNG;
		break;
	/* bringup add for roma touchscreen.*/
	case 22612:
	case 22693:
	case 22694:
	case 0x226B1:
		pr_info("[TP] case 22612\n");
		is_tp_type_got_in_match = true;
		if (strstr(saved_command_line, "oplus22612_tianma_ili7838a_fhd_dsi_cmd_lcm_drv")) {
			tp_used_index = focal_ft3681;
			g_tp_dev_vendor = TP_TIANMA;
		}
		if (strstr(saved_command_line, "oplus22612_tianma_ili7838a_t7_plus_fhd_dsi_cmd_lcm_drv")) {
			tp_used_index = focal_ft3681;
			g_tp_dev_vendor = TP_HLT;
		}
		break;
	case 22083:
	case 22084:
	case 22291:
	case 22292:
		pr_info("[TP] case Lijing\n");
		is_tp_type_got_in_match = true;
		if (strstr(saved_command_line, "ili9883c")) {
			pr_info("[TP] touch ic = ili9883c \n");
			tp_used_index = ili9881_auo;
			g_tp_dev_vendor = TP_BOE;
		}

		if (strstr(saved_command_line, "td4160")) {
			pr_info("[TP] touch ic = td4160 \n");
			tp_used_index = ili9881_auo;
			g_tp_dev_vendor = TP_INX;
		}
		break;
	case 20608:
	case 20609:
	case 0x2060A:
	case 0x2060B:
	case 0x206F0:
	case 0x206FF:
	case 20796:
	case 20795:
	case 0x2070B:
	case 0x2070C:
	case 0x2070E:
	case 21625:
	case 0x216A0:
	case 0x216A1:
	case 0x216B5:
	case 0x216B6:
	case 21747:
	case 21748:
	case 21749:
		pr_info("[TP] case 20609\n");
		is_tp_type_got_in_match = true;

		if (strstr(saved_command_line, "oplus20609_nt36672c_tm_cphy_dsi_vdo_lcm_drv")) {
			pr_info("[TP] touch ic = novatek,nf_nt6672c\n");
			g_tp_dev_vendor = TP_TIANMA;
			tp_used_index = nt36672c;
		}
		if (strstr(saved_command_line, "oplus20609_ili7807s_tm_cphy_dsi_vdo_lcm_drv")) {
			pr_info("[TP] touch ic = novatek,ili9882n(ili7807s)\n");
			g_tp_dev_vendor = TP_TIANMA;
			tp_used_index = ili9882n_inx;
		}
		break;
	default:
		pr_info("other project, no need process here!\n");
		break;
	}
	pr_info("[TP]ic:%d, vendor:%d\n", tp_used_index, g_tp_dev_vendor);
	return true;
}
EXPORT_SYMBOL(tp_judge_ic_match_commandline);

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{
	char *vendor;
	int prj_id = 0;
	prj_id = get_project();
	g_hw_res = hw_res;

	panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
	if (panel_data->test_limit_name == NULL)
	{
		pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
	}
	panel_data->extra = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
	if (panel_data->extra == NULL)
	{
		pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
	}
	if (is_tp_type_got_in_match) {
		panel_data->tp_type = g_tp_dev_vendor;
	}
	if (panel_data->tp_type == TP_UNKNOWN)
	{
		pr_err("[TP]%s type is unknown\n", __func__);
		return 0;
	}

	vendor = GET_TP_DEV_NAME(panel_data->tp_type);

	if (prj_id == 22603 || prj_id == 22604 || prj_id == 22609 || prj_id == 0x2260A ||
			prj_id == 0x2260B || prj_id == 22669 || prj_id == 0x2266A
			|| prj_id == 0x2266B || prj_id == 0x2266C) {
		if(strstr(saved_command_line, "hx83102d")) {
			vendor = "LS";
		}
		if(strstr(saved_command_line, "hx83112f")) {
			vendor = "TXD";
		}
		if (strstr(saved_command_line, "icnl9911c_txd")) {
			vendor = "TXD";
		}
	}

	if (prj_id == 20171 || prj_id == 20172 || prj_id == 20353) {
		memcpy(panel_data->manufacture_info.version, "0xaa0100000", 11);
	}

	if (prj_id == 22603) {
		memcpy(panel_data->manufacture_info.version, "txd", 3);
	}
/*BSP.TP.Function, add tp path for Moss.*/
	if (prj_id == 0x2169E || prj_id == 0x2169F || prj_id == 21711 || prj_id == 21712
		|| prj_id == 0x2162D || prj_id == 0x2162E || prj_id == 0x216C9 || prj_id == 0x216CA) {
        memcpy(panel_data->manufacture_info.version, "0xft3518s00", 11);
	}
	if (prj_id == 21015 || prj_id == 21217) {
		memcpy(panel_data->manufacture_info.version, "0xaa2160000", 11);
		if (strstr(saved_command_line, "boe_nt37701_2ftp_1080p_dsi_cmd") || strstr(saved_command_line, "oplus21015_boe_b12_nt37701_1080p_dsi_cmd")) {
			pr_err("BOE panel is double layer");
			vendor = "BOE_TWO";
		}
	}

	if (prj_id == 20181 || prj_id == 20355) {
		memcpy(panel_data->manufacture_info.version, "0xbd3650000", 11);
	}

	if (prj_id == 21851 || prj_id == 0x212A1) {
		memcpy(panel_data->manufacture_info.version, "0xAA3080000", 11);
	}

	if (prj_id == 21081 || prj_id == 21127 || prj_id == 21305) {
		memcpy(panel_data->manufacture_info.version, "0xAA2660000", 11);
	}

	if (strstr(saved_command_line, "20171_tianma_nt37701") || strstr(saved_command_line, "oplus21015_tianma")) {
		hw_res->TX_NUM = 18;
		hw_res->RX_NUM = 40;
		vendor = "TIANMA";
		pr_info("[TP]hw_res->TX_NUM:%d,hw_res->RX_NUM:%d\n",
		    hw_res->TX_NUM,
		    hw_res->RX_NUM);
	}

	if (prj_id == 20827 || prj_id == 20831 || prj_id == 21881 || prj_id == 21882) {
		memcpy(panel_data->manufacture_info.version, "0xaa4110000", 11);
	}

	if (prj_id == 20131 || prj_id == 20133 || prj_id == 20255 || prj_id == 20257
		|| prj_id == 20644 || prj_id == 20645 || prj_id == 20649 || prj_id == 0x2064A || prj_id == 0x2068D) {
		memcpy(panel_data->manufacture_info.version, "0xbd3520000", 11);
	}
	if (prj_id == 21061) {
		memcpy(panel_data->manufacture_info.version, "0xaa2420000", 11);
	}
	if (prj_id == 20615 || prj_id == 20662 || prj_id == 20619 || prj_id == 21609|| prj_id == 21651) {
		memcpy(panel_data->manufacture_info.version, "focalt_", 7);
	}
	if(strstr(saved_command_line, "20131_tianma_nt37701_1080p_dsi_cmd")) {
		hw_res->TX_NUM = 18;
		hw_res->RX_NUM = 40;
		vendor = "TIANMA";
	} else if (strstr(saved_command_line, "20131_tianma_nt37701_32_1080p_dsi_cmd")) {
	    vendor = "TIANMA";
	} else if (strstr(saved_command_line, "20131_boe_nt37800_1080p_dsi_cmd")) {
	    vendor = "BOE";
	} else if (strstr(saved_command_line, "20131_samsung_ana6705_1080p_dsi_cmd")) {
		vendor = "SAMSUNG";
	}

	if (prj_id == 20015 || prj_id == 20013 || prj_id == 20016 \
|| prj_id == 20108 || prj_id == 20109 || prj_id == 20307) {
		pr_info("[TP] tp_util_get_vendor 20015 rename vendor\n");
		if (strstr(saved_command_line, "nt36672c")) {
			vendor = "JDI";
		}
		if (strstr(saved_command_line, "20625_nt35625b_boe_hlt_b3_vdo_lcm_drv")) {
			vendor = "INX";
		}
		if (strstr(saved_command_line, "20015_ili9882n_truly_hd_vdo_lcm_drv")) {
			vendor = "CDOT";
		}
		if (strstr(saved_command_line, "20015_ili9882n_boe_hd_vdo_lcm_drv")) {
			vendor = "HLT";
		}
		if (strstr(saved_command_line, "20015_ili9882n_innolux_hd_vdo_lcm_drv")) {
			vendor = "INX";
		}
		if (strstr(saved_command_line, "20015_hx83102_truly_vdo_hd_dsi_lcm_drv")) {
			vendor = "TRULY";
		}
		pr_info("[TP] we config this for FW name vendor:%s\n", vendor);
	}

	if (prj_id == 22612 || prj_id == 22693 || prj_id == 22694 || prj_id == 0x226B1) {
		pr_info("[TP] tp_util_get_vendor 22612 rename vendor\n");
		if (strstr(saved_command_line, "oplus22612_tianma_ili7838a_fhd_dsi_cmd_lcm_drv")) {
			vendor = "TIANMA";
		}
		if (strstr(saved_command_line, "oplus22612_tianma_ili7838a_t7_plus_fhd_dsi_cmd_lcm_drv")) {
			vendor = "ULVAC";
		}
	}
	strcpy(panel_data->manufacture_info.manufacture, vendor);
	pr_err("[TP]tp_util_get_vendor line =%d\n", __LINE__);
	pr_err("[TP] enter case %d\n", prj_id);

	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
	         "tp/%d/FW_%s_%s.img",
	         g_tp_prj_id, panel_data->chip_name, vendor);

	if (panel_data->test_limit_name)
	{
		snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
		         "tp/%d/LIMIT_%s_%s.img",
		         g_tp_prj_id, panel_data->chip_name, vendor);
	}
	if (panel_data->extra) {
		snprintf(panel_data->extra, MAX_LIMIT_DATA_LENGTH,
				"tp/%d/BOOT_FW_%s_%s.ihex",
				prj_id, panel_data->chip_name, vendor);
		}

	if (strstr(saved_command_line, "oplus19551_samsung_ams644vk01_1080p_dsi_cmd"))
	{
		memcpy(panel_data->manufacture_info.version, "0xbd2860000", 11); //For 19550 19551 19553 19556 19597
	}
	if (strstr(saved_command_line, "oplus19357_samsung_ams644va04_1080p_dsi_cmd"))
	{
		memcpy(panel_data->manufacture_info.version, "0xfa1920000", 11); //For 19357 19358 19359 19354
	}
	if (strstr(saved_command_line, "oplus19537_samsung_ams643xf01_1080p_dsi_cmd"))
	{
		memcpy(panel_data->manufacture_info.version, "0xdd3400000", 11); //For 19536 19537 19538 19539
	}
	if (strstr(saved_command_line, "oplus20291_samsung_ams643xy01_1080p_dsi_vdo"))
	{
		memcpy(panel_data->manufacture_info.version, "0xFA270000", 11); //For 20291 20292 20293 20294 20295
	}
	if (strstr(saved_command_line, "nt36525b_hlt"))
	{
		memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
		panel_data->firmware_headfile.firmware_data = FW_206A1_NF_NT36525B_HLT;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_NT36525B_HLT);
	}
	if (strstr(saved_command_line, "hx83102d"))
	{
		memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
		panel_data->firmware_headfile.firmware_data = FW_206A1_NF_HX83102D_TRULY;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_HX83102D_TRULY);
	}
	if (strstr(saved_command_line, "ilt9881h"))
	{
		memcpy(panel_data->manufacture_info.version, "HLT_NT25_", 9);
		panel_data->firmware_headfile.firmware_data = FW_206A1_NF_ILT9881H_LS;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_206A1_NF_ILT9881H_LS);
	}

	if (strstr(saved_command_line, "oplus21331_td4160_inx")) {
		memcpy(panel_data->manufacture_info.version, "AA340IOA1", 9);
		panel_data->firmware_headfile.firmware_data = FW_21331_NF_ZHAOYUN;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_21331_NF_ZHAOYUN);
		if ((g_tp_prj_id == 22875) || (g_tp_prj_id == 22876)) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/22875/FW_%s_%s.img",
				"TD4160", "INX");
		} else {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/21331/FW_%s_%s.img",
				"TD4160", "INX");
		}

		if (panel_data->test_limit_name) {
			if ((g_tp_prj_id == 22875) || (g_tp_prj_id == 22876)) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
					"tp/22875/LIMIT_%s_%s.img",
					"TD4160", "INX");
			} else {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
					"tp/21331/LIMIT_%s_%s.img",
					"TD4160", "INX");
			}
		}
	}
        if (strstr(saved_command_line, "oplus21331_ili9883c_hlt")) {
                memcpy(panel_data->manufacture_info.version, "AA340HI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21331_NF_9883A;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21331_NF_9883A);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/21331/FW_%s_%s.img",
                        "9883A", "HLT");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/21331/LIMIT_%s_%s.img",
                                "9883A", "HLT");
                }
        }
	if (strstr(saved_command_line, "oplus21331_ili9883c_boe")) {
                memcpy(panel_data->manufacture_info.version, "AA340BI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21331_NF_9883A;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21331_NF_9883A);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/21331/FW_%s_%s.img",
                        "9883A", "BOE");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/21331/LIMIT_%s_%s.img",
                                "9883A", "BOE");
                }
        }
        if (strstr(saved_command_line, "oplus21361_td4160_inx")) {
                memcpy(panel_data->manufacture_info.version, "AA364IO01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21361_NF_INX;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21361_NF_INX);
        	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                 	"tp/21361/FW_%s_%s.img",
                 	"TD4160", "INX");

        	if (panel_data->test_limit_name) {
                	snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                         	"tp/21361/LIMIT_%s_%s.img",
                         	"TD4160", "INX");
        	}
        }
		if (strstr(saved_command_line, "oplus21361_td4160_truly")) {
                memcpy(panel_data->manufacture_info.version, "AA364TI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21361_NF_TRULY;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21361_NF_TRULY);
        	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                 	"tp/21361/FW_%s_%s.img",
                 	"TD4160", "TRULY");

        	if (panel_data->test_limit_name) {
                	snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                         	"tp/21361/LIMIT_%s_%s.img",
                         	"TD4160", "TRULY");
        	}
        }
        if (strstr(saved_command_line, "oplus21361_ili9883c_hlt")) {
                memcpy(panel_data->manufacture_info.version, "AA364HI01", 9);
                panel_data->firmware_headfile.firmware_data = FW_21361_NF_9883C;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_21361_NF_9883C);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/21361/FW_%s_%s.img",
                        "9883C", "HLT");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/21361/LIMIT_%s_%s.img",
                                "9883C", "HLT");
                }
        }
		if (strstr(saved_command_line, "oplus22261_td4160_truly")) {
                memcpy(panel_data->manufacture_info.version, "AC012XT01", 9);
                panel_data->firmware_headfile.firmware_data = FW_22261_NF_TRULY;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_22261_NF_TRULY);
        	snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                 	"tp/22261/FW_%s_%s.img",
                 	"TD4160", "TRULY");

        	if (panel_data->test_limit_name) {
                	snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                         	"tp/22261/LIMIT_%s_%s.img",
                         	"TD4160", "TRULY");
        	}
        }
        if (strstr(saved_command_line, "oplus22261_ili9883c_hlt")) {
                memcpy(panel_data->manufacture_info.version, "0xAC012HI", 9);
                panel_data->firmware_headfile.firmware_data = FW_22261_NF_9883C;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_22261_NF_9883C);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/22261/FW_%s_%s.img",
                        "9883C", "HLT");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/22261/LIMIT_%s_%s.img",
                                "9883C", "HLT");
                }
        }
		if (strstr(saved_command_line, "oplus22261_ili9883c_yfhlt")) {
                memcpy(panel_data->manufacture_info.version, "0xAC012HI", 9);
                panel_data->firmware_headfile.firmware_data = FW_22261_NF_9883C;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_22261_NF_9883C);
                snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                        "tp/22261/FW_%s_%s.img",
                        "9883C", "HLT");
                if (panel_data->test_limit_name) {
                        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                                "tp/22261/LIMIT_%s_%s.img",
                                "9883C", "HLT");
                }
        }
	if (g_tp_prj_id == 21251) {
		if (strstr(saved_command_line, "oplus21251_boe_ili9882n")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI9882N_BOE.bin", g_tp_prj_id);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI9882N_BOE.ini", g_tp_prj_id);
			}
			strcpy(panel_data->manufacture_info.manufacture, "BOE");
			memcpy(panel_data->manufacture_info.version, "AA252CI", 7);
		} else if (strstr(saved_command_line, "oplus21251_csot_ili7807s")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI7807S_CSOT.bin", g_tp_prj_id);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI7807S_CSOT.ini", g_tp_prj_id);
			}
			strcpy(panel_data->manufacture_info.manufacture, "CSOT");
			memcpy(panel_data->manufacture_info.version, "AA252CI", 7);
			panel_data->firmware_headfile.firmware_data = FW_21251_NF_ILI7807S_CSOT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21251_NF_ILI7807S_CSOT);
		} else {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH, "tp/%d/FW_NF_ILI9882Q_BOE.bin", g_tp_prj_id);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH, "tp/%d/LIMIT_NF_ILI9882Q_BOE.ini", g_tp_prj_id);
			}
			strcpy(panel_data->manufacture_info.manufacture, "BOE");
			memcpy(panel_data->manufacture_info.version, "AA252BI", 7);
			panel_data->firmware_headfile.firmware_data = FW_21251_NF_ILI9882Q_BOE;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21251_NF_ILI9882Q_BOE);
		}
	}

	if ((g_tp_prj_id == 20375) || (g_tp_prj_id == 20376) || (g_tp_prj_id == 20377)
		|| (g_tp_prj_id == 20378) || (g_tp_prj_id == 20379) || (g_tp_prj_id == 131962)) {
		pr_err("[TP] %s project is %d\n", __func__, g_tp_prj_id);
		if (strstr(saved_command_line, "ilt7807s_hlt")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20375/FW_NF_ILI7807S_HLT.bin");
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20375/LIMIT_NF_ILI7807S_HLT.ini");
			}
			strcpy(panel_data->manufacture_info.manufacture, "HLT");
			memcpy(panel_data->manufacture_info.version, "AA070HI01", 9);
			panel_data->firmware_headfile.firmware_data = FW_20375_NF_ILI7807S_HLT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20375_NF_ILI7807S_HLT);
		}
		if (strstr(saved_command_line, "ilt9882n_innolux")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20375/FW_NF_ILI9882N_INX.bin");
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20375/LIMIT_NF_ILI9882N_INX.ini");
			}
			strcpy(panel_data->manufacture_info.manufacture, "INX");
			memcpy(panel_data->manufacture_info.version, "AA070HI01", 9);
			panel_data->firmware_headfile.firmware_data = FW_20375_NF_ILI9882N_INX;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20375_NF_ILI9882N_INX);
		}
		if (strstr(saved_command_line, "ilt9882q_innolux")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20375/FW_NF_ILI9882Q_INX.bin");
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20375/LIMIT_NF_ILI9882Q_INX.ini");
			}
			strcpy(panel_data->manufacture_info.manufacture, "INX");
			memcpy(panel_data->manufacture_info.version, "AA070II01", 9);
			panel_data->firmware_headfile.firmware_data = FW_20375_NF_ILI9882Q_INX;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20375_NF_ILI9882Q_INX);
		}
		if (strstr(saved_command_line, "hx83102d_txd")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20375/FW_NF_HX83102D_TXD.bin");
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20375/LIMIT_NF_HX83102D_TXD.img");
			}
			strcpy(panel_data->manufacture_info.manufacture, "TXD");
			memcpy(panel_data->manufacture_info.version, "AA070tH01", 9);
			panel_data->firmware_headfile.firmware_data = FW_20375_NF_HX83102D_TXD;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20375_NF_HX83102D_TXD);
		}
	}

	switch (prj_id) {
	case 20615:
	case 20662:
	case 20619:
	case 21609:
	case 21651:
		pr_info("[TP] enter case oplus_20615\n");
		snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20615/FW_%s_%s.img",
				"FT3518", vendor);
		if (panel_data->test_limit_name) {
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20615/LIMIT_%s_%s.img",
				"FT3518", vendor);
		}
/*
		if (panel_data->extra) {
			snprintf(panel_data->extra, MAX_LIMIT_DATA_LENGTH,
				"tp/20615/BOOT_FW_%s_%s.ihex",
				"FT3518", vendor);
		}
*/
		panel_data->manufacture_info.fw_path = panel_data->fw_name;
		break;
	case 20171:
	case 20172:
	case 20353:
		snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
			"tp/%d/FW_%s_%s.%s",
			g_tp_prj_id, panel_data->chip_name, vendor, !strcmp(vendor,
			"SAMSUNG") ? "bin" : "img");

		if (panel_data->test_limit_name) {
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
			"tp/%d/LIMIT_%s_%s.img",
			g_tp_prj_id, panel_data->chip_name, vendor);
		}
		panel_data->manufacture_info.fw_path = panel_data->fw_name;
		break;
	case 20131:
	case 20133:
	case 20255:
	case 20257:
		snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
			 "tp/20131/FW_%s_%s.%s",
			  panel_data->chip_name, vendor, !strcmp(vendor,
					"SAMSUNG") ? "bin" : "img");

		if (panel_data->test_limit_name) {
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				 "tp/20131/LIMIT_%s_%s.img",
				 panel_data->chip_name, vendor);
		}
		panel_data->manufacture_info.fw_path = panel_data->fw_name;
		break;
	case 22603:
	case 22604:
	case 22609:
	case 0x2260A:
	case 0x2260B:
	case 22669:
	case 0x2266A:
	case 0x2266B:
	case 0x2266C:
		pr_info("[TP] enter case 22604\n");
		if (tp_used_index == himax_83102d) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/22604/FW_%s_%s.img",
				"NF_HX83102D", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/22604/LIMIT_%s_%s.img",
				"NF_HX83102D", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			pr_info("[TP]: firmware_headfile = FW_22604_NF_NF_HX83102D_LS\n");
			memcpy(panel_data->manufacture_info.version, "ls_hx83102d_", 12);
			panel_data->firmware_headfile.firmware_data = FW_22604_NF_NF_HX83102D_LS;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_22604_NF_NF_HX83102D_LS);
		} else if (tp_used_index == himax_83112f) {
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/22604/FW_%s_%s.img",
					"NF_HX83112F", vendor);
				if (panel_data->test_limit_name) {
					snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
					"tp/22604/LIMIT_%s_%s.img",
					"NF_HX83112F", vendor);
				}
				panel_data->manufacture_info.fw_path = panel_data->fw_name;
				pr_info("[TP]: firmware_headfile = FW_22604_NF_HX83112F_TXD\n");
				memcpy(panel_data->manufacture_info.version, "TXD_HX83112F_", 13);
				panel_data->firmware_headfile.firmware_data = FW_22604_NF_HX83112F_TXD;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_22604_NF_HX83112F_TXD);
		} else if (tp_used_index == icnl9911c_txd) {
			snprintf(panel_data->fw_name, MAX_LIMIT_DATA_LENGTH,
				"tp/22604/FW_%s_%s.bin",
				"ICNL9911C", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/22604/LIMIT_%s_%s.img",
				"ICNL9911C", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			panel_data->firmware_headfile.firmware_data = FW_22603_ICNL9911C_TXD;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_22603_ICNL9911C_TXD);
	case 0x210A0:
		pr_info("[TP] enter case 210A0\n");
		if (tp_used_index == ili7807s_tianma) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/210A0/FW_%s_%s.img",
					"NF_ILI7807S", "TIANMA");
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/210A0/LIMIT_%s_%s.img",
						"NF_ILI7807S", "TIANMA");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "AA231TI", 7);
			panel_data->firmware_headfile.firmware_data = FW_21041_ILI7807S_TIANMA;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21041_ILI7807S_TIANMA);
		}
		if (tp_used_index == ili7807s_jdi) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/21041/FW_%s_%s.img",
					"NF_ILI7807S", "JDI");
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/21041/LIMIT_%s_%s.img",
						"NF_ILI7807S", "JDI");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "AA231DI", 7);
			panel_data->firmware_headfile.firmware_data = FW_21041_ILI7807S_JDI;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21041_ILI7807S_JDI);
		}
		break;
		}
		break;
	case 20391:
		snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20391/FW_%s_%s.img",
				panel_data->chip_name, vendor);

		if (panel_data->test_limit_name) {
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20391/LIMIT_%s_%s.img",
				panel_data->chip_name, vendor);
		}
		panel_data->manufacture_info.fw_path = panel_data->fw_name;
		memcpy(panel_data->manufacture_info.version, "0xFA2720000", 11);
		break;
	case 20151:
	case 20301:
	case 20302:
		snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
				"tp/20151/FW_%s_%s.img",
				"FT3518", "SAMSUNG");

		if (panel_data->test_limit_name) {
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
				"tp/20151/LIMIT_%s_%s.img",
				"FT3518", "SAMSUNG");
		}
		panel_data->manufacture_info.fw_path = panel_data->fw_name;
		memcpy(panel_data->manufacture_info.version, "0xFA2720000", 11);
		break;
	case 20015:
	case 20013:
	case 20016:
	case 20108:
	case 20109:
	case 20307:
		pr_info("[TP] enter case 20015\n");
		if (tp_used_index == nt36525b_boe) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					 "tp/20015/FW_%s_%s.bin",
					 "NF_NT36525B", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						 "tp/20015/LIMIT_%s_%s.img",
						 "NF_NT36525B", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAA006IN", 9);
			panel_data->firmware_headfile.firmware_data = FW_20015_NT36525B_INX;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20015_NT36525B_INX);
		}
		if (tp_used_index == ili9882n_cdot) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					 "tp/20015/FW_%s_%s.bin",
					 "NF_ILI9882N", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						 "tp/20015/LIMIT_%s_%s.ini",
						 "NF_ILI9882N", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "ILI9882N_", 9);
			panel_data->firmware_headfile.firmware_data = FW_20015_ILI9882N_CDOT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20015_ILI9882N_CDOT);
		}
		if (tp_used_index == ili9882n_hlt) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					 "tp/20015/FW_%s_%s.bin",
					 "NF_ILI9882N", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						 "tp/20015/LIMIT_%s_%s.ini",
						 "NF_ILI9882N", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAA006HI", 9);
			panel_data->firmware_headfile.firmware_data = FW_20015_ILI9882N_HLT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20015_ILI9882N_HLT);
		}
		if (tp_used_index == ili9882n_inx) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					 "tp/20015/FW_%s_%s.bin",
					 "NF_ILI9882N", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						 "tp/20015/LIMIT_%s_%s.ini",
						 "NF_ILI9882N", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAA006II", 9);
			panel_data->firmware_headfile.firmware_data = FW_20015_ILI9882N_INX;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20015_ILI9882N_INX);
		}
		if (tp_used_index == himax_83102d) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					 "tp/20015/FW_%s_%s.bin",
					 "NF_HX83102D", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						 "tp/20015/LIMIT_%s_%s.ini",
						 "NF_HX83102D", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAA006TH000", 12);
			panel_data->firmware_headfile.firmware_data = FW_20015_NF_HX83102D_TRULY;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20015_NF_HX83102D_TRULY);
		}
		break;
	case 21101:
	case 21102:
	case 21235:
	case 21236:
	case 21831:
		if (strstr(saved_command_line, "oplus21101_ili9883a_boe_hd")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/21101/FW_%s_%s.img",
					"NF_9883A", "BOE");

			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/21101/LIMIT_%s_%s.img",
						"NF_9883A", "BOE");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAA270BI01", 11);
			panel_data->firmware_headfile.firmware_data = FW_21101_NF_7807S_BOE;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21101_NF_7807S_BOE);
		}

		if (strstr(saved_command_line, "oplus21101_ili9883a_boe_bi_hd")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/21101/FW_%s_%s_%s.img",
					"NF_9883A", "BOE", "THIN");
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/21101/LIMIT_%s_%s.img",
						"NF_9883A", "BOE");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAA270bI01", 11);
			panel_data->firmware_headfile.firmware_data = FW_21101_NF_7807S_BOE_THIN;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21101_NF_7807S_BOE_THIN);
		}

		if (strstr(saved_command_line, "oplus21101_td4160")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/21101/FW_%s_%s.img",
					"TD4160", "INX");

			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/21101/LIMIT_%s_%s.img",
						"TD4160", "INX");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAA270TI01", 11);
			panel_data->firmware_headfile.firmware_data = FW_21101_TD4160_INX;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21101_TD4160_INX);
		}

		if (strstr(saved_command_line, "oplus21101_nt36672c_tm_cphy_dsi_vdo_lcm_drv")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/21101/FW_%s_%s.bin",
					"NF_NT36672C", "TIANMA");

			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/21101/LIMIT_%s_%s.img",
						"NF_NT36672C", "TIANMA");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAC010TN01", 11);
			panel_data->firmware_headfile.firmware_data = FW_21101_NF_NT36672C_TIANMA;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21101_NF_NT36672C_TIANMA);
		}
		break;
	case 22083:
	case 22084:
	case 22291:
	case 22292:
		if (strstr(saved_command_line, "ili9883c")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/22083/FW_%s_%s.img",
					"NF_9883C", "BOE");

			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/22083/LIMIT_%s_%s.img",
						"NF_9883C", "BOE");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "     AC034HI", 12);
			panel_data->firmware_headfile.firmware_data = FW_22083_NF_7807S_BOE;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_22083_NF_7807S_BOE);
		}

		if (strstr(saved_command_line, "td4160")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/22083/FW_%s_%s.img",
					"TD4160", "INX");

			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/22083/LIMIT_%s_%s.img",
						"TD4160", "INX");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, " inx_td4610_", 12);
			panel_data->firmware_headfile.firmware_data = FW_22083_TD4160_INX;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_22083_TD4160_INX);
		}
		break;
	case 21639:
	case 0x2163B:
	case 0x2163C:
	case 0x2163D:
	case 0x216CD:
	case 0x216CE:
		if (strstr(saved_command_line, "oplus2163b_ili9883a")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/2163B/FW_%s_%s.img",
					"NF_9883A", "BOE");

			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/2163B/LIMIT_%s_%s.img",
						"NF_9883A", "BOE");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAA270BI01", 11);
			panel_data->firmware_headfile.firmware_data = FW_2163B_NF_7807S_BOE;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_2163B_NF_7807S_BOE);
		}

		if (strstr(saved_command_line, "oplus2163b_td4160_inx_hd_vdo_lcm_drv")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/2163B/FW_%s_%s.img",
					"TD4160", "INX");

			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/2163B/LIMIT_%s_%s.img",
						"TD4160", "INX");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAA270TI01", 11);
			panel_data->firmware_headfile.firmware_data = FW_2163B_TD4160_INX;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_2163B_TD4160_INX);
		}

		if (strstr(saved_command_line, "oplus21639_nt36672c_tm_cphy_dsi_vdo_lcm_drv")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/2163B/FW_%s_%s.bin",
					"NF_NT36672C", "TIANMA");

			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/2163B/LIMIT_%s_%s.img",
						"NF_NT36672C", "TIANMA");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAB417TMNT01", 13);
			panel_data->firmware_headfile.firmware_data = FW_21639_NF_NT36672C_TIANMA;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_21639_NF_NT36672C_TIANMA);
		}

		if (strstr(saved_command_line, "oplus2163b_td4160_truly_hd_vdo_lcm_drv")) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/2163B/FW_%s_%s.img",
					"TD4160", "TRULY");

			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						"tp/2163B/LIMIT_%s_%s.img",
						"TD4160", "TRULY");
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "0xAB417_TR_TD_01", 16);
			panel_data->firmware_headfile.firmware_data = FW_2163B_TD4160_TRULY;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_2163B_TD4160_TRULY);
		}
		break;
	case 20608:
	case 20609:
	case 0x2060A:
	case 0x2060B:
	case 0x206F0:
	case 0x206FF:
	case 20796:
	case 20795:
	case 0x2070B:
	case 0x2070C:
	case 0x2070E:
	case 21747:
	case 21748:
	case 21749:
		pr_info("[TP] enter case 20609\n");
		if (tp_used_index == nt36672c) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					 "tp/20609/FW_%s_%s.bin",
					 "NF_NT36672C", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						 "tp/20609/LIMIT_%s_%s.img",
						 "NF_NT36672C", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "RA627_TM_NT_", 12);
			panel_data->firmware_headfile.firmware_data = FW_20609_NT36672C_TIANMA;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20609_NT36672C_TIANMA);
		}
		if (tp_used_index == ili9882n_inx) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					 "tp/20609/FW_%s_%s.bin",
					 "NF_ILI7807S", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						 "tp/20609/LIMIT_%s_%s.img",
						 "NF_ILI7807S", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "RA627_TM_ILI_", 13);
			panel_data->firmware_headfile.firmware_data = FW_20609_ILI7807S_TIANMA;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20609_ILI7807S_TIANMA);
		}
		break;
	case 21625:
	case 0x216A0:
	case 0x216A1:
	case 0x216B5:
	case 0x216B6:
		pr_info("[TP] enter case 21625\n");
		if (tp_used_index == nt36672c) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					 "tp/21625/FW_%s_%s.bin",
					 "NF_NT36672C", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						 "tp/21625/LIMIT_%s_%s.img",
						 "NF_NT36672C", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "AB151_TM_NT_", 12);
			panel_data->firmware_headfile.firmware_data = FW_20609_NT36672C_TIANMA;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20609_NT36672C_TIANMA);
		}
		if (tp_used_index == ili9882n_inx) {
			snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					 "tp/21625/FW_%s_%s.bin",
					 "NF_ILI7807S", vendor);
			if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
						 "tp/21625/LIMIT_%s_%s.img",
						 "NF_ILI7807S", vendor);
			}
			panel_data->manufacture_info.fw_path = panel_data->fw_name;
			memcpy(panel_data->manufacture_info.version, "AB151_TM_ILI_", 13);
			panel_data->firmware_headfile.firmware_data = FW_20609_ILI7807S_TIANMA;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20609_ILI7807S_TIANMA);
		}
		break;
	case 20181:
	case 20355:
	case 21081:
	case 21851:
		snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
			"tp/%d/FW_%s_%s.img",
			g_tp_prj_id, panel_data->chip_name, vendor);

		if (panel_data->test_limit_name) {
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
			"tp/%d/LIMIT_%s_%s.img",
			g_tp_prj_id, panel_data->chip_name, vendor);
		}
		panel_data->manufacture_info.fw_path = panel_data->fw_name;
		break;
	case 0x212A1:
		snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
			"tp/212A1/FW_%s_%s.img",
			panel_data->chip_name, vendor);

		if (panel_data->test_limit_name) {
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
			"tp/212A1/LIMIT_%s_%s.img",
			panel_data->chip_name, vendor);
		}
		panel_data->manufacture_info.fw_path = panel_data->fw_name;
		break;
	case 22612:
	case 22693:
	case 22694:
	case 0x226B1:
		pr_info("[TP] enter case 22612/22693/22694\n");
		if (tp_used_index == focal_ft3681) {
			if (g_tp_dev_vendor == TP_TIANMA) {
				pr_info("[TP] 22612 tp vendor tianma\n");
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/22612/FW_%s_%s.img",
					"FT3681", vendor);
				if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
					"tp/22612/LIMIT_%s_%s.img",
					"FT3681", vendor);
				}

				panel_data->manufacture_info.fw_path = panel_data->fw_name;
				memcpy(panel_data->manufacture_info.version, "0xAB525001", 10);
				panel_data->firmware_headfile.firmware_data = FW_22612_FT3681_TIANMA;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_22612_FT3681_TIANMA);
			} else if (g_tp_dev_vendor == TP_HLT) {
				pr_info("[TP] 20682 tp vendor ulvac\n");
				snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
					"tp/22612/FW_%s_%s.img",
					"FT3681", vendor);
				if (panel_data->test_limit_name) {
				snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
					"tp/22612/LIMIT_%s_%s.img",
					"FT3681", vendor);
				}
				panel_data->manufacture_info.fw_path = panel_data->fw_name;
				memcpy(panel_data->manufacture_info.version, "0xAB525041", 10);
				panel_data->firmware_headfile.firmware_data = FW_22612_FT3681_TIANMA;
				panel_data->firmware_headfile.firmware_size = sizeof(FW_22612_FT3681_TIANMA);
			}
		}
		break;
	/*BSP.TP.Function, 2021/9/2, add tp path for Moss.*/
	case 0x2169E:
	case 0x2169F:
	case 21711:
	case 21712:
	case 0x2162D:
	case 0x2162E:
	case 0x216C9:
	case 0x216CA:
		snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
			"tp/2169E/FW_%s_%s.img",
			panel_data->chip_name, vendor);

		if (panel_data->test_limit_name) {
			snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
			"tp/2169E/LIMIT_%s_%s.img",
			panel_data->chip_name, vendor);
		}
		panel_data->manufacture_info.fw_path = panel_data->fw_name;
		break;
	default:
		break;
	}
	panel_data->manufacture_info.fw_path = panel_data->fw_name;
	pr_info("[TP]vendor:%s fw:%s limit:%s\n",
	        vendor,
	        panel_data->fw_name,
	        panel_data->test_limit_name == NULL ? "NO Limit" : panel_data->test_limit_name);

	return 0;
}
EXPORT_SYMBOL(tp_util_get_vendor);

/**
 * Description:
 * pulldown spi7 cs to avoid current leakage
 * because of current sourcing from cs (pullup state) flowing into display module
 **/
void switch_spi7cs_state(bool normal)
{
	if(normal) {
		if (!IS_ERR_OR_NULL(g_hw_res->pin_set_high)) {
			pr_info("%s: going to set spi7 cs to spi mode .\n", __func__);
			pinctrl_select_state(g_hw_res->pinctrl, g_hw_res->pin_set_high);
		} else {
			pr_info("%s: cannot to set spi7 cs to spi mode .\n", __func__);
		}
	} else {
		if (!IS_ERR_OR_NULL(g_hw_res->pin_set_low)) {
			pr_info("%s: going to set spi7 cs to pulldown .\n", __func__);
			pinctrl_select_state(g_hw_res->pinctrl, g_hw_res->pin_set_low);
		} else {
			pr_info("%s: cannot to set spi7 cs to pulldown .\n", __func__);
		}
	}
}
EXPORT_SYMBOL(switch_spi7cs_state);

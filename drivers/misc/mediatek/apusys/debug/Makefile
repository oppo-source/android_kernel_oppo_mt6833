#
# Copyright (C) 2019 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

ifeq ($(CONFIG_MACH_MT6893),y)
	PDIR = mt6885
else ifeq ($(CONFIG_MACH_MT6877),y)
	PDIR = mt6877
else
	PDIR = $(MTK_PLATFORM)
endif


ccflags-y += -I$(srctree)/drivers/misc/mediatek/apusys/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/apusys/power/2.0
ccflags-y += -I$(srctree)/drivers/misc/mediatek/apusys/power/2.0/$(PDIR)
ccflags-y += -I$(obj)/$(PDIR)/

obj-y += debug.o

debug-$(CONFIG_MTK_APUSYS_DEBUG) += apu_tp.o apu_tags.o
debug-objs += debug_driver.o

debug-objs += dump_core.o
debug-objs += secure_perf.o


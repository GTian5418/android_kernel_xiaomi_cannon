/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>

#include "tchbst.h"
#include "io_ctrl.h"
#include "boost_ctrl.h"
#include "mtk_perfmgr_internal.h"
#include "topo_ctrl.h"
#include "uload_ind.h"
#include "syslimiter.h"


int clstr_num;

static int perfmgr_probe(struct platform_device *dev)
{
	return 0;
}

struct platform_device perfmgr_device = {
	.name   = "perfmgr",
	.id        = -1,
};

static int perfmgr_suspend(struct device *dev)
{
#if defined(CONFIG_MTK_PERFMGR_TOUCH_BOOST) && !(defined(CONFIG_MTK_FPSGO) || defined(CONFIG_MTK_FPSGO_V3))
	ktch_suspend();
#endif
	return 0;
}

static int perfmgr_resume(struct device *dev)
{
	return 0;
}
static int perfmgr_remove(struct platform_device *dev)
{

	/*TODO: workaround for k414
	 * topo_ctrl_exit();
	 */
#ifdef CONFIG_MTK_CPU_CTRL
	cpu_ctrl_exit();
#endif
#ifdef CONFIG_MTK_SYSLIMITER
	syslimiter_exit();
#endif

	return 0;
}
static struct platform_driver perfmgr_driver = {
	.probe      = perfmgr_probe,
	.remove     = perfmgr_remove,
	.driver     = {
		.name = "perfmgr",
		.pm = &(const struct dev_pm_ops){
			.suspend = perfmgr_suspend,
			.resume = perfmgr_resume,
		},
	},
};

static int perfmgr_main_data_init(void)
{
#ifdef CONFIG_MTK_TOPO_CTRL
	/* get cluster number from topo_ctrl */
	clstr_num = topo_ctrl_get_nr_clusters();
#endif

	return 0;
}

/*--------------------INIT------------------------*/

static int __init init_perfmgr(void)
{
	struct proc_dir_entry *perfmgr_root = NULL;
	int ret = 0;

	ret = platform_device_register(&perfmgr_device);
	if (ret)
		return ret;
	ret = platform_driver_register(&perfmgr_driver);
	if (ret)
		return ret;

	perfmgr_main_data_init();

	perfmgr_root = proc_mkdir("perfmgr", NULL);
	pr_debug("MTK_TOUCH_BOOST function init_perfmgr_touch\n");

#ifdef CONFIG_MTK_BASE_POWER
#ifdef CONFIG_MTK_PERFMGR_TOUCH_BOOST
	init_tchbst(perfmgr_root);
#endif
	init_boostctrl(perfmgr_root);
#ifdef CONFIG_MTK_SYSLIMITER
	syslimiter_init(perfmgr_root);
#endif
#endif
	init_perfctl(perfmgr_root);

#ifdef CONFIG_MTK_LOAD_TRACKER
	init_uload_ind(NULL);
#endif
	return 0;
}
device_initcall(init_perfmgr);

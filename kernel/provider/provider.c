// SPDX-License-Identifier: GPL-2.0-only
/* DT-free Wonder component bridge. Does not own the PCIe/WLAN device. */
#include <linux/component.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>

#include "wondertap.h"

#define PROVIDER_NAME_WONDER "vendor-wlan-wonder"
#define PROVIDER_NAME_CNSS   "qcom,cnss-vendor-wlan-wonder"

static DEFINE_MUTEX(provider_lock);
static struct platform_device *provider_wonder;
static struct platform_device *provider_cnss;
static const struct wondertap_priv *active_priv;
static bool component_registered;

static int provider_bind(struct device *dev, struct device *master, void *data)
{
	dev_info(dev, "vendor wonder bound to master device %s\n", dev_name(master));
	return 0;
}

static void provider_unbind(struct device *dev, struct device *master, void *data)
{
	dev_info(dev, "vendor wonder unbound from master device %s\n", dev_name(master));
}

static const struct component_ops provider_ops = {
	.bind = provider_bind,
	.unbind = provider_unbind,
};

/* Same declaration/CRC as the reference CNSS export and the PLD caller.
 * Caller owns priv and its callbacks until the matching NULL call returns.
 * Never call this function from a component bind/unbind callback.
 */
int cnss_set_vendor_wonder_priv_data(const void *priv_data)
{
	const struct wondertap_priv *priv = priv_data;
	int ret = 0;

	mutex_lock(&provider_lock);
	if (!provider_wonder && !provider_cnss) {
		ret = -ENODEV;
		goto out;
	}
	if (priv) {
		if (!priv->wonder_ops || priv->ver < 0 || priv->ver >= WONDER_VERSION_MAX) {
			ret = -EINVAL;
			goto out;
		}
		/* A repeated registration must not add a duplicate component. */
		if (active_priv) {
			ret = active_priv == priv ? 0 : -EBUSY;
			goto out;
		}
		if (provider_wonder) {
			platform_set_drvdata(provider_wonder, (void *)priv);
			component_add(&provider_wonder->dev, &provider_ops);
		}
		if (provider_cnss) {
			platform_set_drvdata(provider_cnss, (void *)priv);
			component_add(&provider_cnss->dev, &provider_ops);
		}
		component_registered = true;
		active_priv = priv;
	} else {
		/* Unbind the master before clearing/freeing the caller's ops. */
		if (component_registered) {
			if (provider_wonder)
				component_del(&provider_wonder->dev, &provider_ops);
			if (provider_cnss)
				component_del(&provider_cnss->dev, &provider_ops);
		}
		component_registered = false;
		active_priv = NULL;
		if (provider_wonder)
			platform_set_drvdata(provider_wonder, NULL);
		if (provider_cnss)
			platform_set_drvdata(provider_cnss, NULL);
	}
out:
	mutex_unlock(&provider_lock);
	return ret;
}
EXPORT_SYMBOL(cnss_set_vendor_wonder_priv_data);

static int __init provider_init(void)
{
	struct platform_device_info wonder_info = {
		.name = PROVIDER_NAME_WONDER,
		.id = PLATFORM_DEVID_NONE,
	};
	struct platform_device_info cnss_info = {
		.name = PROVIDER_NAME_CNSS,
		.id = PLATFORM_DEVID_NONE,
	};

	provider_wonder = platform_device_register_full(&wonder_info);
	if (IS_ERR(provider_wonder)) {
		pr_warn("opairshare_wonder_provider: failed to register %s (%ld)\n",
			PROVIDER_NAME_WONDER, PTR_ERR(provider_wonder));
		provider_wonder = NULL;
	} else {
		pr_info("opairshare_wonder_provider: created %s\n", PROVIDER_NAME_WONDER);
	}

	provider_cnss = platform_device_register_full(&cnss_info);
	if (IS_ERR(provider_cnss)) {
		pr_warn("opairshare_wonder_provider: failed to register %s (%ld)\n",
			PROVIDER_NAME_CNSS, PTR_ERR(provider_cnss));
		provider_cnss = NULL;
	} else {
		pr_info("opairshare_wonder_provider: created %s\n", PROVIDER_NAME_CNSS);
	}

	if (!provider_wonder && !provider_cnss)
		return -ENODEV;

	pr_info("opairshare_wonder_provider: bridge ready; stock CNSS untouched\n");
	return 0;
}

static void __exit provider_exit(void)
{
	cnss_set_vendor_wonder_priv_data(NULL);
	/* No platform driver remove callback: device destruction has one owner. */
	if (provider_wonder) {
		platform_device_unregister(provider_wonder);
		provider_wonder = NULL;
	}
	if (provider_cnss) {
		platform_device_unregister(provider_cnss);
		provider_cnss = NULL;
	}
}

module_init(provider_init);
module_exit(provider_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("OPAirShare DT-free Wonder provider bridge, independent of CNSS");

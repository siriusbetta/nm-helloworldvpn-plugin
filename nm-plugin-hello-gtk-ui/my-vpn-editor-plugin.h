#pragma once

#include <NetworkManager.h>

G_BEGIN_DECLS

#define HELLOWORLD_TYPE_VPN_PLUGIN (helloworld_vpn_plugin_get_type())
G_DECLARE_FINAL_TYPE(HelloWorldVpnPlugin,
                     helloworld_vpn_plugin,
                     HELLOWORLD,
                     VPN_PLUGIN,
                     GObject)

NMVpnEditorPlugin *helloworld_vpn_plugin_new(void);

G_END_DECLS

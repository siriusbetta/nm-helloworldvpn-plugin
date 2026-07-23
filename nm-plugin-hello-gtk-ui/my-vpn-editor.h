#pragma once

#include <NetworkManager.h>

G_BEGIN_DECLS

#define HELLOWORLD_TYPE_VPN_EDITOR (helloworld_vpn_editor_get_type())
G_DECLARE_FINAL_TYPE(HelloWorldVpnEditor, helloworld_vpn_editor, HELLOWORLD, VPN_EDITOR, GObject)

NMVpnEditor *helloworld_vpn_editor_new(NMConnection *connection, GError **error);
NMVpnEditor *nm_vpn_editor_factory_helloworld(NMConnection *connection,
                                              GError **error);

G_END_DECLS

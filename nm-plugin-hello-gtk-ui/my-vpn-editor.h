#pragma once

#include <NetworkManager.h>  // ← ЕДИНСТВЕННЫЙ правильный заголовок для libnm
#include "file-chooser-widget.h"

G_BEGIN_DECLS

#define MY_VPN_TYPE_EDITOR (my_vpn_editor_get_type())
G_DECLARE_FINAL_TYPE(MyVpnEditor, my_vpn_editor, MY_VPN, EDITOR, GObject)

NMVpnEditor *my_vpn_editor_new(NMConnection *connection, GError **error);

G_END_DECLS

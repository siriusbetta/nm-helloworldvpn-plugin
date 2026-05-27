#include <NetworkManager.h>
#include "my-vpn-editor.h"

/* Точка входа для libnm. Имя функции должно совпадать с именем в .name файле */
G_MODULE_EXPORT NMVpnEditor *
nm_vpn_editor_factory_myvpn(NMVpnEditorPlugin *plugin,
                            NMConnection *connection,
                            GError **error)
{
    return my_vpn_editor_new(connection, error);
}

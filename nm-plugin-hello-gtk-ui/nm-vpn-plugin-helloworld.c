#include "my-vpn-editor-plugin.h"

/* Factory function - entry point for libnm */
NMVpnEditorPlugin *
nm_vpn_editor_plugin_factory(GError **error)
{
    return helloworld_vpn_plugin_new();
}

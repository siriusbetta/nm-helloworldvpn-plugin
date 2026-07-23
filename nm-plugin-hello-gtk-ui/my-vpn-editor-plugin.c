#include "my-vpn-editor-plugin.h"

#include <gmodule.h>

#define HELLOWORLD_VPN_SERVICE "org.freedesktop.NetworkManager.helloworld"
#define HELLOWORLD_GTK3_EDITOR_LIBRARY "libnm-vpn-plugin-helloworld-editor.so"
#define HELLOWORLD_GTK4_EDITOR_LIBRARY "libnm-gtk4-vpn-plugin-helloworld-editor.so"
#define HELLOWORLD_EDITOR_FACTORY "nm_vpn_editor_factory_helloworld"

typedef NMVpnEditor *(*HelloWorldVpnEditorFactory)(NMConnection *connection,
                                                   GError **error);

static GModule *editor_module;
static HelloWorldVpnEditorFactory editor_factory;

struct _HelloWorldVpnPlugin {
    GObject parent;
};

enum {
    PROP_0,
    PROP_NAME,
    PROP_DESCRIPTION,
    PROP_SERVICE,
};

static void helloworld_vpn_plugin_interface_init(NMVpnEditorPluginInterface *iface);

G_DEFINE_TYPE_WITH_CODE(
    HelloWorldVpnPlugin,
    helloworld_vpn_plugin,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(NM_TYPE_VPN_EDITOR_PLUGIN,
                          helloworld_vpn_plugin_interface_init)
)

static void
helloworld_vpn_plugin_get_property(GObject *object,
                                   guint property_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
    switch (property_id) {
    case PROP_NAME:
        g_value_set_string(value, "HelloWorld");
        break;
    case PROP_DESCRIPTION:
        g_value_set_string(value, "HelloWorld VPN editor plugin");
        break;
    case PROP_SERVICE:
        g_value_set_string(value, HELLOWORLD_VPN_SERVICE);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static NMVpnEditor *
get_editor(NMVpnEditorPlugin *plugin,
           NMConnection *connection,
           GError **error)
{
    const char *editor_library;

    if (!editor_module) {
        GModule *self_module;
        gpointer gtk3_only_symbol = NULL;
        char *editor_path;

        self_module = g_module_open(NULL, 0);
        if (self_module) {
            g_module_symbol(self_module,
                            "gtk_container_add",
                            &gtk3_only_symbol);
            g_module_close(self_module);
        }

        editor_library = gtk3_only_symbol
            ? HELLOWORLD_GTK3_EDITOR_LIBRARY
            : HELLOWORLD_GTK4_EDITOR_LIBRARY;

        editor_path = g_build_filename(PLUGINDIR, editor_library, NULL);
        editor_module = g_module_open(editor_path,
                                      G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
        g_free(editor_path);
        if (!editor_module) {
            g_set_error(error,
                        NM_VPN_PLUGIN_ERROR,
                        NM_VPN_PLUGIN_ERROR_FAILED,
                        "Failed to load VPN editor module %s: %s",
                        editor_library,
                        g_module_error());
            return NULL;
        }
    }

    if (!editor_factory
        && !g_module_symbol(editor_module,
                            HELLOWORLD_EDITOR_FACTORY,
                            (gpointer *) &editor_factory)) {
        g_set_error(error,
                    NM_VPN_PLUGIN_ERROR,
                    NM_VPN_PLUGIN_ERROR_FAILED,
                    "Failed to find VPN editor factory: %s",
                    g_module_error());
        return NULL;
    }

    return editor_factory(connection, error);
}

static NMVpnEditorPluginCapability
get_capabilities(NMVpnEditorPlugin *plugin)
{
    return NM_VPN_EDITOR_PLUGIN_CAPABILITY_NONE;
}

static void
helloworld_vpn_plugin_interface_init(NMVpnEditorPluginInterface *iface)
{
    iface->get_editor = get_editor;
    iface->get_capabilities = get_capabilities;
}

static void
helloworld_vpn_plugin_init(HelloWorldVpnPlugin *self)
{
}

static void
helloworld_vpn_plugin_class_init(HelloWorldVpnPluginClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = helloworld_vpn_plugin_get_property;

    g_object_class_override_property(object_class,
                                     PROP_NAME,
                                     NM_VPN_EDITOR_PLUGIN_NAME);
    g_object_class_override_property(object_class,
                                     PROP_DESCRIPTION,
                                     NM_VPN_EDITOR_PLUGIN_DESCRIPTION);
    g_object_class_override_property(object_class,
                                     PROP_SERVICE,
                                     NM_VPN_EDITOR_PLUGIN_SERVICE);
}

NMVpnEditorPlugin *
helloworld_vpn_plugin_new(void)
{
    return NM_VPN_EDITOR_PLUGIN(
        g_object_new(HELLOWORLD_TYPE_VPN_PLUGIN, NULL)
    );
}

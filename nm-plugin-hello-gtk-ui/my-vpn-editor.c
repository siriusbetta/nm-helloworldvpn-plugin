#include "nm-default.h"
#include "nm-connection.h"
#include "nm-vpn-editor.h"
#include "nm-vpn-editor-plugin.h"

#include "my-vpn-editor.h"

#include <gtk/gtk.h>

typedef struct {
	GtkBuilder *builder;
	GtkWidget *widget;
	GtkWindowGroup *window_group;
	gboolean window_added;
	GHashTable *advanced;
	gboolean new_connection;
	GtkWidget *tls_user_cert_chooser;
} MyVpnEditorPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MyVpnEditor, my_vpn_editor, G_TYPE_OBJECT)

#define MY_VPN_EDITOR_GET_PRIVATE(o) (my_vpn_editor_get_instance_private ((MyVpnEditor*)o))

static void
my_vpn_editor_init (MyVpnEditor *self)
{
	g_message("my_vpn_editor_init");
}

static void
dispose (GObject *object)
{
	g_message("dispose");
	MyVpnEditor *self = MY_VPN_EDITOR (object);
	MyVpnEditorPrivate *priv = MY_VPN_EDITOR_GET_PRIVATE (self);

	g_clear_object (&priv->window_group);
	g_clear_object (&priv->widget);
	g_clear_object (&priv->builder);
	g_clear_pointer (&priv->advanced, g_hash_table_destroy);

	G_OBJECT_CLASS (my_vpn_editor_parent_class)->dispose (object);
}

static void
my_vpn_editor_class_init (MyVpnEditorClass *klass)
{
	g_message("my_vpn_editor_class_init");
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->dispose = dispose;
}

static GObject *
get_widget (NMVpnEditor *iface)
{
	g_message("get_widget");
	MyVpnEditor *self = MY_VPN_EDITOR (iface);
	MyVpnEditorPrivate *priv = MY_VPN_EDITOR_GET_PRIVATE (self);

	return G_OBJECT (priv->widget);
}

static gboolean
update_connection (NMVpnEditor *iface,
                   NMConnection *connection,
                   GError **error)
{
	g_message("update_connection");
	MyVpnEditor *self = MY_VPN_EDITOR (iface);
	MyVpnEditorPrivate *priv = MY_VPN_EDITOR_GET_PRIVATE (self);

	NMSettingVpn *s_vpn = nm_connection_get_setting_vpn (connection);
	if (!s_vpn) {
		s_vpn = (NMSettingVpn *) nm_setting_vpn_new ();
		nm_connection_add_setting (connection, NM_SETTING (s_vpn));
	}

	GtkEntry *entry = GTK_ENTRY (gtk_builder_get_object (priv->builder, "remote_public_key_entry"));
	if (entry) {
		const char *text = gtk_editable_get_text (GTK_EDITABLE (entry));
		if (text && *text)
			nm_setting_vpn_add_data_item (s_vpn, "remote-public-key", text);
	}

	entry = GTK_ENTRY (gtk_builder_get_object (priv->builder, "local_private_key"));
	if (entry) {
		const char *text = gtk_editable_get_text (GTK_EDITABLE (entry));
		if (text && *text)
			nm_setting_vpn_add_secret (s_vpn, "local-private-key", text);
	}

	entry = GTK_ENTRY (gtk_builder_get_object (priv->builder, "local_computer_name"));
	if (entry) {
		const char *text = gtk_editable_get_text (GTK_EDITABLE (entry));
		if (text && *text)
			nm_setting_vpn_add_data_item (s_vpn, "local-computer-name", text);
	}

	GtkSwitch *sw = GTK_SWITCH (gtk_builder_get_object (priv->builder, "publish_switch"));
	if (sw) {
		gboolean active = gtk_switch_get_active (sw);
		nm_setting_vpn_add_data_item (s_vpn, "publish-key", active ? "yes" : "no");
	}

	return TRUE;
}

static void
my_vpn_editor_interface_init (NMVpnEditorInterface *iface)
{
	g_message("my_vpn_editor_interface_init");
	iface->get_widget = get_widget;
	iface->update_connection = update_connection;
}

G_DEFINE_TYPE_EXTENDED (MyVpnEditor, my_vpn_editor, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (NM_TYPE_VPN_EDITOR,
                                               my_vpn_editor_interface_init))

static void
stuff_changed_cb (GtkWidget *widget, gpointer user_data)
{
	g_message("stuff_changed_cb");
	g_signal_emit_by_name (MY_VPN_EDITOR (user_data), "changed");
}

NMVpnEditor *my_vpn_editor_new (NMConnection *connection, GError **error) {
	MyVpnEditor *object = g_object_new (MY_VPN_TYPE_EDITOR, NULL);
	MyVpnEditorPrivate *priv;

	if (!object) {
		g_set_error_literal (error, g_quark_from_static_string ("my-vpn-error"), 0, 
		                     _("could not create my-vpn object"));
		return NULL;
	}

	priv = MY_VPN_EDITOR_GET_PRIVATE (object);
	priv->builder = gtk_builder_new_from_resource(
	    "/org/freedesktop/NetworkManager/helloworld/my-vpn-editor.ui"
	);

	if (!priv->builder) {
		g_set_error_literal (error, g_quark_from_static_string ("my-vpn-error"), 0, 
		                     _("could not load UI from resource"));
		g_object_unref (object);
		return NULL;
	}

	priv->widget = GTK_WIDGET (gtk_builder_get_object (priv->builder, "my-vpn-vbox"));
	if (!priv->widget) {
		g_set_error_literal (error, g_quark_from_static_string ("my-vpn-error"), 0, 
		                     _("could not load UI widget"));
		g_object_unref (object);
		return NULL;
	}
	g_object_ref_sink (priv->widget);

	/* Подключаем сигналы changed для всех редактируемых виджетов */
	GtkWidget *w = GTK_WIDGET (gtk_builder_get_object (priv->builder, "remote_public_key_entry"));
	if (w) g_signal_connect (w, "changed", G_CALLBACK (stuff_changed_cb), object);

	w = GTK_WIDGET (gtk_builder_get_object (priv->builder, "local_private_key"));
	if (w) g_signal_connect (w, "changed", G_CALLBACK (stuff_changed_cb), object);

	w = GTK_WIDGET (gtk_builder_get_object (priv->builder, "local_computer_name"));
	if (w) g_signal_connect (w, "changed", G_CALLBACK (stuff_changed_cb), object);

	w = GTK_WIDGET (gtk_builder_get_object (priv->builder, "publish_switch"));
	if (w) g_signal_connect (w, "notify::active", G_CALLBACK (stuff_changed_cb), object);

	return (NMVpnEditor*) object;
}

G_MODULE_EXPORT NMVpnEditor *
nm_vpn_editor_factory_my_vpn (NMVpnEditorPlugin *editor_plugin,
                              NMConnection *connection,
                              GError **error)
{
	g_message("nm_vpn_editor_factory_my_vpn");
	g_return_val_if_fail (!error || !*error, NULL);

	return my_vpn_editor_new (connection, error);
}
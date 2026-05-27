#include "my-vpn-editor.h"
#include <gtk/gtk.h>

#define MY_VPN_SERVICE_TYPE "org.example.myvpn"
#define MY_VPN_KEY_CONFIG_PATH "config_path"

struct _MyVpnEditor {
    GObject parent_instance;
    FileChooserWidget *file_widget;
    NMConnection *connection;
};

/* Прототипы */
static void nm_vpn_editor_iface_init(NMVpnEditorInterface *iface);
static void my_vpn_editor_class_init(MyVpnEditorClass *klass);

G_DEFINE_TYPE_WITH_CODE(MyVpnEditor, my_vpn_editor, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(NM_TYPE_VPN_EDITOR, nm_vpn_editor_iface_init))

/* === Сигналы и вспомогательные функции === */

static void on_path_changed(FileChooserWidget *widget, const gchar *path, gpointer user_data) {
    MyVpnEditor *self = MY_VPN_EDITOR(user_data);
    NMSettingVpn *s_vpn = nm_connection_get_setting_vpn(self->connection);
    
    if (!s_vpn) {
        s_vpn = NM_SETTING_VPN(nm_setting_vpn_new());
        g_object_set(s_vpn, NM_SETTING_VPN_SERVICE_TYPE, MY_VPN_SERVICE_TYPE, NULL);
        nm_connection_add_setting(self->connection, NM_SETTING(s_vpn));
    }
    
    if (path && *path)
        nm_setting_vpn_add_data_item(s_vpn, MY_VPN_KEY_CONFIG_PATH, path);
    else
        nm_setting_vpn_remove_data_item(s_vpn, MY_VPN_KEY_CONFIG_PATH);
        
    g_signal_emit_by_name(self, "changed");
}

static void load_from_connection(MyVpnEditor *self) {
    NMSettingVpn *s_vpn = nm_connection_get_setting_vpn(self->connection);
    if (!s_vpn) return;
    
    const gchar *path = nm_setting_vpn_get_data_item(s_vpn, MY_VPN_KEY_CONFIG_PATH);
    if (path)
        file_chooser_widget_set_path(self->file_widget, path);
}

/* === Реализация интерфейса NMVpnEditor === */

static GObject *get_widget(NMVpnEditor *iface) {
    MyVpnEditor *self = MY_VPN_EDITOR(iface);
    return G_OBJECT(self->file_widget);
}

// В NM 1.40+ валидация выполняется здесь, check_validity удалён
static gboolean update_connection(NMVpnEditor *iface, NMConnection *connection, GError **error) {
    MyVpnEditor *self = MY_VPN_EDITOR(iface);
    const gchar *path = file_chooser_widget_get_path(self->file_widget);

    if (!path || !*path) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Файл конфигурации не выбран");
        return FALSE;
    }
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Файл не существует: %s", path);
        return FALSE;
    }

    NMSettingVpn *s_vpn = nm_connection_get_setting_vpn(connection);
    if (!s_vpn) {
        s_vpn = NM_SETTING_VPN(nm_setting_vpn_new());
        g_object_set(s_vpn, NM_SETTING_VPN_SERVICE_TYPE, MY_VPN_SERVICE_TYPE, NULL);
        nm_connection_add_setting(connection, NM_SETTING(s_vpn));
    }
    
    nm_setting_vpn_add_data_item(s_vpn, MY_VPN_KEY_CONFIG_PATH, path);
    return TRUE;
}

/* === GObject Boilerplate === */

static void my_vpn_editor_class_init(MyVpnEditorClass *klass) {
    // В данной реализации class_init не требуется, но должен быть определён
    // для G_DEFINE_TYPE_WITH_CODE
}

static void my_vpn_editor_init(MyVpnEditor *self) {
    self->file_widget = FILE_CHOOSER_WIDGET(file_chooser_widget_new());
    g_signal_connect(self->file_widget, "path-changed", G_CALLBACK(on_path_changed), self);
}

/* Обязательная функция инициализации интерфейса */
static void nm_vpn_editor_iface_init(NMVpnEditorInterface *iface) {
    iface->get_widget = get_widget;
    iface->update_connection = update_connection;
    // iface->suggest_address оставляем NULL (опционально)
}

NMVpnEditor *my_vpn_editor_new(NMConnection *connection, GError **error) {
    g_return_val_if_fail(NM_IS_CONNECTION(connection), NULL);
    
    MyVpnEditor *self = g_object_new(MY_VPN_TYPE_EDITOR, NULL);
    self->connection = g_object_ref(connection);
    load_from_connection(self);
    
    return NM_VPN_EDITOR(self);
}

static void my_vpn_editor_dispose(GObject *obj) {
    MyVpnEditor *self = MY_VPN_EDITOR(obj);
    g_clear_object(&self->connection);
    G_OBJECT_CLASS(my_vpn_editor_parent_class)->dispose(obj);
}

#include "file-chooser-widget.h"

struct _FileChooserWidget {
    GtkBox parent_instance;
    GtkEntry *entry;
    GtkButton *choose_btn;
    GtkButton *show_btn;
};

G_DEFINE_TYPE(FileChooserWidget, file_chooser_widget, GTK_TYPE_BOX)

enum {
    PROP_PATH = 1,
    N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = {NULL, };

enum {
    SIGNAL_PATH_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS] = {0, };

/* === Внутренние обработчики === */

static void on_file_selected(GObject *source, GAsyncResult *res, gpointer user_data) {
    FileChooserWidget *self = user_data;
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    GError *error = NULL;

    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);
    if (file) {
        gchar *path = g_file_get_path(file);
        if (path) {
            file_chooser_widget_set_path(self, path);
            g_free(path);
        }
        g_object_unref(file);
    } else if (error && !(error->domain == G_IO_ERROR && error->code == G_IO_ERROR_CANCELLED)) {
        g_warning("Ошибка выбора файла: %s", error->message);
        g_clear_error(&error);
    }
}

static void on_choose_clicked(GtkButton *btn, FileChooserWidget *self) {
    (void)btn;  // Подавляем предупреждение о неиспользуемом параметре
    GtkFileDialog *dialog = gtk_file_dialog_new();
    GtkWindow *toplevel = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(self)));
    if (toplevel) {
        gtk_file_dialog_open(dialog, toplevel, NULL, on_file_selected, self);
    }
    g_object_unref(dialog);
}

static void on_show_clicked(GtkButton *btn, FileChooserWidget *self) {
    (void)btn;
    const gchar *path = file_chooser_widget_get_path(self);
    gchar *message;

    if (path && *path) {
        gchar *name = g_path_get_basename(path);
        message = g_strdup_printf("Имя файла: %s", name);
        g_free(name);
    } else {
        message = g_strdup("Файл не выбран");
    }

    GtkAlertDialog *alert = gtk_alert_dialog_new("%s", message);
    GtkWindow *toplevel = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(self)));
    if (toplevel) {
        gtk_alert_dialog_show(alert, toplevel);
    }

    g_free(message);
    g_object_unref(alert);
}

/* === GObject: свойства === */

static void file_chooser_widget_set_property(GObject *obj, guint prop_id,
                                             const GValue *value, GParamSpec *pspec) {
    FileChooserWidget *self = FILE_CHOOSER_WIDGET(obj);
    switch (prop_id) {
        case PROP_PATH: {
            const gchar *new_path = g_value_get_string(value);
            const gchar *old_path = gtk_editable_get_text(GTK_EDITABLE(self->entry));
            if (g_strcmp0(old_path, new_path) != 0) {
                gtk_editable_set_text(GTK_EDITABLE(self->entry), new_path ?: "");
                g_signal_emit(self, signals[SIGNAL_PATH_CHANGED], 0, new_path);
            }
            break;
        }
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
    }
}

static void file_chooser_widget_get_property(GObject *obj, guint prop_id,
                                             GValue *value, GParamSpec *pspec) {
    FileChooserWidget *self = FILE_CHOOSER_WIDGET(obj);
    switch (prop_id) {
        case PROP_PATH:
            g_value_set_string(value, gtk_editable_get_text(GTK_EDITABLE(self->entry)));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
    }
}

/* === Инициализация класса === */

static void file_chooser_widget_class_init(FileChooserWidgetClass *klass) {
    GObjectClass *gclass = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gclass->set_property = file_chooser_widget_set_property;
    gclass->get_property = file_chooser_widget_get_property;

    properties[PROP_PATH] =
        g_param_spec_string("path", "Path", "Full path to selected file",
                            NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    signals[SIGNAL_PATH_CHANGED] =
        g_signal_new("path-changed",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST,
                     0, NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1, G_TYPE_STRING);

    g_object_class_install_properties(gclass, N_PROPERTIES, properties);
    gtk_widget_class_set_css_name(widget_class, "filechooserwidget");
}

/* === Инициализация экземпляра === */

static void file_chooser_widget_init(FileChooserWidget *self) {
    // В GTK 4 используем GtkOrientable для ориентации
    gtk_orientable_set_orientation(GTK_ORIENTABLE(self), GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_top(GTK_WIDGET(self), 12);
    gtk_widget_set_margin_bottom(GTK_WIDGET(self), 12);
    gtk_widget_set_margin_start(GTK_WIDGET(self), 12);
    gtk_widget_set_margin_end(GTK_WIDGET(self), 12);
    gtk_box_set_spacing(GTK_BOX(self), 8);

    // Строка: поле + кнопка
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_append(GTK_BOX(self), hbox);

    self->entry = GTK_ENTRY(gtk_entry_new());
    // Placeholder — только через GtkEntry, не через GtkEditable
    gtk_entry_set_placeholder_text(self->entry, "Путь к файлу…");
    gtk_widget_set_hexpand(GTK_WIDGET(self->entry), TRUE);
    gtk_box_append(GTK_BOX(hbox), GTK_WIDGET(self->entry));

    self->choose_btn = GTK_BUTTON(gtk_button_new_with_label("📁 Выбрать"));
    g_signal_connect(self->choose_btn, "clicked", G_CALLBACK(on_choose_clicked), self);
    gtk_box_append(GTK_BOX(hbox), GTK_WIDGET(self->choose_btn));

    self->show_btn = GTK_BUTTON(gtk_button_new_with_label("ℹ️ Показать имя"));
    g_signal_connect(self->show_btn, "clicked", G_CALLBACK(on_show_clicked), self);
    gtk_box_append(GTK_BOX(self), GTK_WIDGET(self->show_btn));
}

/* === Публичный API === */

GtkWidget *file_chooser_widget_new(void) {
    return g_object_new(FILE_CHOOSER_TYPE_WIDGET, NULL);
}

const gchar *file_chooser_widget_get_path(FileChooserWidget *self) {
    // Исправлено: FILE_IS_CHOOSER_WIDGET, а не FILE_CHOOSER_IS_WIDGET
    g_return_val_if_fail(FILE_IS_CHOOSER_WIDGET(self), NULL);
    return gtk_editable_get_text(GTK_EDITABLE(self->entry));
}

void file_chooser_widget_set_path(FileChooserWidget *self, const gchar *path) {
    g_return_if_fail(FILE_IS_CHOOSER_WIDGET(self));
    g_object_set(self, "path", path, NULL);
}

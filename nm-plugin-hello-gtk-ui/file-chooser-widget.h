#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define FILE_CHOOSER_TYPE_WIDGET (file_chooser_widget_get_type())
G_DECLARE_FINAL_TYPE(FileChooserWidget, file_chooser_widget, FILE, CHOOSER_WIDGET, GtkBox)

/**
 * FileChooserWidget:
 *
 * Виджет для выбора файла с отображением пути и имени.
 * Наследуется от GtkBox (вертикальный).
 */

// Конструктор
GtkWidget *file_chooser_widget_new(void);

// Геттеры/сеттеры для свойства "path"
const gchar *file_chooser_widget_get_path(FileChooserWidget *self);
void file_chooser_widget_set_path(FileChooserWidget *self, const gchar *path);

// Сигнал: вызывается при изменении пути
/**
 * FileChooserWidget::path-changed:
 * @self: виджет
 * @path: новый путь
 *
 * Сигнал испускается, когда пользователь выбрал новый файл.
 */
// (сигнал регистрируется автоматически через G_SIGNAL_RUN_LAST)

G_END_DECLS

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SEARCH_COMMAND "apt-cache search "
#define INSTALL_COMMAND "sudo apt-get install -y "
#define UPGRADE_COMMAND "sudo apt-get upgrade -y"
#define UNINSTALL_COMMAND "sudo apt-get remove -y "

GtkWidget *search_box;
GtkWidget *search_button;
GtkWidget *package_list;
GtkWidget *selected_list;
GtkWidget *dependencies_checkbox;
GtkWidget *install_button;
GtkWidget *upgrade_button;
GtkWidget *uninstall_button;
GtkWidget *details_button;

char *packages[1024];
char *selected_packages[1024];
int package_count = 0;
int selected_package_count = 0;

void search_packages()
{
  const gchar *query = gtk_entry_get_text(GTK_ENTRY(search_box));
  char command[1024];
  sprintf(command, "%s%s", SEARCH_COMMAND, query);
  FILE *fp = popen(command, "r");
  if (fp == NULL) {
    perror("popen");
    return;
  }
  char line[1024];
  package_count = 0;
  while (fgets(line, 1024, fp) != NULL) {
    packages[package_count] = strdup(strtok(line, " "));
    package_count++;
  }
  pclose(fp);
  gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(package_list))));
  for (int i = 0; i < package_count; i++) {
    gtk_list_store_insert_with_values(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(package_list))),
                                      NULL, -1, 0, packages[i], -1);
  }
}

void install_packages()
{
  char command[1024];
  sprintf(command, "%s", INSTALL_COMMAND);
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dependencies_checkbox))) {
    strcat(command, "--install-recommends ");
  }
  for (int i = 0; i < selected_package_count; i++) {
    strcat(command, selected_packages[i]);
    strcat(command, " ");
  }
  system(command);
  selected_package_count = 0;
  gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(selected_list))));
}

void upgrade_packages()
{
  system(UPGRADE_COMMAND);
}

void uninstall_packages()
{
  char command[1024];
  sprintf(command, "%s", UNINSTALL_COMMAND);
  for (int i = 0; i < selected_package_count; i++) {
    strcat(command, selected_packages[i]);
    strcat(command, " ");
  }
  system(command);
  selected_package_count = 0;
  gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(selected_list))));
}

void show_package_details()
{
  const char *package = selected_packages[0];
  char command[1024];
  sprintf(command,"apt-cache show %s", package);
FILE *fp = popen(command, "r");
if (fp == NULL) {
perror("popen");
return;
}
char details[4096];
int len = 0;
char line[1024];
while (fgets(line, 1024, fp) != NULL) {
len += sprintf(details + len, "%s", line);
}
pclose(fp);
GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, details);
gtk_window_set_title(GTK_WINDOW(dialog), package);
gtk_dialog_run(GTK_DIALOG(dialog));
gtk_widget_destroy(dialog);
}

void add_repository()
{
// TODO: implement add_repository function
}

void manage_security_updates()
{
// TODO: implement manage_security_updates function
}

void update_selected_packages(GtkTreeSelection *selection, gpointer data)
{
GtkTreeModel *model;
GtkTreeIter iter;
if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
char *package;
gtk_tree_model_get(model, &iter, 0, &package, -1);
selected_packages[selected_package_count] = package;
selected_package_count++;
gtk_list_store_insert_with_values(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(selected_list))),
NULL, -1, 0, package, -1);
g_free(package);
}
}

int main(int argc, char **argv)
{
GtkWidget *window;
GtkWidget *main_box;
GtkWidget *search_box_box;
GtkWidget *search_box_hbox;
GtkWidget *search_box_label;
GtkWidget *search_box_clear_button;
GtkWidget *packages_box;
GtkWidget *selected_box;
GtkWidget *actions_box;
GtkWidget *details_box;
GtkWidget *add_repository_button;
GtkWidget *security_updates_button;
GtkListStore *package_list_store;
GtkListStore *selected_list_store;
GtkTreeViewColumn *column;
GtkCellRenderer *renderer;
GtkTreeSelection *selection;

gtk_init(&argc, &argv);

window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_window_set_title(GTK_WINDOW(window), "Package Installer");
gtk_container_set_border_width(GTK_CONTAINER(window), 10);
gtk_widget_set_size_request(window, 800, 600);

main_box = gtk_vbox_new(FALSE, 10);
gtk_container_add(GTK_CONTAINER(window), main_box);

search_box_box = gtk_vbox_new(FALSE, 10);
gtk_box_pack_start(GTK_BOX(main_box), search_box_box, FALSE, FALSE, 0);

search_box_hbox = gtk_hbox_new(FALSE, 10);
gtk_box_pack_start(GTK_BOX(search_box_box), search_box_hbox, FALSE, FALSE, 0);

search_box_label = gtk_label_new("Search:");
gtk_box_pack_start(GTK_BOX(search_box_hbox), search_box_label, FALSE, FALSE, 0);

search_box = gtk_entry_new();
gtk_box_pack_start(GTK_BOX(search_box_hbox), search_box, TRUE, TRUE, 0);

search_button = gtk_button_new_with_label("Search");
g_signal_connect(search_button, "clicked", G_CALLBACK(search_packages), NULL);
gtk_box_pack_start(GTK_BOX(search_box_hbox), search_button, FALSE, FALSE, 0);

search_box_clear_button = gtk_button_new_with_label("Clear");
g_signal_connect(search_box_clear_button, "clicked", G_CALLBACK(gtk_entry_set_text), search_box);
gtk_box_pack_start(GTK_BOX(search_box_hbox), search_box_clear_button, FALSE, FALSE,0);

package_list = gtk_tree_view_new();
column = gtk_tree_view_column_new();
renderer = gtk_cell_renderer_text_new();
gtk_tree_view_column_pack_start(column, renderer, TRUE);
gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
gtk_tree_view_append_column(GTK_TREE_VIEW(package_list), column);
package_list_store = gtk_list_store_new(1, G_TYPE_STRING);
gtk_tree_view_set_model(GTK_TREE_VIEW(package_list), GTK_TREE_MODEL(package_list_store));
g_signal_connect(package_list, "row-activated", G_CALLBACK(update_selected_packages), NULL);

packages_box = gtk_vbox_new(FALSE, 10);
gtk_box_pack_start(GTK_BOX(main_box), packages_box, TRUE, TRUE, 0);

GtkScrolledWindow *packages_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(packages_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(packages_scrolled_window), 400);
gtk_container_add(GTK_CONTAINER(packages_scrolled_window), package_list);
gtk_box_pack_start(GTK_BOX(packages_box), packages_scrolled_window, TRUE, TRUE, 0);

selected_box = gtk_vbox_new(FALSE, 10);
gtk_box_pack_start(GTK_BOX(main_box), selected_box, FALSE, FALSE, 0);

GtkWidget *selected_label = gtk_label_new("Selected packages:");
gtk_box_pack_start(GTK_BOX(selected_box), selected_label, FALSE, FALSE, 0);

selected_list = gtk_tree_view_new();
column = gtk_tree_view_column_new();
renderer = gtk_cell_renderer_text_new();
gtk_tree_view_column_pack_start(column, renderer, TRUE);
gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
gtk_tree_view_append_column(GTK_TREE_VIEW(selected_list), column);
selected_list_store = gtk_list_store_new(1, G_TYPE_STRING);
gtk_tree_view_set_model(GTK_TREE_VIEW(selected_list), GTK_TREE_MODEL(selected_list_store));

GtkScrolledWindow *selected_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(selected_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(selected_scrolled_window), 200);
gtk_container_add(GTK_CONTAINER(selected_scrolled_window), selected_list);
gtk_box_pack_start(GTK_BOX(selected_box), selected_scrolled_window, TRUE, TRUE, 0);

actions_box = gtk_hbox_new(FALSE, 10);
gtk_box_pack_start(GTK_BOX(main_box), actions_box, FALSE, FALSE, 0);

dependencies_checkbox = gtk_check_button_new_with_label("Install recommended packages");
gtk_box_pack_start(GTK_BOX(actions_box), dependencies_checkbox, FALSE, FALSE, 0);

install_button = gtk_button_new_with_label("Install");
g_signal_connect(install_button, "clicked", G_CALLBACK(install_packages), NULL);
gtk_box_pack_start(GTK_BOX(actions_box), install_button, FALSE, FALSE, 0);

upgrade_button = gtk_button_new_with_label("Upgrade");
g_signal_connect(upgrade_button, "clicked", G_CALLBACK(upgrade_packages), NULL);
gtk_box_pack_start(GTK_BOX(actions_box), upgrade_button, FALSE, FALSE, 0);

uninstall_button = gtk_button_new_with_label("Uninstall");
g_signal_connect(uninstall_button, "clicked", G_CALLBACK(uninstall_packages), NULL);
gtk_box_pack_start(GTK_BOX(actions_box), uninstall_button, FALSE, FALSE,0);

details_box = gtk_hbox_new(FALSE, 10);
gtk_box_pack_start(GTK_BOX(main_box), details_box, FALSE, FALSE, 0);

details_button = gtk_button_new_with_label("Details");
g_signal_connect(details_button, "clicked", G_CALLBACK(show_package_details), NULL);
gtk_box_pack_start(GTK_BOX(details_box), details_button, FALSE, FALSE, 0);

add_repository_button = gtk_button_new_with_label("Add repository");
g_signal_connect(add_repository_button, "clicked", G_CALLBACK(add_repository), NULL);
gtk_box_pack_end(GTK_BOX(details_box), add_repository_button, FALSE, FALSE, 0);

security_updates_button = gtk_button_new_with_label("Manage security updates");
g_signal_connect(security_updates_button, "clicked", G_CALLBACK(manage_security_updates), NULL);
gtk_box_pack_end(GTK_BOX(details_box), security_updates_button, FALSE, FALSE, 0);

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(package_list));
gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

g_signal_connect(selection, "changed", G_CALLBACK(update_selected_packages), NULL);

g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

gtk_widget_show_all(window);

gtk_main();

return 0;
}


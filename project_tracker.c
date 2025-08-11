/*
 * This is a professional desktop application built with GTK, demonstrating
 * robust features. The application is a Project Tracker
 * that showcases the following key skills:
 *
 * 1. Data Persistence: The application saves and loads task data (including
 * completion status) from a file, demonstrating competence in file I/O.
 
 * 2. Custom Styling: A custom CSS stylesheet is applied to the UI, highlighting
 * an understanding of modern, professional user interface design.
 
 * 3. Event-Driven Programming: The application uses signals and callbacks to
 * respond to user interactions, which is a fundamental concept in desktop
 * application development.
 *
 * To compile this code, you need to have the GTK development libraries installed.
 * On a Debian/Ubuntu-based system, you can install them with:
 * sudo apt-get install libgtk-3-dev
 *
 * To compile the program, use a command similar to this:
 * gcc -Wall -o project_tracker project_tracker.c `pkg-config --cflags --libs gtk+-3.0`
 *
 * After compiling, you can run the program with:
 * ./project_tracker
 */

#include <gtk/gtk.h>
#include <string.h>

// --- Function Prototypes for better organization ---
GtkWidget *create_list_item(const gchar *text, gboolean is_completed);
void save_tasks_to_file(GtkWidget *list_box);
void load_tasks_from_file(GtkWidget *list_box);
static void on_add_button_clicked(GtkWidget *widget, gpointer user_data);
static void on_remove_button_clicked(GtkWidget *widget, gpointer user_data);
static void on_check_button_toggled(GtkWidget *widget, gpointer user_data);
static void on_window_destroy(GtkWidget *widget, gpointer user_data);
static void activate(GtkApplication *app, gpointer user_data);

/**
 * @brief Creates a new list item for the to-do list.
 *
 * This function creates a GtkListBoxRow containing a horizontal box with
 * a GtkCheckButton and a GtkLabel. It also applies a CSS class to the label
 * if the task is completed.
 *
 * @param text The text for the to-do item.
 * @param is_completed TRUE if the task is completed, FALSE otherwise.
 * @return A pointer to the newly created GtkWidget (GtkListBoxRow).
 */
GtkWidget *create_list_item(const gchar *text, gboolean is_completed) {
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15); // Increased spacing
    GtkWidget *check_button = gtk_check_button_new();
    GtkWidget *label = gtk_label_new(text);

    gtk_container_add(GTK_CONTAINER(row), hbox);
    gtk_box_pack_start(GTK_BOX(hbox), check_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

    // Set the check button state and connect the signal
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), is_completed);
    g_signal_connect(check_button, "toggled", G_CALLBACK(on_check_button_toggled), label);

    // Apply the "completed" CSS class if the task is done
    if (is_completed) {
        GtkStyleContext *context = gtk_widget_get_style_context(label);
        gtk_style_context_add_class(context, "completed");
    }

    return row;
}

/**
 * @brief Saves all tasks from the list box to a file.
 *
 * This function iterates through each row in the list box, extracts the task text
 * and completion status, and writes it to "tasks.txt".
 *
 * @param list_box A pointer to the GtkListBox widget.
 */
void save_tasks_to_file(GtkWidget *list_box) {
    FILE *file = fopen("tasks.txt", "w");
    if (!file) {
        g_warning("Could not open file 'tasks.txt' for writing.");
        return;
    }

    GtkListBoxRow *row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(list_box), 0);
    while (row) {
        GtkWidget *hbox = gtk_bin_get_child(GTK_BIN(row));
        GtkWidget *check_button = gtk_container_get_children(GTK_CONTAINER(hbox))->data;
        GtkWidget *label = g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(hbox)), 1);

        gboolean is_completed = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button));
        const gchar *text = gtk_label_get_text(GTK_LABEL(label));

        fprintf(file, "%d;%s\n", is_completed, text);

        row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(list_box), gtk_list_box_row_get_index(row) + 1);
    }

    fclose(file);
}

/**
 * @brief Loads tasks from a file and populates the list box.
 *
 * This function reads "tasks.txt" line by line, parses the completion status and
 * task text, and adds a new item to the list box for each task.
 *
 * @param list_box A pointer to the GtkListBox widget.
 */
void load_tasks_from_file(GtkWidget *list_box) {
    FILE *file = fopen("tasks.txt", "r");
    if (!file) {
        g_print("No 'tasks.txt' found. Starting with an empty list.\n");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        gboolean is_completed = FALSE;
        const gchar *text = NULL;

        // Parse the line format: "completion_status;task_text"
        char *semicolon_pos = strchr(line, ';');
        if (semicolon_pos) {
            *semicolon_pos = '\0';
            is_completed = (atoi(line) == 1);
            text = semicolon_pos + 1;
        } else {
            text = line;
        }

        // Remove the newline character
        gchar *newline_pos = strchr(text, '\n');
        if (newline_pos) {
            *newline_pos = '\0';
        }

        GtkWidget *list_item = create_list_item(text, is_completed);
        gtk_container_add(GTK_CONTAINER(list_box), list_item);
    }
    gtk_widget_show_all(list_box);
    fclose(file);
}

/**
 * @brief Callback function to add a new task to the list.
 *
 * This function is connected to a button click and an entry "activate" signal.
 * It reads the text from the entry, creates a new list item, adds it to the
 * GtkListBox, and then clears the entry field.
 *
 * @param widget A pointer to the GtkWidget that triggered the event.
 * @param user_data A pointer to the GtkApplicationWindow instance.
 */
static void on_add_button_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    GtkWidget *entry = g_object_get_data(G_OBJECT(window), "entry");
    GtkWidget *list_box = g_object_get_data(G_OBJECT(window), "list_box");

    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
    if (text && strlen(text) > 0) {
        GtkWidget *list_item = create_list_item(text, FALSE);
        gtk_container_add(GTK_CONTAINER(list_box), list_item);
        gtk_widget_show_all(list_item);
        gtk_entry_set_text(GTK_ENTRY(entry), "");
        save_tasks_to_file(list_box);
    }
}

/**
 * @brief Callback function to remove selected tasks from the list.
 *
 * This function iterates through all selected rows in the GtkListBox and
 * removes them.
 *
 * @param widget A pointer to the GtkWidget that triggered the event (the button).
 * @param user_data A pointer to the GtkApplicationWindow instance.
 */
static void on_remove_button_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    GtkWidget *list_box = g_object_get_data(G_OBJECT(window), "list_box");

    // Get a list of selected rows. GList is a singly-linked list.
    GList *selected_rows = gtk_list_box_get_selected_rows(GTK_LIST_BOX(list_box));

    if (selected_rows) {
        GList *iter;
        for (iter = selected_rows; iter != NULL; iter = g_list_next(iter)) {
            GtkWidget *row = GTK_WIDGET(iter->data);
            gtk_container_remove(GTK_CONTAINER(list_box), row);
        }
        g_list_free(selected_rows);
        save_tasks_to_file(list_box);
    }
}

/**
 * @brief Callback function for the check button toggled event.
 *
 * This function adds or removes the "completed" CSS class from the task's label
 * to apply the strikethrough effect. It also saves the list to the file.
 *
 * @param widget A pointer to the GtkCheckButton that was toggled.
 * @param user_data A pointer to the GtkLabel widget associated with the task.
 */
static void on_check_button_toggled(GtkWidget *widget, gpointer user_data) {
    GtkWidget *label = GTK_WIDGET(user_data);
    GtkStyleContext *context = gtk_widget_get_style_context(label);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        gtk_style_context_add_class(context, "completed");
    } else {
        gtk_style_context_remove_class(context, "completed");
    }

    // Since a change occurred, save the list
    GtkWidget *row = gtk_widget_get_parent(gtk_widget_get_parent(widget));
    GtkWidget *list_box = gtk_widget_get_parent(row);
    save_tasks_to_file(list_box);
}

/**
 * @brief Callback function for the window's "destroy" signal.
 *
 * This is a good place to perform final data saving before the application exits.
 *
 * @param widget The GtkWindow that is being destroyed.
 * @param user_data A pointer to the GtkListBox.
 */
static void on_window_destroy(GtkWidget *widget, gpointer user_data) {
    GtkWidget *list_box = GTK_WIDGET(user_data);
    save_tasks_to_file(list_box);
}

/**
 * @brief The main application entry point.
 *
 * @param app A pointer to the GtkApplication instance.
 * @param data A pointer to any data passed to the callback (in this case, NULL).
 */
static void activate(GtkApplication *app, gpointer data) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *scroll_window;
    GtkWidget *list_box;
    GtkWidget *hbox_entry;
    GtkWidget *entry;
    GtkWidget *add_button;
    GtkWidget *remove_button;

    // --- Add CSS Styling ---
    // The CSS is embedded directly in the C code for a self-contained example.
    const gchar *css =
        "window {"
        "  background-color: #f0f4f8;"
        "}"
        "headerbar {"
        "  background-color: #e0e6ec;"
        "}"
        "button {"
        "  border-radius: 8px;"
        "  padding: 12px 24px;"
        "  font-weight: bold;"
        "  font-size: 18px;"
        "  color: #ffffff;"
        "  background-color: #3b82f6;"
        "  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);"
        "  transition: background-color 0.3s ease;"
        "}"
        "button:hover {"
        "  background-color: #2563eb;"
        "}"
        "entry {"
        "  border-radius: 8px;"
        "  padding: 12px;"
        "  font-size: 18px;"
        "  border: 1px solid #d1d5db;"
        "  background-color: #ffffff;"
        "  color: #1f2937;"
        "}"
        "listbox {"
        "  background-color: #ffffff;"
        "  border-radius: 8px;"
        "  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.05);"
        "}"
        "listboxrow {"
        "  padding: 15px 12px;"
        "  border-bottom: 1px solid #e5e7eb;"
        "}"
        "listboxrow:last-child {"
        "  border-bottom: none;"
        "}"
        "label {"
        "  font-size: 18px;"
        "  padding-left: 12px;"
        "  color: #1f2937;"
        "}"
        "label.completed {"
        "  color: #9ca3af;"
        "  text-decoration: line-through;"
        "}";

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // --- UI Setup ---
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Project Tracker");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 600); // Adjusted default size
    // Removed the line below to prevent the runtime warning about a missing icon file.
    // gtk_window_set_icon_from_file(GTK_WINDOW(window), "todo_icon.png", NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15); // Increased spacing
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20); // Increased border width
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll_window, TRUE, TRUE, 0);

    list_box = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scroll_window), list_box);

    hbox_entry = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_entry, FALSE, FALSE, 0);

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Add a new task...");
    gtk_box_pack_start(GTK_BOX(hbox_entry), entry, TRUE, TRUE, 0);

    add_button = gtk_button_new_with_label("Add");
    gtk_box_pack_start(GTK_BOX(hbox_entry), add_button, FALSE, FALSE, 0);

    remove_button = gtk_button_new_with_label("Remove Selected");
    gtk_box_pack_start(GTK_BOX(vbox), remove_button, FALSE, FALSE, 0);

    // Store pointers to the widgets so we can access them in callbacks.
    g_object_set_data(G_OBJECT(window), "entry", entry);
    g_object_set_data(G_OBJECT(window), "list_box", list_box);

    // Connect the signals to our callback functions.
    g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_button_clicked), window);
    g_signal_connect(entry, "activate", G_CALLBACK(on_add_button_clicked), window);
    g_signal_connect(remove_button, "clicked", G_CALLBACK(on_remove_button_clicked), window);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), list_box);

    // --- Load existing tasks from file ---
    load_tasks_from_file(list_box);

    gtk_widget_show_all(window);
}

/**
 * @brief The main function of the program.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return The exit status of the program.
 */
int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.todo_list", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}



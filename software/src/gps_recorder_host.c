/*
 * gps_recorder_host.c
 *
 *  Created on: Nov 29, 2023
 *      Author: andrew
 */

#include </usr/include/gtk-3.0/gtk/gtk.h>
#include "/usr/include/osmgpsmap-1.0/osm-gps-map.h"
#include <stdbool.h>
#include <syslog.h>
#include "gps_recorder_device.h"
#include "filter.h"
#include "persistence.h"

#include "/usr/include/osmgpsmap-1.0/osm-gps-map.h"

// structure to pass around data
// required by various event handlers
typedef struct _app_data
{
	GtkWidget *main_window;
	GtkWidget *status_label;
	GtkWidget *file_tree_view;
	GtkWidget *status_button;
	GtkWidget *map;
	FILE *device;
} app_data_t;

static GdkRGBA red =
{ 1, 0, 0 };
static GdkRGBA blue =
{ 0, 0, 1 };

/** */
GtkWidget* osm_map_new(void)
{
	const double home_latitude = ini_get_home_latitude();
	const double home_longitude = ini_get_home_longitude();
	OsmGpsMapSource_t source = OSM_GPS_MAP_SOURCE_OPENSTREETMAP;

	OsmGpsMap *map = (OsmGpsMap*) g_object_new(OSM_TYPE_GPS_MAP, "map-source", source, "tile-cache", "/tmp/osm-cache", NULL);
	OsmGpsMapLayer * osd = (OsmGpsMapLayer*) g_object_new(OSM_TYPE_GPS_MAP_OSD, "show-scale", TRUE, "show-coordinates", TRUE, "show-crosshair", TRUE, "show-dpad", FALSE, "show-zoom", TRUE, "show-gps-in-dpad", TRUE, "show-gps-in-zoom", FALSE, "dpad-radius", 20, NULL);
	osm_gps_map_layer_add(OSM_GPS_MAP(map), osd);
	osm_gps_map_set_center_and_zoom(map, home_latitude, home_longitude, 17);

	return GTK_WIDGET(map);
}

/** */
static void on_destroy(GtkWidget *window, gpointer user_data)
{
	gps_recorder_device_close(((app_data_t*) user_data)->device);
}

/** */
bool status_button_pressed_event(GtkTreeView *view, GdkEventButton *event, gpointer user_data)
{
	app_data_t *app_data = (app_data_t*) user_data;

	/** */
	void add_status(bool fix_obtained, char *info)
	{
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->main_window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "fix %s obtained\n%s", fix_obtained ? "" : "not", info);

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

	}
	gps_recorder_get_status(app_data->device, add_status);

	return true;
}
/** */
static void file_tree_view_row_activated(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data)
{
	app_data_t *app_data = (app_data_t*) user_data;

	gchar *filename;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(view);
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 0, &filename, -1);
	int points = 0;

	OsmGpsMapTrack *track = osm_gps_map_gps_get_track(OSM_GPS_MAP(app_data->map));

	while (osm_gps_map_track_n_points(track))
	{
		osm_gps_map_track_remove_point(track, 0);
	}

	g_object_set(track, "line-width", 2.0, NULL);

	osm_gps_map_track_set_color(track, &red);

	gtk_label_set_text(GTK_LABEL(app_data->status_label), "getting data ...");

	while (gtk_events_pending())
	{
		gtk_main_iteration_do(true);
	}

	/** */
	void add_point_to_map(long time, double latitude, double longitude, double speed, double course)
	{
		OsmGpsMapPoint *map_point = osm_gps_map_point_new_degrees(latitude, longitude);
		osm_gps_map_point_set_user_data(map_point, (void*) time);
		osm_gps_map_track_add_point(track, map_point);

		char buffer[32];
		sprintf(buffer, "%d points displayed", ++points);
		gtk_label_set_text(GTK_LABEL(app_data->status_label), buffer);

		while (gtk_events_pending())
		{
			gtk_main_iteration_do(true);
		}
	}

	gps_recorder_device_list_file_points(app_data->device, filename, add_point_to_map);
}

/** */
static void populate_file_tree_view(app_data_t *app_data)
{
	GtkWidget *file_tree_view = app_data->file_tree_view;

	GtkTreeStore *store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_INT);

	/** */
	void add_file_to_list(char *name, int size)
	{
		GtkTreeIter iter;
		gtk_tree_store_append(store, &iter, NULL);
		gtk_tree_store_set(store, &iter, 0, name, -1);
		gtk_tree_store_set(store, &iter, 1, size, -1);

		while (gtk_events_pending())
		{
			gtk_main_iteration_do(true);
		}
	}

	gps_recorder_list_files(app_data->device, add_file_to_list);
	gtk_tree_view_set_model(GTK_TREE_VIEW(file_tree_view), GTK_TREE_MODEL(store));

	g_object_unref(store);
}

/** */
void delete(void *p, gpointer user_data)
{
	app_data_t *app_data = (app_data_t*) user_data;
	gchar *filename;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(app_data->file_tree_view));
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(app_data->file_tree_view));
	gtk_tree_selection_get_selected(selection, &model, &iter);
	gtk_tree_model_get(model, &iter, 0, &filename, -1);
	gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

	gps_recorder_delete_file(app_data->device, filename);
	populate_file_tree_view(app_data);
}

/** */
void load(void *p, gpointer user_data)
{
	app_data_t *app_data = (app_data_t*) user_data;
	gchar *filename;
	gchar *os_filename;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(app_data->file_tree_view));
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(app_data->file_tree_view));
	gtk_tree_selection_get_selected(selection, &model, &iter);
	gtk_tree_model_get(model, &iter, 0, &filename, -1);

	GtkWidget *dialog;
	gint res;

	dialog = gtk_file_chooser_dialog_new("Load File", GTK_WINDOW(app_data->main_window), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	res = gtk_dialog_run(GTK_DIALOG(dialog));

	os_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	if (res == GTK_RESPONSE_ACCEPT)
	{
		gtk_label_set_text(GTK_LABEL(app_data->status_label), "getting data ...");

		OsmGpsMapTrack *track = osm_gps_map_gps_get_track(OSM_GPS_MAP(app_data->map));

		while (osm_gps_map_track_n_points(track))
		{
			osm_gps_map_track_remove_point(track, 0);
		}

		g_object_set(track, "line-width", 2.0, NULL);

		osm_gps_map_track_set_color(track, &blue);

		FILE *file = fopen(os_filename, "r");

		if (file != NULL)
		{
			char buffer[64];
			int points = 0;
			while (true)
			{
				if (!fgets(buffer, sizeof(buffer), file))
				{
					break;
				}

				double latitude;
				double longitude;
				double speed;
				double course;
				long time;
				sscanf(buffer, "%ld\t%lf\t%lf\t%lf\t%lf", &time, &latitude, &longitude, &speed, &course);
				OsmGpsMapPoint *map_point = osm_gps_map_point_new_degrees(latitude, longitude);
				osm_gps_map_point_set_user_data(map_point, (void*) time);
				osm_gps_map_track_add_point(track, map_point);
				sprintf(buffer, "%d points displayed", ++points);
				gtk_label_set_text(GTK_LABEL(app_data->status_label), buffer);
			}

			fclose(file);
		}
		else
		{
			printf("could not open file\n");
		}
	}

	gtk_widget_destroy(dialog);
}

/** */
void save(void *p, gpointer user_data)
{
	app_data_t *app_data = (app_data_t*) user_data;
	gchar *filename;
	gchar *os_filename;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(app_data->file_tree_view));
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(app_data->file_tree_view));
	gtk_tree_selection_get_selected(selection, &model, &iter);
	gtk_tree_model_get(model, &iter, 0, &filename, -1);

	GtkWidget *dialog;
	gint res;

	dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(app_data->main_window), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), filename);

	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if (res == GTK_RESPONSE_ACCEPT)
	{
		os_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		FILE *file = fopen(os_filename, "w");

		if (file != NULL)
		{
			/** */
			void add_point_to_file(long time, double latitude, double longitude, double speed, double course)
			{
				fprintf(file, "%ld\t%lf\t%lf\t%lf\t%lf\n", time, latitude, longitude, speed, course);
				while (gtk_events_pending())
				{
					gtk_main_iteration_do(true);
				}
			}

			gps_recorder_device_list_file_points(app_data->device, filename, add_point_to_file);
			g_free(filename);
			g_free(os_filename);
			fclose(file);
		}
		else
		{
			printf("could not open file\n");

		}
	}

	gtk_widget_destroy(dialog);
}

/** */
bool file_tree_view_button_pressed_event(GtkTreeView *view, GdkEventButton *event, gpointer user_data)
{
	app_data_t *app_data = (app_data_t*) user_data;

	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		// create and display a popup menu of possible actions, connect up handlers and pass selected filename
		GtkWidget *popup_menu = gtk_menu_new();
		GtkWidget *delete_item = gtk_menu_item_new_with_label("delete");
		GtkWidget *save_item = gtk_menu_item_new_with_label("save");
		GtkWidget *load_item = gtk_menu_item_new_with_label("load");

		gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), delete_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), save_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), load_item);
		gtk_widget_show_all(popup_menu);

		g_signal_connect(save_item, "activate", (GCallback) save, app_data);
		g_signal_connect(delete_item, "activate", (GCallback) delete, app_data);
		g_signal_connect(load_item, "activate", (GCallback) load, app_data);
		gtk_menu_popup_at_widget(GTK_MENU(popup_menu), GTK_WIDGET(view), GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_EAST, NULL);
		return true;
	}

	return false;
}

/** */
void map_button_pressed_event(OsmGpsMap *map, GdkEventButton *event, gpointer user_data)
{
	app_data_t *app_data = (app_data_t*) user_data;
	float lat1, lon1, lat2, lon2;

	if (event->type == GDK_BUTTON_PRESS)
	{

		OsmGpsMapPoint *point = osm_gps_map_get_event_location(map, event);
		osm_gps_map_point_get_degrees(point, &lat1, &lon1);

		OsmGpsMapTrack *track = osm_gps_map_gps_get_track(OSM_GPS_MAP(app_data->map));
		GSList *list_of_points = osm_gps_map_track_get_points(track);
		GSList *elem;

		for (elem = list_of_points; elem; elem = elem->next)
		{
			OsmGpsMapPoint *point = (OsmGpsMapPoint*) elem->data;

			osm_gps_map_point_get_degrees(point, &lat2, &lon2);

			if (distance_between_points(lat1, lon1, lat2, lon2) < 2.0)
			{
				long time = (long) osm_gps_map_point_get_user_data(point);

				GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->main_window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
						          "Latitude %8.4lf longitude %8.4lf at %2ld:%2ld:%2ld\n", lat1, lon1, (time / 10000), (time / 100) % 100, time % 100);


				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				break;
			}
		}
	}
	else if (event->type == GDK_2BUTTON_PRESS)
	{
		OsmGpsMapPoint *point = osm_gps_map_get_event_location(map, event);
		osm_gps_map_point_get_degrees(point, &lat1, &lon1);

		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->main_window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
				 "Do you want to set home location to latitude %8.4lf longitude %8.4lf?\n", lat1, lon1);
		int res = gtk_dialog_run(GTK_DIALOG(dialog));

		if (res == GTK_RESPONSE_YES)
		{
			ini_set_home_latitude(lat1);
			ini_set_home_longitude(lon1);
		}

		gtk_widget_destroy(dialog);
	}
}

/** */
static void app_activate(GtkApplication *application, gpointer user_data)
{
	GtkWidget *main_box;
	GtkWidget *map_box;
	GtkWidget *status_box;

	app_data_t *app_data = (app_data_t*) user_data;

	char * device_name = ini_get_device_name();
	app_data->device = gps_recorder_device_open(device_name);

	if (!app_data->device)
	{
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(NULL), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
				                                                     "Device %s is not available", device_name);

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		g_application_quit(user_data);
	}

	app_data->main_window = gtk_application_window_new(application);
	gtk_window_set_title(GTK_WINDOW(app_data->main_window), "GPS Recorder Host");
	gtk_window_set_default_size(GTK_WINDOW(app_data->main_window), 800, 480);

	map_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(map_box), false);

	status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(status_box), false);

	main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(main_box), false);

	gtk_container_add(GTK_CONTAINER(app_data->main_window), main_box);

	gtk_box_pack_start(GTK_BOX(main_box), map_box, true, true, 2);
	gtk_box_pack_start(GTK_BOX(main_box), status_box, false, false, 2);

	app_data->map = osm_map_new();
	app_data->file_tree_view = gtk_tree_view_new();
	GtkCellRenderer *renderer;

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(app_data->file_tree_view), -1, "File name   ", renderer, "text", 0, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(app_data->file_tree_view), -1, "File size   ", renderer, "text", 1, NULL);
	gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(app_data->file_tree_view), GTK_TREE_VIEW_GRID_LINES_BOTH);

	gtk_box_pack_start(GTK_BOX(map_box), app_data->file_tree_view, false, false, 0);
	gtk_box_pack_start(GTK_BOX(map_box), app_data->map, true, true, 0);

	app_data->status_label = gtk_label_new("Ready");
	app_data->status_button = gtk_button_new_with_label("Status");
	g_signal_connect(app_data->status_button, "clicked", G_CALLBACK(status_button_pressed_event), NULL);

	gtk_box_pack_start(GTK_BOX(status_box), app_data->status_label, true, false, 0);
	gtk_box_pack_start(GTK_BOX(status_box), app_data->status_button, false, false, 10);

	g_signal_connect(G_OBJECT(app_data->main_window), "destroy", G_CALLBACK(on_destroy), app_data);
	g_signal_connect(G_OBJECT(app_data->file_tree_view), "row-activated", G_CALLBACK(file_tree_view_row_activated), app_data);
	g_signal_connect(G_OBJECT(app_data->file_tree_view), "button-press-event", G_CALLBACK(file_tree_view_button_pressed_event), app_data);
	g_signal_connect(G_OBJECT(app_data->status_button), "button-press-event", G_CALLBACK(status_button_pressed_event), app_data);
	g_signal_connect(G_OBJECT(app_data->map), "button-press-event", G_CALLBACK(map_button_pressed_event), app_data);

	populate_file_tree_view(app_data);
	gtk_widget_show_all(app_data->main_window);
}

/*
 *  program entry point
 */
int main(int argc, char **argv)
{
	GtkApplication *application;
	int stat;
	app_data_t app_data;

	if(!ini_load())
	{
		ini_create_default();
	}

	application = gtk_application_new("com.sd.gps_recorder_host", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(application, "activate", G_CALLBACK(app_activate), (gpointer) & app_data);
	stat = g_application_run(G_APPLICATION(application), argc, argv);
	g_object_unref(application);
	return stat;
}

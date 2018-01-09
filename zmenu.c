/* 
 * Compile with:
 *  gcc -o helloworld helloworld.c `pkg-config --cflags --libs gtk+-2.0`
 *
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libzgui/zwin.h>

GtkEntry *entry;
GtkTreeModelFilter *filter;
//GtkTreeViewColumn *col;
int count = 0;
int show_all = 1;

void   row_activated(GtkTreeView       *tree_view,
		GtkTreePath       *path,
		GtkTreeViewColumn *column,
		gpointer           data)
{
	GtkTreeModel *model = (GtkTreeModel *)data;
	GtkTreeIter  iter;
	gtk_tree_model_get_iter (model, &iter, path);
	gchar* str;
	gtk_tree_model_get (model, &iter, 0, &str, -1);
	printf("%s\n",str);
	g_free(str);
	gtk_main_quit();
}

#if 0
void   row_selected(GtkTreeView       *tree_view,
		gpointer           data)
{

	GtkTreePath *path;


	gtk_tree_view_get_cursor (tree_view, &path, NULL);
	if(path){
		GtkTreeModel *model = (GtkTreeModel *)data;
		GtkTreeIter  iter;
		gtk_tree_model_get_iter (model, &iter, path);
		gchar* str;
		gtk_tree_model_get (model, &iter, 0, &str, -1);
		//printf("select %s\n",str);
		//	gtk_entry_set_text(entry, str);
		g_free(str);
	}

}
#endif

gboolean
is_visible(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	gchar* str;
	gboolean match;
	gchar* temp;
	gchar * search;
	char *saveptr;
	const gchar* part;

	if(*(int*)data == 1) {
		return true;
	}

	const char *key = gtk_entry_get_text(GTK_ENTRY(entry));
	if(key[0] == 0)
	{
		return true;
	}


	gtk_tree_model_get (model, iter, 0, &str, -1);

	match = FALSE;
	if (G_LIKELY (str))
	{
		//printf("%s ######\t\t",str);
		temp = g_utf8_casefold (str, -1);
		search = g_utf8_casefold (key, -1);
		part = (char *) strtok_r (search, " ", &saveptr);
		if (part) {
			//printf("%s", part);
			match = (strstr (temp, part) != NULL);
			//printf(" %s\t", match? "Y" : "N");
			if (match) {
				while(match && (part = (char *) strtok_r (NULL, " ", &saveptr)) != NULL) {
					// TODO: if we type 'aa aa' we should match 2 different aa strings.
					//printf("%s", part);
					//printf(" %s\t", match? "Y" : "N");
					//  g_free (part);
					match = (strstr (temp, part) != NULL);
				}
			}
		}
		g_free (temp);
		g_free (str);
		//printf("%s\n", match? "TRUE" : "FALSE");
	}

	if(match){
		count++;
	}

	return match;
}

static gboolean
win_key_cb (GtkEntry* entry, GdkEventKey* pKey, void *tree_view)
{
	GtkTreePath *path;
	GtkTreeViewColumn *focus_column;

	if (pKey->keyval == GDK_Escape) {
		gtk_main_quit();
		return TRUE;
	} else if(pKey->keyval == GDK_Insert){
		gchar *str = NULL;
		if ((pKey->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK) {
			str = gtk_clipboard_wait_for_text (gtk_clipboard_get (GDK_SELECTION_PRIMARY));
			if(!str)
			str = gtk_clipboard_wait_for_text (gtk_clipboard_get (GDK_SELECTION_SECONDARY));
			if(!str)
			str = gtk_clipboard_wait_for_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
		}else {
			str = gtk_clipboard_wait_for_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
		}
		if (str) {
			g_free (str);
		}
		return TRUE;
	} else if(pKey->keyval == GDK_Up){


		gtk_tree_view_get_cursor (tree_view, &path, &focus_column);
		if(path){
			gtk_tree_path_prev(path);
			gtk_tree_view_set_cursor (tree_view, path, focus_column, FALSE);
		}

		return TRUE;
	} else if(pKey->keyval == GDK_Down){


		gtk_tree_view_get_cursor (tree_view, &path, &focus_column);
		if(path){
			gtk_tree_path_next( path);
			gtk_tree_view_set_cursor (tree_view, path, focus_column, FALSE);
		}

		return TRUE;
	} else if(pKey->keyval == GDK_Tab){
		gtk_tree_view_get_cursor (tree_view, &path, NULL);
		if(path){
		GtkTreeIter  iter;
		gtk_tree_model_get_iter (GTK_TREE_MODEL(filter), &iter, path);
		gchar* str;
		gtk_tree_model_get (GTK_TREE_MODEL(filter), &iter, 0, &str, -1);
		//printf("select %s\n",str);
		gtk_entry_set_text(entry, str);
		gtk_entry_set_position(entry, -1);
		}
		return TRUE;
	} else if(pKey->keyval == GDK_Return){
		printf("%s\n",gtk_entry_get_text(entry));
		gtk_main_quit();
		return TRUE;
	} else {
		show_all = 0;
		//zwin_event_emit(win, ZWinCommandChange);
		//gtk_entry_completion_complete (win->completion);
	}
	return FALSE;
}


static gboolean
win_command_cb (GtkWidget* entry, void *tree_view)
{
	GtkTreePath *path;
	count = 0;
	gtk_tree_model_filter_refilter (filter);
	if(count){
		GtkTreeViewColumn *column = gtk_tree_view_get_column (tree_view, 0);
		path =  gtk_tree_path_new_first();
		gtk_tree_view_set_cursor (tree_view, path, column, FALSE);
		gtk_tree_path_free(path);
	}
	return FALSE;
}

static GtkWidget *
create_view_and_model (int argc, char **argv)
{
	GtkCellRenderer     *renderer;
	GtkWidget           *view;
	GtkListStore  *store;
	GtkTreeIter    iter;
	GtkTreePath *path;
	FILE *f;
	char *args[32];
	char buf[1024];
	int i = 0;
	int j = 0;

	if(argv[1]) {
	f = fopen(argv[1], "r");
	if(!f)
		exit(-1);
	} else {
		f = stdin;
	}
	view = gtk_tree_view_new ();
	renderer = gtk_cell_renderer_text_new ();

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
					  -1,      
					  NULL,  
					  renderer,
					  "text", 0,
					  NULL);

	store   = gtk_list_store_new(1, G_TYPE_STRING);

	while(fgets(buf, 1024, f)){
		char *p = buf;
		char *q = buf;
		i = 0;
		gtk_list_store_append (store, &iter);
		while(*p){
			if (*p == '\n' || *p == 0 || *p == EOF)
			{
				GValue value = { 0, };
				*p = 0;
				g_value_init(&value, G_TYPE_STRING);
				g_value_set_string(&value, q);
				/* Append a row and fill in some data */
				gtk_list_store_set_value (store, &iter, i, &value);
				i++;
				q = p+1;
			}
			p++;
		}
	}


	filter = gtk_tree_model_filter_new(store, NULL);
	gtk_tree_model_filter_set_visible_func (filter, is_visible, &show_all, NULL);

	gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL(filter));
	gtk_tree_view_set_enable_search(view, false);

	/* The tree view has acquired its own reference to the
	 *  model, so we can drop ours. That way the model will
	 *  be freed automatically when the tree view is destroyed */

	g_object_unref (store);
	g_object_unref (filter);
	gtk_tree_view_set_headers_visible (view, FALSE);

	g_signal_connect (G_OBJECT (view), "row-activated", G_CALLBACK (row_activated), filter);
	//g_signal_connect (G_OBJECT (view), "cursor-changed", G_CALLBACK (row_selected), filter);

	return view;
}


#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdlib.h>


#include <gdk/gdkx.h>

Window target_win;
Display *dpy;
/* needed by dsimple.c */

void usage()
{
  fprintf(stderr, "Bad arguments\n");
}

#define OPAQUE	0xffffffff
#define OPACITY	"_NET_WM_WINDOW_OPACITY"

/* nothing fancy */
int transset(Window target_win, unsigned int opacity)
{

  int gotd = 0;
  double d = 0;

dpy = XOpenDisplay(NULL);

  Atom actual;
  int format;
  unsigned long n, left;

  if (opacity == OPAQUE)
    XDeleteProperty (dpy, target_win, XInternAtom(dpy, OPACITY, False));
  /* set it */
  else
    XChangeProperty(dpy, target_win, XInternAtom(dpy, OPACITY, False), 
		    XA_CARDINAL, 32, PropModeReplace, 
		    (unsigned char *) &opacity, 1L);
  XSync(dpy, False);
  

  /* all done, wasn't that simple */
  return 0;
}

int
main (int argc, char **argv)
{
  GtkWidget *window, *view, *scrollwin, *vbox, *hbox, *label;

  gtk_init(&argc,&argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW(window), 400, 400);
  //gdk_window_set_override_redirect(GTK_WIDGET(window)->window, true);

  g_signal_connect(window, "delete_event", gtk_main_quit, NULL);
  
  vbox = gtk_vbox_new(FALSE, 0);

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), (hbox), FALSE, FALSE, 0);

  if(argc > 1){
  	label  = GTK_WIDGET(gtk_label_new(argv[1]));
  	gtk_box_pack_start(GTK_BOX (hbox), (label), FALSE, FALSE, 0);
	argc--;
	argv++;
  }

  entry=gtk_entry_new ();
  gtk_box_pack_end(GTK_BOX (hbox), (entry), TRUE, TRUE, 0);
  //gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(hbox), TRUE, TRUE, 0);

  scrollwin = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin), 
			GTK_POLICY_AUTOMATIC, 
			GTK_POLICY_AUTOMATIC);

  
  view = create_view_and_model (argc, argv);
  gtk_container_add(GTK_CONTAINER(scrollwin), view);

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(scrollwin), TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (win_command_cb), view);
  g_signal_connect (G_OBJECT (entry), "key-press-event", G_CALLBACK (win_key_cb), GTK_TREE_VIEW(view));
  gtk_container_add(GTK_CONTAINER(window), vbox);

  gtk_window_set_modal(window, true);
  gtk_window_set_decorated(window, false);
  gtk_window_set_type_hint(window, GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_widget_show_all(window);
//  transset(GDK_WINDOW_XID(GTK_WIDGET(scrollwin)->window), 0xC0000000);
//  gdk_window_set_opacity (GTK_WIDGET(window)->window, 0.5);

  gtk_main();

  return 0;
}

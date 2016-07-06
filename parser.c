#include "parser.h"

#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gmenu-tree.h>
#undef GMENU_I_KNOW_THIS_IS_UNSTABLE

#include <vdr/tools.h>


static void print_entry(GMenuTreeEntry *entry, const char *indent)
{
  if (entry == NULL) {
     isyslog("desktop: entry is NULL");
     return;
     }

  const char *name = gmenu_tree_entry_get_name(entry);
  const char *generic_name = gmenu_tree_entry_get_generic_name(entry);
  const char *display_name = gmenu_tree_entry_get_display_name(entry);
  const char *file_id = gmenu_tree_entry_get_desktop_file_id(entry);
  const char *file_path = gmenu_tree_entry_get_desktop_file_path(entry);
  isyslog("desktop: %sentry name      = %s", indent, name);
  isyslog("desktop: %s      generic   = %s", indent, generic_name);
  isyslog("desktop: %s      display   = %s", indent, display_name);
  isyslog("desktop: %s      file_id   = %s", indent, file_id);
  isyslog("desktop: %s      file_path = %s", indent, file_path);
}

static void print_menu(GMenuTreeDirectory *directory, const char *indent)
{
  if (directory == NULL) {
     isyslog("desktop: directory is NULL");
     return;
     }

  const char *name = gmenu_tree_directory_get_name(directory);
  isyslog("desktop: %sdirectory = %s", indent, name);

  GSList *contents = gmenu_tree_directory_get_contents(directory);
  if (contents != NULL) {
     GSList *c = contents;
     while (c != NULL) {
           if (c->data != NULL) {
              GMenuTreeItem *item = GMENU_TREE_ITEM(c->data);
              GMenuTreeItemType type = gmenu_tree_item_get_type(item);
              if (type == GMENU_TREE_ITEM_DIRECTORY) {
                 GMenuTreeDirectory *subdir = GMENU_TREE_DIRECTORY(item);
                 print_menu(subdir, *cString::sprintf("%s ", indent));
                 }
              else if (type == GMENU_TREE_ITEM_ENTRY) {
                 GMenuTreeEntry *entry = GMENU_TREE_ENTRY(item);
                 print_entry(entry, *cString::sprintf("%s ", indent));
                 }
              }
           c = g_slist_next(c);
           }
     }
}

void cParser::Test(const char *filename)
{
  GMenuTree *tree = gmenu_tree_lookup(filename, GMENU_TREE_FLAGS_NONE);
  if (tree == NULL) {
     isyslog("desktop: tree is NULL");
     return;
     }

  GMenuTreeDirectory *root = gmenu_tree_get_root_directory(tree);
  print_menu(root, "");

  gmenu_tree_unref(tree);
}

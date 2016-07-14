#include "menu.h"

#include <vdr/thread.h>
#include <vdr/tools.h>


class cDesktopMenuItem : public cOsdItem {
private:
  GMenuTreeDirectory *directory;
  GMenuTreeEntry *entry;

public:
  cDesktopMenuItem(GMenuTreeDirectory *Directory)
  {
    entry = NULL;
    directory = Directory;
    SetText(gmenu_tree_directory_get_name(directory));
  }

  cDesktopMenuItem(GMenuTreeEntry *Entry)
  {
    directory = NULL;
    entry = Entry;
    GDesktopAppInfo *appInfo = gmenu_tree_entry_get_app_info(entry);
    char *name = g_desktop_app_info_get_string(appInfo, "Name");
    SetText(name);
    free(name);
  }

  virtual ~cDesktopMenuItem(void)
  {
    if (directory != NULL) {
       gmenu_tree_item_unref(directory);
       directory = NULL;
       }
    if (entry != NULL) {
       gmenu_tree_item_unref(entry);
       entry = NULL;
       }
  }

  bool IsDirectory(void) const
  {
    return (directory != NULL);
  }

  GMenuTreeDirectory *Directory(void) const
  {
    return directory;
  }

  GMenuTreeEntry *Entry(void) const
  {
    return entry;
  }
  };


cString cDesktopMenu::PluginConfDir;

cDesktopMenu::cDesktopMenu(const char *menu_filename)
:cOsdMenu("Desktop")
{
  tree = gmenu_tree_new_for_path(menu_filename, GMENU_TREE_FLAGS_NONE);
  if (tree == NULL) {
     directory = NULL;
     esyslog("desktop: tree is NULL, used menu-file %s", menu_filename);
     }
  else
     directory = gmenu_tree_get_root_directory(tree);

  SetMenuCategory(mcPlugin);
  Set();
  if (Current() < 0)
     SetCurrent(First());
  Display();
}

cDesktopMenu::cDesktopMenu(GMenuTreeDirectory *Directory)
:cOsdMenu(gmenu_tree_directory_get_name(Directory))
{
  tree = NULL;
  directory = Directory;

  SetMenuCategory(mcPlugin);
  Set();
  if (Current() < 0)
     SetCurrent(First());
  Display();
}

cDesktopMenu::~cDesktopMenu(void)
{
  if (directory != NULL) {
     gmenu_tree_item_unref(directory);
     directory = NULL;
     }
  if (tree != NULL) {
     gmenu_tree_item_unref(tree);
     tree = NULL;
     }
}

void cDesktopMenu::Set(void)
{
  Clear();
  if (directory == NULL)
     return;

  GMenuTreeIter *iter = gmenu_tree_directory_iter(directory);
  if (iter != NULL) {
     GMenuTreeItemType type;
     while ((type = gmenu_tree_iter_next(iter)) != GMENU_TREE_ITEM_INVALID) {
           if (type == GMENU_TREE_ITEM_DIRECTORY)
              Add(new cDesktopMenuItem(gmenu_tree_iter_get_directory(iter)));
           else if (type == GMENU_TREE_ITEM_ENTRY)
              Add(new cDesktopMenuItem(gmenu_tree_iter_get_entry(iter)));
           }
     gmenu_tree_iter_unref(iter);
     }
}

eOSState cDesktopMenu::Run()
{
  cDesktopMenuItem *di = (cDesktopMenuItem*)Get(Current());
  if (di) {
     if (di->IsDirectory())
        AddSubMenu(new cDesktopMenu(di->Directory()));
     else {
        cString cmd = cString::sprintf("%s/starter", *PluginConfDir);
        if (FileSize(*cmd) <= 0)
           Skins.QueueMessage(mtError, *cString::sprintf("missing starter-script %s", *cmd));
        else {
           const char *desktop_file_path = gmenu_tree_entry_get_desktop_file_path(di->Entry());
           cString dfp = strescape(desktop_file_path, "\\\"$");
           isyslog("desktop: starting \"%s\"", *dfp);
           SystemExec(*cString::sprintf("%s \"%s\"", *cmd, *dfp), true);
           }
        return osEnd;
        }
     }
  return osContinue;
}

eOSState cDesktopMenu::ProcessKey(eKeys Key)
{
  eOSState state = cOsdMenu::ProcessKey(Key);
  if (state == osUnknown) {
     switch (Key) {
       case kOk: return Run();
       default: break;
       }
     }
  return state;
}

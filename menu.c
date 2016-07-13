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
    SetText(gmenu_tree_entry_get_name(entry));
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
  tree = gmenu_tree_lookup(menu_filename, GMENU_TREE_FLAGS_NONE);
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
     gmenu_tree_unref(tree);
     tree = NULL;
     }
}

void cDesktopMenu::Set(void)
{
  Clear();
  if (directory == NULL)
     return;

  GSList *contents = gmenu_tree_directory_get_contents(directory);
  if (contents != NULL) {
     GSList *c = contents;
     while (c != NULL) {
           if (c->data != NULL) {
              GMenuTreeItem *item = GMENU_TREE_ITEM(c->data);
              GMenuTreeItemType type = gmenu_tree_item_get_type(item);
              if (type == GMENU_TREE_ITEM_DIRECTORY)
                 Add(new cDesktopMenuItem(GMENU_TREE_DIRECTORY(item)));
              else if (type == GMENU_TREE_ITEM_ENTRY)
                 Add(new cDesktopMenuItem(GMENU_TREE_ENTRY(item)));
              }
           c = g_slist_next(c);
           }
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

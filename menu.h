#ifndef __DESKTOP_MENU_H
#define __DESKTOP_MENU_H

#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gmenu-tree.h>
#undef GMENU_I_KNOW_THIS_IS_UNSTABLE

#include <vdr/osdbase.h>

class cDesktopMenuItem;


class cDesktopMenu : public cOsdMenu {
private:
  GMenuTree *tree;
  GMenuTreeDirectory *directory;

  void Set(void);
  eOSState Run(void);

public:
  static cString PluginConfDir;

  cDesktopMenu(const char *menu_filename);
  cDesktopMenu(GMenuTreeDirectory *Directory);
  virtual ~cDesktopMenu(void);

  virtual eOSState ProcessKey(eKeys Key);
  };

#endif

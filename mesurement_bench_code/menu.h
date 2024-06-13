#include <stdint.h>
#include <string.h>

#define MAX_ITEMS 10
#define MAX_ITEM_NAME_LENGTH 25

#define ERROR 0x00U
#define OK 0x01U

typedef struct Menu {
  int8_t _menu_state;
  int8_t _max_items;
  char _menu_name[MAX_ITEM_NAME_LENGTH];
  char _items_name[MAX_ITEMS][MAX_ITEM_NAME_LENGTH];
  struct Menu *prevMenu;
  struct Menu *subMenus[MAX_ITEMS - 1];
}Menu;

Menu *rootMenu;

void setMenuName(Menu *currMenu, char str[MAX_ITEM_NAME_LENGTH]) {  // use after initMenu for root menu
  strncpy(currMenu->_menu_name, str, MAX_ITEM_NAME_LENGTH - 1);
  // currMenu->_menu_name[MAX_ITEM_NAME_LENGTH - 1] = '\0';
}

uint8_t initMenu(Menu *currMenu, uint8_t items, char str[][MAX_ITEM_NAME_LENGTH]) {
  if (items > MAX_ITEMS) {
    return ERROR;  //error errror error
  }
  currMenu->_max_items = items;
  for (int i = 0; i < items; i++) {
    strncpy(currMenu->_items_name[i], str[i], MAX_ITEM_NAME_LENGTH - 1);
    // currMenu->_items_name[i][MAX_ITEM_NAME_LENGTH - 1] = '\0';
  }

  // if (currMenu->subMenus[0] == NULL) {
  if (currMenu->prevMenu == NULL) {
    strncpy(currMenu->_menu_name, "menuERROR", MAX_ITEM_NAME_LENGTH - 1);
  } else {
    strncpy(currMenu->_menu_name, "someMenu", MAX_ITEM_NAME_LENGTH - 1);
  }
  return OK;
}

int8_t selectItem(Menu *currMenu, int8_t item) {
  currMenu->_menu_state = item;

  if (item > currMenu->_max_items) {
    currMenu->_menu_state = 1;
  } else if (item < 1) {
    currMenu->_menu_state = currMenu->_max_items;
  }
  return currMenu->_menu_state;
}

int8_t getMaxItems(Menu *currMenu) {
  return currMenu->_max_items;
}

int8_t getMenuState(Menu *currMenu) {
  return currMenu->_menu_state;
}

char *getMenuName(Menu *currMenu) {
  return currMenu->_menu_name;
}

char *getItemName(Menu *currMenu, int8_t poz) {
  return currMenu->_items_name[poz - 1];
}

int8_t getItem(Menu *currMenu, int8_t item) {
  return selectItem(currMenu, item);
}
//================================================================
int8_t nextItem(Menu *currMenu) {
  return selectItem(currMenu, ++currMenu->_menu_state);
}

int8_t prevItem(Menu *currMenu) {
  return selectItem(currMenu, --currMenu->_menu_state);
}
//================================================================
void setMenuState(Menu *currMenu, int8_t item) {
  currMenu->_menu_state = item;
}

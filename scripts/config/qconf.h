/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2002 Roman Zippel <zippel@linux-m68k.org>
 */

<<<<<<< HEAD
#include <QCheckBox>
#include <QDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QTextBrowser>
#include <QTreeWidget>

#include "expr.h"

class ConfigList;
class ConfigItem;
=======
#include <QTextBrowser>
#include <QTreeWidget>
#include <QMainWindow>
#include <QHeaderView>
#include <qsettings.h>
#include <QPushButton>
#include <QSettings>
#include <QLineEdit>
#include <QSplitter>
#include <QCheckBox>
#include <QDialog>
#include "expr.h"

class ConfigView;
class ConfigList;
class ConfigItem;
class ConfigLineEdit;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
class ConfigMainWindow;

class ConfigSettings : public QSettings {
public:
	ConfigSettings();
	QList<int> readSizes(const QString& key, bool *ok);
	bool writeSizes(const QString& key, const QList<int>& value);
};

enum colIdx {
<<<<<<< HEAD
	promptColIdx, nameColIdx, dataColIdx
=======
	promptColIdx, nameColIdx, noColIdx, modColIdx, yesColIdx, dataColIdx, colNr
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
};
enum listMode {
	singleMode, menuMode, symbolMode, fullMode, listMode
};
enum optionMode {
	normalOpt = 0, allOpt, promptOpt
};

class ConfigList : public QTreeWidget {
	Q_OBJECT
	typedef class QTreeWidget Parent;
public:
<<<<<<< HEAD
	ConfigList(QWidget *parent, const char *name = 0);
	~ConfigList();
	void reinit(void);
	ConfigItem* findConfigItem(struct menu *);
	void setSelected(QTreeWidgetItem *item, bool enable) {
		for (int i = 0; i < selectedItems().size(); i++)
			selectedItems().at(i)->setSelected(false);

		item->setSelected(enable);
	}
=======
	ConfigList(ConfigView* p, const char *name = 0);
	void reinit(void);
	ConfigView* parent(void) const
	{
		return (ConfigView*)Parent::parent();
	}
	ConfigItem* findConfigItem(struct menu *);
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

protected:
	void keyPressEvent(QKeyEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void focusInEvent(QFocusEvent *e);
	void contextMenuEvent(QContextMenuEvent *e);

public slots:
	void setRootMenu(struct menu *menu);

<<<<<<< HEAD
	void updateList();
=======
	void updateList(ConfigItem *item);
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	void setValue(ConfigItem* item, tristate val);
	void changeValue(ConfigItem* item);
	void updateSelection(void);
	void saveSettings(void);
<<<<<<< HEAD
	void setOptionMode(QAction *action);
	void setShowName(bool on);

signals:
	void menuChanged(struct menu *menu);
	void menuSelected(struct menu *menu);
	void itemSelected(struct menu *menu);
	void parentSelected(void);
	void gotFocus(struct menu *);
	void showNameChanged(bool on);
=======
signals:
	void menuChanged(struct menu *menu);
	void menuSelected(struct menu *menu);
	void parentSelected(void);
	void gotFocus(struct menu *);
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

public:
	void updateListAll(void)
	{
		updateAll = true;
<<<<<<< HEAD
		updateList();
		updateAll = false;
	}
=======
		updateList(NULL);
		updateAll = false;
	}
	ConfigList* listView()
	{
		return this;
	}
	ConfigItem* firstChild() const
	{
		return (ConfigItem *)children().first();
	}
	void addColumn(colIdx idx)
	{
		showColumn(idx);
	}
	void removeColumn(colIdx idx)
	{
		hideColumn(idx);
	}
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	void setAllOpen(bool open);
	void setParentMenu(void);

	bool menuSkip(struct menu *);

	void updateMenuList(ConfigItem *parent, struct menu*);
<<<<<<< HEAD
	void updateMenuList(struct menu *menu);

	bool updateAll;

	bool showName;
=======
	void updateMenuList(ConfigList *parent, struct menu*);

	bool updateAll;

	QPixmap symbolYesPix, symbolModPix, symbolNoPix;
	QPixmap choiceYesPix, choiceNoPix;
	QPixmap menuPix, menuInvPix, menuBackPix, voidPix;

	bool showName, showRange, showData;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	enum listMode mode;
	enum optionMode optMode;
	struct menu *rootEntry;
	QPalette disabledColorGroup;
	QPalette inactivedColorGroup;
	QMenu* headerPopup;
<<<<<<< HEAD

	static QList<ConfigList *> allLists;
	static void updateListForAll();
	static void updateListAllForAll();

	static QAction *showNormalAction, *showAllAction, *showPromptAction;
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
};

class ConfigItem : public QTreeWidgetItem {
	typedef class QTreeWidgetItem Parent;
public:
	ConfigItem(ConfigList *parent, ConfigItem *after, struct menu *m, bool v)
	: Parent(parent, after), nextItem(0), menu(m), visible(v), goParent(false)
	{
		init();
	}
	ConfigItem(ConfigItem *parent, ConfigItem *after, struct menu *m, bool v)
	: Parent(parent, after), nextItem(0), menu(m), visible(v), goParent(false)
	{
		init();
	}
	ConfigItem(ConfigList *parent, ConfigItem *after, bool v)
	: Parent(parent, after), nextItem(0), menu(0), visible(v), goParent(true)
	{
		init();
	}
	~ConfigItem(void);
	void init(void);
<<<<<<< HEAD
=======
	void okRename(int col);
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	void updateMenu(void);
	void testUpdateMenu(bool v);
	ConfigList* listView() const
	{
		return (ConfigList*)Parent::treeWidget();
	}
	ConfigItem* firstChild() const
	{
		return (ConfigItem *)Parent::child(0);
	}
	ConfigItem* nextSibling()
	{
		ConfigItem *ret = NULL;
		ConfigItem *_parent = (ConfigItem *)parent();

		if(_parent) {
			ret = (ConfigItem *)_parent->child(_parent->indexOfChild(this)+1);
		} else {
			QTreeWidget *_treeWidget = treeWidget();
			ret = (ConfigItem *)_treeWidget->topLevelItem(_treeWidget->indexOfTopLevelItem(this)+1);
		}

		return ret;
	}
<<<<<<< HEAD
=======
	void setText(colIdx idx, const QString& text)
	{
		Parent::setText(idx, text);
	}
	QString text(colIdx idx) const
	{
		return Parent::text(idx);
	}
	void setPixmap(colIdx idx, const QIcon &icon)
	{
		Parent::setIcon(idx, icon);
	}
	const QIcon pixmap(colIdx idx) const
	{
		return icon(idx);
	}
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	// TODO: Implement paintCell

	ConfigItem* nextItem;
	struct menu *menu;
	bool visible;
	bool goParent;
<<<<<<< HEAD

	static QIcon symbolYesIcon, symbolModIcon, symbolNoIcon;
	static QIcon choiceYesIcon, choiceNoIcon;
	static QIcon menuIcon, menubackIcon;
};

class ConfigItemDelegate : public QStyledItemDelegate
{
private:
	struct menu *menu;
public:
	ConfigItemDelegate(QObject *parent = nullptr)
		: QStyledItemDelegate(parent) {}
	QWidget *createEditor(QWidget *parent,
			      const QStyleOptionViewItem &option,
			      const QModelIndex &index) const override;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
			  const QModelIndex &index) const override;
=======
};

class ConfigLineEdit : public QLineEdit {
	Q_OBJECT
	typedef class QLineEdit Parent;
public:
	ConfigLineEdit(ConfigView* parent);
	ConfigView* parent(void) const
	{
		return (ConfigView*)Parent::parent();
	}
	void show(ConfigItem *i);
	void keyPressEvent(QKeyEvent *e);

public:
	ConfigItem *item;
};

class ConfigView : public QWidget {
	Q_OBJECT
	typedef class QWidget Parent;
public:
	ConfigView(QWidget* parent, const char *name = 0);
	~ConfigView(void);
	static void updateList(ConfigItem* item);
	static void updateListAll(void);

	bool showName(void) const { return list->showName; }
	bool showRange(void) const { return list->showRange; }
	bool showData(void) const { return list->showData; }
public slots:
	void setShowName(bool);
	void setShowRange(bool);
	void setShowData(bool);
	void setOptionMode(QAction *);
signals:
	void showNameChanged(bool);
	void showRangeChanged(bool);
	void showDataChanged(bool);
public:
	ConfigList* list;
	ConfigLineEdit* lineEdit;

	static ConfigView* viewList;
	ConfigView* nextView;

	static QAction *showNormalAction;
	static QAction *showAllAction;
	static QAction *showPromptAction;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
};

class ConfigInfoView : public QTextBrowser {
	Q_OBJECT
	typedef class QTextBrowser Parent;
<<<<<<< HEAD
	QMenu *contextMenu;
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
public:
	ConfigInfoView(QWidget* parent, const char *name = 0);
	bool showDebug(void) const { return _showDebug; }

public slots:
	void setInfo(struct menu *menu);
	void saveSettings(void);
	void setShowDebug(bool);
<<<<<<< HEAD
	void clicked (const QUrl &url);
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

signals:
	void showDebugChanged(bool);
	void menuSelected(struct menu *);

protected:
	void symbolInfo(void);
	void menuInfo(void);
	QString debug_info(struct symbol *sym);
	static QString print_filter(const QString &str);
	static void expr_print_help(void *data, struct symbol *sym, const char *str);
<<<<<<< HEAD
	void contextMenuEvent(QContextMenuEvent *event);
=======
	QMenu *createStandardContextMenu(const QPoint & pos);
	void contextMenuEvent(QContextMenuEvent *e);
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

	struct symbol *sym;
	struct menu *_menu;
	bool _showDebug;
};

class ConfigSearchWindow : public QDialog {
	Q_OBJECT
	typedef class QDialog Parent;
public:
<<<<<<< HEAD
	ConfigSearchWindow(ConfigMainWindow *parent);
=======
	ConfigSearchWindow(ConfigMainWindow* parent, const char *name = 0);
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

public slots:
	void saveSettings(void);
	void search(void);

protected:
	QLineEdit* editField;
	QPushButton* searchButton;
	QSplitter* split;
<<<<<<< HEAD
	ConfigList *list;
=======
	ConfigView* list;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	ConfigInfoView* info;

	struct symbol **result;
};

class ConfigMainWindow : public QMainWindow {
	Q_OBJECT

	char *configname;
	static QAction *saveAction;
	static void conf_changed(void);
public:
	ConfigMainWindow(void);
public slots:
	void changeMenu(struct menu *);
<<<<<<< HEAD
	void changeItens(struct menu *);
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	void setMenuLink(struct menu *);
	void listFocusChanged(void);
	void goBack(void);
	void loadConfig(void);
	bool saveConfig(void);
	void saveConfigAs(void);
	void searchConfig(void);
	void showSingleView(void);
	void showSplitView(void);
	void showFullView(void);
	void showIntro(void);
	void showAbout(void);
	void saveSettings(void);

protected:
	void closeEvent(QCloseEvent *e);

	ConfigSearchWindow *searchWindow;
<<<<<<< HEAD
	ConfigList *menuList;
	ConfigList *configList;
	ConfigInfoView *helpText;
=======
	ConfigView *menuView;
	ConfigList *menuList;
	ConfigView *configView;
	ConfigList *configList;
	ConfigInfoView *helpText;
	QToolBar *toolBar;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	QAction *backAction;
	QAction *singleViewAction;
	QAction *splitViewAction;
	QAction *fullViewAction;
	QSplitter *split1;
	QSplitter *split2;
};

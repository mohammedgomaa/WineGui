/**
 * Copyright (c) 2019 WineGUI
 *
 * \file    main_window.cc
 * \brief   Main GTK+ Window class
 * \author  Melroy van den Berg <webmaster1989@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "main_window.h"

#include "signal_dispatcher.h"
#include <iostream>
#include <locale>

/**
 * \brief Contructor
 */
MainWindow::MainWindow(Menu& menu)
: vbox(Gtk::ORIENTATION_VERTICAL),
  paned(Gtk::ORIENTATION_HORIZONTAL),
  right_box(Gtk::Orientation::ORIENTATION_VERTICAL),
  separator1(Gtk::ORIENTATION_HORIZONTAL)
{
  // Set some Window properties
  set_title("WineGUI - WINE Manager");
  set_default_size(1000, 600);
  set_position(Gtk::WIN_POS_CENTER_ALWAYS);

  this->set_icon_from_file(IMAGE_LOCATION "logo.png");

  // Add menu to box (top), no expand/fill
  vbox.pack_start(menu, false, false);

  // Add paned to box (below menu)
  // NOTE: expand/fill = true
  vbox.pack_end(paned);

  // Create rest to vbox
  CreateLeftPanel();
  CreateRightPanel();

  // Using a Vertical box container
  add(vbox);

  // Right panel menu signals
  new_button.signal_clicked().connect(sigc::mem_fun(*this,
    &MainWindow::on_new_bottle_button_clicked));
  newBottleAssistant.signal_apply().connect(sigc::mem_fun(*this,
    &MainWindow::on_new_bottle_apply));
  run_button.signal_clicked().connect(sigc::mem_fun(*this,
    &MainWindow::on_run_button_clicked));
  settings_button.signal_clicked().connect(sigc::mem_fun(*this,
    &MainWindow::on_not_implemented));
  manage_button.signal_clicked().connect(sigc::mem_fun(*this,
    &MainWindow::on_not_implemented));
  reboot_button.signal_clicked().connect(sigc::mem_fun(*this,
    &MainWindow::on_not_implemented));// TODO: Really handy at all?

  // Show the widget children
  show_all_children();
}

/**
 * \brief Destructor
 */
MainWindow::~MainWindow() {
}

/**
 * \brief Set signal dispatcher
 */
void MainWindow::SetDispatcher(SignalDispatcher& signalDispatcher)
{
  // Trigger on_hide_window afer hide signal (for immidate response after pressing quit button)
  signalDispatcher.hideMainWindow.connect(sigc::mem_fun(*this, &MainWindow::on_hide_window));

  listbox.signal_row_selected().connect(sigc::mem_fun(*this, &MainWindow::on_row_clicked));
  // Send listbox (left panel) signal to dispatcher
  listbox.signal_button_press_event().connect(sigc::mem_fun(signalDispatcher, &SignalDispatcher::on_button_press_event));

  // Redirect new Bottle Assistant signal to dispatcher
  newBottleAssistant.newBottleFinished.connect(sigc::mem_fun(signalDispatcher, &SignalDispatcher::on_update_bottles));
}

/**
 * \brief Just hide the main window
 */
void MainWindow::on_hide_window()
{
  hide();
}

/**
 * \brief Change detailed window on listbox row clicked event
 */
void MainWindow::on_row_clicked(Gtk::ListBoxRow* row)
{
  if(row != nullptr) {
    SetDetailedInfo(*(BottleItem*)row);
    // Inform Bottle Manager
    activeBottle.emit((BottleItem*)row);
  }
}

/**
 * \brief Append a single Wine Bottle to the left panel
 * \param[in] bottle - A single WineBottle class
 */
void MainWindow::AppendWineBottle(BottleItem& bottle)
{
  listbox.add(bottle);
  // Update show
  show_all_children();
}

/**
 * \brief Set a list/vector of bottles to the left panel
 * \param[in] bottles - Wine Bottle item vector array
 */
void MainWindow::SetWineBottles(std::list<BottleItem>& bottles)
{
  // Clear whole listbox
  std::vector<Gtk::Widget*> children = listbox.get_children();
  for(Gtk::Widget* el: children) {
    listbox.remove(*el);
  }

  for(BottleItem& bottle : bottles)
  {
    listbox.add(bottle);
  }
  listbox.show_all();
}

/**
 * \brief set the detailed info panel on the right
 * \param[in] bottle - Wine Bottle item object
 */
void MainWindow::SetDetailedInfo(BottleItem& bottle)
{
  // Set right side of the GUI
  name.set_text(bottle.name());
  Glib::ustring windows = BottleTypes::toString(bottle.windows());
  windows += " (" + BottleTypes::toString(bottle.bit()) + ')';
  window_version.set_text(windows);
  wine_version.set_text("v" + bottle.wine_version());
  wine_location.set_text(bottle.wine_location());
  c_drive_location.set_text(bottle.wine_c_drive());
  wine_last_changed.set_text(bottle.wine_last_changed());
  audio_driver.set_text(BottleTypes::toString(bottle.audio_driver()));
  virtual_desktop.set_text(bottle.virtual_desktop());
}

/**
 * \brief Just show an error message
 * \param[in] message - Show this error message
 */
void MainWindow::ShowErrorMessage(const Glib::ustring& message)
{
  // false = no markup
  Gtk::MessageDialog dialog(*this, message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
  dialog.set_title("An error has occurred!");
  dialog.set_modal(true);
  dialog.run();
}

/**
 * \brief Confirm dialog (Yes/No message)
 * \param[in] message - Show this message during confirmation
 * \return True if user pressed confirm (yes), otherwise False
 */
bool MainWindow::ShowConfirmDialog(const Glib::ustring& message)
{
  // false = no markup
  Gtk::MessageDialog dialog(*this, message, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
  dialog.set_title("Are you sure?");
  dialog.set_modal(true);
  int result = dialog.run();
  bool return_value = false;
  if(result == Gtk::RESPONSE_YES)
  {
    return_value = true;
  }
  return return_value;
}

/**
 * \brief Create left side of the GUI
 */
void MainWindow::CreateLeftPanel()
{
  // Vertical scroll only
  scrolled_window.set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_AUTOMATIC);

  // Add scrolled window with listbox to paned
  paned.pack1(scrolled_window, false, true);
  scrolled_window.set_size_request(240, -1);

  // Set function that will add seperators between each item
  listbox.set_header_func(sigc::ptr_fun(&MainWindow::cc_list_box_update_header_func));

  // Add list box to scrolled window
  scrolled_window.add(listbox);
}

/**
 * \brief Create right side of the GUI
 */
void MainWindow::CreateRightPanel()
{
  toolbar.set_toolbar_style(Gtk::ToolbarStyle::TOOLBAR_BOTH);

  // Buttons in toolbar
  Gtk::Image* new_image = Gtk::manage(new Gtk::Image());
  new_image->set_from_icon_name("list-add", Gtk::IconSize(Gtk::ICON_SIZE_LARGE_TOOLBAR));
  new_button.set_label("New");
  new_button.set_icon_widget(*new_image);
  toolbar.insert(new_button, 0);

  Gtk::Image* run_image = Gtk::manage(new Gtk::Image());
  run_image->set_from_icon_name("system-run", Gtk::IconSize(Gtk::ICON_SIZE_LARGE_TOOLBAR));
  run_button.set_label("Run Program...");
  run_button.set_icon_widget(*run_image);
  toolbar.insert(run_button, 1);

  Gtk::Image* settings_image = Gtk::manage(new Gtk::Image());
  settings_image->set_from_icon_name("preferences-other", Gtk::IconSize(Gtk::ICON_SIZE_LARGE_TOOLBAR));
  settings_button.set_label("Settings");
  settings_button.set_icon_widget(*settings_image);
  toolbar.insert(settings_button, 2);

  Gtk::Image* manage_image = Gtk::manage(new Gtk::Image());
  manage_image->set_from_icon_name("system-software-install", Gtk::IconSize(Gtk::ICON_SIZE_LARGE_TOOLBAR));
  manage_button.set_label("Manage");
  manage_button.set_icon_widget(*manage_image);
  toolbar.insert(manage_button, 3);

  Gtk::Image* reboot_image = Gtk::manage(new Gtk::Image());
  reboot_image->set_from_icon_name("view-refresh", Gtk::IconSize(Gtk::ICON_SIZE_LARGE_TOOLBAR));
  reboot_button.set_label("Reboot");
  reboot_button.set_icon_widget(*reboot_image);
  toolbar.insert(reboot_button, 4);

  // Add toolbar to right box
  right_box.add(toolbar);
  right_box.add(separator1);

  // Add detail section below toolbar
  detail_grid.set_margin_top(5);
  detail_grid.set_margin_end(5);
  detail_grid.set_margin_bottom(8);
  detail_grid.set_margin_start(8);
  detail_grid.set_column_spacing(8);
  detail_grid.set_row_spacing(12);

  // General heading
  Gtk::Image* general_icon = Gtk::manage(new Gtk::Image());
  general_icon->set_from_icon_name("computer", Gtk::IconSize(Gtk::ICON_SIZE_MENU));
  Gtk::Label* general_label = Gtk::manage(new Gtk::Label());
  general_label->set_markup("<b>General</b>");
  detail_grid.attach(*general_icon, 0, 0, 1, 1);
  detail_grid.attach_next_to(*general_label, *general_icon, Gtk::PositionType::POS_RIGHT, 1, 1);

  // Name
  Gtk::Label* name_label = Gtk::manage(new Gtk::Label("Name:", 0.0, -1));
  name.set_xalign(0.0);
  detail_grid.attach(*name_label, 0, 1, 2, 1);
  detail_grid.attach_next_to(name, *name_label, Gtk::PositionType::POS_RIGHT, 1, 1);

  // Windows version + bit os
  Gtk::Label* window_version_label = Gtk::manage(new Gtk::Label("Windows:", 0.0, -1));
  window_version.set_xalign(0.0);
  // Label consumes 2 columns
  detail_grid.attach(*window_version_label, 0, 2, 2, 1);
  detail_grid.attach_next_to(window_version, *window_version_label, Gtk::PositionType::POS_RIGHT, 1, 1);

  // Wine version
  Gtk::Label* wine_version_label = Gtk::manage(new Gtk::Label("Wine Version:", 0.0, -1));
  wine_version.set_xalign(0.0);
  detail_grid.attach(*wine_version_label, 0, 3, 2, 1);
  detail_grid.attach_next_to(wine_version, *wine_version_label, Gtk::PositionType::POS_RIGHT, 1, 1);

  // Wine location
  Gtk::Label* wine_location_label = Gtk::manage(new Gtk::Label("Wine Location:", 0.0, -1));
  wine_location.set_xalign(0.0);
  detail_grid.attach(*wine_location_label, 0, 4, 2, 1);
  detail_grid.attach_next_to(wine_location, *wine_location_label, Gtk::PositionType::POS_RIGHT, 1, 1);

  // Wine C drive location
  Gtk::Label* c_drive_location_label = Gtk::manage(new Gtk::Label("C:\\ Drive Location:", 0.0, -1));
  c_drive_location.set_xalign(0.0);
  detail_grid.attach(*c_drive_location_label, 0, 5, 2, 1);
  detail_grid.attach_next_to(c_drive_location, *c_drive_location_label, Gtk::PositionType::POS_RIGHT, 1, 1);

  // Wine last changed
  Gtk::Label* wine_last_changed_label = Gtk::manage(new Gtk::Label("Wine Last Changed:", 0.0, -1));
  wine_last_changed.set_xalign(0.0);
  detail_grid.attach(*wine_last_changed_label, 0, 6, 2, 1);
  detail_grid.attach_next_to(wine_last_changed, *wine_last_changed_label, Gtk::PositionType::POS_RIGHT, 1, 1);
  // End General
  detail_grid.attach(*new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL), 0, 7, 3, 1);

  // Audio heading
  Gtk::Image* audio_icon = Gtk::manage(new Gtk::Image());
  audio_icon->set_from_icon_name("audio-speakers", Gtk::IconSize(Gtk::ICON_SIZE_MENU));
  Gtk::Label* audio_label = Gtk::manage(new Gtk::Label());
  audio_label->set_markup("<b>Audio</b>");
  detail_grid.attach(*audio_icon, 0, 8, 1, 1);
  detail_grid.attach_next_to(*audio_label, *audio_icon, Gtk::PositionType::POS_RIGHT, 1, 1);

  // Audio driver
  Gtk::Label* audio_driver_label = Gtk::manage(new Gtk::Label("Audio Driver:", 0.0, -1));
  audio_driver.set_xalign(0.0);
  detail_grid.attach(*audio_driver_label, 0, 9, 2, 1);
  detail_grid.attach_next_to(audio_driver, *audio_driver_label, Gtk::PositionType::POS_RIGHT, 1, 1);
  // End Audio driver
  detail_grid.attach(*new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL), 0, 10, 3, 1);

  // Display heading
  Gtk::Image* display_icon = Gtk::manage(new Gtk::Image());
  display_icon->set_from_icon_name("video-display", Gtk::IconSize(Gtk::ICON_SIZE_MENU));
  Gtk::Label* display_label = Gtk::manage(new Gtk::Label());
  display_label->set_markup("<b>Display</b>");
  detail_grid.attach(*display_icon, 0, 11, 1, 1);
  detail_grid.attach_next_to(*display_label, *display_icon, Gtk::PositionType::POS_RIGHT, 1, 1);

  // Virtual Desktop
  Gtk::Label* virtual_desktop_label = Gtk::manage(new Gtk::Label("Virtual Desktop\n(Window Mode):", 0.0, -1));
  virtual_desktop.set_xalign(0.0);
  detail_grid.attach(*virtual_desktop_label, 0, 12, 2, 1);
  detail_grid.attach_next_to(virtual_desktop, *virtual_desktop_label, Gtk::PositionType::POS_RIGHT, 1, 1);
  // End Display
  detail_grid.attach(*new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL), 0, 13, 3, 1);

  // Add detail grid to box
  right_box.pack_start(detail_grid, false, false);

  // Add box to paned
  paned.add2(right_box);
}

/**
 * \brief Override update header function of GTK Listbox with custom layout
 * \param[in] row
 * \param[in] before
 */
void MainWindow::cc_list_box_update_header_func(Gtk::ListBoxRow* m_row, Gtk::ListBoxRow* before)
{
  GtkWidget *current;
  GtkListBoxRow *row = m_row->gobj();
  if (before == NULL) {
    gtk_list_box_row_set_header(row, NULL);
    return;
  }
  current = gtk_list_box_row_get_header(row);
  if (current == NULL){
    current = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_show(current);
    gtk_list_box_row_set_header(row, current);
  }
}

/**
 * \brief Signal when the new button is clicked in the top toolbar/menu
 */
void MainWindow::on_new_bottle_button_clicked()
{
  newBottleAssistant.set_transient_for(*this);
  newBottleAssistant.show();
}

/**
 * \brief Signal when the new assistant/wizard is finished and applied
 */
void MainWindow::on_new_bottle_apply()
{
  Glib::ustring name;
  Glib::ustring virtual_desktop_resolution;
  BottleTypes::Windows windows_version;
  BottleTypes::Bit bit;
  BottleTypes::AudioDriver audio;

  // Retrieve assistant results
  newBottleAssistant.GetResult(name, virtual_desktop_resolution, windows_version, bit, audio);

  // Emit signal to Bottle Manager
  newBottle.emit(name, virtual_desktop_resolution, windows_version, bit, audio);
}

/**
 * \brief Signal when the Run Program... button is clicked in top toolbar/menu
 */
void MainWindow::on_run_button_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a file",
    Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("_Open", Gtk::RESPONSE_OK);

  auto filter_win = Gtk::FileFilter::create();
  filter_win->set_name("Windows Executable/MSI Installer");
  filter_win->add_mime_type("application/x-ms-dos-executable");
  filter_win->add_mime_type("application/x-msi");
  dialog.add_filter(filter_win);

  auto filter_any = Gtk::FileFilter::create();
  filter_any->set_name("Any file");
  filter_any->add_pattern("*");
  dialog.add_filter(filter_any);

  //Show the dialog and wait for a user response:
  int result = dialog.run();
  
  //Handle the response:
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
      string filename = dialog.get_filename();
      // Just guess based on extenstion
      string ext = filename.substr(filename.find_last_of(".") + 1);
      // To lower case
      std::transform(ext.begin(), ext.end(), ext.begin(), 
        [](unsigned char c){ return std::tolower(c); }
      );
      if(ext == "exe")
      {
        runProgram.emit(filename, false);
      }
      else if(ext == "msi")
      {
        // Run as MSI (true=MSI)
        runProgram.emit(filename, true);
      }
      else 
      {
        // fall-back: try run as Exe
        runProgram.emit(filename, false);
      }
      break;
    }
    case(Gtk::RESPONSE_CANCEL):
    {
      // Cancelled, do nothing
      break;
    }
    default:
    {
      // Unexpected button, ignore
      break;
    }
  }
}

/**
 * \brief Handler when the bottle is created, notify the new bottle assistant.
 * Pass through the signal from the dispatcher to the 'new bottle assistant'.
 */
void MainWindow::on_new_bottle_created()
{
  newBottleAssistant.BottleCreated();
}

/**
 * \brief Not implemented feature
 */
void MainWindow::on_not_implemented()
{
  Gtk::MessageDialog dialog(*this, "This feature is not yet implemented. Sorry :\\", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
  dialog.set_title("An error has occurred!");
  dialog.set_modal(false);
  dialog.run();
}
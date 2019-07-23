/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/** @file
 * Configuration settings.
 * This file contains the global configuration settings (read from file or
 * from a command line parameter).
 */


#ifndef swc_core_config_Config_h
#define swc_core_config_Config_h


#include "Properties.h"


namespace SWC { namespace Config {

class Settings {

  private:
  /** A global (recursive) configuration mutex */
  std::recursive_mutex rec_mutex;

  PropertiesDesc *cmdline_descp = NULL;
  PropertiesDesc *cmdline_hidden_descp = NULL;
  PropertiesDesc *file_descp = NULL;

  void init_options();

  public:
  String install_path;
  String executable;
  /** This singleton map stores all options */
  PropertiesPtr properties;

  Settings() {
    properties = std::make_shared<Properties>();
  }
  virtual ~Settings(){}

  void init(int argc, char *argv[]);
  void init_app_options();

  void init_comm_options();
  
  void init_fs_options();

  /** Check existence of a configuration value
   *
   * @param name The name of the option to search for
   * @return true if there is an option with this name
   */
  bool has(const String &name);

  /** Check if a configuration value is defaulted
   *
   * @param name The name of the option
   * @return true if this option's value is the default value
   */
  bool defaulted(const String &name);

  /** Retrieves a configuration value
   *
   * This is a template function and usually not used directly. The file
   * Properties.h provides global functions like get_bool(), get_string()
   * that are usually used.
   *
   * @param name The name of the option
   * @return The option's value
   */
  template <typename T>
  T get(const String &name) {
    HT_ASSERT(properties);
    return properties->get<T>(name);
  }

  /** Retrieves a guarded value ptr
   *
   * This is a template function and usually not used directly. The file
   * Properties.h provides global functions like get_bool(), get_string()
   * that are usually used.
   *
   * @param name The name of the option
   * @return The option's value
   */
  template <typename T>
  T* get_ptr(const String &name) {
    HT_ASSERT(properties);
    return properties->get_ptr<T>(name);
  }

  /** Retrieves a configuration value (or a default value, if the value
   * was not set)
   *
   * @param name The name of the option
   * @param default_value The default value which is returned if the value was
   *        not set
   * @return The option's value
   */
  template <typename T>
  T get(const String &name, T default_value) {
    HT_ASSERT(properties);
    return properties->get<T>(name, default_value);
  }

  /**
   * Get the command line options description
   *
   * @param usage Optional usage string (first time)
   * @return Reference to the Description object
   */
  PropertiesDesc &cmdline_desc(const char *usage=nullptr);

  /** Set the command line options description
   *
   * @param desc Reference to the Description object
   */
  void cmdline_desc(PropertiesDesc &desc);

  /** Get the config file options description
   *
   * @param usage - optional usage string
   * @return Reference to the Description object
   */
  PropertiesDesc &file_desc(const char *usage = NULL);

  /** Set the config file options description
   *
   * @param desc Reference to the Description object
   */
  void file_desc(PropertiesDesc &desc);

  /**
   * Initialization helper; parses the argc/argv parameters into properties,
   * reads the configuration file, handles "help" and "help-config" parameters
   *
   * @param argc Number of elements in argv
   * @param argv Name of binary and command line arguments
   */
  void parse_args(int argc, char *argv[]);

  /**
   * Parses a configuration file and stores all configuration options into
   * the option descriptor
   *
   * @param fname The filename of the configuration file
   * @param desc Reference to the Description object
   * @throws Error::CONFIG_BAD_CFG_FILE on error
   */
  void parse_file(const String &fname, PropertiesDesc &desc);

  String reparse_file(const String &fname);

  /**
   * Setup command line option alias for config file option.
   * Typically used in policy init_options functions.
   * The command line option has higher priority.
   *
   * Requires use of sync_alias() afterwards.
   *
   * @param cmdline_opt Command line option name
   * @param file_opt Configuration file option name
   * @param overwrite If true then existing aliases are overwritten
   */
  void alias(const String &cmdline_opt, const String &file_opt);

  String usage_str(const char *usage) {
    if (!usage)
      usage = "Usage: %s [options]\n\nOptions";

    if (strstr(usage, "%s"))
      return format(usage, executable.c_str());

    return usage;
  }
};
  /** @}*/

static std::shared_ptr<Settings> settings = std::make_shared<Settings>();
}}

#endif // swc_core_config_Config_h

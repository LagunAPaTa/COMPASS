/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBTABLE_H_
#define DBTABLE_H_

#include "configurable.h"

class DBTableColumn;
class DBTableWidget;
class DBSchema;
class DBInterface;

/**
 * @brief Database table definition
 *
 * Has some parameters (name, name in database, key column name, description) and a collection of
 * DBTableColumn instances.
 */
class DBTable : public Configurable
{
  public:
    /// @brief Constructor
    DBTable(const std::string& class_id, const std::string& instance_id, DBSchema& schema,
            DBInterface& db_interface);
    /// @brief Destructor
    virtual ~DBTable();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    /// @brief Returns the identifier name
    const std::string& name() const { return name_; }

    /// @brief Sets the description
    void info(const std::string& info) { info_ = info; }
    /// @brief Returns the description
    const std::string& info() const { return info_; }

    bool hasColumn(const std::string& name) const;
    /// @brief Returns the DBTableColumn identified by the given name
    const DBTableColumn& column(const std::string& name) const;
    /// @brief Returns number of defined database columns
    unsigned int numColumns() const { return columns_.size(); }
    /// @brief Deletes a given column
    void deleteColumn(const std::string& name);

    /// @brief Returns container with all table columns
    const std::map<std::string, DBTableColumn*>& columns() const { return columns_; }

    /// @brief Returns if the name of the key column is defined
    bool hasKey() const { return key_name_.size() > 0; }
    /// @brief Sets the name of the key column
    void key(const std::string& key) { key_name_ = key; }
    /// @brief Returns name of the key column
    const std::string& key() const { return key_name_; }

    void populate();
    void update();

    void lock();
    bool isLocked() { return locked_; }

    DBTableWidget* widget();

    void updateOnDatabase();  // check what informations is present in the current db

    bool existsInDB() const { return exists_in_db_; }

  private:
    DBSchema& schema_;
    DBInterface& db_interface_;

    bool locked_{false};

    DBTableWidget* widget_{nullptr};

    /// Table name identifier
    std::string name_;
    /// Description
    std::string info_;
    /// Name of the key column
    std::string key_name_;
    /// Container with all table columns (column name -> DBTableColumn)
    std::map<std::string, DBTableColumn*> columns_;

    bool exists_in_db_{false};

  protected:
    virtual void checkSubConfigurables();
};

#endif /* DBTABLE_H_ */

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

#include "dbtable.h"

#include "atsdb.h"
#include "dbinterface.h"
#include "dbschema.h"
#include "dbtablecolumn.h"
#include "dbtableinfo.h"
#include "dbtablewidget.h"
#include "logger.h"

DBTable::DBTable(const std::string& class_id, const std::string& instance_id, DBSchema& schema,
                 DBInterface& db_interface)
    : Configurable(class_id, instance_id, &schema), schema_(schema), db_interface_(db_interface)
{
    registerParameter("name", &name_, "");
    registerParameter("info", &info_, "");
    registerParameter("key_name", &key_name_, "");

    createSubConfigurables();
}

DBTable::~DBTable()
{
    logdbg << "DBTable: ~DBTable: name " << name_;

    for (auto it : columns_)
        delete it.second;
    columns_.clear();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

void DBTable::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "DBTable: generateSubConfigurable: " << classId() << " instance " << instanceId();

    if (class_id == "DBTableColumn")
    {
        DBTableColumn* column =
            new DBTableColumn("DBTableColumn", instance_id, this, db_interface_);
        assert(column->name().size() != 0);
        assert(columns_.find(column->name()) == columns_.end());
        columns_.insert(std::pair<std::string, DBTableColumn*>(column->name(), column));

        if (column->isKey())
            key_name_ = column->name();
    }
    else
        throw std::runtime_error("DBTable: generateSubConfigurable: unknown class_id " + class_id);
}
void DBTable::checkSubConfigurables()
{
    // move along, sir.
}

bool DBTable::hasColumn(const std::string& name) const
{
    return columns_.find(name) != columns_.end();
}

const DBTableColumn& DBTable::column(const std::string& name) const
{
    assert(columns_.find(name) != columns_.end());
    return *columns_.at(name);
}

void DBTable::deleteColumn(const std::string& name)
{
    assert(hasColumn(name));
    DBTableColumn* col = columns_.at(name);
    columns_.erase(columns_.find(name));
    delete col;
}

void DBTable::populate()
{
    loginf << "DBTable: populate: table " << name_;

    assert(ATSDB::instance().ready());
    for (auto it : ATSDB::instance().interface().tableInfo().at(name_).columns())
    {
        if (columns_.count(it.first) == 0)
        {
            loginf << "DBTable: populate: table " << name_ << ": adding new column " << it.first;

            Configuration& config = addNewSubConfiguration("DBTableColumn", it.first);
            config.addParameterString("name", it.first);
            config.addParameterString("type", it.second.type());
            config.addParameterBool("is_key", it.second.key());
            config.addParameterString("comment", it.second.comment());
            generateSubConfigurable("DBTableColumn", it.first);
        }
    }
}

void DBTable::update()
{
    assert(ATSDB::instance().ready());

    loginf << "DBTable: update: table " << name_;

    const std::map<std::string, DBTableColumnInfo> db_table_columns =
        ATSDB::instance().interface().tableInfo().at(name_).columns();

    std::vector<std::string> cols_to_be_removed;

    for (auto& col_it : columns_)  // check if defined columns still exist in DB
    {
        if (db_table_columns.count(col_it.first) != 1)
        {
            loginf << "DBTable: update: table " << name_ << ": column '" << col_it.first
                   << "' not defined in database";
            cols_to_be_removed.push_back(col_it.first);
        }
    }

    for (auto& col_name_it : cols_to_be_removed)
    {
        loginf << "DBTable: update: table " << name_ << ": deleting column '" << col_name_it << "'";
        deleteColumn(col_name_it);
    }

    for (auto col_it : db_table_columns)
    {
        if (columns_.count(col_it.first) == 0)
        {
            loginf << "DBTable: update: table " << name_ << ": adding new column " << col_it.first;

            Configuration& config = addNewSubConfiguration("DBTableColumn", col_it.first);
            config.addParameterString("name", col_it.first);
            config.addParameterString("type", col_it.second.type());
            config.addParameterBool("is_key", col_it.second.key());
            config.addParameterString("comment", col_it.second.comment());
            generateSubConfigurable("DBTableColumn", col_it.first);
        }
    }

    loginf << "DBTable: update: table " << name_ << " done";
}

void DBTable::lock()
{
    locked_ = true;

    if (widget_)
        widget_->lock();
}

DBTableWidget* DBTable::widget()
{
    if (!widget_)
    {
        widget_ = new DBTableWidget(*this);

        if (locked_)
            widget_->lock();
    }

    return widget_;
}

void DBTable::updateOnDatabase()
{
    exists_in_db_ = false;

    logdbg << "DBTable: updateOnDatabase: " << name_;

    for (auto col_it : columns_)
    {
        col_it.second->updateOnDatabase();

        exists_in_db_ |= col_it.second->existsInDB();  // exists if any exist
        logdbg << "DBTable " << name_ << "::updateOnDatabase: exists " << exists_in_db_ << " col "
               << col_it.first << " exists " << col_it.second->existsInDB();
    }

    // exists_in_db_ = exists;

    loginf << "DBTable: updateOnDatabase: " << name_ << " exists in db " << exists_in_db_;
}

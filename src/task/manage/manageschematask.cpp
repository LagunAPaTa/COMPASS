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

#include "manageschematask.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "manageschemataskwidget.h"
#include "taskmanager.h"

ManageSchemaTask::ManageSchemaTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Task("ManageSchemaTask", "Manage DB Schema", true, true, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_manage_schema.json")
{
    tooltip_ =
        "Allows management of the database schema configuration and is reserved for expert users. "
        "This task can not be run, but is performed using the GUI elements.";
}

TaskWidget* ManageSchemaTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ManageSchemaTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &ManageSchemaTaskWidget::expertModeChangedSlot);
    }

    return widget_.get();
}

void ManageSchemaTask::deleteWidget() { widget_.reset(nullptr); }

void ManageSchemaTask::generateSubConfigurable(const std::string& class_id,
                                               const std::string& instance_id)
{
    throw std::runtime_error("ManageSchemaTask: generateSubConfigurable: unknown class_id " +
                             class_id);
}

bool ManageSchemaTask::checkPrerequisites() { return ATSDB::instance().interface().ready(); }

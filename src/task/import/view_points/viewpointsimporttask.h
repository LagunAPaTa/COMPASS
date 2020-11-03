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

#ifndef VIEWPOINTSIMPORTTASK_H
#define VIEWPOINTSIMPORTTASK_H

#include "configurable.h"
#include "task.h"

#include <QObject>
#include <memory>

#include "json.hpp"

class ViewPointsImportTaskWidget;
class TaskManager;
class SavedFile;

class ViewPointsImportTask : public Task, public Configurable
{
    Q_OBJECT

public slots:

public:
    ViewPointsImportTask(const std::string& class_id, const std::string& instance_id,
                         TaskManager& task_manager);
    virtual ~ViewPointsImportTask();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual TaskWidget* widget();
    virtual void deleteWidget();

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired();

    const std::map<std::string, SavedFile*>& fileList() { return file_list_; }
    bool hasFile(const std::string& filename) { return file_list_.count(filename) > 0; }
    void addFile(const std::string& filename);
    void removeCurrentFilename();
    void removeAllFiles ();
    std::string currentFilename() const;
    void currentFilename(const std::string& value);

    std::string currentError() const;

    bool canImport ();
    void import ();

    const nlohmann::json& currentData() const;

    bool finished() const;

protected:
    std::string current_filename_;
    nlohmann::json current_data_;

    std::map<std::string, SavedFile*> file_list_;

    std::string current_error_;
    bool finished_ {false};

    std::unique_ptr<ViewPointsImportTaskWidget> widget_;

    virtual void checkSubConfigurables() {}

    void parseCurrentFile ();
    void checkParsedData (); // throws exceptions for errors
};

#endif // VIEWPOINTSIMPORTTASK_H

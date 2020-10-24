#include "evaluationmanager.h"
#include "evaluationmanagerwidget.h"
#include "evaluationdatawidget.h"
#include "evaluationstandard.h"
#include "evaluationstandardwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/config.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbobjectmanagerloadwidget.h"
#include "dbobjectinfowidget.h"
#include "sector.h"
#include "metadbovariable.h"
#include "dbovariable.h"
#include "buffer.h"
#include "filtermanager.h"
#include "datasourcesfilter.h"
#include "datasourcesfilterwidget.h"
#include "viewabledataconfig.h"
#include "viewmanager.h"
#include "stringconv.h"
#include "dbovariableorderedset.h"

#include "json.hpp"

#include <QTabWidget>

#include "boost/date_time/posix_time/posix_time.hpp"

#include <memory>
#include <fstream>

using namespace Utils;
using namespace std;
using namespace nlohmann;

EvaluationManager::EvaluationManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb)
    : Configurable(class_id, instance_id, atsdb, "eval.json"), atsdb_(*atsdb), data_(*this), results_gen_(*this)
{
    registerParameter("dbo_name_ref", &dbo_name_ref_, "RefTraj");
    registerParameter("active_sources_ref", &active_sources_ref_, json::object());

    registerParameter("dbo_name_tst", &dbo_name_tst_, "Tracker");
    registerParameter("active_sources_tst", &active_sources_tst_, json::object());

    registerParameter("current_standard", &current_standard_, "");

    registerParameter("use_grp_in_sector", &use_grp_in_sector_, json::object());
    registerParameter("use_requirement", &use_requirement_, json::object());

    createSubConfigurables();
}

void EvaluationManager::init(QTabWidget* tab_widget)
{
    loginf << "EvaluationManager: init";

    assert (!initialized_);
    assert (tab_widget);

    updateReferenceDBO();
    updateTestDBO();

    initialized_ = true;

    tab_widget->addTab(widget(), "Evaluation");

    if (!ATSDB::instance().objectManager().hasAssociations())
        widget()->setDisabled(true);
}

bool EvaluationManager::canLoadData ()
{
    assert (initialized_);
    return ATSDB::instance().objectManager().hasAssociations() && hasCurrentStandard();
}

void EvaluationManager::loadData ()
{
    loginf << "EvaluationManager: loadData";

    assert (initialized_);

    reference_data_loaded_ = false;
    test_data_loaded_ = false;
    data_loaded_ = false;

    evaluated_ = false;

    DBObjectManager& object_man = ATSDB::instance().objectManager();

    // set use filters
    object_man.useFilters(true);
    object_man.loadWidget()->updateUseFilters();

    // clear data
    data_.clear();

    // set if load for dbos
    for (auto& obj_it : object_man)
    {
        obj_it.second->infoWidget()->setLoad(obj_it.first == dbo_name_ref_
                                             || obj_it.first == dbo_name_tst_);
    }

    // set ref data sources filters
    FilterManager& fil_man = ATSDB::instance().filterManager();

    fil_man.disableAllFilters();

    if (dbo_name_ref_ != dbo_name_tst_)
    {
        {
            DataSourcesFilter* ref_filter = fil_man.getDataSourcesFilter(dbo_name_ref_);
            ref_filter->setActive(true);

            for (auto& fil_ds_it : ref_filter->dataSources())
            {
                assert (data_sources_ref_.count(fil_ds_it.first));
                fil_ds_it.second.setActive(data_sources_ref_.at(fil_ds_it.first).isActive());
            }

            ref_filter->widget()->update();
        }

        {
            DataSourcesFilter* tst_filter = fil_man.getDataSourcesFilter(dbo_name_tst_);
            tst_filter->setActive(true);

            for (auto& fil_ds_it : tst_filter->dataSources())
            {
                assert (data_sources_tst_.count(fil_ds_it.first));
                fil_ds_it.second.setActive(data_sources_tst_.at(fil_ds_it.first).isActive());
            }

            tst_filter->widget()->update();
        }
    }
    else // same ref / tst dbo
    {
        DataSourcesFilter* filter = fil_man.getDataSourcesFilter(dbo_name_ref_);
        filter->setActive(true);

        for (auto& fil_ds_it : filter->dataSources())
        {
            assert (data_sources_tst_.count(fil_ds_it.first));
            fil_ds_it.second.setActive(data_sources_ref_.at(fil_ds_it.first).isActive()
                                       || data_sources_tst_.at(fil_ds_it.first).isActive());
        }

        filter->widget()->update();
    }


    // reference data
    {
        assert (object_man.existsObject(dbo_name_ref_));
        DBObject& dbo_ref = object_man.object(dbo_name_ref_);

        connect(&dbo_ref, &DBObject::newDataSignal, this, &EvaluationManager::newDataSlot);
        connect(&dbo_ref, &DBObject::loadingDoneSignal, this, &EvaluationManager::loadingDoneSlot);
    }

    // test data

    if (dbo_name_ref_ != dbo_name_tst_) // otherwise already connected
    {
        assert (object_man.existsObject(dbo_name_tst_));
        DBObject& dbo_tst = object_man.object(dbo_name_tst_);

        connect(&dbo_tst, &DBObject::newDataSignal, this, &EvaluationManager::newDataSlot);
        connect(&dbo_tst, &DBObject::loadingDoneSignal, this, &EvaluationManager::loadingDoneSlot);

    }

    needs_additional_variables_ = true;

    object_man.loadSlot();

    needs_additional_variables_ = false;

    if (widget_)
        widget_->updateButtons();
}

bool EvaluationManager::canEvaluate ()
{
    assert (initialized_);
    return data_loaded_ && hasCurrentStandard();
}

void EvaluationManager::newDataSlot(DBObject& object)
{
    loginf << "EvaluationManager: newDataSlot: obj " << object.name() << " buffer size " << object.data()->size();
}
void EvaluationManager::loadingDoneSlot(DBObject& object)
{
    loginf << "EvaluationManager: loadingDoneSlot: obj " << object.name() << " buffer size " << object.data()->size();

    DBObjectManager& object_man = ATSDB::instance().objectManager();

    if (object.name() == dbo_name_ref_)
    {
        DBObject& dbo_ref = object_man.object(dbo_name_ref_);

        disconnect(&dbo_ref, &DBObject::newDataSignal, this, &EvaluationManager::newDataSlot);
        disconnect(&dbo_ref, &DBObject::loadingDoneSignal, this, &EvaluationManager::loadingDoneSlot);

        data_.addReferenceData(dbo_ref, object.data());

        reference_data_loaded_ = true;
    }

    if (object.name() == dbo_name_tst_)
    {
        DBObject& dbo_tst = object_man.object(dbo_name_tst_);

        if (dbo_name_ref_ != dbo_name_tst_) // otherwise already disconnected
        {
            disconnect(&dbo_tst, &DBObject::newDataSignal, this, &EvaluationManager::newDataSlot);
            disconnect(&dbo_tst, &DBObject::loadingDoneSignal, this, &EvaluationManager::loadingDoneSlot);
        }

        data_.addTestData(dbo_tst, object.data());

        test_data_loaded_ = true;
    }

    bool data_loaded_tmp = reference_data_loaded_ && test_data_loaded_;

    loginf << "EvaluationManager: loadingDoneSlot: data loaded " << data_loaded_;

    if (data_loaded_tmp)
    {
        loginf << "EvaluationManager: loadingDoneSlot: finalizing";

        boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

        data_.finalize();

        boost::posix_time::time_duration time_diff =  boost::posix_time::microsec_clock::local_time() - start_time;

        loginf << "EvaluationManager: loadingDoneSlot: finalize done "
               << String::timeStringFromDouble(time_diff.total_milliseconds() / 1000.0, true);
    }

    data_loaded_ = data_loaded_tmp;

    if (widget_)
        widget_->updateButtons();
}

void EvaluationManager::evaluate ()
{
    loginf << "EvaluationManager: evaluate";

    assert (initialized_);
    assert (data_loaded_);
    assert (hasCurrentStandard());

    results_gen_.evaluate(data_, currentStandard());

    evaluated_ = true;

    if (widget_)
        widget_->updateButtons();
}

bool EvaluationManager::canGenerateReport ()
{
    assert (initialized_);
    return evaluated_;
}

void EvaluationManager::generateReport ()
{
    loginf << "EvaluationManager: generateReport";

    assert (initialized_);
    assert (data_loaded_);
    assert (evaluated_);

    assert (pdf_gen_);
    pdf_gen_->dialog().exec();

    if (widget_)
        widget_->updateButtons();
}

void EvaluationManager::close()
{
    initialized_ = false;
}

bool EvaluationManager::needsAdditionalVariables ()
{
    return needs_additional_variables_;
}

void EvaluationManager::addVariables (const std::string dbo_name, DBOVariableSet& read_set)
{
    loginf << "EvaluationManager: addVariables: dbo_name " << dbo_name;

    DBObjectManager& object_man = ATSDB::instance().objectManager();

    read_set.add(object_man.metaVariable("rec_num").getFor(dbo_name));
    read_set.add(object_man.metaVariable("ds_id").getFor(dbo_name));
    read_set.add(object_man.metaVariable("tod").getFor(dbo_name));
    read_set.add(object_man.metaVariable("pos_lat_deg").getFor(dbo_name));
    read_set.add(object_man.metaVariable("pos_long_deg").getFor(dbo_name));
    read_set.add(object_man.metaVariable("target_addr").getFor(dbo_name));
    read_set.add(object_man.metaVariable("modec_code_ft").getFor(dbo_name));
    read_set.add(object_man.metaVariable("mode3a_code").getFor(dbo_name));

    // TODO add required variables from standard requirements

    //        read_set.add(object_man.metaVariable("groundspeed_kt").getFor(dbo_name_ref_));
    //        read_set.add(object_man.metaVariable("heading_deg").getFor(dbo_name_ref_));
}

EvaluationManager::~EvaluationManager()
{
    sector_layers_.clear();
}

void EvaluationManager::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "EvaluationStandard")
    {
        EvaluationStandard* standard = new EvaluationStandard(class_id, instance_id, *this);
        logdbg << "EvaluationManager: generateSubConfigurable: adding standard " << standard->name();

        assert(standards_.find(standard->name()) == standards_.end());

        standards_[standard->name()].reset(standard);
    }
    else if (class_id == "EvaluationResultsReportPDFGenerator")
    {
        assert (!pdf_gen_);
        pdf_gen_.reset(new EvaluationResultsReport::PDFGenerator(class_id, instance_id, *this));
        assert (pdf_gen_);
    }
    else
        throw std::runtime_error("EvaluationManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

EvaluationManagerWidget* EvaluationManager::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationManagerWidget(*this));

    assert(widget_);
    return widget_.get();
}

void EvaluationManager::checkSubConfigurables()
{
    if (!pdf_gen_)
        generateSubConfigurable("EvaluationResultsReportPDFGenerator", "EvaluationResultsReportPDFGenerator0");

    assert (pdf_gen_);
}

bool EvaluationManager::hasSectorLayer (const std::string& layer_name)
{
    assert (sectors_loaded_);

    auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                             [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

    return iter != sector_layers_.end();
}

//void EvaluationManager::renameSectorLayer (const std::string& name, const std::string& new_name)
//{
//    // TODO
//}

std::shared_ptr<SectorLayer> EvaluationManager::sectorLayer (const std::string& layer_name)
{
    assert (sectors_loaded_);
    assert (hasSectorLayer(layer_name));

    auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                             [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});
    assert (iter != sector_layers_.end());

    return *iter;
}

void EvaluationManager::loadSectors()
{
    loginf << "EvaluationManager: loadSectors";

    assert (!sectors_loaded_);

    sector_layers_ = ATSDB::instance().interface().loadSectors();

    sectors_loaded_ = true;
}

unsigned int EvaluationManager::getMaxSectorId ()
{
    assert (sectors_loaded_);

    unsigned int id = 0;
    for (auto& sec_lay_it : sector_layers_)
        for (auto& sec_it : sec_lay_it->sectors())
            if (sec_it->id() > id)
                id = sec_it->id();

    return id;
}

void EvaluationManager::createNewSector (const std::string& name, const std::string& layer_name,
                                         std::vector<std::pair<double,double>> points)
{
    loginf << "EvaluationManager: createNewSector: name " << name << " layer_name " << layer_name
           << " num points " << points.size();

    assert (sectors_loaded_);
    assert (!hasSector(name, layer_name));

    unsigned int id = getMaxSectorId()+1;

    shared_ptr<Sector> sector = make_shared<Sector> (id, name, layer_name, points);

    // add to existing sectors
    if (!hasSectorLayer(layer_name))
        sector_layers_.push_back(make_shared<SectorLayer>(layer_name));

    assert (hasSectorLayer(layer_name));

    sectorLayer(layer_name)->addSector(sector);

    assert (hasSector(name, layer_name));
    sector->save();
}

bool EvaluationManager::hasSector (const string& name, const string& layer_name)
{
    assert (sectors_loaded_);

    if (!hasSectorLayer(layer_name))
        return false;

    return sectorLayer(layer_name)->hasSector(name);
}

bool EvaluationManager::hasSector (unsigned int id)
{
    assert (sectors_loaded_);

    for (auto& sec_lay_it : sector_layers_)
    {
        auto& sectors = sec_lay_it->sectors();
        auto iter = std::find_if(sectors.begin(), sectors.end(),
                                 [id](const shared_ptr<Sector>& x) { return x->id() == id;});
        if (iter != sectors.end())
            return true;
    }
    return false;
}

std::shared_ptr<Sector> EvaluationManager::sector (const string& name, const string& layer_name)
{
    assert (sectors_loaded_);
    assert (hasSector(name, layer_name));

    return sectorLayer(layer_name)->sector(name);
}

std::shared_ptr<Sector> EvaluationManager::sector (unsigned int id)
{
    assert (sectors_loaded_);
    assert (hasSector(id));

    for (auto& sec_lay_it : sector_layers_)
    {
        auto& sectors = sec_lay_it->sectors();
        auto iter = std::find_if(sectors.begin(), sectors.end(),
                                 [id](const shared_ptr<Sector>& x) { return x->id() == id;});
        if (iter != sectors.end())
            return *iter;
    }

    logerr << "EvaluationManager: sector: id " << id << " not found";
    assert (false);
}

void EvaluationManager::moveSector(unsigned int id, const std::string& old_layer_name, const std::string& new_layer_name)
{
    assert (sectors_loaded_);
    assert (hasSector(id));

    shared_ptr<Sector> tmp_sector = sector(id);

    assert (hasSectorLayer(old_layer_name));
    sectorLayer(old_layer_name)->removeSector(tmp_sector);

    if (!hasSectorLayer(new_layer_name))
        sector_layers_.push_back(make_shared<SectorLayer>(new_layer_name));

    assert (hasSectorLayer(new_layer_name));
    sectorLayer(new_layer_name)->addSector(tmp_sector);

    assert (hasSector(tmp_sector->name(), new_layer_name));
    tmp_sector->save();
}

std::vector<std::shared_ptr<SectorLayer>>& EvaluationManager::sectorsLayers()
{
    assert (sectors_loaded_);

    return sector_layers_;
}

void EvaluationManager::saveSector(unsigned int id)
{
    assert (sectors_loaded_);
    assert (hasSector(id));

    saveSector(sector(id));
}

void EvaluationManager::saveSector(std::shared_ptr<Sector> sector)
{
    assert (sectors_loaded_);
    assert (hasSector(sector->name(), sector->layerName()));
    ATSDB::instance().interface().saveSector(sector);

    emit sectorsChangedSignal();
}

void EvaluationManager::deleteSector(shared_ptr<Sector> sector)
{
    assert (sectors_loaded_);
    assert (hasSector(sector->name(), sector->layerName()));

    string layer_name = sector->layerName();

    assert (hasSectorLayer(layer_name));

    sectorLayer(layer_name)->removeSector(sector);

    // remove sector layer if empty
    if (!sectorLayer(layer_name)->size())
    {
        auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                                 [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

        assert (iter != sector_layers_.end());
        sector_layers_.erase(iter);
    }

    ATSDB::instance().interface().deleteSector(sector);

    emit sectorsChangedSignal();
}

void EvaluationManager::deleteAllSectors()
{
    assert (sectors_loaded_);
    sector_layers_.clear();

    ATSDB::instance().interface().deleteAllSectors();

    emit sectorsChangedSignal();
}


void EvaluationManager::importSectors (const std::string& filename)
{
    loginf << "EvaluationManager: importSectors: filename '" << filename << "'";

    assert (sectors_loaded_);

    sector_layers_.clear();
    ATSDB::instance().interface().clearSectorsTable();

    std::ifstream input_file(filename, std::ifstream::in);

    try
    {
        json j = json::parse(input_file);

        if (!j.contains("sectors"))
        {
            logerr << "EvaluationManager: importSectors: file does not contain sectors";
            return;
        }

        json& sectors = j["sectors"];

        if (!sectors.is_array())
        {
            logerr << "EvaluationManager: importSectors: file sectors is not an array";
            return;
        }

        unsigned int id;
        string name;
        string layer_name;
        string json_str;

        for (auto& j_sec_it : sectors.get<json::array_t>())
        {
            if (!j_sec_it.contains("id")
                    || !j_sec_it.contains("name")
                    || !j_sec_it.contains("layer_name")
                    || !j_sec_it.contains("points"))
            {
                logerr << "EvaluationManager: importSectors: ill-defined sectors skipped, json '" << j_sec_it.dump(4)
                       << "'";
                continue;
            }

            id = j_sec_it.at("id");
            name = j_sec_it.at("name");
            layer_name = j_sec_it.at("layer_name");

            shared_ptr<Sector> new_sector = make_shared<Sector>(id, name, layer_name, j_sec_it.dump());

            if (!hasSectorLayer(layer_name))
                sector_layers_.push_back(make_shared<SectorLayer>(layer_name));

            sectorLayer(layer_name)->addSector(new_sector);

            assert (hasSector(name, layer_name));

            new_sector->save();

            loginf << "EvaluationManager: importSectors: loaded sector '" << name << "' in layer '"
                   << layer_name << "' num points " << sector(name, layer_name)->size();
        }
    }
    catch (json::exception& e)
    {
        logerr << "EvaluationManager: importSectors: could not load file '"
               << filename << "'";
        throw e;
    }

    emit sectorsChangedSignal();
}

void EvaluationManager::exportSectors (const std::string& filename)
{
    loginf << "EvaluationManager: exportSectors: filename '" << filename << "'";

    assert (sectors_loaded_);

    json j;

    j["sectors"] = json::array();
    json& sectors = j["sectors"];

    unsigned int cnt = 0;

    for (auto& sec_lay_it : sector_layers_)
    {
        for (auto& sec_it : sec_lay_it->sectors())
        {
            sectors[cnt] = sec_it->jsonData();
            ++cnt;
        }
    }

    std::ofstream output_file;
    output_file.open(filename, std::ios_base::out);

    output_file << j.dump(4);

}

std::string EvaluationManager::dboNameRef() const
{
    return dbo_name_ref_;
}

void EvaluationManager::dboNameRef(const std::string& name)
{
    loginf << "EvaluationManager: dboNameRef: name " << name;

    dbo_name_ref_ = name;

    updateReferenceDBO();
}

bool EvaluationManager::hasValidReferenceDBO ()
{
    if (!dbo_name_ref_.size())
        return false;

    if (!ATSDB::instance().objectManager().existsObject(dbo_name_ref_))
        return false;

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_ref_);

    if (!object.hasCurrentDataSourceDefinition())
        return false;

    return object.hasDataSources();
}


std::set<int> EvaluationManager::activeDataSourcesRef()
{
    set<int> active_sources;

    for (auto& ds_it : data_sources_ref_)
        if (ds_it.second.isActive())
            active_sources.insert(ds_it.first);

    return active_sources;
}

std::string EvaluationManager::dboNameTst() const
{
    return dbo_name_tst_;
}

void EvaluationManager::dboNameTst(const std::string& name)
{
    loginf << "EvaluationManager: dboNameTst: name " << name;

    dbo_name_tst_ = name;

    updateTestDBO();
}

bool EvaluationManager::hasValidTestDBO ()
{
    if (!dbo_name_tst_.size())
        return false;

    if (!ATSDB::instance().objectManager().existsObject(dbo_name_tst_))
        return false;

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_tst_);

    if (!object.hasCurrentDataSourceDefinition())
        return false;

    return object.hasDataSources();
}

std::set<int> EvaluationManager::activeDataSourcesTst()
{
    set<int> active_sources;

    for (auto& ds_it : data_sources_tst_)
        if (ds_it.second.isActive())
            active_sources.insert(ds_it.first);

    return active_sources;
}


bool EvaluationManager::dataLoaded() const
{
    return data_loaded_;
}

bool EvaluationManager::evaluated() const
{
    return evaluated_;
}

EvaluationData& EvaluationManager::getData()
{
    return data_;
}

bool EvaluationManager::hasCurrentStandard()
{
    return current_standard_.size() && hasStandard(current_standard_);
}

std::string EvaluationManager::currentStandardName() const
{
    return current_standard_;
}

void EvaluationManager::currentStandardName(const std::string& current_standard)
{
    current_standard_ = current_standard;

    if (current_standard_.size())
        assert (hasStandard(current_standard_));

    emit currentStandardChangedSignal();

    if (widget_)
        widget_->updateButtons();
}

EvaluationStandard& EvaluationManager::currentStandard()
{
    assert (hasCurrentStandard());

    return *standards_.at(current_standard_).get();
}

bool EvaluationManager::hasStandard(const std::string& name)
{
    return standards_.count(name);
}

void EvaluationManager::addStandard(const std::string& name)
{
    loginf << "EvaluationManager: addStandard: name " << name;

    assert (!hasStandard(name));

    std::string instance = "EvaluationStandard" + name + "0";

    Configuration& config = addNewSubConfiguration("EvaluationStandard", instance);
    config.addParameterString("name", name);

    generateSubConfigurable("EvaluationStandard", instance);

    emit standardsChangedSignal();

    currentStandardName(name);
}

void EvaluationManager::deleteCurrentStandard()
{
    loginf << "EvaluationManager: deleteCurrentStandard: name " << current_standard_;

    assert (hasCurrentStandard());

    standards_.erase(current_standard_);

    emit standardsChangedSignal();

    currentStandardName("");
}

std::vector<std::string> EvaluationManager::currentRequirementNames()
{
    std::vector<std::string> names;

    if (hasCurrentStandard())
    {
        for (auto& req_grp_it : currentStandard())
        {
            for (auto& req_it : *req_grp_it.second)
            {
                if (find(names.begin(), names.end(), req_it->name()) == names.end())
                    names.push_back(req_it->name());
            }
        }
    }

    return names;
}

EvaluationResultsGenerator& EvaluationManager::resultsGenerator()
{
    return results_gen_;
}

bool EvaluationManager::sectorsLoaded() const
{
    return sectors_loaded_;
}

void EvaluationManager::updateReferenceDBO()
{
    loginf << "EvaluationManager: updateReferenceDBO";
    
    data_sources_ref_.clear();
    active_sources_ref_.clear();
    
    if (!hasValidReferenceDBO())
        return;

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_ref_);

    if (object.hasDataSources())
        updateReferenceDataSources();

    if (object.hasActiveDataSourcesInfo())
        updateReferenceDataSourcesActive();
}

void EvaluationManager::updateReferenceDataSources()
{
    loginf << "EvaluationManager: updateReferenceDataSources";

    assert (hasValidReferenceDBO());

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_ref_);

    for (auto ds_it = object.dsBegin(); ds_it != object.dsEnd(); ++ds_it)
    {
        if (data_sources_ref_.find(ds_it->first) == data_sources_ref_.end())
        {
            if (!active_sources_ref_.contains(to_string(ds_it->first)))
                active_sources_ref_[to_string(ds_it->first)] = true; // init with default true

            // needed for old compiler
            json::boolean_t& active = active_sources_ref_[to_string(ds_it->first)].get_ref<json::boolean_t&>();

            data_sources_ref_.emplace(std::piecewise_construct,
                                      std::forward_as_tuple(ds_it->first),  // args for key
                                      std::forward_as_tuple(ds_it->first, ds_it->second.name(),
                                                            active));
        }
    }
}

void EvaluationManager::updateReferenceDataSourcesActive()
{
    loginf << "EvaluationManager: updateReferenceDataSourcesActive";

    assert (hasValidReferenceDBO());

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_ref_);

    assert (object.hasActiveDataSourcesInfo());

    for (auto& srcit : data_sources_ref_)
        srcit.second.setActiveInData(false);

    for (auto& it : object.getActiveDataSources())
    {
        assert(data_sources_ref_.find(it) != data_sources_ref_.end());
        ActiveDataSource& src = data_sources_ref_.at(it);
        src.setActiveInData(true);
    }

    for (auto& srcit : data_sources_ref_)
    {
        if (!srcit.second.isActiveInData())
            srcit.second.setActive(false);
    }
}

void EvaluationManager::updateTestDBO()
{
    loginf << "EvaluationManager: updateTestDBO";

    data_sources_tst_.clear();
    active_sources_tst_.clear();

    if (!hasValidTestDBO())
        return;

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_tst_);

    if (object.hasDataSources())
        updateTestDataSources();

    if (object.hasActiveDataSourcesInfo())
        updateTestDataSourcesActive();
}

void EvaluationManager::updateTestDataSources()
{
    loginf << "EvaluationManager: updateTestDataSources";

    assert (hasValidTestDBO());

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_tst_);

    for (auto ds_it = object.dsBegin(); ds_it != object.dsEnd(); ++ds_it)
    {
        if (data_sources_tst_.find(ds_it->first) == data_sources_tst_.end())
        {
            if (!active_sources_tst_.contains(to_string(ds_it->first)))
                active_sources_tst_[to_string(ds_it->first)] = true; // init with default true

            // needed for old compiler
            json::boolean_t& active = active_sources_tst_[to_string(ds_it->first)].get_ref<json::boolean_t&>();

            data_sources_tst_.emplace(std::piecewise_construct,
                                      std::forward_as_tuple(ds_it->first),  // args for key
                                      std::forward_as_tuple(ds_it->first, ds_it->second.name(),
                                                            active));
        }
    }
}

void EvaluationManager::updateTestDataSourcesActive()
{
    loginf << "EvaluationManager: updateTestDataSourcesActive";

    assert (hasValidTestDBO());

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_tst_);

    assert (object.hasActiveDataSourcesInfo());

    for (auto& srcit : data_sources_tst_)
        srcit.second.setActiveInData(false);

    for (auto& it : object.getActiveDataSources())
    {
        assert(data_sources_tst_.find(it) != data_sources_tst_.end());
        ActiveDataSource& src = data_sources_tst_.at(it);
        src.setActiveInData(true);
    }

    for (auto& srcit : data_sources_tst_)
    {
        if (!srcit.second.isActiveInData())
            srcit.second.setActive(false);
    }
}

void EvaluationManager::setViewableDataConfig (const nlohmann::json::object_t& data)
{
    if (viewable_data_cfg_)
    {
        ATSDB::instance().viewManager().unsetCurrentViewPoint();
        viewable_data_cfg_ = nullptr;
    }

    viewable_data_cfg_.reset(new ViewableDataConfig(data));

    ATSDB::instance().viewManager().setCurrentViewPoint(viewable_data_cfg_.get());
}

void EvaluationManager::showUTN (unsigned int utn)
{
    loginf << "EvaluationManager: showUTN: utn " << utn;

    nlohmann::json data = getBaseViewableDataConfig();
    data["filters"]["UTNs"]["utns"] = to_string(utn);

    loginf << "EvaluationManager: showUTN: showing";
    setViewableDataConfig(data);
}

std::unique_ptr<nlohmann::json::object_t> EvaluationManager::getViewableForUTN (unsigned int utn)
{
    nlohmann::json::object_t data = getBaseViewableDataConfig();
    data["filters"]["UTNs"]["utns"] = to_string(utn);

    return std::unique_ptr<nlohmann::json::object_t>{new nlohmann::json::object_t(move(data))};
}

std::unique_ptr<nlohmann::json::object_t> EvaluationManager::getViewableForEvaluation (
        const std::string& req_grp_id, const std::string& result_id)
{
    nlohmann::json::object_t data = getBaseViewableNoDataConfig();

    data["evaluation_results"]["show_results"] = true;
    data["evaluation_results"]["req_grp_id"] = req_grp_id;
    data["evaluation_results"]["result_id"] = result_id;

    return std::unique_ptr<nlohmann::json::object_t>{new nlohmann::json::object_t(move(data))};
}

std::unique_ptr<nlohmann::json::object_t> EvaluationManager::getViewableForEvaluation (
        unsigned int utn, const std::string& req_grp_id, const std::string& result_id)
{
    nlohmann::json::object_t data = getBaseViewableDataConfig();
    data["filters"]["UTNs"]["utns"] = to_string(utn);

    data["evaluation_results"]["show_results"] = true;
    data["evaluation_results"]["req_grp_id"] = req_grp_id;
    data["evaluation_results"]["result_id"] = result_id;

    return std::unique_ptr<nlohmann::json::object_t>{new nlohmann::json::object_t(move(data))};
}

void EvaluationManager::showResultId (const std::string& id)
{
    loginf << "EvaluationManager: showResultId: id '" << id << "'";

    assert (widget_);
    widget_->showResultId(id);
}

void EvaluationManager::setUseTargetData (unsigned int utn, bool value)
{
    loginf << "EvaluationManager: setUseTargetData: utn " << utn << " use " << value;

    data_.setUseTargetData(utn, value);
    updateResultsToUseChangeOf(utn);
}

void EvaluationManager::updateResultsToUseChangeOf (unsigned int utn)
{
    if (evaluated_)
    {
        results_gen_.updateToUseChangeOf(utn);

        if (widget_)
        {
            widget_->expandResults();
            widget_->reshowLastResultId();
        }
    }
}

json::boolean_t& EvaluationManager::useGroupInSectorLayer(const std::string& sector_layer_name,
                                                          const std::string& group_name)
{
    assert (hasCurrentStandard());

    // standard_name->sector_layer_name->req_grp_name->bool use
    if (!use_grp_in_sector_.contains(current_standard_)
            || !use_grp_in_sector_.at(current_standard_).contains(sector_layer_name)
            || !use_grp_in_sector_.at(current_standard_).at(sector_layer_name).contains(group_name))
        use_grp_in_sector_[current_standard_][sector_layer_name][group_name] = true;

    return use_grp_in_sector_[current_standard_][sector_layer_name][group_name].get_ref<json::boolean_t&>();
}

void EvaluationManager::useGroupInSectorLayer(const std::string& sector_layer_name,
                                              const std::string& group_name, bool value)
{
    assert (hasCurrentStandard());

    loginf << "EvaluationManager: useGroupInSector: standard_name " << current_standard_
           << " sector_layer_name " << sector_layer_name << " group_name " << group_name << " value " << value;

    use_grp_in_sector_[current_standard_][sector_layer_name][group_name] = value;
}

json::boolean_t& EvaluationManager::useRequirement(const std::string& standard_name, const std::string& group_name,
                                                   const std::string& req_name)
{
    // standard_name->req_grp_name->req_grp_name->bool use
    if (!use_requirement_.contains(standard_name)
            || !use_requirement_.at(standard_name).contains(group_name)
            || !use_requirement_.at(standard_name).at(group_name).contains(req_name))
        use_requirement_[standard_name][group_name][req_name] = true;

    return use_requirement_[standard_name][group_name][req_name].get_ref<json::boolean_t&>();
}

EvaluationResultsReport::PDFGenerator& EvaluationManager::pdfGenerator() const
{
    assert (pdf_gen_);
    return *pdf_gen_;
}

nlohmann::json::object_t EvaluationManager::getBaseViewableDataConfig ()
{
    nlohmann::json data;
    //    "db_objects": [
    //    "Tracker"
    //    ],
    // "filters": {
    //    "Tracker Data Sources": {
    //    "active_sources": [
    //    13040,
    //    13041
    //    ]
    //    }
    //    }

    data["db_objects"] = vector<string>{dbo_name_ref_, dbo_name_tst_};

    // ref srcs
    {
        vector<unsigned int> active_ref_srcs;

        for (auto& ds_it : data_sources_ref_)
            active_ref_srcs.push_back(ds_it.first);

        data["filters"][dbo_name_ref_+" Data Sources"]["active_sources"] = active_ref_srcs;
    }

    // tst srcs
    {
        vector<unsigned int> active_tst_srcs;

        for (auto& ds_it : data_sources_tst_)
            active_tst_srcs.push_back(ds_it.first);

        data["filters"][dbo_name_tst_+" Data Sources"]["active_sources"] = active_tst_srcs;
    }

    return data;
}

nlohmann::json::object_t EvaluationManager::getBaseViewableNoDataConfig ()
{
    nlohmann::json data;

    data["db_objects"] = vector<string>{};

    return data;
}

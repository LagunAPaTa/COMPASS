#ifndef BUFFERTABLEMODEL_H
#define BUFFERTABLEMODEL_H

#include <memory>

#include <QAbstractTableModel>

class Buffer;
class DBObject;
class BufferCSVExportJob;

class BufferTableModel : public QAbstractTableModel
{
    Q_OBJECT

public slots:
    void exportJobObsoleteSlot ();
    void exportJobDoneSlot();

public:
    BufferTableModel(QObject *parent, DBObject &object);
    virtual ~BufferTableModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void clearData ();
    void setData (std::shared_ptr <Buffer> buffer);

    void saveAsCSV (const std::string &file_name, bool overwrite);

protected:
    DBObject &object_;
    std::shared_ptr <Buffer> buffer_;

    std::shared_ptr <BufferCSVExportJob> export_job_;
};

#endif // BUFFERTABLEMODEL_H

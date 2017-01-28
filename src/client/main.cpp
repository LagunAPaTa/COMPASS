
#include "boost/date_time/posix_time/posix_time.hpp"

#include "config.h"
#include "logger.h"
#include "configurationmanager.h"

#include "buffer.h"
#include "arraylist.h"

#include "atsdb.h"
#include "client.h"
#include "mainwindow.h"

void test_array_list(unsigned int test_size)
{
    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    loginf << "Main: test_array_list: arraylist double";

    start_time = boost::posix_time::microsec_clock::local_time();

    ArrayListTemplate<double> list;

    for (unsigned int cnt=0; cnt < test_size; cnt++)
        list.set(cnt, 1.0);

    for (unsigned int cnt=0; cnt < list.size(); cnt++)
    {
        assert (list.get(cnt) == 1.0);
        assert (list.isNone(cnt) == false);
    }

    list.clear();
    for (unsigned int cnt=0; cnt < list.size(); cnt++)
    {
        assert (list.isNone(cnt) == true);
    }

    stop_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time - start_time;
    loginf  << "Main: test_array_list: done after " << diff.total_milliseconds() << "ms";
}

void test_buffer_smart (unsigned int test_size)
{
    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    loginf << "Main: test_buffer_smart: buffer double";

    start_time = boost::posix_time::microsec_clock::local_time();

    Buffer buffer;
    assert (buffer.firstWrite() == true);
    buffer.addProperty("my_bonny", PropertyDataType::DOUBLE);

    ArrayListTemplate<double> &list = buffer.getDouble("my_bonny");

    for (unsigned int cnt=0; cnt < test_size; cnt++)
        list.set(cnt, 1.0);

    assert (list.size() == test_size);
    assert (buffer.firstWrite() == false);

    for (unsigned int cnt=0; cnt < list.size(); cnt++)
    {
        assert (list.get(cnt) == 1.0);
        assert (list.isNone(cnt) == false);
    }

    buffer.getDouble("my_bonny").clear();
    for (unsigned int cnt=0; cnt < list.size(); cnt++)
    {
        assert (list.isNone(cnt) == true);
    }

    stop_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time - start_time;
    loginf  << "Main: test_buffer_smart: done after " << diff.total_milliseconds() << "ms";

}

void test_buffer (unsigned int test_size)
{
    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    loginf << "Main: test_buffer: buffer double";

    start_time = boost::posix_time::microsec_clock::local_time();

    Buffer buffer;
    assert (buffer.firstWrite() == true);
    buffer.addProperty("my_bonny", PropertyDataType::DOUBLE);


    for (unsigned int cnt=0; cnt < test_size; cnt++)
        buffer.getDouble("my_bonny").set(cnt, 1.0);

    assert (buffer.size() == test_size);
    assert (buffer.firstWrite() == false);

    for (unsigned int cnt=0; cnt < buffer.getDouble("my_bonny").size(); cnt++)
    {
        assert (buffer.getDouble("my_bonny").get(cnt) == 1.0);
        assert (buffer.getDouble("my_bonny").isNone(cnt) == false);
    }

    buffer.getDouble("my_bonny").clear();
    for (unsigned int cnt=0; cnt < buffer.getDouble("my_bonny").size(); cnt++)
    {
        assert (buffer.getDouble("my_bonny").isNone(cnt) == true);
    }

    stop_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time - start_time;
    loginf  << "Main: test_buffer: done after " << diff.total_milliseconds() << "ms";

}

int main (int argc, char **argv)
{
    try
    {
        Config config ("conf/client.conf");
        assert (config.existsId("version"));
        assert (config.existsId("main_configuration_file"));
        assert (config.existsId("log_properties_file"));
        assert (config.existsId("save_config_on_exit"));

        Logger::getInstance().init(config.getString("log_properties_file"));

        loginf << "ATSDBClient: startup version " << config.getString("version") << " using file '" << config.getString("main_configuration_file") << "'";

        ConfigurationManager::getInstance().init (config.getString("main_configuration_file"));

        Client mf(argc, argv);

        MainWindow window;

        window.show();

        return mf.exec();

//        MySQLConnectionInfo info (DB_TYPE_MYSQLpp, "job_awam_0023", "localhost", "sassc", "sassc", 3306);

//        ATSDB::getInstance().connect(&info);
//        ATSDB::getInstance().open("job_awam_0023");

//        if (config.getBool("save_config_on_exit"))
//            ConfigurationManager::getInstance().saveConfiguration();

//        ATSDB::getInstance().shutdown();

//        unsigned int test_size=10000000;
//        test_array_list(test_size);
//        test_buffer(test_size);
//        test_buffer_smart(test_size);

        loginf << "ATSDBClient: shutdown";

        return 0;
    }
    catch (std::exception &ex)
    {
        logerr  << "Main: Caught Exception '" << ex.what() << "'";

        ATSDB::getInstance().shutdown();

        //WorkerThreadManager::getInstance().shutdown();

        return -1;
    }
    catch(...)
    {
        logerr  << "Main: Caught Exception";

        ATSDB::getInstance().shutdown();

        //WorkerThreadManager::getInstance().shutdown();

        return -1;
    }
    loginf << "Main: Shutdown";
    return 0;
}

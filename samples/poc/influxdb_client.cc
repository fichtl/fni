#include <string>

#include "InfluxDB.h"
#include "InfluxDBFactory.h"
#include "spdlog/spdlog.h"

int main()
{
        auto client = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=dni-test");

        try
        {
                client->createDatabaseIfNotExists();
                client->write(influxdb::Point{"test"}
                                  .addField("value", 10)
                                  .addTag("host", "localhost"));
        }
        catch (std::exception& e)
        {
                spdlog::error(e.what());
                exit(-1);
        }

        std::vector<influxdb::Point> points = client->query("SELECT * FROM test");

        for (auto& point : points)
        {
                spdlog::info(
                    "{}: {}({})", point.getName(), point.getFields(), point.getTags());
        }

        return 0;
}

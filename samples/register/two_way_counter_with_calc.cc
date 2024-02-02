#include <chrono>
#include <thread>

#include "dni/framework/graph.h"
#include "dni/framework/datum.h"

int run_times = 5;

void func(dni::Graph* g)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        for (int i = 0; i < run_times; i++)
        {
                int num = 10 + i;
                dni::Datum d(num);
                std::cout << "[" << __FILE__ << " - " << __FUNCTION__ << ": " << __LINE__
                          << "], d: " << d << "g: 0x" << g << std::endl;

                g->AddDatumToInputStream("GInSSS", d);
                g->AddDatumToInputStream("GInTTT", d);

                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }

        std::cout << "thread quit" << std::endl;
}


int main()
{
        std::cout << "main ..." << std::endl;
        std::string path = "/home/zhouxu/works/2024/50_dni/0325/dni/samples/register/two_way_counter_with_calc.pbtxt";

        auto gc = dni::ParsePbtxtToGraphConfig(path);
        if (!gc.has_value())
        {
                std::cout << "invalid pbtxt path: " << path << std::endl;
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        g->PrepareForRun();

        std::thread t1(func, g);

        for (int i = 0; i < run_times; i++)
        {
                g->Run();
                std::string out = std::string("GOutSSS");
                auto ret = g->GetResult<const int>(out);
                std::cout << "[" << i << "]" << "GOutSSS result is: " << ret << std::endl;

                std::string out2 = std::string("GOutTTT");
                auto ret2 = g->GetResult<const int>(out2);
                std::cout << "[" << i << "]" << "GOutTTT result is: " << ret2 << std::endl;

                g->ClearResult();
        }



        t1.join();

        g->Finish();

        std::cout << "main over" << std::endl;

        return 0;
}

#include "proto_server.h"

#include <chrono>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"

using InputMap = std::map<std::string, dni::Datum>;

#define SHM_KEY 0x1234   // for ringbuffer_t struct

/////////////////////////////////////////////////////////////////////
// ring buffer
typedef struct {
        volatile uint32_t head;
        volatile uint32_t tail;
        size_t size;
} ringbuffer_t;

uint8_t* buffer_create(size_t sz)
{
        int shm_id;

        shm_id = shmget(SHM_KEY, sz, IPC_CREAT | 0666);
        if (shm_id == -1)
        {
                perror("parent: shmget SHM_KEY___2");
                return NULL;
        }

        void* addr = shmat(shm_id, NULL, 0);
        if (addr == (void*) -1)
        {
                perror("parent: shmat SHM_KEY___2");
                return NULL;
        }

        return (uint8_t*) addr;
}

void buffer_free(uint8_t* buffer)
{
        if (shmdt((const void*) (buffer)) == -1)
        {
                perror("recv: shmdt buffer");
                return;
        }
}

Status LandingServiceImpl::Talk(
    ServerContext* context, const TalkRequest* request, TalkResponse* response)
{
        return Status::OK;
}

Status LandingServiceImpl::TalkOneAnswerMore(
    ServerContext* context, const TalkRequest* request,
    ServerWriter<TalkResponse>* writer)
{
        size_t slot_size = (size_t) 20 * 1024 * 1024;   // one slot size, Bytes
        int slot_count = 100;                           // slot count

        // [notice] no need to copy data from share memory
        unsigned char* nic_data = 0;   // (unsigned char*) malloc(slot_size);
        size_t rbsize = (size_t) (slot_size * slot_count + sizeof(ringbuffer_t));

        uint8_t* rb_buffer = buffer_create(rbsize);
        uint8_t* data_buffer = rb_buffer + sizeof(ringbuffer_t);

        ringbuffer_t* rb_header = (ringbuffer_t*) rb_buffer;
        rb_header->head = 0;
        rb_header->tail = 0;
        rb_header->size = (size_t) (slot_size * slot_count);

        size_t used;

        printHeaders(context);
        if (client != nullptr)
        {
                grpc::ClientContext c;
                propagateHeaders(context, c);
                TalkResponse talkResponse;
                const std::unique_ptr<::grpc::ClientReader<TalkResponse>>& response(
                    client->TalkOneAnswerMore(&c, *request));
                while (response->Read(&talkResponse))
                {
                        writer->Write(talkResponse);
                }

                return Status::OK;
        }

        const std::string& pbtxt = request->data();
        SPDLOG_DEBUG(
            "TalkOneAnswerMore REQUEST: data={}, meta={}", pbtxt, request->meta());

        auto gc = dni::ParseStringToGraphConfig(pbtxt);
        if (!gc)
        {
                SPDLOG_CRITICAL("invalid pbtxt config: {}", pbtxt);
                return Status::CANCELLED;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        std::string out = "dms_rules";

        SPDLOG_DEBUG("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        std::vector<uint32_t> all_known_ips = {};

        std::vector<double_t> netdevs;
        netdevs.push_back(4.00 * 1e9);
        netdevs.push_back(0.6);
        netdevs.push_back(16.00 * 1e6);
        netdevs.push_back(1.00);

        while (1)
        {
                // read data from ringbuffer
                if (rb_header->head <= rb_header->tail)
                        used = rb_header->tail - rb_header->head;
                else
                        used = rb_header->size - (rb_header->head - rb_header->tail);

                if (used < slot_size)
                {
                        // SPDLOG_DEBUG("no data in RB, continue...");
                        continue;
                }

                // [notice] no need to copy data from share memory
                // memcpy(nic_data, data_buffer + rb_header->head, slot_size);
                nic_data = data_buffer + rb_header->head;

                rb_header->head = (rb_header->head + slot_size) % rb_header->size;

                // add data to ddos graph3
                // a new task to replace pcap task to parse data
                g->AddDatumToInputStream("rbData", dni::Datum(nic_data));
                g->AddDatumToInputStream("all_known_ips", dni::Datum(all_known_ips));
                g->AddDatumToInputStream("netdevs_1", dni::Datum(netdevs));

                g->RunOnce();

                g->Wait();

                auto ret = g->GetResult<dni::snding::NICDMSRulesMap>(out);
                SPDLOG_DEBUG("Gout {} result is: {}", out, ret);

                // guess need to convert ret to string and send to client, tmp
                std::string ret_str;
                int i = 0;
                for (auto&& dms_rule : ret)
                {
                        ret_str += (std::to_string(i) + ":\n");
                        ret_str +=
                            (std::string("hostNicSign: ") + dms_rule.first + "\n[\n");
                        ret_str += fmt::to_string(dms_rule.second);
                        ret_str += "\n]\n\n";

                        i++;
                }

                g->ClearResult();

                TalkResponse response;
                response.set_status(200);
                TalkResult* talkResult;
                talkResult = response.add_results();
                buildGraphResult(pbtxt, ret_str, talkResult);
                writer->Write(response);

                SPDLOG_DEBUG("writer->Write, {}", ret_str);
        }

        return Status::OK;
}

Status LandingServiceImpl::TalkMoreAnswerOne(
    ServerContext* context, ServerReader<TalkRequest>* reader, TalkResponse* response)
{
        return Status::OK;
}

Status LandingServiceImpl::TalkBidirectional(
    ServerContext* context, ServerReaderWriter<TalkResponse, TalkRequest>* stream)
{
        return Status::OK;
}

void LandingServiceImpl::buildGraphResult(
    const std::string& pbtxt, const std::string& ret, TalkResult* talkResult)
{
        talkResult->set_id(Utils::now());
        talkResult->set_type(ResultType::OK);

        google::protobuf::Map<std::string, std::string>* pMap = talkResult->mutable_kv();

        const std::string& uuid = Utils::uuid();
        (*pMap)["id"] = uuid;
        (*pMap)["idx"] = pbtxt;
        (*pMap)["meta"] = "C++";
        (*pMap)["data"] = ret;
}

void LandingServiceImpl::printHeaders(const ServerContext* context)
{
        const multimap<grpc::string_ref, grpc::string_ref>& metadata =
            context->client_metadata();
        for (const auto& iter : metadata)
        {
                const grpc::string_ref& key = iter.first;
                const grpc::string_ref& value = iter.second;
                SPDLOG_DEBUG(
                    "->H {}: {}", std::string(key.begin(), key.end()),
                    std::string(value.begin(), value.end()));
        }
}

void LandingServiceImpl::propagateHeaders(
    const ServerContext* context, grpc::ClientContext& c)
{
        const multimap<grpc::string_ref, grpc::string_ref>& metadata =
            context->client_metadata();
        for (const auto& iter : metadata)
        {
                const grpc::string_ref& key = iter.first;
                const grpc::string_ref& value = iter.second;
                SPDLOG_DEBUG(
                    "->H {}: {}", std::string(key.begin(), key.end()),
                    std::string(value.begin(), value.end()));
                // c.AddMetadata((basic_string<char> &&) key, (basic_string<char>
                // &&) value);
        }
}

void LandingServiceImpl::setChannel(const std::shared_ptr<Channel>& channel) {}

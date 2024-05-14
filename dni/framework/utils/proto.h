#include <google/protobuf/text_format.h>

namespace dni {

        namespace proto = ::google::protobuf;

        using TextProto = proto::TextFormat;

        const char kASNApisPrefix[] = "type.asnapis.io/";

        class CustomizedAnyFinder: public TextProto::Finder {
                const proto::Descriptor* FindAnyType(
                    const proto::Message& message, const std::string& prefix,
                    const std::string& name) const override
                {
                        if (prefix != kASNApisPrefix)
                        {
                                return nullptr;
                        }
                        return message.GetDescriptor()
                            ->file()
                            ->pool()
                            ->FindMessageTypeByName(name);
                }
        };

}   // namespace dni

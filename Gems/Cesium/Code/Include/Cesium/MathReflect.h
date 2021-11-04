#pragma once

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/Json/BaseJsonSerializer.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstddef>

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(glm::dvec3, "{1E2EB371-18B6-4B35-B974-81E0E8CCF2A3}");
}

namespace Cesium
{
    struct MathSerialization
    {
        static void Reflect(AZ::ReflectContext* context);
    };

    template<typename VecType>
    class GlmVecSerializer : public AZ::SerializeContext::IDataSerializer
    {
        std::size_t Save(const void* classPtr, AZ::IO::GenericStream& stream, [[maybe_unused]] bool isDataBigEndian) override
        {
            if (classPtr == nullptr)
            {
                return 0;
            }

            const VecType* instance = reinterpret_cast<const VecType*>(classPtr);
            return static_cast<size_t>(stream.Write(sizeof(VecType), glm::value_ptr(*instance)));
        }

        std::size_t DataToText(AZ::IO::GenericStream& in, AZ::IO::GenericStream& out, [[maybe_unused]] bool isDataBigEndian) override
        {
            if (in.GetLength() < sizeof(VecType))
            {
                return 0;
            }

            VecType instance;
            in.Read(sizeof(VecType), reinterpret_cast<void*>(&instance));

            AZStd::string outText = "[";
            AZStd::string separator = "";
            for (typename VecType::length_type i = 0; i < instance.length(); ++i)
            {
                outText += separator + AZStd::to_string(instance[i]);
                separator = ",";
            }
            outText += "]";

            return static_cast<size_t>(out.Write(outText.size(), outText.data()));
        }

        std::size_t TextToData(
            const char* text,
            [[maybe_unused]] unsigned int textVersion,
            AZ::IO::GenericStream& stream,
            [[maybe_unused]] bool isDataBigEndian) override
        {
            VecType instance{ 0 };
            std::size_t begin = 1;
            std::size_t end = 1;
            typename VecType::length_type i = 0;
            while (begin <= end && text[begin] != '\0' && i < instance.length())
            {
                while (text[end] != ']' && text[end] != ',' && text[end] != '\0')
                {
                    ++end;
                }

                instance[i] = std::atof(&text[begin]);
                begin = end + 1;
                end = begin;
                ++i;
            }

            stream.Seek(0, AZ::IO::GenericStream::ST_SEEK_BEGIN);
            return static_cast<size_t>(stream.Write(sizeof(VecType), reinterpret_cast<void*>(&instance)));
        }

        bool Load(void* classPtr, AZ::IO::GenericStream& stream, unsigned int version, [[maybe_unused]] bool isDataBigEndian) override
        {
            AZ_UNUSED(version);
            if (stream.GetLength() < sizeof(VecType))
            {
                return false;
            }

            VecType* instance = reinterpret_cast<VecType*>(classPtr);
            stream.Read(sizeof(VecType), reinterpret_cast<void*>(&instance));
            return true;
        }

        bool CompareValueData(const void* lhs, const void* rhs) override
        {
            const VecType* lhsInstance = reinterpret_cast<const VecType*>(lhs);
            const VecType* rhsInstance = reinterpret_cast<const VecType*>(rhs);
            return *lhsInstance == *rhsInstance;
        }
    };

    template<typename VecType>
    class GlmVecJsonSerializer : public AZ::BaseJsonSerializer
    {
    public:
        AZ_RTTI(GlmVecJsonSerializer, "{338EA004-2521-4052-BB64-A8505F3ABBD7}", AZ::BaseJsonSerializer);
        AZ_CLASS_ALLOCATOR(GlmVecJsonSerializer, AZ::SystemAllocator, 0);

        AZ::JsonSerializationResult::Result Load(void* outputValue, const AZ::Uuid& outputValueTypeId, const rapidjson::Value& inputValue,
            AZ::JsonDeserializerContext& context) override
        {
            namespace JSR = AZ::JsonSerializationResult;

            if (outputValueTypeId != azrtti_typeid<VecType>())
            {
                return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Unable to deserialize glm::dvec from json because of type mismatch.");
            }

            VecType* instance = reinterpret_cast<VecType*>(outputValue);
            if (instance == nullptr)
            {
                return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Unable to deserialize nullptr glm::dvec from json.");
            }

            if (inputValue.GetType() == rapidjson::Type::kArrayType)
            {
                const auto& arrayValue = inputValue.GetArray();
                if (arrayValue.Size() == static_cast<rapidjson::SizeType>(instance->length()))
                {
                    for (rapidjson::SizeType i = 0; i < arrayValue.Size(); ++i)
                    {
                        (*instance)[i] = arrayValue[i].GetDouble();
                    }
                }
                else
                {
                    return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Json array size mismatches with the size of glm::dvec.");
                }
            }
            else
            {
                return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Cannot deserialize glm::dvec for json format that is not an array.");
            }

            return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Success, "Successfully deserialize glm::dvec");
        }
        
        AZ::JsonSerializationResult::Result Store(rapidjson::Value& outputValue, const void* inputValue, [[maybe_unused]] const void* defaultValue,
            const AZ::Uuid& valueTypeId,
            AZ::JsonSerializerContext& context) override
        {
            namespace JSR = AZ::JsonSerializationResult;

            if (valueTypeId != azrtti_typeid<VecType>())
            {
                return context.Report(JSR::Tasks::WriteValue, JSR::Outcomes::Invalid, "Unable to serialize glm::dvec to json because of type mismatch.");
            }

            const VecType* instance = reinterpret_cast<const VecType*>(inputValue);
            if (instance == nullptr)
            {
                return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Unable to serialize nullptr glm::dvec to json.");
            }

            outputValue.SetArray();
            outputValue.Reserve(instance->length(), context.GetJsonAllocator());
            for (typename VecType::length_type i = 0; i < instance->length(); ++i)
            {
                outputValue.PushBack((*instance)[i], context.GetJsonAllocator());
            }
            
            return context.Report(JSR::Tasks::WriteValue, JSR::Outcomes::Success, "Successfully serialize glm::dvec");
        }

        OperationFlags GetOperationsFlags() const override
        {
            return OperationFlags::InitializeNewInstance;
        }
    };
} // namespace Cesium

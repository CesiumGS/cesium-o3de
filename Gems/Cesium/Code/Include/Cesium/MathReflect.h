#pragma once

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/Json/BaseJsonSerializer.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/std/string/string.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstddef>

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(glm::dmat3, "{889D681F-010D-4B5C-B2CE-81097383B329}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dmat4, "{8C9ECA4A-052D-47AC-A969-D5E5CF41D79A}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dvec2, "{51022DB5-F187-4FB9-8DC2-533F81AD337E}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dvec3, "{1E2EB371-18B6-4B35-B974-81E0E8CCF2A3}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dvec4, "{9657DD0A-632F-4FA1-A01F-6BCB1CB08B33}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dquat, "{B4A0CC9A-FE9B-4777-8F94-C14C1540A3C5}");
}

namespace Cesium
{
    struct MathSerialization
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

    private:
        template<typename VecType>
        static void ReflectGlmVecBehavior(AZ::BehaviorContext* context, const AZStd::string& name);

        static void ReflectGlmQuatBehavior(AZ::BehaviorContext* context);
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

            return static_cast<std::size_t>(out.Write(outText.size(), outText.data()));
        }

        std::size_t TextToData(
            const char* text,
            [[maybe_unused]] unsigned int textVersion,
            AZ::IO::GenericStream& stream,
            [[maybe_unused]] bool isDataBigEndian) override
        {
            VecType instance{};
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
            stream.Read(sizeof(VecType), glm::value_ptr(*instance));
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

    class GlmDVec2JsonSerializer : public GlmVecJsonSerializer<glm::dvec2>
    {
    public:
        AZ_RTTI(GlmDVec2JsonSerializer, "{179887FA-8034-4264-B6A1-216D6A7FD16C}", GlmVecJsonSerializer<glm::dvec2>);
        AZ_CLASS_ALLOCATOR(GlmDVec2JsonSerializer, AZ::SystemAllocator, 0);
    };

    class GlmDVec3JsonSerializer : public GlmVecJsonSerializer<glm::dvec3>
    {
    public:
        AZ_RTTI(GlmDVec3JsonSerializer, "{9BCD6B7B-845D-4073-B20C-1C822861A1AA}", GlmVecJsonSerializer<glm::dvec3>);
        AZ_CLASS_ALLOCATOR(GlmDVec3JsonSerializer, AZ::SystemAllocator, 0);
    };

    class GlmDVec4JsonSerializer : public GlmVecJsonSerializer<glm::dvec4>
    {
    public:
        AZ_RTTI(GlmDVec4JsonSerializer, "{1A1F8E80-6408-4224-B0FC-8E0F9DD6B119}", GlmVecJsonSerializer<glm::dvec4>);
        AZ_CLASS_ALLOCATOR(GlmDVec4JsonSerializer, AZ::SystemAllocator, 0);
    };

    class GlmDQuatJsonSerializer : public GlmVecJsonSerializer<glm::dquat>
    {
    public:
        AZ_RTTI(GlmDQuatJsonSerializer, "{CDCD546F-EEFE-44AD-B525-2E00C8E2FC52}", GlmVecJsonSerializer<glm::dquat>);
        AZ_CLASS_ALLOCATOR(GlmDQuatJsonSerializer, AZ::SystemAllocator, 0);
    };
} // namespace Cesium

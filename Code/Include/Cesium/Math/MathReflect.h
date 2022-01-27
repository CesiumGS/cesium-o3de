#pragma once

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/Json/BaseJsonSerializer.h>
#include <AzCore/Serialization/Json/RegistrationContext.h>
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
    AZ_TYPE_INFO_SPECIALIZE(glm::dvec2, "{51022DB5-F187-4FB9-8DC2-533F81AD337E}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dvec3, "{1E2EB371-18B6-4B35-B974-81E0E8CCF2A3}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dvec4, "{9657DD0A-632F-4FA1-A01F-6BCB1CB08B33}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dquat, "{B4A0CC9A-FE9B-4777-8F94-C14C1540A3C5}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dmat2, "{76A74122-EDA2-403A-B9EE-F271C15F108D}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dmat3, "{889D681F-010D-4B5C-B2CE-81097383B329}");
    AZ_TYPE_INFO_SPECIALIZE(glm::dmat4, "{8C9ECA4A-052D-47AC-A969-D5E5CF41D79A}");
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

        template<typename MatType>
        static void ReflectGlmDMatBehavior(AZ::BehaviorContext* context, const AZStd::string& name);

        static void ReflectGlmQuatBehavior(AZ::BehaviorContext* context);

        static void ReflectGlmVector(AZ::BehaviorContext::ClassBuilder<glm::dvec2> &builder);

        static void ReflectGlmVector(AZ::BehaviorContext::ClassBuilder<glm::dvec3> &builder);
        
        static void ReflectGlmVector(AZ::BehaviorContext::ClassBuilder<glm::dvec4> &builder);
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

    template<typename MatType>
    class GlmDMatSerializer : public AZ::SerializeContext::IDataSerializer
    {
        std::size_t Save(const void* classPtr, AZ::IO::GenericStream& stream, [[maybe_unused]] bool isDataBigEndian) override
        {
            if (classPtr == nullptr)
            {
                return 0;
            }

            const MatType* instance = reinterpret_cast<const MatType*>(classPtr);
            return static_cast<size_t>(stream.Write(sizeof(MatType), glm::value_ptr(*instance)));
        }

        std::size_t DataToText(AZ::IO::GenericStream& in, AZ::IO::GenericStream& out, [[maybe_unused]] bool isDataBigEndian) override
        {
            using VecType = typename MatType::col_type;

            if (in.GetLength() < sizeof(MatType))
            {
                return 0;
            }

            MatType instance{};
            in.Read(sizeof(MatType), reinterpret_cast<void*>(&instance));

            AZStd::string outText = "[";
            AZStd::string separator = "";
            for (typename MatType::length_type i = 0; i < instance.length(); ++i)
            {
                for (typename VecType::length_type j = 0; j < instance[0].length(); ++j)
                {
                    outText += separator + AZStd::to_string(instance[i][j]);
                    separator = ",";
                }
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
            MatType instance{};
            std::size_t begin = 1;
            std::size_t end = 1;
            typename MatType::length_type i = 0;
            typename MatType::length_type totalElements = instance.length() * instance[0].length();
            while (begin <= end && text[begin] != '\0' && i < totalElements)
            {
                while (text[end] != ']' && text[end] != ',' && text[end] != '\0')
                {
                    ++end;
                }

                typename MatType::length_type col = i / instance.length();
                typename MatType::length_type row = i % instance[0].length();
                instance[col][row] = std::atof(&text[begin]);
                begin = end + 1;
                end = begin;
                ++i;
            }

            return static_cast<size_t>(stream.Write(sizeof(MatType), reinterpret_cast<void*>(&instance)));
        }

        bool Load(void* classPtr, AZ::IO::GenericStream& stream, unsigned int version, [[maybe_unused]] bool isDataBigEndian) override
        {
            AZ_UNUSED(version);
            if (stream.GetLength() < sizeof(MatType))
            {
                return false;
            }

            MatType* instance = reinterpret_cast<MatType*>(classPtr);
            stream.Read(sizeof(MatType), glm::value_ptr(*instance));
            return true;
        }

        bool CompareValueData(const void* lhs, const void* rhs) override
        {
            const MatType* lhsInstance = reinterpret_cast<const MatType*>(lhs);
            const MatType* rhsInstance = reinterpret_cast<const MatType*>(rhs);
            return *lhsInstance == *rhsInstance;
        }
    };

    template<typename VecType>
    class GlmVecJsonSerializer : public AZ::BaseJsonSerializer
    {
    public:
        AZ_RTTI(GlmVecJsonSerializer, "{338EA004-2521-4052-BB64-A8505F3ABBD7}", AZ::BaseJsonSerializer);
        AZ_CLASS_ALLOCATOR(GlmVecJsonSerializer, AZ::SystemAllocator, 0);

        AZ::JsonSerializationResult::Result Load(
            void* outputValue,
            const AZ::Uuid& outputValueTypeId,
            const rapidjson::Value& inputValue,
            AZ::JsonDeserializerContext& context) override
        {
            namespace JSR = AZ::JsonSerializationResult;

            if (outputValueTypeId != azrtti_typeid<VecType>())
            {
                return context.Report(
                    JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Unable to deserialize glm::dvec from json because of type mismatch.");
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
                    return context.Report(
                        JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Json array size mismatches with the size of glm::dvec.");
                }
            }
            else
            {
                return context.Report(
                    JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Cannot deserialize glm::dvec for json format that is not an array.");
            }

            return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Success, "Successfully deserialize glm::dvec");
        }

        AZ::JsonSerializationResult::Result Store(
            rapidjson::Value& outputValue,
            const void* inputValue,
            [[maybe_unused]] const void* defaultValue,
            const AZ::Uuid& valueTypeId,
            AZ::JsonSerializerContext& context) override
        {
            namespace JSR = AZ::JsonSerializationResult;

            if (valueTypeId != azrtti_typeid<VecType>())
            {
                return context.Report(
                    JSR::Tasks::WriteValue, JSR::Outcomes::Invalid, "Unable to serialize glm::dvec to json because of type mismatch.");
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

    template<typename MatType>
    class GlmDMatJsonSerializer : public AZ::BaseJsonSerializer
    {
    public:
        AZ_RTTI(GlmDMatJsonSerializer, "{44B47C67-E32F-48E9-BFF0-20C3620F49E1}", AZ::BaseJsonSerializer);
        AZ_CLASS_ALLOCATOR(GlmDMatJsonSerializer, AZ::SystemAllocator, 0);

        AZ::JsonSerializationResult::Result Load(
            void* outputValue,
            const AZ::Uuid& outputValueTypeId,
            const rapidjson::Value& inputValue,
            AZ::JsonDeserializerContext& context) override
        {
            namespace JSR = AZ::JsonSerializationResult;
            using VecType = typename MatType::col_type;

            if (outputValueTypeId != azrtti_typeid<MatType>())
            {
                return context.Report(
                    JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Unable to deserialize glm::dmat from json because of type mismatch.");
            }

            MatType* instance = reinterpret_cast<MatType*>(outputValue);
            if (instance == nullptr)
            {
                return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Unable to deserialize nullptr glm::dmat from json.");
            }

            if (inputValue.GetType() == rapidjson::Type::kArrayType)
            {
                const auto& arrayValue = inputValue.GetArray();
                if (arrayValue.Size() == static_cast<rapidjson::SizeType>(instance->length()))
                {
                    auto columnSerializer = context.GetRegistrationContext()->GetSerializerForType(azrtti_typeid<VecType>());
                    for (rapidjson::SizeType i = 0; i < arrayValue.Size(); ++i)
                    {
                        VecType& col = (*instance)[i];
                        auto result = columnSerializer->Load(&col, azrtti_typeid<VecType>(), arrayValue[i], context);
                        if (result.GetResultCode().GetOutcome() != JSR::Outcomes::Success)
                        {
                            return result;
                        }
                    }
                }
                else
                {
                    return context.Report(
                        JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Json array size mismatches with the size of glm::dmat.");
                }
            }
            else
            {
                return context.Report(
                    JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Cannot deserialize glm::dmat for json format that is not an array.");
            }

            return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Success, "Successfully deserialize glm::dmat");

        }

        AZ::JsonSerializationResult::Result Store(
            rapidjson::Value& outputValue,
            const void* inputValue,
            [[maybe_unused]] const void* defaultValue,
            const AZ::Uuid& valueTypeId,
            AZ::JsonSerializerContext& context) override
        {
            namespace JSR = AZ::JsonSerializationResult;
            using VecType = typename MatType::col_type;

            if (valueTypeId != azrtti_typeid<MatType>())
            {
                return context.Report(
                    JSR::Tasks::WriteValue, JSR::Outcomes::Invalid, "Unable to serialize glm::dmat to json because of type mismatch.");
            }

            const MatType* instance = reinterpret_cast<const MatType*>(inputValue);
            if (instance == nullptr)
            {
                return context.Report(JSR::Tasks::ReadField, JSR::Outcomes::Invalid, "Unable to serialize nullptr glm::dmat to json.");
            }

            outputValue.SetArray();
            outputValue.Reserve(instance->length(), context.GetJsonAllocator());
            auto columnSerializer = context.GetRegistrationContext()->GetSerializerForType(azrtti_typeid<VecType>());
            for (typename MatType::length_type i = 0; i < instance->length(); ++i)
            {
                const VecType& colValue = (*instance)[i];
                rapidjson::Value columnJson;
                auto result = columnSerializer->Store(columnJson, &colValue, nullptr, azrtti_typeid<VecType>(), context);
                if (result.GetResultCode().GetOutcome() != JSR::Outcomes::Success)
                {
                    return result;
                }

                outputValue.PushBack(columnJson, context.GetJsonAllocator());
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

    class GlmDMat2JsonSerializer : public GlmDMatJsonSerializer<glm::dmat2>
    {
    public:
        AZ_RTTI(GlmDMat2JsonSerializer, "{91F705E2-522E-4CF5-BFB9-688E3B510E2F}", GlmDMatJsonSerializer<glm::dmat2>);
        AZ_CLASS_ALLOCATOR(GlmDMat2JsonSerializer, AZ::SystemAllocator, 0);
    };

    class GlmDMat3JsonSerializer : public GlmDMatJsonSerializer<glm::dmat3>
    {
    public:
        AZ_RTTI(GlmDMat3JsonSerializer, "{2707FC00-7EED-44AC-860B-A6165469553E}", GlmDMatJsonSerializer<glm::dmat3>);
        AZ_CLASS_ALLOCATOR(GlmDMat3JsonSerializer, AZ::SystemAllocator, 0);
    };

    class GlmDMat4JsonSerializer : public GlmDMatJsonSerializer<glm::dmat4>
    {
    public:
        AZ_RTTI(GlmDMat4JsonSerializer, "{C268E7C8-1FBE-4560-A34D-CFCEA9E1D7C8}", GlmDMatJsonSerializer<glm::dmat4>);
        AZ_CLASS_ALLOCATOR(GlmDMat4JsonSerializer, AZ::SystemAllocator, 0);
    };
} // namespace Cesium

#include <Cesium/MathReflect.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/Json/RegistrationContext.h>

namespace Cesium
{
    void MathSerialization::Reflect(AZ::ReflectContext* context)
    {
        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<glm::dvec3>()->Serializer<GlmVecSerializer<glm::dvec3>>();
            serializeContext->Class<glm::dquat>()->Serializer<GlmVecSerializer<glm::dquat>>();
        }

        AZ::JsonRegistrationContext* jsonRegistrationContext = azrtti_cast<AZ::JsonRegistrationContext*>(context);
        if (jsonRegistrationContext)
        {
            jsonRegistrationContext->Serializer<GlmDVec3JsonSerializer>()->HandlesType<glm::dvec3>();
            jsonRegistrationContext->Serializer<GlmDQuatJsonSerializer>()->HandlesType<glm::dquat>();
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<glm::dvec3>("DVector3")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
                ;

            behaviorContext->Class<glm::dquat>("DQuaternion")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
                ;
        }
    }
} // namespace Cesium

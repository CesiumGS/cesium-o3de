#include <Cesium/MathReflect.h>
#include <AzCore/Serialization/Json/RegistrationContext.h>

namespace Cesium
{
    void MathSerialization::Reflect(AZ::ReflectContext* context)
    {
        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<glm::dvec2>()->Serializer<GlmVecSerializer<glm::dvec2>>();
            serializeContext->Class<glm::dvec3>()->Serializer<GlmVecSerializer<glm::dvec3>>();
            serializeContext->Class<glm::dvec4>()->Serializer<GlmVecSerializer<glm::dvec4>>();
            serializeContext->Class<glm::dquat>()->Serializer<GlmVecSerializer<glm::dquat>>();
        }

        AZ::JsonRegistrationContext* jsonRegistrationContext = azrtti_cast<AZ::JsonRegistrationContext*>(context);
        if (jsonRegistrationContext)
        {
            jsonRegistrationContext->Serializer<GlmDVec2JsonSerializer>()->HandlesType<glm::dvec2>();
            jsonRegistrationContext->Serializer<GlmDVec3JsonSerializer>()->HandlesType<glm::dvec3>();
            jsonRegistrationContext->Serializer<GlmDVec4JsonSerializer>()->HandlesType<glm::dvec4>();
            jsonRegistrationContext->Serializer<GlmDQuatJsonSerializer>()->HandlesType<glm::dquat>();
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            ReflectGlmVecBehavior<glm::dvec2>(behaviorContext, "DVector2");
            ReflectGlmVecBehavior<glm::dvec3>(behaviorContext, "DVector3");
            ReflectGlmVecBehavior<glm::dvec4>(behaviorContext, "DVector4");
            ReflectGlmQuatBehavior(behaviorContext);

        }
    }

    template<typename VecType>
    void MathSerialization::ReflectGlmVecBehavior(AZ::BehaviorContext* behaviorContext, const AZStd::string& name)
    {
        auto classBuilder = behaviorContext->Class<VecType>(name.c_str())
                                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                                ->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value);
        for (std::size_t i = 0; i < VecType::length(); ++i)
        {
            if (i == 0)
            {
                classBuilder->Property(
                    "X",
                    [](const VecType* vec)
                    {
                        return (*vec)[0];
                    },
                    [](VecType* vec, double x)
                    {
                        (*vec)[0] = x;
                    });
            }
            else if (i == 1)
            {
                classBuilder->Property(
                    "Y",
                    [](const VecType* vec)
                    {
                        return (*vec)[1];
                    },
                    [](VecType* vec, double y)
                    {
                        (*vec)[1] = y;
                    });
            }
            else if (i == 2)
            {
                classBuilder->Property(
                    "Z",
                    [](const VecType* vec)
                    {
                        return (*vec)[2];
                    },
                    [](VecType* vec, double z)
                    {
                        (*vec)[2] = z;
                    });
            }
            else if (i == 3)
            {
                classBuilder->Property(
                    "W",
                    [](const VecType* vec)
                    {
                        return (*vec)[3];
                    },
                    [](VecType* vec, double w)
                    {
                        (*vec)[3] = w;
                    });
            }
        }

        classBuilder
            ->Method(
                "Add",
                [](const VecType& lhs, const VecType& rhs)
                {
                    return lhs + rhs;
                })
            ->Method(
                "Subtract",
                [](const VecType& lhs, const VecType& rhs)
                {
                    return lhs - rhs;
                })
            ->Method(
                "Multiply",
                [](const VecType& lhs, const VecType& rhs)
                {
                    return lhs * rhs;
                })
            ->Method(
                "Divide",
                [](const VecType& lhs, const VecType& rhs)
                {
                    return lhs / rhs;
                })
            ->Method(
                "MultiplyByConstant",
                [](const VecType& lhs, double rhs)
                {
                    return lhs * rhs;
                })
            ->Method(
                "DivideByConstant",
                [](const VecType& lhs, double rhs)
                {
                    return lhs / rhs;
                })
            ->Method(
                "Normalized",
                [](const VecType& vec)
                {
                    return glm::normalize(vec);
                })
            ->Method(
                "Length",
                [](const VecType& vec)
                {
                    return glm::length(vec);
                })
            ->Method(
                "LengthSq",
                [](const VecType& vec)
                {
                    return glm::dot(vec, vec);
                })
            ->Method(
                "Distance",
                [](const VecType& lhs, const VecType& rhs)
                {
                    return glm::distance(lhs, rhs);
                })
            ->Method(
                "Dot",
                [](const VecType& lhs, const VecType& rhs)
                {
                    return glm::dot(lhs, rhs);
                })
            ->Method(
                "Abs",
                [](const VecType& vec)
                {
                    return glm::abs(vec);
                })
            ->Method(
                "Min",
                [](const VecType& lhs, const VecType& rhs)
                {
                    return glm::min(lhs, rhs);
                })
            ->Method(
                "Max",
                [](const VecType& lhs, const VecType& rhs)
                {
                    return glm::max(lhs, rhs);
                })
            ->Method(
                "Clamp",
                [](const VecType& x, const VecType& min, const VecType& max)
                {
                    return glm::clamp(x, min, max);
                },
                { AZ::BehaviorParameterOverrides("Value"), AZ::BehaviorParameterOverrides("Min"), AZ::BehaviorParameterOverrides("Max") })
            ->Method(
                "Equal",
                [](const VecType& lhs, const VecType& rhs)
                {
                    return lhs == rhs;
                })
            ;
    }

    void MathSerialization::ReflectGlmQuatBehavior(AZ::BehaviorContext* behaviorContext)
    {
        behaviorContext->Class<glm::dquat>("DQuaternion")
            ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
            ->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
            ->Property(
                "X",
                [](const glm::dquat* quat)
                {
                    return (*quat)[0];
                },
                [](glm::dquat* quat, double x)
                {
                    (*quat)[0] = x;
                })
            ->Property(
                "Y",
                [](const glm::dquat* quat)
                {
                    return (*quat)[1];
                },
                [](glm::dquat* quat, double y)
                {
                    (*quat)[1] = y;
                })
            ->Property(
                "Z",
                [](const glm::dquat* quat)
                {
                    return (*quat)[2];
                },
                [](glm::dquat* quat, double z)
                {
                    (*quat)[2] = z;
                })
            ->Property(
                "W",
                [](const glm::dquat* quat)
                {
                    return (*quat)[3];
                },
                [](glm::dquat* quat, double w)
                {
                    (*quat)[3] = w;
                })
            ->Method(
                "Add",
                [](const glm::dquat& lhs, const glm::dquat& rhs)
                {
                    return lhs + rhs;
                })
            ->Method(
                "Subtract",
                [](const glm::dquat& lhs, const glm::dquat& rhs)
                {
                    return lhs - rhs;
                })
            ->Method(
                "Multiply",
                [](const glm::dquat& lhs, const glm::dquat& rhs)
                {
                    return lhs * rhs;
                })
            ->Method(
                "MultiplyByConstant",
                [](const glm::dquat& lhs, double rhs)
                {
                    return lhs * rhs;
                })
            ->Method(
                "DivideByConstant",
                [](const glm::dquat& lhs, double rhs)
                {
                    return lhs / rhs;
                })
            ->Method(
                "MultiplyByDVector3",
                [](const glm::dquat& lhs, const glm::dvec3& rhs)
                {
                    return lhs * rhs;
                })
            ->Method(
                "MultiplyByDVector4",
                [](const glm::dquat& lhs, const glm::dvec4& rhs)
                {
                    return lhs * rhs;
                })
            ->Method(
                "Normalized",
                [](const glm::dquat& quat)
                {
                    return glm::normalize(quat);
                })
            ->Method(
                "Length",
                [](const glm::dquat& quat)
                {
                    return glm::length(quat);
                })
            ->Method(
                "LengthSq",
                [](const glm::dquat& quat)
                {
                    return glm::dot(quat, quat);
                })
            ->Method(
                "Dot",
                [](const glm::dquat& lhs, const glm::dquat& rhs)
                {
                    return glm::dot(lhs, rhs);
                })
            ->Method(
                "Conjugate",
                [](const glm::dquat& quat)
                {
                    return glm::conjugate(quat);
                })
            ->Method(
                "Inverse",
                [](const glm::dquat& quat)
                {
                    return glm::inverse(quat);
                })
            ->Method(
                "EulerAngles",
                [](const glm::dquat& quat)
                {
                    return glm::eulerAngles(quat);
                })
            ->Method(
                "Yaw",
                [](const glm::dquat& quat)
                {
                    return glm::yaw(quat);
                })
            ->Method(
                "Pitch",
                [](const glm::dquat& quat)
                {
                    return glm::pitch(quat);
                })
            ->Method(
                "Roll",
                [](const glm::dquat& quat)
                {
                    return glm::roll(quat);
                })
            ->Method(
                "Lerp",
                [](const glm::dquat& x, const glm::dquat& y, double time)
                {
                    return glm::lerp(x, y, time);
                },
                { AZ::BehaviorParameterOverrides("X"), AZ::BehaviorParameterOverrides("Y"), AZ::BehaviorParameterOverrides("Time") })
            ->Method(
                "Slerp",
                [](const glm::dquat& x, const glm::dquat& y, double time)
                {
                    return glm::slerp(x, y, time);
                },
                { AZ::BehaviorParameterOverrides("X"), AZ::BehaviorParameterOverrides("Y"), AZ::BehaviorParameterOverrides("Time") })
            ->Method(
                "Equal",
                [](const glm::dquat& lhs, const glm::dquat& rhs)
                {
                    return lhs == rhs;
                });
    }

} // namespace Cesium

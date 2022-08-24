#include <Cesium/Math/MathReflect.h>
#include <Cesium/Math/MathHelper.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector4.h>
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
            serializeContext->Class<glm::dmat2>()->Serializer<GlmDMatSerializer<glm::dmat2>>();
            serializeContext->Class<glm::dmat3>()->Serializer<GlmDMatSerializer<glm::dmat3>>();
            serializeContext->Class<glm::dmat4>()->Serializer<GlmDMatSerializer<glm::dmat4>>();
        }

        AZ::JsonRegistrationContext* jsonRegistrationContext = azrtti_cast<AZ::JsonRegistrationContext*>(context);
        if (jsonRegistrationContext)
        {
            jsonRegistrationContext->Serializer<GlmDVec2JsonSerializer>()->HandlesType<glm::dvec2>();
            jsonRegistrationContext->Serializer<GlmDVec3JsonSerializer>()->HandlesType<glm::dvec3>();
            jsonRegistrationContext->Serializer<GlmDVec4JsonSerializer>()->HandlesType<glm::dvec4>();
            jsonRegistrationContext->Serializer<GlmDQuatJsonSerializer>()->HandlesType<glm::dquat>();
            jsonRegistrationContext->Serializer<GlmDMat2JsonSerializer>()->HandlesType<glm::dmat2>();
            jsonRegistrationContext->Serializer<GlmDMat3JsonSerializer>()->HandlesType<glm::dmat3>();
            jsonRegistrationContext->Serializer<GlmDMat4JsonSerializer>()->HandlesType<glm::dmat4>();
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            ReflectGlmVecBehavior<glm::dvec2>(behaviorContext, "DVector2");
            ReflectGlmVecBehavior<glm::dvec3>(behaviorContext, "DVector3");
            ReflectGlmVecBehavior<glm::dvec4>(behaviorContext, "DVector4");
            ReflectGlmQuatBehavior(behaviorContext);
            ReflectGlmDMatBehavior<glm::dmat2>(behaviorContext, "DMatrix2");
            ReflectGlmDMatBehavior<glm::dmat3>(behaviorContext, "DMatrix3");
            ReflectGlmDMatBehavior<glm::dmat4>(behaviorContext, "DMatrix4");
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
            ->Method(
                "CreateIdentity",
                []()
                {
                    return VecType{ 1.0 };
                })
            ->Method(
                "CreateZero",
                []()
                {
                    return VecType{ 0.0 };
                })
            ->Method(
                "CreateFromConstant",
                [](double c)
                {
                    return VecType{ c };
                });

        ReflectGlmVector(*classBuilder);
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
                "CreateFromDMat3",
                [](const glm::dmat3* rotation)
                {
                    return glm::dquat(*rotation);
                })
            ->Method(
                "CreateFromDMat4",
                [](const glm::dmat4* rotation)
                {
                    return glm::dquat(*rotation);
                })
            ->Method(
                "Create",
                [](double x, double y, double z, double w)
                {
                    return glm::dquat(w, x, y, z);
                })
            ->Method(
                "CreateFromO3DEQuaternion",
                [](const AZ::Quaternion& quaternion)
                {
                    return MathHelper::ToDQuaternion(quaternion);
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
                "ToDMatrix4",
                [](const glm::dquat& quat)
                {
                    return glm::mat4_cast(quat);
                })
            ->Method(
                "ToDMatrix3",
                [](const glm::dquat& quat)
                {
                    return glm::mat3_cast(quat);
                })
            ->Method(
                "ToO3DEQuaternion",
                [](const glm::dquat& quat)
                {
                    return AZ::Quaternion(
                        static_cast<float>(quat.x), static_cast<float>(quat.y), static_cast<float>(quat.z), static_cast<float>(quat.w));
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

    void MathSerialization::ReflectGlmVector(AZ::BehaviorContext::ClassBuilder<glm::dvec2>& builder)
    {
        builder
            .Method(
                "Create",
                [](double x, double y)
                {
                    return glm::dvec2(x, y);
                },
                { AZ::BehaviorParameterOverrides("X"), AZ::BehaviorParameterOverrides("Y") })
            ->Method(
                "CreateFromDVector3",
                [](const glm::dvec3& v)
                {
                    return glm::dvec2(v);
                })
            ->Method(
                "CreateFromDVector4",
                [](const glm::dvec4& v)
                {
                    return glm::dvec2(v);
                })
            ->Method(
                "CreateFromO3DEVector2",
                [](const AZ::Vector2& v)
                {
                    return glm::dvec2(v.GetX(), v.GetY());
                })
            ->Method(
                "CreateFromO3DEVector3",
                [](const AZ::Vector3& v)
                {
                    return glm::dvec2(v.GetX(), v.GetY());
                })
            ->Method(
                "CreateFromO3DEVector4",
                [](const AZ::Vector4& v)
                {
                    return glm::dvec2(v.GetX(), v.GetY());
                })
            ->Method(
                "ToO3DEVector2",
                [](const glm::dvec2& v)
                {
                    return AZ::Vector2(static_cast<float>(v.x), static_cast<float>(v.y));
                });
    }

    void MathSerialization::ReflectGlmVector(AZ::BehaviorContext::ClassBuilder<glm::dvec3>& builder)
    {
        builder
            .Method(
                "Create",
                [](double x, double y, double z)
                {
                    return glm::dvec3(x, y, z);
                },
                { AZ::BehaviorParameterOverrides("X"), AZ::BehaviorParameterOverrides("Y"), AZ::BehaviorParameterOverrides("Z") })
            ->Method(
                "CreateFromDVector2",
                [](const glm::dvec2& v, double z)
                {
                    return glm::dvec3(v, z);
                },
                { AZ::BehaviorParameterOverrides("DVector2"), AZ::BehaviorParameterOverrides("Z") })
            ->Method(
                "CreateFromDVector4",
                [](const glm::dvec4& v)
                {
                    return glm::dvec3(v);
                })
            ->Method(
                "CreateFromO3DEVector2",
                [](const AZ::Vector2& v, double z)
                {
                    return glm::dvec3(v.GetX(), v.GetY(), z);
                },
                { AZ::BehaviorParameterOverrides("Vector2"), AZ::BehaviorParameterOverrides("Z") })
            ->Method(
                "CreateFromO3DEVector3",
                [](const AZ::Vector3& v)
                {
                    return glm::dvec3(v.GetX(), v.GetY(), v.GetZ());
                })
            ->Method(
                "CreateFromO3DEVector4",
                [](const AZ::Vector4& v)
                {
                    return glm::dvec3(v.GetX(), v.GetY(), v.GetZ());
                })
            ->Method(
                "ToO3DEVector3",
                [](const glm::dvec3& v)
                {
                    return AZ::Vector3(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z));
                });
    }

    void MathSerialization::ReflectGlmVector(AZ::BehaviorContext::ClassBuilder<glm::dvec4>& builder)
    {
        builder
            .Method(
                "Create",
                [](double x, double y, double z, double w)
                {
                    return glm::dvec4(x, y, z, w);
                },
                { AZ::BehaviorParameterOverrides("X"), AZ::BehaviorParameterOverrides("Y"), AZ::BehaviorParameterOverrides("Z"),
                  AZ::BehaviorParameterOverrides("W") })
            ->Method(
                "CreateFromDVector3",
                [](const glm::dvec3& v, double w)
                {
                    return glm::dvec4(v, w);
                },
                { AZ::BehaviorParameterOverrides("DVector3"), AZ::BehaviorParameterOverrides("W") })
            ->Method(
                "CreateFromO3DEVector3",
                [](const AZ::Vector3& v, double w)
                {
                    return glm::dvec4(v.GetX(), v.GetY(), v.GetZ(), w);
                })
            ->Method(
                "CreateFromO3DEVector4",
                [](const AZ::Vector4& v)
                {
                    return glm::dvec4(v.GetX(), v.GetY(), v.GetZ(), v.GetW());
                })
            ->Method(
                "ToO3DEVector4",
                [](const glm::dvec4& v)
                {
                    return AZ::Vector4(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z), static_cast<float>(v.w));
                });
        ;
    }

    template<typename MatType>
    void MathSerialization::ReflectGlmDMatBehavior(AZ::BehaviorContext* behaviorContext, const AZStd::string& name)
    {
        auto classBuilder = behaviorContext->Class<MatType>(name.c_str())
                                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                                ->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
                                ->Method(
                                    "GetColumn",
                                    [](const MatType& mat, typename MatType::length_type col)
                                    {
                                        return mat[col];
                                    },
                                    { AZ::BehaviorParameterOverrides("Matrix"), AZ::BehaviorParameterOverrides("ColumnIndex") })
                                ->Method(
                                    "SetColumn",
                                    [](MatType& mat, typename MatType::length_type col, const typename MatType::col_type& colValue)
                                    {
                                        mat[col] = colValue;
                                    },
                                    { AZ::BehaviorParameterOverrides("Matrix"), AZ::BehaviorParameterOverrides("ColumnIndex"),
                                      AZ::BehaviorParameterOverrides("ColumnValue") })
                                ->Method(
                                    "GetElement",
                                    [](const MatType& mat, typename MatType::length_type col, typename MatType::length_type row)
                                    {
                                        return mat[col][row];
                                    },
                                    { AZ::BehaviorParameterOverrides("Matrix"), AZ::BehaviorParameterOverrides("ColumnIndex"),
                                      AZ::BehaviorParameterOverrides("RowIndex") })
                                ->Method(
                                    "SetElement",
                                    [](MatType& mat, typename MatType::length_type col, typename MatType::length_type row, double value)
                                    {
                                        mat[col][row] = value;
                                    },
                                    { AZ::BehaviorParameterOverrides("Matrix"), AZ::BehaviorParameterOverrides("ColumnIndex"),
                                      AZ::BehaviorParameterOverrides("RowIndex"), AZ::BehaviorParameterOverrides("Value") })
                                ->Method(
                                    "Add",
                                    [](const MatType& lhs, const MatType& rhs)
                                    {
                                        return lhs + rhs;
                                    })
                                ->Method(
                                    "Subtract",
                                    [](const MatType& lhs, const MatType& rhs)
                                    {
                                        return lhs - rhs;
                                    })
                                ->Method(
                                    "Multiply",
                                    [](const MatType& lhs, const MatType& rhs)
                                    {
                                        return lhs * rhs;
                                    })
                                ->Method(
                                    "MultiplyByConstant",
                                    [](const MatType& lhs, double rhs)
                                    {
                                        return lhs * rhs;
                                    })
                                ->Method(
                                    "DivideByConstant",
                                    [](const MatType& lhs, double rhs)
                                    {
                                        return lhs / rhs;
                                    })
                                ->Method(
                                    "MultiplyByVector",
                                    [](const MatType& lhs, const typename MatType::col_type& rhs)
                                    {
                                        return lhs * rhs;
                                    })
                                ->Method(
                                    "MultiplyVectorWithMatrix",
                                    [](const typename MatType::col_type& lhs, const MatType& rhs)
                                    {
                                        return lhs * rhs;
                                    })
                                ->Method(
                                    "Inverse",
                                    [](const MatType& mat)
                                    {
                                        return glm::inverse(mat);
                                    })
                                ->Method(
                                    "Transpose",
                                    [](const MatType& mat)
                                    {
                                        return glm::transpose(mat);
                                    })
                                ->Method(
                                    "Determinant",
                                    [](const MatType& mat)
                                    {
                                        return glm::determinant(mat);
                                    })
                                ->Method(
                                    "CreateIdentity",
                                    []()
                                    {
                                        return MatType{ 1.0 };
                                    })
                                ->Method(
                                    "CreateDiagonal",
                                    [](double c)
                                    {
                                        return MatType{ c };
                                    })
                                ->Method(
                                    "Equal",
                                    [](const MatType& lhs, const MatType& rhs)
                                    {
                                        return lhs == rhs;
                                    });

        ReflectGlmMatrix(*classBuilder);
    }

    void MathSerialization::ReflectGlmMatrix([[maybe_unused]] AZ::BehaviorContext::ClassBuilder<glm::dmat2>& builder)
    {
    }

    void MathSerialization::ReflectGlmMatrix([[maybe_unused]] AZ::BehaviorContext::ClassBuilder<glm::dmat3>& builder)
    {
    }

    void MathSerialization::ReflectGlmMatrix(AZ::BehaviorContext::ClassBuilder<glm::dmat4>& builder)
    {
        builder
            .Method(
                "Translate",
                [](const glm::dmat4& mat, const glm::dvec3& translate)
                {
                    return glm::translate(mat, translate);
                },
                { AZ::BehaviorParameterOverrides("Matrix"), AZ::BehaviorParameterOverrides("Translate") })
            ->Method(
                "Rotate",
                [](const glm::dmat4& mat, double angle, const glm::dvec3& axis)
                {
                    return glm::rotate(mat, angle, axis);
                },
                { AZ::BehaviorParameterOverrides("Matrix"), AZ::BehaviorParameterOverrides("Angle"),
                  AZ::BehaviorParameterOverrides("Axis") })
            ->Method(
                "Scale",
                [](const glm::dmat4& mat, const glm::dvec3& scale)
                {
                    return glm::scale(mat, scale);
                },
                { AZ::BehaviorParameterOverrides("Matrix"), AZ::BehaviorParameterOverrides("Scale") })
            ->Method(
                "CreateFromO3DETransformAndNonUniformScale",
                [](const AZ::Transform& transform, const AZ::Vector3& nonUniformScale)
                {
                    return MathHelper::ConvertTransformAndScaleToDMat4(transform, nonUniformScale);
                },
                { AZ::BehaviorParameterOverrides("Transform"),
                  AZ::BehaviorParameterOverrides("NonUniformScale", "", new AZ::BehaviorDefaultValue(AZ::Vector3(1.0, 1.0, 1.0))) });
    }
} // namespace Cesium

#include "Cesium/Gltf/GltfPrimitiveBuilder.h"
#include "Cesium/Gltf/BitangentAndTangentGenerator.h"
#include "Cesium/Systems/CesiumSystem.h"
#include "Cesium/Systems/CriticalAssetManager.h"
#include "Cesium/Math/MathHelper.h"
#include <Atom/RPI.Reflect/Model/ModelAsset.h>
#include <Atom/RPI.Reflect/Buffer/BufferAsset.h>
#include <Atom/RPI.Reflect/Model/ModelLodAsset.h>
#include <Atom/RPI.Reflect/Buffer/BufferAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelLodAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelAssetCreator.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/limits.h>

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#include <AzCore/PlatformDef.h>
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>
#include <CesiumGltf/MeshPrimitive.h>
#include <CesiumGltf/AccessorView.h>
#include <CesiumUtility/Math.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

#include <cassert>
#include <cstdint>
#include <numeric>

namespace Cesium
{
    struct GltfTrianglePrimitiveBuilder::CommonAccessorViews final
    {
        CommonAccessorViews(const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive)
            : m_positionAccessor{ nullptr }
        {
            // retrieve required positions first
            auto positionAttribute = primitive.attributes.find("POSITION");
            if (positionAttribute == primitive.attributes.end())
            {
                return;
            }

            m_positionAccessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, positionAttribute->second);
            if (!m_positionAccessor)
            {
                return;
            }

            m_positions = CesiumGltf::AccessorView<glm::vec3>(model, *m_positionAccessor);
            if (m_positions.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return;
            }

            // get normal view
            auto normalAttribute = primitive.attributes.find("NORMAL");
            if (normalAttribute != primitive.attributes.end())
            {
                m_normals = CesiumGltf::AccessorView<glm::vec3>(model, normalAttribute->second);
            }

            // get tangent
            auto tangentAttribute = primitive.attributes.find("TANGENT");
            if (tangentAttribute != primitive.attributes.end())
            {
                m_tangents = CesiumGltf::AccessorView<glm::vec4>(model, tangentAttribute->second);
            }
        }

        const CesiumGltf::Accessor* m_positionAccessor;
        CesiumGltf::AccessorView<glm::vec3> m_positions;
        CesiumGltf::AccessorView<glm::vec3> m_normals;
        CesiumGltf::AccessorView<glm::vec4> m_tangents;
    };

    GltfTrianglePrimitiveBuilder::LoadContext::LoadContext()
        : m_generateFlatNormal{ false }
        , m_generateTangent{ false }
        , m_generateUnIndexedMesh{ false }
    {
    }

    GltfTrianglePrimitiveBuilder::VertexRawBuffer::VertexRawBuffer()
        : m_buffer{}
        , m_format{ AZ::RHI::Format::Unknown }
        , m_elementCount{ 0 }
    {
    }

    GltfTrianglePrimitiveBuilder::VertexCustomAttribute::VertexCustomAttribute(
        const GltfShaderVertexAttribute& shaderAttribute, VertexRawBuffer&& buffer)
        : m_shaderAttribute{ shaderAttribute }
        , m_buffer{ std::move(buffer) }
    {
    }

    void GltfTrianglePrimitiveBuilder::Create(
        const CesiumGltf::Model& model,
        const CesiumGltf::MeshPrimitive& primitive,
        const GltfLoadMaterial& material,
        GltfLoadPrimitive& result)
    {
        Reset();

        // Construct common accessor views. This is needed to begin determine loading context
        CommonAccessorViews commonAccessorViews{ model, primitive };
        if (commonAccessorViews.m_positions.status() != CesiumGltf::AccessorViewStatus::Valid)
        {
            return;
        }

        if (commonAccessorViews.m_positions.size() == 0)
        {
            return;
        }

        // construct bounding volume
        auto positionAccessor = commonAccessorViews.m_positionAccessor;
        AZ::Aabb aabb = AZ::Aabb::CreateNull();
        if (positionAccessor->min.size() == 3 && positionAccessor->max.size() == 3)
        {
            aabb = AZ::Aabb::CreateFromMinMaxValues(
                static_cast<float>(positionAccessor->min[0]), static_cast<float>(positionAccessor->min[1]),
                static_cast<float>(positionAccessor->min[2]), static_cast<float>(positionAccessor->max[0]),
                static_cast<float>(positionAccessor->max[1]), static_cast<float>(positionAccessor->max[2]));
        }
        else
        {
            aabb = CreateAabbFromPositions(commonAccessorViews.m_positions);
        }

        // set indices
        if (!CreateIndices(commonAccessorViews, model, primitive))
        {
            return;
        }

        // We should expect indices size is a multiple of 3
        if (m_indices.size() % 3 != 0)
        {
            return;
        }

        // determine loading context
        DetermineLoadContext(commonAccessorViews, material);

        // Create attributes. The order call of the functions is important
        CreatePositionsAttribute(commonAccessorViews);
        CreateNormalsAttribute(commonAccessorViews);
        CreateUVsAttributes(commonAccessorViews, model, primitive);
        CreateTangentsAndBitangentsAttributes(commonAccessorViews);
        CreateCustomAttributes(model, primitive, material);

        // after retrieving all the attributes, we reindex the indices if it's un-indexed mesh
        if (m_context.m_generateUnIndexedMesh)
        {
            std::iota(m_indices.begin(), m_indices.end(), 0);
        }

        // calculate buffer view descriptor for each attribute and total buffer size to store all of them
        // in a single buffer
        auto positionBufferViewDescriptor =
            AZ::RHI::BufferViewDescriptor::CreateTyped(0, static_cast<std::uint32_t>(m_positions.size()), AZ::RHI::Format::R32G32B32_FLOAT);
        std::size_t totalBufferSize = m_positions.size() * sizeof(glm::vec3);

        std::size_t normalByteOffset = MathHelper::Align(totalBufferSize, sizeof(glm::vec3));
        auto normalBufferViewDescriptor = AZ::RHI::BufferViewDescriptor::CreateTyped(
            static_cast<std::uint32_t>(normalByteOffset / sizeof(glm::vec3)), static_cast<std::uint32_t>(m_normals.size()),
            AZ::RHI::Format::R32G32B32_FLOAT);
        totalBufferSize = normalByteOffset + m_normals.size() * sizeof(glm::vec3);

        std::size_t bitangentByteOffset = MathHelper::Align(totalBufferSize, sizeof(glm::vec3));
        auto bitangentBufferViewDescriptor = AZ::RHI::BufferViewDescriptor::CreateTyped(
            static_cast<std::uint32_t>(bitangentByteOffset / sizeof(glm::vec3)), static_cast<std::uint32_t>(m_bitangents.size()),
            AZ::RHI::Format::R32G32B32_FLOAT);
        totalBufferSize = bitangentByteOffset + m_bitangents.size() * sizeof(glm::vec3);

        std::size_t tangentByteOffset = MathHelper::Align(totalBufferSize, sizeof(glm::vec4));
        auto tangentBufferViewDescriptor = AZ::RHI::BufferViewDescriptor::CreateTyped(
            static_cast<std::uint32_t>(tangentByteOffset / sizeof(glm::vec4)), static_cast<std::uint32_t>(m_tangents.size()),
            AZ::RHI::Format::R32G32B32A32_FLOAT);
        totalBufferSize = tangentByteOffset + m_tangents.size() * sizeof(glm::vec4);

        AZStd::array<AZ::RHI::BufferViewDescriptor, 2> uvBufferViewDescriptors;
        for (std::size_t i = 0; i < uvBufferViewDescriptors.size(); ++i)
        {
            if (!m_uvs[i].m_buffer.empty())
            {
                std::size_t formatSize = AZ::RHI::GetFormatSize(m_uvs[i].m_format);
                std::size_t offset = MathHelper::Align(totalBufferSize, formatSize);
                uvBufferViewDescriptors[i] = AZ::RHI::BufferViewDescriptor::CreateTyped(
                    static_cast<std::uint32_t>(offset / formatSize), static_cast<std::uint32_t>(m_uvs[i].m_elementCount),
                    m_uvs[i].m_format);
                totalBufferSize = offset + m_uvs[i].m_buffer.size();
            }
            else
            {
                // since this UVs buffer is empty, we just assign its region to tangent buffer as dummy buffer since we don't
                // care about its value anyway and tangent offset is also a multiple of R32G32_FLOAT size
                std::size_t formatSize = AZ::RHI::GetFormatSize(AZ::RHI::Format::R32G32_FLOAT);
                std::size_t offset = tangentByteOffset;
                uvBufferViewDescriptors[i] = AZ::RHI::BufferViewDescriptor::CreateTyped(
                    static_cast<std::uint32_t>(offset / formatSize), static_cast<std::uint32_t>(m_tangents.size()),
                    AZ::RHI::Format::R32G32_FLOAT);
            }
        }

        AZStd::vector<AZ::RHI::BufferViewDescriptor> customAttribBufferViewDescriptors;
        if (!m_customAttributes.empty())
        {
            customAttribBufferViewDescriptors.reserve(m_customAttributes.size());
            for (const auto& customAttribute : m_customAttributes)
            {
                std::size_t formatSize = AZ::RHI::GetFormatSize(customAttribute.m_buffer.m_format);
                std::size_t offset = MathHelper::Align(totalBufferSize, formatSize);
                customAttribBufferViewDescriptors.emplace_back(AZ::RHI::BufferViewDescriptor::CreateTyped(
                    static_cast<std::uint32_t>(offset / formatSize), static_cast<std::uint32_t>(customAttribute.m_buffer.m_elementCount),
                    customAttribute.m_buffer.m_format));
                totalBufferSize = offset + customAttribute.m_buffer.m_buffer.size();
            }
        }

        std::size_t offset = MathHelper::Align(totalBufferSize, sizeof(std::uint32_t));
        auto indicesBufferViewDescriptor = AZ::RHI::BufferViewDescriptor::CreateTyped(
            static_cast<std::uint32_t>(offset / sizeof(std::uint32_t)), static_cast<std::uint32_t>(m_indices.size()),
            AZ::RHI::Format::R32_UINT);
        totalBufferSize = offset + m_indices.size() * sizeof(std::uint32_t);

        // populate the raw buffer with attributes data
        AZStd::vector<std::byte> buffer;
        buffer.resize_no_construct(totalBufferSize);
        CopySubregionBuffer(buffer, m_indices.data(), indicesBufferViewDescriptor);
        CopySubregionBuffer(buffer, m_positions.data(), positionBufferViewDescriptor);
        CopySubregionBuffer(buffer, m_normals.data(), normalBufferViewDescriptor);
        CopySubregionBuffer(buffer, m_bitangents.data(), bitangentBufferViewDescriptor);
        CopySubregionBuffer(buffer, m_tangents.data(), tangentBufferViewDescriptor);

        for (std::size_t i = 0; i < m_uvs.size(); ++i)
        {
            if (!m_uvs[i].m_buffer.empty())
            {
                CopySubregionBuffer(buffer, m_uvs[i].m_buffer.data(), uvBufferViewDescriptors[i]);
            }
        }

        for (std::size_t i = 0; i < m_customAttributes.size(); ++i)
        {
            if (!m_customAttributes[i].m_buffer.m_buffer.empty())
            {
                CopySubregionBuffer(buffer, m_customAttributes[i].m_buffer.m_buffer.data(), customAttribBufferViewDescriptors[i]);
            }
        }

        AZ::Data::Asset<AZ::RPI::BufferAsset> bufferAsset = CreateBufferAsset(buffer);

        // create LOD asset
        AZ::Data::AssetId lodAssetId = CesiumInterface::Get()->GetCriticalAssetManager().GenerateRandomAssetId();
        AZ::RPI::ModelLodAssetCreator lodCreator;
        lodCreator.Begin(lodAssetId);
        lodCreator.AddLodStreamBuffer(bufferAsset);

        // create mesh
        lodCreator.BeginMesh();
        lodCreator.SetMeshIndexBuffer(AZ::RPI::BufferAssetView(bufferAsset, indicesBufferViewDescriptor));
        lodCreator.AddMeshStreamBuffer(
            AZ::RHI::ShaderSemantic("POSITION"), AZ::Name(), AZ::RPI::BufferAssetView(bufferAsset, positionBufferViewDescriptor));
        lodCreator.AddMeshStreamBuffer(
            AZ::RHI::ShaderSemantic("NORMAL"), AZ::Name(), AZ::RPI::BufferAssetView(bufferAsset, normalBufferViewDescriptor));
        lodCreator.AddMeshStreamBuffer(
            AZ::RHI::ShaderSemantic("BITANGENT"), AZ::Name(), AZ::RPI::BufferAssetView(bufferAsset, bitangentBufferViewDescriptor));
        lodCreator.AddMeshStreamBuffer(
            AZ::RHI::ShaderSemantic("TANGENT"), AZ::Name(), AZ::RPI::BufferAssetView(bufferAsset, tangentBufferViewDescriptor));

        for (std::size_t i = 0; i < uvBufferViewDescriptors.size(); ++i)
        {
            lodCreator.AddMeshStreamBuffer(
                AZ::RHI::ShaderSemantic("UV", i), AZ::Name(), AZ::RPI::BufferAssetView(bufferAsset, uvBufferViewDescriptors[i]));
        }

        for (std::size_t i = 0; i < m_customAttributes.size(); ++i)
        {
            lodCreator.AddMeshStreamBuffer(
                m_customAttributes[i].m_shaderAttribute.m_shaderSemantic, m_customAttributes[i].m_shaderAttribute.m_shaderAttributeName,
                AZ::RPI::BufferAssetView(bufferAsset, customAttribBufferViewDescriptors[i]));
        }

        lodCreator.SetMeshAabb(std::move(aabb));
        lodCreator.EndMesh();

        AZ::Data::Asset<AZ::RPI::ModelLodAsset> lodAsset;
        lodCreator.End(lodAsset);

        // create model asset
        AZ::Data::AssetId modelAssetId = CesiumInterface::Get()->GetCriticalAssetManager().GenerateRandomAssetId();

        AZ::RPI::ModelAssetCreator modelCreator;
        modelCreator.Begin(modelAssetId);
        modelCreator.AddLodAsset(std::move(lodAsset));

        AZ::Data::Asset<AZ::RPI::ModelAsset> modelAsset;
        modelCreator.End(modelAsset);

        result.m_modelAsset = std::move(modelAsset);
        result.m_materialId = primitive.material;
    }

    void GltfTrianglePrimitiveBuilder::DetermineLoadContext(const CommonAccessorViews& accessorViews, const GltfLoadMaterial& material)
    {
        // check if we should generate normal
        bool isNormalAccessorValid = accessorViews.m_normals.status() == CesiumGltf::AccessorViewStatus::Valid;
        bool hasEnoughNormalVertices = accessorViews.m_normals.size() == accessorViews.m_positions.size();
        m_context.m_generateFlatNormal = !isNormalAccessorValid || !hasEnoughNormalVertices;

        // check if we should generate tangent
        if (material.m_needTangents)
        {
            bool isTangentAccessorValid = accessorViews.m_tangents.status() == CesiumGltf::AccessorViewStatus::Valid;
            bool hasEnoughTangentVertices = accessorViews.m_tangents.size() == accessorViews.m_positions.size();
            m_context.m_generateTangent = !isTangentAccessorValid || !hasEnoughTangentVertices;
        }
        else
        {
            m_context.m_generateTangent = false;
        }

        // check if we should generate unindexed mesh
        m_context.m_generateUnIndexedMesh = m_context.m_generateFlatNormal || m_context.m_generateTangent;
    }

    template<typename AccessorType>
    void GltfTrianglePrimitiveBuilder::CopyAccessorToBuffer(
        const CesiumGltf::AccessorView<AccessorType>& attributeAccessorView, AZStd::vector<AccessorType>& attributes)
    {
        if (m_context.m_generateUnIndexedMesh)
        {
            // mesh has indices
            attributes.resize(m_indices.size());
            for (std::size_t i = 0; i < m_indices.size(); ++i)
            {
                std::int64_t index = static_cast<std::int64_t>(m_indices[i]);
                attributes[static_cast<std::size_t>(i)] = attributeAccessorView[index];
            }
        }
        else
        {
            attributes.resize(static_cast<std::size_t>(attributeAccessorView.size()));
            for (std::int64_t i = 0; i < attributeAccessorView.size(); ++i)
            {
                attributes[static_cast<std::size_t>(i)] = attributeAccessorView[i];
            }
        }
    }

    template<typename AccessorType>
    void GltfTrianglePrimitiveBuilder::CopyAccessorToBuffer(
        const CesiumGltf::AccessorView<AccessorType>& accessorView, AZStd::vector<std::byte>& buffer)
    {
        if (m_context.m_generateUnIndexedMesh)
        {
            buffer.resize(m_indices.size() * sizeof(AccessorType));
            AccessorType* value = reinterpret_cast<AccessorType*>(buffer.data());
            for (std::size_t i = 0; i < m_indices.size(); ++i)
            {
                std::int64_t index = static_cast<std::int64_t>(m_indices[i]);
                value[i] = accessorView[index];
            }
        }
        else
        {
            buffer.resize(static_cast<std::size_t>(accessorView.size()) * sizeof(AccessorType));
            AccessorType* value = reinterpret_cast<AccessorType*>(buffer.data());
            for (std::int64_t i = 0; i < accessorView.size(); ++i)
            {
                value[i] = accessorView[i];
            }
        }
    }

    AZ::Data::Asset<AZ::RPI::BufferAsset> GltfTrianglePrimitiveBuilder::CreateBufferAsset(const AZStd::vector<std::byte>& buffer)
    {
        AZ::RHI::BufferViewDescriptor bufferViewDescriptor;
        bufferViewDescriptor.m_elementOffset = 0;
        bufferViewDescriptor.m_elementCount = static_cast<std::uint32_t>(buffer.size());
        bufferViewDescriptor.m_elementSize = sizeof(std::uint8_t);
        bufferViewDescriptor.m_elementFormat = AZ::RHI::Format::R8_UINT;

        AZ::RHI::BufferDescriptor bufferDescriptor;
        bufferDescriptor.m_bindFlags = AZ::RHI::BufferBindFlags::InputAssembly | AZ::RHI::BufferBindFlags::ShaderRead;
        bufferDescriptor.m_byteCount = bufferViewDescriptor.m_elementCount * bufferViewDescriptor.m_elementSize;

        AZ::Data::AssetId bufferAssetId = CesiumInterface::Get()->GetCriticalAssetManager().GenerateRandomAssetId();

        AZ::RPI::BufferAssetCreator creator;
        creator.Begin(bufferAssetId);
        creator.SetBuffer(buffer.data(), bufferDescriptor.m_byteCount, bufferDescriptor);
        creator.SetBufferViewDescriptor(bufferViewDescriptor);
        creator.SetUseCommonPool(AZ::RPI::CommonBufferPoolType::StaticInputAssembly);

        AZ::Data::Asset<AZ::RPI::BufferAsset> bufferAsset;
        creator.End(bufferAsset);

        return bufferAsset;
    }

    bool GltfTrianglePrimitiveBuilder::CreateIndices(
        const CommonAccessorViews& accessorViews, const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive)
    {
        const CesiumGltf::Accessor* indicesAccessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, primitive.indices);
        if (!indicesAccessor)
        {
            m_indices.resize(static_cast<std::size_t>(accessorViews.m_positions.size()));
            std::iota(m_indices.begin(), m_indices.end(), 0);
            return true;
        }

        // Gltf primitive says it is an indexed mesh, but if the view of the accessor is invalid, we will terminate the loader right away.
        if (indicesAccessor->type != CesiumGltf::AccessorSpec::Type::SCALAR)
        {
            return false;
        }

        if (indicesAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE)
        {
            CesiumGltf::AccessorView<std::uint8_t> view{ model, *indicesAccessor };
            return CreateIndices(primitive, view);
        }
        else if (indicesAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT)
        {
            CesiumGltf::AccessorView<std::uint16_t> view{ model, *indicesAccessor };
            return CreateIndices(primitive, view);
        }
        else if (indicesAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_INT)
        {
            CesiumGltf::AccessorView<std::uint32_t> view{ model, *indicesAccessor };
            return CreateIndices(primitive, view);
        }

        return false;
    }

    template<typename IndexType>
    bool GltfTrianglePrimitiveBuilder::CreateIndices(
        const CesiumGltf::MeshPrimitive& primitive, const CesiumGltf::AccessorView<IndexType>& indicesAccessorView)
    {
        if (indicesAccessorView.status() != CesiumGltf::AccessorViewStatus::Valid)
        {
            return false;
        }

        if (primitive.mode == CesiumGltf::MeshPrimitive::Mode::TRIANGLES)
        {
            if (indicesAccessorView.size() % 3 != 0)
            {
                return false;
            }

            m_indices.resize(static_cast<std::size_t>(indicesAccessorView.size()));
            for (std::int64_t i = 0; i < indicesAccessorView.size(); ++i)
            {
                m_indices[static_cast<std::size_t>(i)] = static_cast<std::uint32_t>(indicesAccessorView[i]);
            }

            return true;
        }

        if (primitive.mode == CesiumGltf::MeshPrimitive::Mode::TRIANGLE_STRIP)
        {
            if (indicesAccessorView.size() <= 2)
            {
                return false;
            }

            m_indices.reserve(static_cast<std::size_t>(indicesAccessorView.size() - 2) * 3);
            for (std::int64_t i = 0; i < indicesAccessorView.size() - 2; ++i)
            {
                if (i % 2)
                {
                    m_indices.emplace_back(static_cast<std::uint32_t>(indicesAccessorView[i]));
                    m_indices.emplace_back(static_cast<std::uint32_t>(indicesAccessorView[i + 2]));
                    m_indices.emplace_back(static_cast<std::uint32_t>(indicesAccessorView[i + 1]));
                }
                else
                {
                    m_indices.emplace_back(static_cast<std::uint32_t>(indicesAccessorView[i]));
                    m_indices.emplace_back(static_cast<std::uint32_t>(indicesAccessorView[i + 1]));
                    m_indices.emplace_back(static_cast<std::uint32_t>(indicesAccessorView[i + 2]));
                }
            }

            return true;
        }

        if (primitive.mode == CesiumGltf::MeshPrimitive::Mode::TRIANGLE_FAN)
        {
            if (indicesAccessorView.size() <= 2)
            {
                return false;
            }

            m_indices.reserve(static_cast<std::size_t>(indicesAccessorView.size() - 2) * 3);
            for (std::int64_t i = 0; i < indicesAccessorView.size() - 2; ++i)
            {
                m_indices.emplace_back(static_cast<std::uint32_t>(indicesAccessorView[0]));
                m_indices.emplace_back(static_cast<std::uint32_t>(indicesAccessorView[i + 1]));
                m_indices.emplace_back(static_cast<std::uint32_t>(indicesAccessorView[i + 2]));
            }

            return true;
        }

        return false;
    }

    void GltfTrianglePrimitiveBuilder::CreatePositionsAttribute(const CommonAccessorViews& commonAccessorViews)
    {
        assert(commonAccessorViews.m_positions.status() == CesiumGltf::AccessorViewStatus::Valid);
        assert(commonAccessorViews.m_positions.size() > 0);
        CopyAccessorToBuffer(commonAccessorViews.m_positions, m_positions);
    }

    void GltfTrianglePrimitiveBuilder::CreateNormalsAttribute(const CommonAccessorViews& commonAccessorViews)
    {
        if (m_context.m_generateFlatNormal)
        {
            // if we are at this point, positions is already un-indexed
            assert(m_context.m_generateUnIndexedMesh);
            assert(m_positions.size() > 0);
            assert(m_positions.size() % 3 == 0);
            CreateFlatNormal();
        }
        else
        {
            assert(commonAccessorViews.m_normals.status() == CesiumGltf::AccessorViewStatus::Valid);
            assert(commonAccessorViews.m_normals.size() > 0);
            assert(commonAccessorViews.m_normals.size() == commonAccessorViews.m_positions.size());
            CopyAccessorToBuffer(commonAccessorViews.m_normals, m_normals);
        }
    }

    void GltfTrianglePrimitiveBuilder::CreateUVsAttributes(
        const CommonAccessorViews& commonAccessorViews, const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive)
    {
        for (std::size_t i = 0; i < m_uvs.size(); ++i)
        {
            auto uvAttribute = primitive.attributes.find("TEXCOORD_" + std::to_string(i));
            if (uvAttribute == primitive.attributes.end())
            {
                continue;
            }

            const CesiumGltf::Accessor* uvAccessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, uvAttribute->second);
            if (uvAccessor->type != CesiumGltf::AccessorSpec::Type::VEC2)
            {
                continue;
            }

            if (uvAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::FLOAT)
            {
                CesiumGltf::AccessorView<glm::vec2> view{ model, *uvAccessor };
                if (view.status() != CesiumGltf::AccessorViewStatus::Valid)
                {
                    continue;
                }

                if (view.size() != commonAccessorViews.m_positions.size())
                {
                    continue;
                }

                CopyAccessorToBuffer(view, m_uvs[i].m_buffer);
                m_uvs[i].m_elementCount = m_uvs[i].m_buffer.size() / (sizeof(float) * 2);
                m_uvs[i].m_format = AZ::RHI::Format::R32G32_FLOAT;
            }
            else if (uvAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE)
            {
                CesiumGltf::AccessorView<glm::u8vec2> view{ model, *uvAccessor };
                if (view.status() != CesiumGltf::AccessorViewStatus::Valid)
                {
                    continue;
                }

                if (view.size() != commonAccessorViews.m_positions.size())
                {
                    continue;
                }

                CopyAccessorToBuffer(view, m_uvs[i].m_buffer);
                m_uvs[i].m_elementCount = m_uvs[i].m_buffer.size() / (sizeof(std::uint8_t) * 2);
                m_uvs[i].m_format = AZ::RHI::Format::R8G8_UNORM;
            }
            else if (uvAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT)
            {
                CesiumGltf::AccessorView<glm::u16vec2> view{ model, *uvAccessor };
                if (view.status() != CesiumGltf::AccessorViewStatus::Valid)
                {
                    continue;
                }

                if (view.size() != commonAccessorViews.m_positions.size())
                {
                    continue;
                }

                CopyAccessorToBuffer(view, m_uvs[i].m_buffer);
                m_uvs[i].m_elementCount = m_uvs[i].m_buffer.size() / (sizeof(std::uint16_t) * 2);
                m_uvs[i].m_format = AZ::RHI::Format::R16G16_UNORM;
            }
        }
    }

    void GltfTrianglePrimitiveBuilder::CreateTangentsAndBitangentsAttributes(const CommonAccessorViews& commonAccessorViews)
    {
        if (m_context.m_generateTangent)
        {
            // positions, normals, and uvs should be unindexed at this point
            assert(m_context.m_generateUnIndexedMesh);
            assert(m_positions.size() == m_normals.size());
            assert(m_positions.size() > 0);
            assert(m_positions.size() % 3 == 0);
            assert(m_normals.size() > 0);
            assert(m_normals.size() % 3 == 0);

            // Try to generate tangents and bitangents
            bool success = false;
            for (std::size_t i = 0; i < m_uvs.size(); ++i)
            {
                if (!m_uvs[i].m_buffer.empty())
                {
                    if (m_uvs[i].m_format == AZ::RHI::Format::R32G32_FLOAT)
                    {
                        AZStd::span<glm::vec2> uvs(reinterpret_cast<glm::vec2*>(m_uvs[i].m_buffer.data()), m_uvs[i].m_elementCount);
                        success = BitangentAndTangentGenerator::Generate(m_positions, m_normals, uvs, m_tangents, m_bitangents);
                    }
                    else if (m_uvs[i].m_format == AZ::RHI::Format::R8G8_UNORM)
                    {
                        AZStd::span<glm::u8vec2> uvs(reinterpret_cast<glm::u8vec2*>(m_uvs[i].m_buffer.data()), m_uvs[i].m_elementCount);
                        success = BitangentAndTangentGenerator::Generate(m_positions, m_normals, uvs, m_tangents, m_bitangents);
                    }
                    else if (m_uvs[i].m_format == AZ::RHI::Format::R16G16_UNORM)
                    {
                        AZStd::span<glm::u16vec2> uvs(reinterpret_cast<glm::u16vec2*>(m_uvs[i].m_buffer.data()), m_uvs[i].m_elementCount);
                        success = BitangentAndTangentGenerator::Generate(m_positions, m_normals, uvs, m_tangents, m_bitangents);
                    }
                    else
                    {
                        // This code path should not happen since we check uv accessors before this function
                        assert(false);
                    }
                }

                if (success)
                {
                    break;
                }
            }

            // if we still cannot generate MikkTSpace, then we generate dummy
            if (!success)
            {
                m_tangents.resize(m_positions.size(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
                m_bitangents.resize(m_positions.size(), glm::vec3(0.0f, 1.0f, 0.0f));
            }

            return;
        }

        // check if tangents accessor is valid. If it is, we just copy to the buffer
        const CesiumGltf::AccessorView<glm::vec4>& tangents = commonAccessorViews.m_tangents;
        if ((tangents.status() == CesiumGltf::AccessorViewStatus::Valid) && (tangents.size() > 0) &&
            (tangents.size() == commonAccessorViews.m_positions.size()))
        {
            // copy tangents to vector
            CopyAccessorToBuffer(commonAccessorViews.m_tangents, m_tangents);

            // create bitangents
            m_bitangents.resize(m_tangents.size(), glm::vec3(0.0f));
            for (std::size_t i = 0; i < m_tangents.size(); ++i)
            {
                m_bitangents[i] = glm::cross(m_normals[i], glm::vec3(m_tangents[i])) * m_tangents[i].w;
            }

            return;
        }

        // generate dummy if accessor is not valid
        m_tangents.resize(m_positions.size(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        m_bitangents.resize(m_positions.size(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void GltfTrianglePrimitiveBuilder::CreateCustomAttributes(
        const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive, const GltfLoadMaterial& material)
    {
        m_customAttributes.reserve(material.m_customVertexAttributes.size());
        for (const auto& customAttribute : material.m_customVertexAttributes)
        {
            auto accessorIt = primitive.attributes.find(customAttribute.first.c_str());
            if (accessorIt == primitive.attributes.end())
            {
                continue;
            }

            const CesiumGltf::Accessor* accessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, accessorIt->second);
            if (!accessor)
            {
                continue;
            }

            if (!DoesRHIVertexFormatSupported(*accessor, customAttribute.second.m_format))
            {
                continue;
            }

            switch (accessor->componentType)
            {
            case CesiumGltf::AccessorSpec::ComponentType::BYTE:
                CreateCustomAttribute<std::int8_t>(model, *accessor, customAttribute.second);
                break;
            case CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE:
                CreateCustomAttribute<std::uint8_t>(model, *accessor, customAttribute.second);
                break;
            case CesiumGltf::AccessorSpec::ComponentType::SHORT:
                CreateCustomAttribute<std::int16_t>(model, *accessor, customAttribute.second);
                break;
            case CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT:
                CreateCustomAttribute<std::uint16_t>(model, *accessor, customAttribute.second);
                break;
            case CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_INT:
                CreateCustomAttribute<std::uint32_t>(model, *accessor, customAttribute.second);
                break;
            case CesiumGltf::AccessorSpec::ComponentType::FLOAT:
                CreateCustomAttribute<float>(model, *accessor, customAttribute.second);
                break;
            default:
                break;
            }
        }
    }

    template<typename ComponentType>
    void GltfTrianglePrimitiveBuilder::CreateCustomAttribute(
        const CesiumGltf::Model& model, const CesiumGltf::Accessor& accessor, const GltfShaderVertexAttribute& customShaderAttribute)
    {
        if (accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR)
        {
            CesiumGltf::AccessorView<ComponentType> accessorView{ model, accessor };
            VertexRawBuffer vertexBuffer;
            vertexBuffer.m_elementCount = static_cast<std::size_t>(accessorView.size());
            vertexBuffer.m_format = customShaderAttribute.m_format;
            CopyAccessorToBuffer(accessorView, vertexBuffer.m_buffer);

            m_customAttributes.emplace_back(customShaderAttribute, std::move(vertexBuffer));
        }
        else if (accessor.type == CesiumGltf::AccessorSpec::Type::VEC2)
        {
            CesiumGltf::AccessorView<glm::vec<2, ComponentType, glm::defaultp>> accessorView{ model, accessor };
            VertexRawBuffer vertexBuffer;
            vertexBuffer.m_elementCount = static_cast<std::size_t>(accessorView.size());
            vertexBuffer.m_format = customShaderAttribute.m_format;
            CopyAccessorToBuffer(accessorView, vertexBuffer.m_buffer);

            m_customAttributes.emplace_back(customShaderAttribute, std::move(vertexBuffer));
        }
        else if (accessor.type == CesiumGltf::AccessorSpec::Type::VEC3)
        {
            CesiumGltf::AccessorView<glm::vec<3, ComponentType, glm::defaultp>> accessorView{ model, accessor };
            VertexRawBuffer vertexBuffer;
            vertexBuffer.m_elementCount = static_cast<std::size_t>(accessorView.size());
            vertexBuffer.m_format = customShaderAttribute.m_format;
            CopyAccessorToBuffer(accessorView, vertexBuffer.m_buffer);

            m_customAttributes.emplace_back(customShaderAttribute, std::move(vertexBuffer));
        }
        else if (accessor.type == CesiumGltf::AccessorSpec::Type::VEC4)
        {
            CesiumGltf::AccessorView<glm::vec<4, ComponentType, glm::defaultp>> accessorView{ model, accessor };
            VertexRawBuffer vertexBuffer;
            vertexBuffer.m_elementCount = static_cast<std::size_t>(accessorView.size());
            vertexBuffer.m_format = customShaderAttribute.m_format;
            CopyAccessorToBuffer(accessorView, vertexBuffer.m_buffer);

            m_customAttributes.emplace_back(customShaderAttribute, std::move(vertexBuffer));
        }
    }

    void GltfTrianglePrimitiveBuilder::CreateFlatNormal()
    {
        m_normals.resize(m_positions.size());
        for (std::size_t i = 0; i < m_positions.size(); i += 3)
        {
            const glm::vec3& p0 = m_positions[i];
            const glm::vec3& p1 = m_positions[i + 1];
            const glm::vec3& p2 = m_positions[i + 2];
            glm::vec3 normal = glm::cross(p1 - p0, p2 - p0);
            if (CesiumUtility::Math::equalsEpsilon(glm::dot(normal, normal), 0.0, CesiumUtility::Math::EPSILON5))
            {
                normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            else
            {
                normal = glm::normalize(normal);
            }

            m_normals[i] = normal;
            m_normals[i + 1] = normal;
            m_normals[i + 2] = normal;
        }
    }

    void GltfTrianglePrimitiveBuilder::CopySubregionBuffer(
        AZStd::vector<std::byte>& buffer, const void* src, const AZ::RHI::BufferViewDescriptor& descriptor)
    {
        std::size_t offset = descriptor.m_elementOffset * descriptor.m_elementSize;
        std::size_t totalBytes = descriptor.m_elementCount * descriptor.m_elementSize;
        memcpy(buffer.data() + offset, src, totalBytes);
    }

    void GltfTrianglePrimitiveBuilder::Reset()
    {
        m_context = LoadContext{};
        m_indices.clear();
        m_positions.clear();
        m_normals.clear();
        m_tangents.clear();
        m_bitangents.clear();
        for (std::size_t i = 0; i < m_uvs.size(); ++i)
        {
            m_uvs[i].m_buffer.clear();
            m_uvs[i].m_elementCount = 0;
            m_uvs[i].m_format = AZ::RHI::Format::Unknown;
        }

        m_customAttributes.clear();
    }

    AZ::Aabb GltfTrianglePrimitiveBuilder::CreateAabbFromPositions(const CesiumGltf::AccessorView<glm::vec3>& positionAccessorView)
    {
        AZ::Aabb aabb = AZ::Aabb::CreateNull();
        for (std::int64_t i = 0; i < positionAccessorView.size(); ++i)
        {
            const glm::vec3& position = positionAccessorView[i];
            aabb.AddPoint(AZ::Vector3(position.x, position.y, position.z));
        }

        return aabb;
    }

    bool GltfTrianglePrimitiveBuilder::DoesRHIVertexFormatSupported(const CesiumGltf::Accessor& accessor, AZ::RHI::Format format)
    {
        switch (format)
        {
        // type VEC4
        case AZ::RHI::Format::R32G32B32A32_FLOAT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::FLOAT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && !accessor.normalized;
        case AZ::RHI::Format::R32G32B32A32_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_INT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && !accessor.normalized;

        case AZ::RHI::Format::R16G16B16A16_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && !accessor.normalized;
        case AZ::RHI::Format::R16G16B16A16_SINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && !accessor.normalized;

        case AZ::RHI::Format::R8G8B8A8_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && !accessor.normalized;
        case AZ::RHI::Format::R8G8B8A8_SINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && !accessor.normalized;

        case AZ::RHI::Format::R16G16B16A16_UNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && accessor.normalized;
        case AZ::RHI::Format::R16G16B16A16_SNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && accessor.normalized;

        case AZ::RHI::Format::R8G8B8A8_UNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && accessor.normalized;
        case AZ::RHI::Format::R8G8B8A8_SNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC4 && accessor.normalized;

        // type VEC3
        case AZ::RHI::Format::R32G32B32_FLOAT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::FLOAT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC3 && !accessor.normalized;
        case AZ::RHI::Format::R32G32B32_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_INT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC3 && !accessor.normalized;

        // type VEC2
        case AZ::RHI::Format::R32G32_FLOAT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::FLOAT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && !accessor.normalized;
        case AZ::RHI::Format::R32G32_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_INT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && !accessor.normalized;

        case AZ::RHI::Format::R16G16_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && !accessor.normalized;
        case AZ::RHI::Format::R16G16_SINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && !accessor.normalized;

        case AZ::RHI::Format::R8G8_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && !accessor.normalized;
        case AZ::RHI::Format::R8G8_SINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && !accessor.normalized;

        case AZ::RHI::Format::R16G16_UNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && accessor.normalized;
        case AZ::RHI::Format::R16G16_SNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && accessor.normalized;

        case AZ::RHI::Format::R8G8_UNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && accessor.normalized;
        case AZ::RHI::Format::R8G8_SNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::VEC2 && accessor.normalized;

        // Type SCALAR
        case AZ::RHI::Format::R32_FLOAT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::FLOAT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && !accessor.normalized;
        case AZ::RHI::Format::R32_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_INT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && !accessor.normalized;

        case AZ::RHI::Format::R16_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && !accessor.normalized;
        case AZ::RHI::Format::R16_SINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && !accessor.normalized;

        case AZ::RHI::Format::R8_UINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && !accessor.normalized;
        case AZ::RHI::Format::R8_SINT:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && !accessor.normalized;

        case AZ::RHI::Format::R16_UNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && accessor.normalized;
        case AZ::RHI::Format::R16_SNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::SHORT &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && accessor.normalized;

        case AZ::RHI::Format::R8_UNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && accessor.normalized;
        case AZ::RHI::Format::R8_SNORM:
            return accessor.componentType == CesiumGltf::AccessorSpec::ComponentType::BYTE &&
                accessor.type == CesiumGltf::AccessorSpec::Type::SCALAR && accessor.normalized;

        default:
            break;
        }

        return false;
    }
} // namespace Cesium

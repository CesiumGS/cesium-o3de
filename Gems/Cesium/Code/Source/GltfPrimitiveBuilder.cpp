#include "GltfPrimitiveBuilder.h"
#include "BitangentAndTangentGenerator.h"
#include <CesiumUtility/Math.h>
#include <Atom/RPI.Reflect/Model/ModelLodAsset.h>
#include <Atom/RPI.Reflect/Buffer/BufferAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelLodAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelAssetCreator.h>
#include <cassert>
#include <cstdint>

namespace Cesium
{
    GltfTrianglePrimitiveBuilder::GPUBuffer::GPUBuffer()
        : m_buffer{}
        , m_format{ AZ::RHI::Format::Unknown }
        , m_elementCount{ 0 }
    {
    }

    GltfTrianglePrimitiveBuilder::CommonAccessorViews::CommonAccessorViews(
        const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive)
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

        // get uvs
        for (std::size_t i = 0; i < 2; ++i)
        {
            auto uvAttribute = primitive.attributes.find("UV_" + std::to_string(i));
            if (uvAttribute != primitive.attributes.end())
            {
                auto uvAccessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, uvAttribute->second);
                if (uvAccessor)
                {
                    if (uvAccessor->type == CesiumGltf::AccessorSpec::Type::VEC2)
                    {
                        if (uvAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::FLOAT)
                        {
                            m_uvs[i] = CesiumGltf::AccessorView<glm::vec2>(model, *uvAccessor);
                        }
                    }
                }
            }
        }
    }

    GltfTrianglePrimitiveBuilder::LoadContext::LoadContext()
        : m_generateFlatNormal{ false }
        , m_generateTangent{ false }
        , m_generateUnIndexedMesh{ false }
    {
    }

    AZ::Data::Asset<AZ::RPI::ModelAsset> GltfTrianglePrimitiveBuilder::Create(
        const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive)
    {
        // Construct common accessor views. This is needed to begin determine loading context
        CommonAccessorViews commonAccessorViews{ model, primitive };
        if (commonAccessorViews.m_positions.status() != CesiumGltf::AccessorViewStatus::Valid)
        {
            return AZ::Data::Asset<AZ::RPI::ModelAsset>();
        }

        if (commonAccessorViews.m_positions.size() == 0)
        {
            return AZ::Data::Asset<AZ::RPI::ModelAsset>();
        }

        // construct bounding volume
        auto positionAccessor = commonAccessorViews.m_positionAccessor;
        AZ::Aabb aabb = AZ::Aabb::CreateNull();
        if (positionAccessor->min.size() == 3 && positionAccessor->max.size() == 3)
        {
            aabb = AZ::Aabb::CreateFromMinMaxValues(
                positionAccessor->min[0], positionAccessor->min[1], positionAccessor->min[2], positionAccessor->max[0],
                positionAccessor->max[1], positionAccessor->max[2]);
        }
        else
        {
            aabb = CreateAabbFromPositions(commonAccessorViews.m_positions);
        }

        // set indices
        if (CreateIndices(model, primitive))
        {
            return AZ::Data::Asset<AZ::RPI::ModelAsset>();
        }

        // if indices is empty, so the mesh is non-indexed mesh. We should expect positions size
        // is a multiple of 3
        if (m_indices.empty() && (commonAccessorViews.m_positions.size() % 3 == 0))
        {
            return AZ::Data::Asset<AZ::RPI::ModelAsset>();
        }

        // determine loading context
        DetermineLoadContext(commonAccessorViews);

        // Create attributes. The order call of the functions is important
        CreatePositionsAttribute(commonAccessorViews);
        CreateNormalsAttribute(commonAccessorViews);
        CreateUVsAttributes(commonAccessorViews, model, primitive);
        CreateTangentsAndBitangentsAttributes(commonAccessorViews);
        CreateColorsAttribute(commonAccessorViews, model, primitive);

        // create buffer assets
        auto indicesBuffer = CreateIndicesBufferAsset(model, primitive);
        auto positionBuffer = CreateBufferAsset(m_positions.data(), m_positions.size(), AZ::RHI::Format::R32G32B32_FLOAT);
        auto normalBuffer = CreateBufferAsset(m_normals.data(), m_normals.size(), AZ::RHI::Format::R32G32B32_FLOAT);
        auto tangentBuffer = CreateBufferAsset(m_tangents.data(), m_tangents.size(), AZ::RHI::Format::R32G32B32A32_FLOAT);
        auto bitangentBuffer = CreateBufferAsset(m_bitangents.data(), m_bitangents.size(), AZ::RHI::Format::R32G32B32_FLOAT);

        AZStd::array<AZ::Data::Asset<AZ::RPI::BufferAsset>, 2> uvBuffers;
        for (std::size_t i = 0; i < m_uvs.size(); ++i)
        {
            uvBuffers[i] = CreateBufferAsset(m_uvs[i].m_buffer.data(), m_uvs[i].m_elementCount, m_uvs[i].m_format);
        }

        AZ::Data::Asset<AZ::RPI::BufferAsset> colorBuffer;
        if (!m_colors.m_buffer.empty())
        {
            colorBuffer = CreateBufferAsset(m_colors.m_buffer.data(), m_colors.m_elementCount, m_colors.m_format);
        }

        // create LOD asset
        AZ::RPI::ModelLodAssetCreator lodCreator;
        lodCreator.Begin(AZ::Uuid::CreateRandom());
        if (indicesBuffer)
        {
            lodCreator.AddLodStreamBuffer(indicesBuffer);
        }

        if (colorBuffer)
        {
            lodCreator.AddLodStreamBuffer(colorBuffer);
        }

        lodCreator.AddLodStreamBuffer(positionBuffer);
        lodCreator.AddLodStreamBuffer(normalBuffer);
        lodCreator.AddLodStreamBuffer(tangentBuffer);
        lodCreator.AddLodStreamBuffer(bitangentBuffer);

        for (std::size_t i = 0; i < uvBuffers.size(); ++i)
        {
            if (uvBuffers[i])
            {
                lodCreator.AddLodStreamBuffer(uvBuffers[i]);
            }
        }

        // create mesh
        lodCreator.BeginMesh();
        if (indicesBuffer)
        {
            lodCreator.SetMeshIndexBuffer(AZ::RPI::BufferAssetView(indicesBuffer, indicesBuffer->GetBufferViewDescriptor()));
        }

        if (colorBuffer)
        {
            lodCreator.AddMeshStreamBuffer(
                AZ::RHI::ShaderSemantic("COLOR"), AZ::Name(),
                AZ::RPI::BufferAssetView(colorBuffer, colorBuffer->GetBufferViewDescriptor()));
        }

        lodCreator.AddMeshStreamBuffer(
            AZ::RHI::ShaderSemantic("POSITION"), AZ::Name(),
            AZ::RPI::BufferAssetView(positionBuffer, positionBuffer->GetBufferViewDescriptor()));
        lodCreator.AddMeshStreamBuffer(
            AZ::RHI::ShaderSemantic("NORMAL"), AZ::Name(), AZ::RPI::BufferAssetView(normalBuffer, normalBuffer->GetBufferViewDescriptor()));
        lodCreator.AddMeshStreamBuffer(
            AZ::RHI::ShaderSemantic("TANGENT"), AZ::Name(),
            AZ::RPI::BufferAssetView(tangentBuffer, tangentBuffer->GetBufferViewDescriptor()));
        lodCreator.AddMeshStreamBuffer(
            AZ::RHI::ShaderSemantic("BITANGENT"), AZ::Name(),
            AZ::RPI::BufferAssetView(bitangentBuffer, bitangentBuffer->GetBufferViewDescriptor()));

        for (std::size_t i = 0; i < uvBuffers.size(); ++i)
        {
            if (uvBuffers[i])
            {
                lodCreator.AddMeshStreamBuffer(
                    AZ::RHI::ShaderSemantic("UV", i), AZ::Name(),
                    AZ::RPI::BufferAssetView(uvBuffers[i], uvBuffers[i]->GetBufferViewDescriptor()));
            }
        }

        lodCreator.SetMeshAabb(std::move(aabb));
        lodCreator.EndMesh();

        AZ::Data::Asset<AZ::RPI::ModelLodAsset> lodAsset;
        lodCreator.End(lodAsset);

        // create model asset
        AZ::RPI::ModelAssetCreator modelCreator;
        modelCreator.Begin(AZ::Uuid::CreateRandom());
        modelCreator.AddLodAsset(std::move(lodAsset));

        AZ::Data::Asset<AZ::RPI::ModelAsset> modelAsset;
        modelCreator.End(modelAsset);

        return modelAsset;
    }

    void GltfTrianglePrimitiveBuilder::DetermineLoadContext(const CommonAccessorViews& accessorViews)
    {
        // check if we should generate normal
        bool isNormalAccessorValid = accessorViews.m_normals.status() == CesiumGltf::AccessorViewStatus::Valid;
        bool hasEnoughNormalVertices = accessorViews.m_normals.size() == accessorViews.m_positions.size();
        m_context.m_generateFlatNormal = !isNormalAccessorValid || !hasEnoughNormalVertices;

        // check if we should generate tangent
        bool isTangentAccessorValid = accessorViews.m_tangents.status() == CesiumGltf::AccessorViewStatus::Valid;
        bool hasEnoughTangentVertices = accessorViews.m_tangents.size() == accessorViews.m_positions.size();
        m_context.m_generateTangent = !isTangentAccessorValid || !hasEnoughTangentVertices;

        // check if we should generate unindexed mesh
        m_context.m_generateUnIndexedMesh = m_context.m_generateFlatNormal || m_context.m_generateTangent;
    }

    template<typename AccessorType>
    void GltfTrianglePrimitiveBuilder::CopyAccessorToBuffer(
        const CesiumGltf::AccessorView<AccessorType>& attributeAccessorView, AZStd::vector<AccessorType>& attributes)
    {
        if (m_context.m_generateUnIndexedMesh && m_indices.size() > 0)
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
            // if m_generateUnIndexedMesh is true but mesh has no indices, that means mesh is already un-indexed. Copy accessor over
            // if m_generateUnIndexedMesh is false, copy accessor over
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
        if (m_context.m_generateUnIndexedMesh && m_indices.size() > 0)
        {
            buffer.resize(m_indices.size() * sizeof(AccessorType));
            AccessorType* value = reinterpret_cast<AccessorType*>(buffer.data());
            for (std::size_t i = 0; i < m_indices.size(); ++i)
            {
                value[i] = accessorView[static_cast<std::int64_t>(i)];
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

    AZ::Data::Asset<AZ::RPI::BufferAsset> GltfTrianglePrimitiveBuilder::CreateBufferAsset(
        const void* data, const std::size_t elementCount, AZ::RHI::Format format)
    {
        AZ::RHI::BufferViewDescriptor bufferViewDescriptor = AZ::RHI::BufferViewDescriptor::CreateTyped(0, elementCount, format);
        AZ::RHI::BufferDescriptor bufferDescriptor;
        bufferDescriptor.m_bindFlags = AZ::RHI::BufferBindFlags::InputAssembly | AZ::RHI::BufferBindFlags::ShaderRead;
        bufferDescriptor.m_byteCount = bufferViewDescriptor.m_elementCount * bufferViewDescriptor.m_elementSize;

        AZ::RPI::BufferAssetCreator creator;
        creator.Begin(AZ::Uuid::CreateRandom());
        creator.SetBuffer(data, bufferDescriptor.m_byteCount, bufferDescriptor);
        creator.SetBufferViewDescriptor(bufferViewDescriptor);
        creator.SetUseCommonPool(AZ::RPI::CommonBufferPoolType::StaticInputAssembly);

        AZ::Data::Asset<AZ::RPI::BufferAsset> bufferAsset;
        creator.End(bufferAsset);

        return bufferAsset;
    }

    bool GltfTrianglePrimitiveBuilder::CreateIndices(const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive)
    {
        const CesiumGltf::Accessor* indicesAccessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, primitive.indices);
        if (!indicesAccessor)
        {
            // it's optional to have no indices
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
            auto uvAttribute = primitive.attributes.find("UV_" + std::to_string(i));
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
                m_uvs[i].m_elementCount = view.size();
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
                m_uvs[i].m_elementCount = view.size();
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
                m_uvs[i].m_elementCount = view.size();
                m_uvs[i].m_format = AZ::RHI::Format::R16G16_UNORM;
            }
        }
    }

    void GltfTrianglePrimitiveBuilder::CreateTangentsAndBitangentsAttributes(const CommonAccessorViews& commonAccessorViews)
    {
        if (m_context.m_generateTangent)
        {
            // positions, normals, and uvs should be unindexed at this point
            assert(generateUnIndexedMesh);
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
                        AZStd::array_view<glm::vec2> uvs(
                            reinterpret_cast<const glm::vec2*>(m_uvs[i].m_buffer.data()), m_uvs[i].m_elementCount);
                        success = BitangentAndTangentGenerator::Generate(m_positions, m_normals, uvs, m_tangents, m_bitangents);
                    }
                    else if (m_uvs[i].m_format == AZ::RHI::Format::R8G8_UNORM)
                    {
                        AZStd::array_view<glm::u8vec2> uvs(
                            reinterpret_cast<const glm::u8vec2*>(m_uvs[i].m_buffer.data()), m_uvs[i].m_elementCount);
                        success = BitangentAndTangentGenerator::Generate(m_positions, m_normals, uvs, m_tangents, m_bitangents);
                    }
                    else if (m_uvs[i].m_format == AZ::RHI::Format::R16G16_UNORM)
                    {
                        AZStd::array_view<glm::u16vec2> uvs(
                            reinterpret_cast<const glm::u16vec2*>(m_uvs[i].m_buffer.data()), m_uvs[i].m_elementCount);
                        success = BitangentAndTangentGenerator::Generate(m_positions, m_normals, uvs, m_tangents, m_bitangents);
                    }
                    else
                    {
                        // This code path should not happen since we check uv accessors before this function
                        assert(false);
                    }
                }
            }

            // if we still cannot generate MikkTSpace, then we generate dummy
            if (!success)
            {
                m_tangents.resize(m_positions.size(), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                m_bitangents.resize(m_positions.size(), glm::vec3(0.0f, 0.0f, 0.0f));
            }
        }
        else
        {
            assert(commonAccessorViews.m_tangents.status() == CesiumGltf::AccessorViewStatus::Valid);
            assert(commonAccessorViews.m_tangents.size() > 0);
            assert(commonAccessorViews.m_tangents.size() == commonAccessorViews.m_positions.size());

            // copy tangents to vector
            CopyAccessorToBuffer(commonAccessorViews.m_tangents, m_tangents);

            // create bitangents
            m_bitangents.resize(m_tangents.size(), glm::vec3(0.0f));
            for (std::size_t i = 0; i < m_tangents.size(); ++i)
            {
                m_bitangents[i] = glm::cross(m_normals[i], glm::vec3(m_tangents[i])) * m_tangents[i].w;
            }
        }
    }

    void GltfTrianglePrimitiveBuilder::CreateColorsAttribute(
        const CommonAccessorViews& commonAccessorViews, const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive)
    {
        auto colorAttribute = primitive.attributes.find("COLOR_0");
        if (colorAttribute == primitive.attributes.end())
        {
            return;
        }

        const CesiumGltf::Accessor* colorAccessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, colorAttribute->second);
        if (colorAccessor->type == CesiumGltf::AccessorSpec::Type::VEC3)
        {
            CreateVec3ColorsAttribute(commonAccessorViews, model, *colorAccessor);
        }
        else if (colorAccessor->type == CesiumGltf::AccessorSpec::Type::VEC4)
        {
            CreateVec4ColorsAttribute(commonAccessorViews, model, *colorAccessor);
        }
    }

    void GltfTrianglePrimitiveBuilder::CreateVec3ColorsAttribute(
        const CommonAccessorViews& commonAccessorViews, const CesiumGltf::Model& model, const CesiumGltf::Accessor& colorAccessor)
    {
        if (colorAccessor.componentType == CesiumGltf::AccessorSpec::ComponentType::FLOAT)
        {
            CesiumGltf::AccessorView<glm::vec3> view{ model, colorAccessor };
            if (view.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return;
            }

            if (view.size() != commonAccessorViews.m_positions.size())
            {
                return;
            }

            CopyVec3ColorAccessorToVec4Buffer(view, 1.0f, m_colors.m_buffer);
            m_colors.m_elementCount = view.size();
            m_colors.m_format = AZ::RHI::Format::R32G32B32A32_FLOAT;
        }
        else if (colorAccessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE)
        {
            CesiumGltf::AccessorView<glm::u8vec3> view{ model, colorAccessor };
            if (view.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return;
            }

            if (view.size() != commonAccessorViews.m_positions.size())
            {
                return;
            }

            CopyVec3ColorAccessorToVec4Buffer(view, static_cast<glm::u8>(256), m_colors.m_buffer);
            m_colors.m_elementCount = view.size();
            m_colors.m_format = AZ::RHI::Format::R8G8B8A8_UNORM;
        }
        else if (colorAccessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT)
        {
            CesiumGltf::AccessorView<glm::u16vec3> view{ model, colorAccessor };
            if (view.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return;
            }

            if (view.size() != commonAccessorViews.m_positions.size())
            {
                return;
            }

            CopyVec3ColorAccessorToVec4Buffer(view, static_cast<glm::u16>(65536), m_colors.m_buffer);
            m_colors.m_elementCount = view.size();
            m_colors.m_format = AZ::RHI::Format::R16G16B16A16_UNORM;
        }
    }

    void GltfTrianglePrimitiveBuilder::CreateVec4ColorsAttribute(
        const CommonAccessorViews& commonAccessorViews, const CesiumGltf::Model& model, const CesiumGltf::Accessor& colorAccessor)
    {
        if (colorAccessor.componentType == CesiumGltf::AccessorSpec::ComponentType::FLOAT)
        {
            CesiumGltf::AccessorView<glm::vec4> view{ model, colorAccessor };
            if (view.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return;
            }

            if (view.size() != commonAccessorViews.m_positions.size())
            {
                return;
            }

            CopyAccessorToBuffer(view, m_colors.m_buffer);
            m_colors.m_elementCount = view.size();
            m_colors.m_format = AZ::RHI::Format::R32G32B32A32_FLOAT;
        }
        else if (colorAccessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE)
        {
            CesiumGltf::AccessorView<glm::u8vec4> view{ model, colorAccessor };
            if (view.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return;
            }

            if (view.size() != commonAccessorViews.m_positions.size())
            {
                return;
            }

            CopyAccessorToBuffer(view, m_colors.m_buffer);
            m_colors.m_elementCount = view.size();
            m_colors.m_format = AZ::RHI::Format::R8G8B8A8_UNORM;
        }
        else if (colorAccessor.componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT)
        {
            CesiumGltf::AccessorView<glm::u16vec4> view{ model, colorAccessor };
            if (view.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return;
            }

            if (view.size() != commonAccessorViews.m_positions.size())
            {
                return;
            }

            CopyAccessorToBuffer(view, m_colors.m_buffer);
            m_colors.m_elementCount = view.size();
            m_colors.m_format = AZ::RHI::Format::R16G16B16A16_UNORM;
        }
    }

    template<typename Type, glm::qualifier Q>
    void GltfTrianglePrimitiveBuilder::CopyVec3ColorAccessorToVec4Buffer(
        const CesiumGltf::AccessorView<glm::vec<3, Type, Q>>& accessorView, Type w, AZStd::vector<std::byte>& buffer)
    {
        using AccessorType = glm::vec<3, Type, Q>;
        using BufferType = glm::vec<4, Type, Q>;

        if (m_context.m_generateUnIndexedMesh && m_indices.size() > 0)
        {
            buffer.resize(m_indices.size() * sizeof(BufferType));
            BufferType* value = reinterpret_cast<BufferType*>(buffer.data());
            for (std::size_t i = 0; i < m_indices.size(); ++i)
            {
                const AccessorType& valueView = accessorView[static_cast<std::int64_t>(i)]; 
                value[i].x = valueView.x;
                value[i].y = valueView.y;
                value[i].z = valueView.z;
                value[i].w = w;
            }
        }
        else
        {
            buffer.resize(static_cast<std::size_t>(accessorView.size()) * sizeof(BufferType));
            BufferType* value = reinterpret_cast<BufferType*>(buffer.data());
            for (std::int64_t i = 0; i < accessorView.size(); ++i)
            {
                const AccessorType& valueView = accessorView[static_cast<std::int64_t>(i)]; 
                value[i].x = valueView.x;
                value[i].y = valueView.y;
                value[i].z = valueView.z;
                value[i].w = w;
            }
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

    AZ::Data::Asset<AZ::RPI::BufferAsset> GltfTrianglePrimitiveBuilder::CreateIndicesBufferAsset(
        const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive)
    {
        if (m_context.m_generateUnIndexedMesh || m_indices.empty())
        {
            return AZ::Data::Asset<AZ::RPI::BufferAsset>();
        }

        const CesiumGltf::Accessor* accessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, primitive.indices);
        assert(accessor != nullptr);
        assert(accessor->type == CesiumGltf::AccessorSpec::Type::SCALAR);

        if (accessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE)
        {
            AZStd::vector<std::uint8_t> data(m_indices.size());  
            for (std::size_t i = 0; i < data.size(); ++i)
            {
                data[i] = static_cast<std::uint8_t>(m_indices[i]);
            }

            return CreateBufferAsset(data.data(), data.size(), AZ::RHI::Format::R8_UINT);
        }
        else if (accessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT)
        {
            AZStd::vector<std::uint16_t> data(m_indices.size());  
            for (std::size_t i = 0; i < data.size(); ++i)
            {
                data[i] = static_cast<std::uint8_t>(m_indices[i]);
            }

            return CreateBufferAsset(data.data(), data.size(), AZ::RHI::Format::R16_UINT);
        }
        else if (accessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_INT)
        {
            AZStd::vector<std::uint32_t> data(m_indices.size());  
            for (std::size_t i = 0; i < data.size(); ++i)
            {
                data[i] = static_cast<std::uint8_t>(m_indices[i]);
            }

            return CreateBufferAsset(data.data(), data.size(), AZ::RHI::Format::R32_UINT);
        }
        else
        {
            // this code path should not run since we check the validity of indices accessor at the very beginning
            assert(false);
            return AZ::Data::Asset<AZ::RPI::BufferAsset>();
        }
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
} // namespace Cesium

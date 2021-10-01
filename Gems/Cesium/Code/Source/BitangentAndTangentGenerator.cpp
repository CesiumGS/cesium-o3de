#include "BitangentAndTangentGenerator.h"
#include <mikkelsen/mikktspace.h>

namespace Cesium
{
    struct BitangentAndTangentGenerator::MikktspaceCustomData
    {
        AZStd::array_view<glm::vec3> positions{};
        AZStd::array_view<glm::vec3> normals{};
        AZStd::array_view<glm::vec2> uvs{};
        AZStd::array_view<glm::u8vec2> unorm_u8_uvs{};
        AZStd::array_view<glm::u16vec2> unorm_u16_uvs{};
        AZStd::vector<glm::vec4>* tangents{nullptr};
        AZStd::vector<glm::vec3>* bitangents{nullptr};
    };

    struct BitangentAndTangentGenerator::MikktspaceMethods
    {
        static int GetNumFaces(const SMikkTSpaceContext* context)
        {
            MikktspaceCustomData* customData = static_cast<MikktspaceCustomData*>(context->m_pUserData);
            return static_cast<int>(customData->positions.size()) / 3;
        }

        static int GetNumVerticesOfFace([[maybe_unused]] const SMikkTSpaceContext* context, [[maybe_unused]] int face)
        {
            return 3;
        }

        static void GetPosition(const SMikkTSpaceContext* context, float posOut[], const int face, const int vert)
        {
            MikktspaceCustomData* customData = static_cast<MikktspaceCustomData*>(context->m_pUserData);
            const AZStd::array_view<glm::vec3>& positions = customData->positions;
            std::size_t vertexIndex = static_cast<std::size_t>(face * 3 + vert);
            const glm::vec3& position = positions[vertexIndex];
            posOut[0] = position.x;
            posOut[1] = position.y;
            posOut[2] = position.z;
        }

        static void GetNormal(const SMikkTSpaceContext* context, float normOut[], const int face, const int vert)
        {
            MikktspaceCustomData* customData = static_cast<MikktspaceCustomData*>(context->m_pUserData);
            const AZStd::array_view<glm::vec3>& normals = customData->normals;
            std::size_t vertexIndex = static_cast<std::size_t>(face * 3 + vert);
            const glm::vec3& normal = normals[vertexIndex];
            normOut[0] = normal.x;
            normOut[1] = normal.y;
            normOut[2] = normal.z;
        }

        static void GetTexCoord(const SMikkTSpaceContext* context, float texOut[], const int face, const int vert)
        {
            MikktspaceCustomData* customData = static_cast<MikktspaceCustomData*>(context->m_pUserData);
            const AZStd::array_view<glm::vec2>& uvs = customData->uvs;
            if (uvs.empty())
            {
                texOut[0] = 0.0f;
                texOut[1] = 0.0f;
            }
            else
            {
                std::size_t vertexIndex = static_cast<std::size_t>(face * 3 + vert);
                const glm::vec2& uv = uvs[vertexIndex];
                texOut[0] = uv.x;
                texOut[1] = uv.y;
            }
        }

        static void GetTexCoord_UNORM_U8(const SMikkTSpaceContext* context, float texOut[], const int face, const int vert)
        {
            MikktspaceCustomData* customData = static_cast<MikktspaceCustomData*>(context->m_pUserData);
            const AZStd::array_view<glm::u8vec2>& uvs = customData->unorm_u8_uvs;
            if (uvs.empty())
            {
                texOut[0] = 0.0f;
                texOut[1] = 0.0f;
            }
            else
            {
                std::size_t vertexIndex = static_cast<std::size_t>(face * 3 + vert);
                const glm::vec2& uv = uvs[vertexIndex];
                texOut[0] = static_cast<float>(uv.x) / 256.0f;
                texOut[1] = static_cast<float>(uv.y) / 256.0f;
            }
        }

        static void GetTexCoord_UNORM_U16(const SMikkTSpaceContext* context, float texOut[], const int face, const int vert)
        {
            MikktspaceCustomData* customData = static_cast<MikktspaceCustomData*>(context->m_pUserData);
            const AZStd::array_view<glm::u16vec2>& uvs = customData->unorm_u16_uvs;
            if (uvs.empty())
            {
                texOut[0] = 0.0f;
                texOut[1] = 0.0f;
            }
            else
            {
                std::size_t vertexIndex = static_cast<std::size_t>(face * 3 + vert);
                const glm::vec2& uv = uvs[vertexIndex];
                texOut[0] = static_cast<float>(uv.x) / 65536.0f;
                texOut[1] = static_cast<float>(uv.y) / 65536.0f;
            }
        }

        static void SetTSpace(const SMikkTSpaceContext* context, const float tangent[], const float bitangent[], const float magS, const float magT, const tbool isOrientationPreserving, const int face, const int vert)
        {
            MikktspaceCustomData* customData = static_cast<MikktspaceCustomData*>(context->m_pUserData);
            AZStd::vector<glm::vec4>& tangents = *customData->tangents;
            AZStd::vector<glm::vec3>& bitangents = *customData->bitangents;
            std::size_t vertexIndex = static_cast<std::size_t>(face * 3 + vert);
            float sign = isOrientationPreserving ? 1.0f : -1.0f;
            tangents[vertexIndex] = glm::vec4(tangent[0] * magS, tangent[1] * magS, tangent[2] * magS, sign);
            bitangents[vertexIndex] = glm::vec3(bitangent[0] * magT, bitangent[1] * magT, bitangent[2] * magT);
        }
    };

    bool BitangentAndTangentGenerator::Generate(
        const AZStd::array_view<glm::vec3>& positions,
        const AZStd::array_view<glm::vec3>& normals,
        const AZStd::array_view<glm::vec2>& uvs,
        AZStd::vector<glm::vec4>& tangents,
        AZStd::vector<glm::vec3>& bitangents)
    {
        tangents.resize(positions.size());
        bitangents.resize(positions.size());

        SMikkTSpaceInterface mikkInterface;
        mikkInterface.m_getNumFaces = MikktspaceMethods::GetNumFaces;
        mikkInterface.m_getNormal = MikktspaceMethods::GetNormal;
        mikkInterface.m_getPosition = MikktspaceMethods::GetPosition;
        mikkInterface.m_getTexCoord = MikktspaceMethods::GetTexCoord;
        mikkInterface.m_setTSpace = MikktspaceMethods::SetTSpace;
        mikkInterface.m_setTSpaceBasic = nullptr;
        mikkInterface.m_getNumVerticesOfFace = MikktspaceMethods::GetNumVerticesOfFace;

        // Set the MikkT custom data.
        MikktspaceCustomData customData;
        customData.positions = positions;
        customData.normals = normals;
        customData.uvs = uvs;
        customData.tangents = &tangents;
        customData.bitangents = &bitangents;

        // Generate the tangents.
        SMikkTSpaceContext mikkContext;
        mikkContext.m_pInterface = &mikkInterface;
        mikkContext.m_pUserData = &customData;
        return (genTangSpaceDefault(&mikkContext) == 0);
    }

    bool BitangentAndTangentGenerator::Generate(
        const AZStd::array_view<glm::vec3>& positions,
        const AZStd::array_view<glm::vec3>& normals,
        const AZStd::array_view<glm::u8vec2>& uvs,
        AZStd::vector<glm::vec4>& tangents,
        AZStd::vector<glm::vec3>& bitangents)
    {
        tangents.resize(positions.size());
        bitangents.resize(positions.size());

        SMikkTSpaceInterface mikkInterface;
        mikkInterface.m_getNumFaces = MikktspaceMethods::GetNumFaces;
        mikkInterface.m_getNormal = MikktspaceMethods::GetNormal;
        mikkInterface.m_getPosition = MikktspaceMethods::GetPosition;
        mikkInterface.m_getTexCoord = MikktspaceMethods::GetTexCoord_UNORM_U8;
        mikkInterface.m_setTSpace = MikktspaceMethods::SetTSpace;
        mikkInterface.m_setTSpaceBasic = nullptr;
        mikkInterface.m_getNumVerticesOfFace = MikktspaceMethods::GetNumVerticesOfFace;

        // Set the MikkT custom data.
        MikktspaceCustomData customData;
        customData.positions = positions;
        customData.normals = normals;
        customData.unorm_u8_uvs = uvs;
        customData.tangents = &tangents;
        customData.bitangents = &bitangents;

        // Generate the tangents.
        SMikkTSpaceContext mikkContext;
        mikkContext.m_pInterface = &mikkInterface;
        mikkContext.m_pUserData = &customData;
        return (genTangSpaceDefault(&mikkContext) == 0);
    }

    bool BitangentAndTangentGenerator::Generate(
        const AZStd::array_view<glm::vec3>& positions,
        const AZStd::array_view<glm::vec3>& normals,
        const AZStd::array_view<glm::u16vec2>& uvs,
        AZStd::vector<glm::vec4>& tangents,
        AZStd::vector<glm::vec3>& bitangents)
    {
        tangents.resize(positions.size());
        bitangents.resize(positions.size());

        SMikkTSpaceInterface mikkInterface;
        mikkInterface.m_getNumFaces = MikktspaceMethods::GetNumFaces;
        mikkInterface.m_getNormal = MikktspaceMethods::GetNormal;
        mikkInterface.m_getPosition = MikktspaceMethods::GetPosition;
        mikkInterface.m_getTexCoord = MikktspaceMethods::GetTexCoord_UNORM_U16;
        mikkInterface.m_setTSpace = MikktspaceMethods::SetTSpace;
        mikkInterface.m_setTSpaceBasic = nullptr;
        mikkInterface.m_getNumVerticesOfFace = MikktspaceMethods::GetNumVerticesOfFace;

        // Set the MikkT custom data.
        MikktspaceCustomData customData;
        customData.positions = positions;
        customData.normals = normals;
        customData.unorm_u16_uvs = uvs;
        customData.tangents = &tangents;
        customData.bitangents = &bitangents;

        // Generate the tangents.
        SMikkTSpaceContext mikkContext;
        mikkContext.m_pInterface = &mikkInterface;
        mikkContext.m_pUserData = &customData;
        return (genTangSpaceDefault(&mikkContext) == 0);
    }
} // namespace Cesium

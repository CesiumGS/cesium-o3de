#pragma once

#include <AzCore/std/containers/span.h>
#include <AzCore/std/containers/vector.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct BitangentAndTangentGenerator
    {
    public:
        static bool Generate(
            const AZStd::span<glm::vec3>& positions,
            const AZStd::span<glm::vec3>& normals,
            const AZStd::span<glm::vec2>& uvs,
            AZStd::vector<glm::vec4>& tangents,
            AZStd::vector<glm::vec3>& bitangents);

        static bool Generate(
            const AZStd::span<glm::vec3>& positions,
            const AZStd::span<glm::vec3>& normals,
            const AZStd::span<glm::u8vec2>& unorm_uvs,
            AZStd::vector<glm::vec4>& tangents,
            AZStd::vector<glm::vec3>& bitangents);

        static bool Generate(
            const AZStd::span<glm::vec3>& positions,
            const AZStd::span<glm::vec3>& normals,
            const AZStd::span<glm::u16vec2>& unorm_uvs,
            AZStd::vector<glm::vec4>& tangents,
            AZStd::vector<glm::vec3>& bitangents);

    private:
        struct MikktspaceCustomData;

        struct MikktspaceMethods;
    };
} // namespace Cesium

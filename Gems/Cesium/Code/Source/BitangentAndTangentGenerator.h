#pragma once

#include <AzCore/std/containers/vector.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct BitangentAndTangentGenerator
    {
    public:
        static bool Generate(
            const AZStd::vector<glm::vec3>& positions,
            const AZStd::vector<glm::vec3>& normals,
            const AZStd::vector<glm::vec2>& uvs,
            AZStd::vector<glm::vec4>& tangents,
            AZStd::vector<glm::vec3>& bitangents);

    private:
        struct MikktspaceCustomData;

        struct MikktspaceMethods;
    };
} // namespace Cesium

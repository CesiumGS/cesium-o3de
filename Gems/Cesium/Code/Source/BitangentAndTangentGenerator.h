#pragma once

#include <AzCore/std/containers/vector.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct BitangentAndTangentGenerator
    {
        static void GenerateTangentAndBitangent(
            const AZStd::vector<glm::vec3>& positions,
            const AZStd::vector<glm::vec3>& normals,
            AZStd::vector<glm::vec4>& tangents,
            AZStd::vector<glm::vec3>& bitangents);
    };
}

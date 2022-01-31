#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/Memory.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/Event.h>
#include <glm/glm.hpp>

namespace Cesium
{
    using ECEFPositionChangeEvent = AZ::Event<glm::dvec3>;

    class ECEFPickerComponentHelper final
    {
		enum class PositionType
		{
			Cartesian,
			Cartographic,
		};

		enum class SamplePositionMethod
		{
			EntityCoordinate,
			CameraPosition
		};

		struct DegreeCartographic final
		{
			AZ_RTTI(DegreeCartographic, "{477784B5-7A3D-4721-88CD-99A147BABFB0}");
			AZ_CLASS_ALLOCATOR(DegreeCartographic, AZ::SystemAllocator, 0);

			static void Reflect(AZ::ReflectContext* context);

			DegreeCartographic();

			DegreeCartographic(double longitude, double latitude, double height);

			double m_longitude;
			double m_latitude;
			double m_height;
		};

    public:
        AZ_RTTI(ECEFPickerComponentHelper, "{F8AAE323-83AC-4B8B-8BB8-D3CD27DD85E2}");
        AZ_CLASS_ALLOCATOR(ECEFPickerComponentHelper, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

		glm::dvec3 GetPosition() const;

		void SetPosition(const glm::dvec3& position);

		void SetPosition(const glm::dvec3& position, ECEFPositionChangeEvent::Handler& excludeHandler);

        ECEFPositionChangeEvent m_onPositionChangeEvent;

	private:
        AZ::u32 SamplePositionOfEntity();

        AZ::u32 SamplePositionOfCamera();

        AZ::u32 OnPositionAsCartesianChanged();

        AZ::u32 OnPositionAsCartographicChanged();

        bool UseEntityCoordinateSampleMethod() const;

        bool UseCameraPositionSampleMethod() const;

        bool UsePositionAsCartesian() const;

        bool UsePositionAsCartographic() const;

        SamplePositionMethod m_samplePositionMethod;
        PositionType m_positionType;
        AZ::EntityId m_sampledEntityId;
        glm::dvec3 m_position{ 0.0 };
        DegreeCartographic m_cartographic{ 0.0, 0.0, 0.0 };
    };
} // namespace Cesium
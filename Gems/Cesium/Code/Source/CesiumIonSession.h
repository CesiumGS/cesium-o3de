#pragma once

#include <CesiumIonClient/Connection.h>
#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/AsyncSystem.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/EBus/Event.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <string>
#include <optional>
#include <memory>
#include <vector>

namespace Cesium
{
    using IonSessionUpdatedEvent = AZ::Event<>;

    struct IonAssetItem
    {
        AZStd::string m_tilesetName{};
        std::uint32_t m_tilesetIonAssetId{0};
        int m_imageryIonAssetId{-1};
    };

    class CesiumIonSession : public AZ::Component, public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumIonSession, "{A9008492-0CAF-4B7E-9477-316F26FD1389}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        CesiumIonSession();

        ~CesiumIonSession() noexcept;

        bool IsConnected() const
        {
            return this->m_connection.has_value();
        }

        bool IsConnecting() const
        {
            return this->m_isConnecting;
        }

        bool IsResuming() const
        {
            return this->m_isResuming;
        }

        bool IsProfileLoaded() const
        {
            return this->m_profile.has_value();
        }

        bool IsLoadingProfile() const
        {
            return this->m_isLoadingProfile;
        }

        bool IsAssetListLoaded() const
        {
            return this->m_assets.has_value();
        }

        bool IsLoadingAssetList() const
        {
            return this->m_isLoadingAssets;
        }

        bool IsTokenListLoaded() const
        {
            return this->m_tokens.has_value();
        }

        bool IsLoadingTokenList() const
        {
            return this->m_isLoadingTokens;
        }

        bool IsAssetAccessTokenLoaded() const
        {
            return this->m_assetAccessToken.has_value();
        }

        bool IsLoadingAssetAccessToken() const
        {
            return this->m_isLoadingAssetAccessToken;
        }

        void Connect();

        void Resume();

        void Disconnect();

        void RefreshProfile();

        void RefreshAssets();

        void RefreshTokens();

        void RefreshAssetAccessToken();

        const std::optional<CesiumIonClient::Connection>& GetConnection() const;

        const CesiumIonClient::Profile& GetProfile();

        const CesiumIonClient::Assets& GetAssets();

        const std::vector<CesiumIonClient::Token>& GetTokens();

        const CesiumIonClient::Token& GetAssetAccessToken();

        const std::string& GetAuthorizeUrl() const
        {
            return this->m_authorizeUrl;
        }

        AzToolsFramework::EntityIdList GetSelectedEntities() const;

        void AddTilesetToLevel(AZStd::shared_ptr<IonAssetItem> item);

        void AddImageryToLevel(std::uint32_t ionImageryAssetId);

        IonSessionUpdatedEvent ConnectionUpdated;

        IonSessionUpdatedEvent ProfileUpdated;

        IonSessionUpdatedEvent AssetsUpdated;

        IonSessionUpdatedEvent TokensUpdated;

        IonSessionUpdatedEvent AssetAccessTokenUpdated;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        bool RefreshProfileIfNeeded();

        bool RefreshAssetsIfNeeded();

        bool RefreshTokensIfNeeded();

        bool RefreshAssetAccessTokenIfNeeded();

        void SaveAccessToken(const AZStd::string& ionAccessToken);

        void ReadAccessToken();

        // runtime variable
        CesiumAsync::AsyncSystem m_asyncSystem;
        std::shared_ptr<CesiumAsync::IAssetAccessor> m_assetAccessor;

        std::optional<CesiumIonClient::Connection> m_connection;
        std::optional<CesiumIonClient::Profile> m_profile;
        std::optional<CesiumIonClient::Assets> m_assets;
        std::optional<std::vector<CesiumIonClient::Token>> m_tokens;
        std::optional<CesiumIonClient::Token> m_assetAccessToken;

        bool m_isConnecting{false};
        bool m_isResuming{false};
        bool m_isLoadingProfile{false};
        bool m_isLoadingAssets{false};
        bool m_isLoadingTokens{false};
        bool m_isLoadingAssetAccessToken{false};

        bool m_loadProfileQueued{false};
        bool m_loadAssetsQueued{false};
        bool m_loadTokensQueued{false};
        bool m_loadAssetAccessTokenQueued{false};

        std::string m_authorizeUrl;

        // configurations to save
        AZStd::string m_ionAccessToken;
    };

    using CesiumIonSessionInterface = AZ::Interface<CesiumIonSession>;
} // namespace Cesium

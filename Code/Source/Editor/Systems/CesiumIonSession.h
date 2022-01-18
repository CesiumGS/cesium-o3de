#pragma once

#include <AzCore/Interface/Interface.h>
#include <AzCore/EBus/Event.h>
#include <AzCore/RTTI/TypeInfo.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <CesiumIonClient/Connection.h>
#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/AsyncSystem.h>
#include <AzCore/std/string/string.h>
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

    class CesiumIonSession final
    {
    public:
        CesiumIonSession();

        ~CesiumIonSession() noexcept;

        bool IsConnected() const;

        bool IsConnecting() const;

        bool IsResuming() const;

        bool IsProfileLoaded() const;

        bool IsLoadingProfile() const;

        bool IsAssetListLoaded() const;

        bool IsLoadingAssetList() const;

        bool IsTokenListLoaded() const;

        bool IsLoadingTokenList() const;

        bool IsAssetAccessTokenLoaded() const;

        bool IsLoadingAssetAccessToken() const;

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

        const std::string& GetAuthorizeUrl() const;

        void Flush();

        IonSessionUpdatedEvent ConnectionUpdated;

        IonSessionUpdatedEvent ProfileUpdated;

        IonSessionUpdatedEvent AssetsUpdated;

        IonSessionUpdatedEvent TokensUpdated;

        IonSessionUpdatedEvent AssetAccessTokenUpdated;

    private:

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

} // namespace Cesium

namespace AZ {
    AZ_TYPE_INFO_SPECIALIZE(Cesium::CesiumIonSession, "{499233C0-47FF-4D80-99E4-649A1C9E4BBE}");
}

namespace Cesium {
    using CesiumIonSessionInterface = AZ::Interface<CesiumIonSession>;
}

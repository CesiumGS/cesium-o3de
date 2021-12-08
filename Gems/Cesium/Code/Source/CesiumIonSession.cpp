#include "CesiumIonSession.h"
#include "CesiumSystemComponentBus.h"
#include "PlatformInfo.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <QDesktopServices>
#include <QUrl>

namespace Cesium
{
    void CesiumIonSession::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<CesiumIonSession, AZ::Component>()
                ->Version(0)
                ->Field("ionAccessToken", &CesiumIonSession::m_ionAccessToken)
                ;
        }
    }

    void CesiumIonSession::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumIonSessionService"));
    }

    void CesiumIonSession::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumIonSessionService"));
    }

    void CesiumIonSession::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("CesiumService"));
    }

    void CesiumIonSession::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("CesiumService"));
    }

    CesiumIonSession::CesiumIonSession()
        : m_asyncSystem{ CesiumInterface::Get()->GetTaskProcessor() }
        , m_assetAccessor{ CesiumInterface::Get()->GetAssetAccessor(IOKind::Http) }
    {
        if (CesiumIonSessionInterface::Get() == nullptr)
        {
            CesiumIonSessionInterface::Register(this);
        }
    }

    void CesiumIonSession::Init()
    {
    }

    void CesiumIonSession::Activate()
    {
    }

    void CesiumIonSession::Deactivate()
    {
    }

    void CesiumIonSession::Connect()
    {
        if (this->IsConnecting() || this->IsConnected() || this->IsResuming())
        {
            return;
        }

        this->m_isConnecting = true;

        CesiumIonClient::Connection::authorize(
            this->m_asyncSystem, this->m_assetAccessor, "Cesium for Unreal", 190, "/cesium-for-unreal/oauth2/callback",
            { "assets:list", "assets:read", "profile:read", "tokens:read", "tokens:write", "geocode" },
            [this](const std::string& url)
            {
                this->m_authorizeUrl = url;
                QDesktopServices::openUrl(QUrl(this->m_authorizeUrl.c_str()));
            })
            .thenInMainThread(
                [this](CesiumIonClient::Connection&& connection)
                {
                    this->m_isConnecting = false;
                    this->m_connection = std::move(connection);

                    m_ionAccessToken = this->m_connection.value().getAccessToken().c_str();

                    this->ConnectionUpdated.Signal();
                })
            .catchInMainThread(
                [this](std::exception&&)
                {
                    this->m_isConnecting = false;
                    this->m_connection = std::nullopt;
                    this->ConnectionUpdated.Signal();
                });
    }

    void CesiumIonSession::Resume()
    {
        if (m_ionAccessToken.empty())
        {
            // No existing session to resume.
            return;
        }

        this->m_isResuming = true;

        this->m_connection = CesiumIonClient::Connection(
            this->m_asyncSystem, this->m_assetAccessor, m_ionAccessToken.c_str());

        // Verify that the connection actually works.
        this->m_connection.value()
            .me()
            .thenInMainThread(
                [this](CesiumIonClient::Response<CesiumIonClient::Profile>&& response)
                {
                    if (!response.value.has_value())
                    {
                        this->m_connection.reset();
                    }
                    this->m_isResuming = false;
                    this->ConnectionUpdated.Signal();
                })
            .catchInMainThread(
                [this](std::exception&&)
                {
                    this->m_isResuming = false;
                    this->m_connection.reset();
                });
    }

    void CesiumIonSession::Disconnect()
    {
        this->m_connection.reset();
        this->m_profile.reset();
        this->m_assets.reset();
        this->m_tokens.reset();
        this->m_assetAccessToken.reset();

        m_ionAccessToken.clear();

        this->ConnectionUpdated.Signal();
        this->ProfileUpdated.Signal();
        this->AssetsUpdated.Signal();
        this->TokensUpdated.Signal();
        this->AssetAccessTokenUpdated.Signal();
    }

    void CesiumIonSession::RefreshProfile()
    {
        if (!this->m_connection || this->m_isLoadingProfile)
        {
            this->m_loadProfileQueued = true;
            return;
        }

        this->m_isLoadingProfile = true;
        this->m_loadProfileQueued = false;

        this->m_connection->me()
            .thenInMainThread(
                [this](CesiumIonClient::Response<CesiumIonClient::Profile>&& profile)
                {
                    this->m_isLoadingProfile = false;
                    this->m_profile = std::move(profile.value);
                    this->ProfileUpdated.Signal();
                    this->RefreshProfileIfNeeded();
                })
            .catchInMainThread(
                [this](std::exception&&)
                {
                    this->m_isLoadingProfile = false;
                    this->m_profile = std::nullopt;
                    this->ProfileUpdated.Signal();
                    this->RefreshProfileIfNeeded();
                });
    }

    void CesiumIonSession::RefreshAssets()
    {
        if (!this->m_connection || this->m_isLoadingAssets)
        {
            return;
        }

        this->m_isLoadingAssets = true;
        this->m_loadAssetsQueued = false;

        this->m_connection->assets()
            .thenInMainThread(
                [this](CesiumIonClient::Response<CesiumIonClient::Assets>&& assets)
                {
                    this->m_isLoadingAssets = false;
                    this->m_assets = std::move(assets.value);
                    this->AssetsUpdated.Signal();
                    this->RefreshAssetsIfNeeded();
                })
            .catchInMainThread(
                [this](std::exception&&)
                {
                    this->m_isLoadingAssets = false;
                    this->m_assets = std::nullopt;
                    this->AssetsUpdated.Signal();
                    this->RefreshAssetsIfNeeded();
                });
    }

    void CesiumIonSession::RefreshTokens()
    {
        if (!this->m_connection || this->m_isLoadingAssets)
        {
            return;
        }

        this->m_isLoadingTokens = true;
        this->m_loadTokensQueued = false;

        this->m_connection->tokens()
            .thenInMainThread(
                [this](CesiumIonClient::Response<std::vector<CesiumIonClient::Token>>&& tokens)
                {
                    this->m_isLoadingTokens = false;
                    this->m_tokens = std::move(tokens.value);
                    this->TokensUpdated.Signal();
                    this->RefreshTokensIfNeeded();
                    this->RefreshAssetAccessTokenIfNeeded();
                })
            .catchInMainThread(
                [this](std::exception&&)
                {
                    this->m_isLoadingTokens = false;
                    this->m_tokens = std::nullopt;
                    this->TokensUpdated.Signal();
                    this->RefreshTokensIfNeeded();
                });
    }

    void CesiumIonSession::RefreshAssetAccessToken()
    {
        if (this->m_isLoadingAssetAccessToken)
        {
            return;
        }

        if (!this->m_connection || !this->IsTokenListLoaded())
        {
            this->m_loadAssetAccessTokenQueued = true;
            this->RefreshTokens();
            return;
        }

        this->m_isLoadingAssetAccessToken = true;
        this->m_loadAssetAccessTokenQueued = false;

        std::string tokenName = PlatformInfo::GetProjectName().c_str();
        tokenName += " (Created by Cesium for O3DE)";

        const std::vector<CesiumIonClient::Token>& tokenList = this->GetTokens();
        const CesiumIonClient::Token* pFound = nullptr;

        for (auto& token : tokenList)
        {
            if (token.name == tokenName)
            {
                pFound = &token;
            }
        }

        if (pFound)
        {
            this->m_assetAccessToken = CesiumIonClient::Token(*pFound);
            this->m_isLoadingAssetAccessToken = false;
            this->AssetAccessTokenUpdated.Signal();
        }
        else
        {
            this->m_connection->createToken(tokenName, { "assets:read" }, std::nullopt)
                .thenInMainThread(
                    [this](CesiumIonClient::Response<CesiumIonClient::Token>&& token)
                    {
                        this->m_assetAccessToken = std::move(token.value);
                        this->m_isLoadingAssetAccessToken = false;
                        this->AssetAccessTokenUpdated.Signal();
                    });
        }
    }

    const std::optional<CesiumIonClient::Connection>& CesiumIonSession::GetConnection() const
    {
        return this->m_connection;
    }

    const CesiumIonClient::Profile& CesiumIonSession::GetProfile()
    {
        static const CesiumIonClient::Profile empty{};
        if (this->m_profile)
        {
            return *this->m_profile;
        }
        else
        {
            this->RefreshProfile();
            return empty;
        }
    }

    const CesiumIonClient::Assets& CesiumIonSession::GetAssets()
    {
        static const CesiumIonClient::Assets empty;
        if (this->m_assets)
        {
            return *this->m_assets;
        }
        else
        {
            this->RefreshAssets();
            return empty;
        }
    }

    const std::vector<CesiumIonClient::Token>& CesiumIonSession::GetTokens()
    {
        static const std::vector<CesiumIonClient::Token> empty;
        if (this->m_tokens)
        {
            return *this->m_tokens;
        }
        else
        {
            this->RefreshTokens();
            return empty;
        }
    }

    const CesiumIonClient::Token& CesiumIonSession::GetAssetAccessToken()
    {
        static const CesiumIonClient::Token empty{};
        if (this->m_assetAccessToken)
        {
            return *this->m_assetAccessToken;
        }
        else
        {
            this->RefreshAssetAccessToken();
            return empty;
        }
    }

    bool CesiumIonSession::RefreshProfileIfNeeded()
    {
        if (this->m_loadProfileQueued || !this->m_profile.has_value())
        {
            this->RefreshProfile();
        }
        return this->IsProfileLoaded();
    }

    bool CesiumIonSession::RefreshAssetsIfNeeded()
    {
        if (this->m_loadAssetsQueued || !this->m_assets.has_value())
        {
            this->RefreshAssets();
        }
        return this->IsAssetListLoaded();
    }

    bool CesiumIonSession::RefreshTokensIfNeeded()
    {
        if (this->m_loadTokensQueued || !this->m_tokens.has_value())
        {
            this->RefreshTokens();
        }
        return this->IsTokenListLoaded();
    }

    bool CesiumIonSession::RefreshAssetAccessTokenIfNeeded()
    {
        if (this->m_loadAssetAccessTokenQueued || !this->m_assetAccessToken.has_value())
        {
            this->RefreshAssetAccessToken();
        }
        return this->IsAssetAccessTokenLoaded();
    }
} // namespace Cesium

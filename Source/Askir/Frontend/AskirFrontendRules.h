#pragma once
#include <cmath>

/** Pure gates used by the native frontend before any session state changes. */
namespace AskirFrontendRules
{
constexpr double FreshStartTimeoutSeconds=30.;
inline bool MakeFreshStartDeadline(double Now,double& OutDeadline)
{
    if(!std::isfinite(Now)||Now<0.||!std::isfinite(Now+FreshStartTimeoutSeconds)||Now+FreshStartTimeoutSeconds<=Now)return false;
    OutDeadline=Now+FreshStartTimeoutSeconds;return true;
}
inline bool FreshStartExpired(double Now,double Deadline)
{
    // Malformed clocks fail toward retaining/recovering the backup, never an endless input lock.
    return !std::isfinite(Now)||Now<0.||!std::isfinite(Deadline)||Deadline<=0.||Now>=Deadline;
}
struct StartReadiness
{
    bool ModeValid=false;
    bool RuntimeReady=false;
    bool PersistenceBusy=false;
    bool FreshStartPending=false;
    bool AuthoritativeTravelPending=false;
    bool TargetWorldPackageValid=false;
};
inline bool CanPrepareStart(const StartReadiness& State)
{
    return State.ModeValid&&State.RuntimeReady&&!State.PersistenceBusy&&!State.FreshStartPending&&
        !State.AuthoritativeTravelPending&&State.TargetWorldPackageValid;
}
inline bool CanStartSession(bool Prepared,bool HasActiveSession,bool DiscardConfirmed)
{
    return Prepared&&(!HasActiveSession||DiscardConfirmed);
}
struct FreshWorldReadiness
{
    bool FreshStartPending=false;
    bool HasSourceBackup=false;
    bool ModeValid=false;
    bool RuntimeReady=false;
    bool AuthoritativeTravelPending=false;
    bool WorldValid=false;
    bool IsOriginalSourceWorld=true;
    bool WorldTearingDown=false;
    bool TargetMapMatches=false;
    bool ControlledPlayerReady=false;
};
inline bool CanCommitFreshWorld(const FreshWorldReadiness& State)
{
    return State.FreshStartPending&&State.HasSourceBackup&&State.ModeValid&&State.RuntimeReady&&
        !State.AuthoritativeTravelPending&&State.WorldValid&&!State.IsOriginalSourceWorld&&
        !State.WorldTearingDown&&State.TargetMapMatches&&State.ControlledPlayerReady;
}
inline bool NeedsAutomaticRecovery(bool WorldValid,bool LocalController,bool LocalViewport,bool RuntimeHUD)
{
    return !(WorldValid&&LocalController&&LocalViewport&&RuntimeHUD);
}
inline bool CanResumeLiveSource(bool WorldValid,bool IsOriginalSourceWorld,bool WorldTearingDown,bool ControlledPlayerReady)
{
    return WorldValid&&IsOriginalSourceWorld&&!WorldTearingDown&&ControlledPlayerReady;
}
}

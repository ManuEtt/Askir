#pragma once
#include <algorithm>
#include <cmath>

// These rules are shared by the live settings backend and the standalone tests.
namespace AskirSettingsRules
{
constexpr double MinSensitivity=.2;
constexpr double MaxSensitivity=3.;
constexpr double ConfirmationDuration=15.;

inline bool ValidVolume(double Value){return std::isfinite(Value)&&Value>=0.&&Value<=1.;}
inline bool ValidSensitivity(double Value){return std::isfinite(Value)&&Value>=MinSensitivity&&Value<=MaxSensitivity;}
inline bool ValidFrameRate(double Value){return std::isfinite(Value)&&(Value==0.||(Value>=30.&&Value<=240.));}
inline bool ValidResolution(int Width,int Height)
{
    // Reject malformed config data before it reaches the window system. Exact
    // display-mode availability remains the responsibility of Unreal and the OS.
    return Width>=640&&Width<=16384&&Height>=480&&Height<=16384&&
        double(Width)/double(Height)>=.5&&double(Width)/double(Height)<=4.;
}
inline bool ValidWindowMode(int Mode){return Mode>=0&&Mode<=2;}
inline bool ValidQuality(int Level){return Level>=-1&&Level<=4;}
inline double EffectiveVolume(double Value){return ValidVolume(Value)?Value:1.;}
inline double EffectiveSensitivity(double Value){return ValidSensitivity(Value)?Value:1.;}
inline double CameraAxis(double Value,double Sensitivity,bool Invert)
{
    if(!std::isfinite(Value))return 0.;
    const double Result=Value*EffectiveSensitivity(Sensitivity)*(Invert?-1.:1.);
    return std::isfinite(Result)?Result:0.;
}
inline bool CanStartConfirmation(double Now,double& Deadline)
{
    if(!std::isfinite(Now)||Now<0.||!std::isfinite(Now+ConfirmationDuration)||Now+ConfirmationDuration<=Now)return false;
    Deadline=Now+ConfirmationDuration;
    return true;
}
inline bool CanConfirm(double Now,double Deadline)
{
    return std::isfinite(Now)&&std::isfinite(Deadline)&&Now>=0.&&Deadline>0.&&Now<Deadline;
}
inline int SecondsRemaining(double Now,double Deadline)
{
    if(!CanConfirm(Now,Deadline))return 0;
    // Capping before the integer conversion also handles corrupt deadlines.
    return int(std::ceil(std::min(ConfirmationDuration,Deadline-Now)));
}
inline bool CanSaveWithoutPreview(int RequestedWidth,int RequestedHeight,int RequestedMode,
    int CurrentWidth,int CurrentHeight,int CurrentMode)
{
    return ValidResolution(RequestedWidth,RequestedHeight)&&ValidWindowMode(RequestedMode)&&
        RequestedWidth==CurrentWidth&&RequestedHeight==CurrentHeight&&RequestedMode==CurrentMode;
}

// The real backend owns one session; UI rebuilds and repeated button events
// cannot replace its rollback point or turn a preview into a disk commit.
class EditSession
{
    bool Editing=false;
    bool Pending=false;
    double Deadline=0.;
public:
    bool IsEditing() const{return Editing;}
    bool HasPreview() const{return Pending;}
    bool CanApply() const{return Editing&&!Pending;}
    bool Begin()
    {
        if(Editing)return false;
        Editing=true;Pending=false;Deadline=0.;return true;
    }
    bool StartPreview(double Now)
    {
        double NewDeadline=0.;
        if(!CanApply()||!CanStartConfirmation(Now,NewDeadline))return false;
        Pending=true;Deadline=NewDeadline;return true;
    }
    bool NeedsRollback(double Now) const{return Pending&&!CanConfirm(Now,Deadline);}
    int Remaining(double Now) const{return Pending?SecondsRemaining(Now,Deadline):0;}
    bool Confirm(double Now)
    {
        if(!Editing||!Pending||!CanConfirm(Now,Deadline))return false;
        Cancel();return true;
    }
    bool RevertPreview()
    {
        if(!Pending)return false;
        Pending=false;Deadline=0.;return true;
    }
    bool Finish()
    {
        if(!CanApply())return false;
        Cancel();return true;
    }
    void Cancel(){Editing=false;Pending=false;Deadline=0.;}
};
}

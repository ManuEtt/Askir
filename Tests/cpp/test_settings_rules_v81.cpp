#include "Settings/AskirSettingsRules.h"
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
int Checks=0;
void Check(bool Condition,const char* Message)
{
    ++Checks;
    if(!Condition){std::cerr<<"FAILED: "<<Message<<'\n';std::exit(1);}
}
}

int main()
{
    using namespace AskirSettingsRules;
    const double NaN=std::numeric_limits<double>::quiet_NaN();
    const double Infinity=std::numeric_limits<double>::infinity();
    for(double Invalid:{NaN,Infinity,-Infinity})
    {
        Check(!ValidVolume(Invalid),"Nonfinite volume rejected before persistence");
        Check(!ValidSensitivity(Invalid),"Nonfinite sensitivity rejected before input");
        Check(!ValidFrameRate(Invalid),"Nonfinite framerate rejected before rendering");
        Check(EffectiveVolume(Invalid)==1.,"Invalid config volume repairs to audible default");
        Check(EffectiveSensitivity(Invalid)==1.,"Invalid config sensitivity repairs to usable default");
        Check(CameraAxis(Invalid,1.,false)==0.,"Invalid input cannot poison camera rotation");
        Check(CameraAxis(2.,Invalid,false)==2.,"Invalid sensitivity repairs before input scaling");
        double Deadline=123.;
        Check(!CanStartConfirmation(Invalid,Deadline)&&Deadline==123.,"Invalid start time cannot mutate confirmation");
        Check(!CanConfirm(Invalid,25.),"Invalid current time cannot confirm preview");
        Check(!CanConfirm(10.,Invalid),"Invalid deadline cannot confirm preview");
        Check(SecondsRemaining(10.,Invalid)==0,"Invalid deadline never reaches unsafe integer conversion");
    }
    Check(ValidVolume(0.)&&ValidVolume(1.),"Volume accepts mute and full amplitude");
    Check(!ValidVolume(-.0001)&&!ValidVolume(1.0001),"Volume bounds reject amplification and negative gain");
    Check(ValidSensitivity(.2)&&ValidSensitivity(3.),"Sensitivity endpoints accepted");
    Check(!ValidSensitivity(.199999)&&!ValidSensitivity(3.000001),"Sensitivity bounds protect camera input");
    Check(CameraAxis(2.,3.,false)==6.&&CameraAxis(2.,3.,true)==-6.,"Camera input uses chosen sensitivity and inversion");
    Check(CameraAxis(-2.,.5,false)==-1.&&CameraAxis(-2.,.5,true)==1.,"Negative camera axis retains correct inversion");
    Check(CameraAxis(std::numeric_limits<double>::max(),3.,false)==0.,"Overflowing camera product denied");
    Check(ValidFrameRate(0.)&&ValidFrameRate(30.)&&ValidFrameRate(240.),"Unlimited and supported framerate bounds accepted");
    Check(!ValidFrameRate(-1.)&&!ValidFrameRate(29.99)&&!ValidFrameRate(240.01),"Out-of-range framerate rejected");
    Check(ValidResolution(640,480)&&ValidResolution(1920,1080)&&ValidResolution(3440,1440),"Minimum, standard, and ultrawide display sizes accepted");
    Check(!ValidResolution(0,0)&&!ValidResolution(-1920,1080)&&!ValidResolution(1920,-1080),"Malformed display dimensions denied");
    Check(!ValidResolution(16385,1080)&&!ValidResolution(1920,16385),"Oversized display dimensions denied");
    Check(!ValidResolution(std::numeric_limits<int>::max(),1),"Integer extreme cannot overflow aspect ratio math");
    Check(!ValidResolution(1921,480)&&ValidResolution(1920,481),"Aspect ratio rejects values beyond four-to-one limit");
    // At exactly 4:1, 1920x480 is a valid narrow surround-style window.
    Check(ValidResolution(2560,640),"Four-to-one aspect boundary accepted");
    Check(!ValidResolution(640,1281)&&ValidResolution(640,1280),"Portrait aspect boundary is bounded");
    for(int Value=-16;Value<=16;++Value)
    {
        Check(ValidWindowMode(Value)==(Value>=0&&Value<=2),"Only Unreal fullscreen mode enum values accepted");
        Check(ValidQuality(Value)==(Value>=-1&&Value<=4),"Scalability accepts custom sentinel and five presets");
    }
    for(int Step=-100;Step<=4000;++Step)
    {
        const double Value=double(Step)/1000.;
        Check(ValidVolume(Value)==(Step>=0&&Step<=1000),"Volume validation across continuous UI range");
        Check(ValidSensitivity(Value)==(Step>=200&&Step<=3000),"Sensitivity validation across continuous UI range");
    }
    double Deadline=0.;
    Check(CanStartConfirmation(10.,Deadline)&&Deadline==25.,"Preview receives fifteen real seconds");
    Check(SecondsRemaining(10.,Deadline)==15,"Countdown begins at fifteen");
    Check(SecondsRemaining(10.001,Deadline)==15,"Fractional first second rounds up");
    Check(SecondsRemaining(24.999,Deadline)==1,"Last fractional second remains confirmable");
    Check(CanConfirm(24.999,Deadline)&&!CanConfirm(25.,Deadline),"At deadline confirmation expires, inclusive timeout");
    Check(SecondsRemaining(25.,Deadline)==0&&SecondsRemaining(26.,Deadline)==0,"Expired previews cannot display negative time");
    Check(!CanStartConfirmation(-1.,Deadline),"Negative monotonic time denied");
    Check(!CanStartConfirmation(std::numeric_limits<double>::max(),Deadline),"Imprecise overflowing deadline denied");
    Check(SecondsRemaining(0.,std::numeric_limits<double>::max())==15,"Corrupt large finite deadline cannot overflow conversion");
    for(int Millisecond=0;Millisecond<=16000;++Millisecond)
    {
        const double Now=10.+double(Millisecond)/1000.;
        const int Remaining=SecondsRemaining(Now,25.);
        Check(CanConfirm(Now,25.)==(Millisecond<15000),"Real-time countdown ignores game pause and dilation");
        Check(Remaining>=0&&Remaining<=15,"Countdown remains within displayed bounds");
        Check((Remaining>0)==CanConfirm(Now,25.),"Displayed confirmation availability matches commit rule");
    }
    for(int Mode=0;Mode<=2;++Mode)
    {
        Check(CanSaveWithoutPreview(1920,1080,Mode,1920,1080,Mode),"Nonvideo changes may save immediately");
        Check(!CanSaveWithoutPreview(2560,1440,Mode,1920,1080,Mode),"Resolution change requires confirmation");
        Check(!CanSaveWithoutPreview(1920,1080,Mode,1920,1080,(Mode+1)%3),"Window-mode change requires confirmation");
    }
    Check(!CanSaveWithoutPreview(0,0,0,0,0,0),"Equal but invalid resolutions cannot bypass preview validation");
    EditSession Session;
    Check(!Session.IsEditing()&&!Session.HasPreview()&&!Session.CanApply(),"Session begins closed");
    Check(!Session.StartPreview(10.)&&!Session.Confirm(10.)&&!Session.Finish(),"Unopened edit cannot preview or commit");
    Check(Session.Begin()&&Session.CanApply(),"Begin enables nonvideo draft");
    Check(!Session.Begin(),"Repeated opening cannot replace the rollback snapshot");
    Check(!Session.StartPreview(NaN)&&Session.CanApply(),"Invalid clock leaves original session intact");
    Check(Session.StartPreview(10.)&&Session.HasPreview()&&!Session.CanApply(),"Video preview freezes the draft until accepted or reverted");
    Check(!Session.StartPreview(11.)&&!Session.Finish(),"Repeated apply cannot reset deadline or persist video without confirmation");
    Check(!Session.NeedsRollback(24.99)&&Session.Remaining(24.99)==1,"Live preview retains final second");
    Check(!Session.Confirm(25.)&&Session.NeedsRollback(25.),"Expired confirm cannot save and requires restoration");
    Check(Session.RevertPreview()&&Session.IsEditing()&&Session.CanApply()&&!Session.HasPreview(),"Timeout rollback preserves original edit snapshot");
    Check(!Session.RevertPreview(),"Duplicate revert does not alter a normal draft");
    Check(Session.StartPreview(100.)&&Session.Confirm(114.99),"Accepted preview commits once before deadline");
    Check(!Session.IsEditing()&&!Session.HasPreview()&&!Session.Confirm(114.99),"Accepted transaction cannot be replayed");
    Check(Session.Begin()&&Session.Finish()&&!Session.IsEditing(),"Nonvideo acceptance closes edit exactly once");
    Check(Session.Begin()&&Session.StartPreview(200.),"New transaction can preview independently");
    Session.Cancel();Session.Cancel();
    Check(!Session.IsEditing()&&!Session.HasPreview()&&Session.Remaining(210.)==0,"Repeated cancellation clears every pending confirmation");
    for(int Millisecond=0;Millisecond<=16000;Millisecond+=10)
    {
        EditSession Attempt;
        Check(Attempt.Begin()&&Attempt.StartPreview(200.),"Fresh preview sequence opens");
        const double Now=200.+double(Millisecond)/1000.;
        const bool Confirmed=Attempt.Confirm(Now);
        Check(Confirmed==(Millisecond<15000),"Production session confirms only within its real-time window");
        if(!Confirmed)
        {
            Check(Attempt.HasPreview()&&Attempt.NeedsRollback(Now),"Rejected late confirmation retains rollback obligation");
            Check(Attempt.RevertPreview()&&Attempt.CanApply(),"Late confirmation can restore and resume editing");
        }
    }
    std::cout<<Checks<<" settings assertions passed\n";
}

// Copyright eeGeo Ltd (2012-2014), All Rights Reserved

#ifndef __ExampleApp__Example__
#define __ExampleApp__Example__

#include "IAppOnMap.h"
#include "Camera.h"
#include "Rendering.h"
#include "RenderCamera.h"
#include "CameraState.h"
#include "VRCameraState.h"
#include "MapModule.h"
#include "CameraFrustumStreamingVolume.h"
#include "../Modules/UI/Animations/IDv3Animateable.h"
#include <string>

namespace Examples
{
    class IExample
    {
        Eegeo::Camera::RenderCamera m_RenderCamera;
    public:
        virtual ~IExample() { }

        virtual std::string Name() const = 0;

        virtual void Start() = 0;
        virtual void EarlyUpdate(float dt) = 0;
        virtual void Update(float dt) = 0;
        virtual void PreWorldDraw() = 0;
        virtual void Draw() = 0;
        virtual void Suspend()= 0;
        virtual void AfterCameraUpdate() { }
        virtual void UpdateHeadOrientation(const float headTansform[]) { }
        virtual void UpdateCardboardProfile(const float cardboardProfile[]){}
        virtual Eegeo::Streaming::IStreamingVolume& GetCurrentStreamingVolume(const Eegeo::Modules::Map::MapModule& mapModule) const { return mapModule.GetStreamingVolume(); }
        
        virtual void NotifyScreenPropertiesChanged(const Eegeo::Rendering::ScreenProperties& screenProperties) = 0;
        
        // TODO: All examples should be returing a RenderCamera used by example, temporaray hack to make it work with VR example.
        virtual Eegeo::Camera::RenderCamera& GetRenderCamera(){return m_RenderCamera;}
        
        virtual Eegeo::Camera::CameraState GetCurrentCameraState() const = 0;
        virtual const Eegeo::VRCamera::VRCameraState& GetCurrentVRCameraState() { return *(new Eegeo::VRCamera::VRCameraState(Eegeo::dv3::Zero(), GetCurrentCameraState(), GetCurrentCameraState(), Eegeo::m33::CreateIdentity(), Eegeo::m33::CreateIdentity(), Eegeo::m33::CreateIdentity())); }

        virtual void NotifyViewNeedsLayout() = 0;
        
        virtual void Event_TouchRotate 			(const AppInterface::RotateData& data) = 0;
        virtual void Event_TouchRotate_Start	(const AppInterface::RotateData& data) = 0;
        virtual void Event_TouchRotate_End 		(const AppInterface::RotateData& data) = 0;
        
        virtual void Event_TouchPinch 			(const AppInterface::PinchData& data) = 0;
        virtual void Event_TouchPinch_Start 	(const AppInterface::PinchData& data) = 0;
        virtual void Event_TouchPinch_End 		(const AppInterface::PinchData& data) = 0;
        
        virtual void Event_TouchPan				(const AppInterface::PanData& data) = 0;
        virtual void Event_TouchPan_Start		(const AppInterface::PanData& data) = 0;
        virtual void Event_TouchPan_End 		(const AppInterface::PanData& data) = 0;
        
        virtual void Event_TouchTap 			(const AppInterface::TapData& data) = 0;
        virtual void Event_TouchDoubleTap		(const AppInterface::TapData& data) = 0;
        virtual void Event_TouchDown 			(const AppInterface::TouchData& data) = 0;
        virtual void Event_TouchMove 			(const AppInterface::TouchData& data) = 0;
        virtual void Event_TouchUp 				(const AppInterface::TouchData& data) = 0;
    };

}

#endif /* defined(__ExampleApp__Example__) */

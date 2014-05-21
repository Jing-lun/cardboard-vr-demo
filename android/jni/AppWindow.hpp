#include "Graphics.h"
#include "AndroidFileIO.h"
#include "AndroidHttpCache.h"
#include "AndroidTextureFileLoader.h"
#include "AndroidWebRequestService.hpp"
#include "ExampleApp.h"
#include "AndroidTaskQueue.h"
#include "AndroidWebLoadRequestFactory.h"
#include "AndroidInputProcessor.h"
#include "AndroidLocationService.h"
#include "AndroidUrlEncoder.h"
#include "GlobeCameraInterestPointProvider.h"
#include "TerrainHeightProvider.h"
#include "AndroidInputHandler.h"
#include "AndroidInputBoxFactory.h"
#include "AndroidKeyboardInputFactory.h"
#include "AndroidAlertBoxFactory.h"
#include "NativeUIFactories.h"
#include "TerrainHeightRepository.h"
#include "GlobalShadowing.h"
#include "ExampleController.h"
#include "UiThreadToNativeThreadTaskQueue.h"
#include "AndroidRouteMatchingExampleViewFactory.h"
#include "AndroidRouteSimulationExampleViewFactory.h"
#include "ExampleCameraJumpController.h"

namespace Eegeo
{
	class EegeoWorld;
}

struct PersistentAppState
{
	PersistentAppState(PersistentAppState* pValues)
	:lastGlobeCameraLatLong(0,0,0)
	{
		valid = pValues!=NULL;
		if(valid)
		{
			lastGlobeCameraLatLong = pValues->lastGlobeCameraLatLong;
			lastGlobeCameraHeadingDegrees = pValues->lastGlobeCameraHeadingDegrees;
			lastGlobeCameraDistanceToInterest = pValues->lastGlobeCameraDistanceToInterest;
		}
	}

	bool valid;
	Eegeo::Space::LatLongAltitude lastGlobeCameraLatLong;
	float lastGlobeCameraHeadingDegrees;
	float lastGlobeCameraDistanceToInterest;
};


class AppWindow : public Eegeo::Android::Input::IAndroidInputHandler
{
private:
	ExampleApp* m_pExampleApp;
	UiThreadToNativeThreadTaskQueue m_uiThreadToNativeThreadTaskQueue;
	Examples::ExampleController* m_pExampleController;
	PersistentAppState persistentState;
	Eegeo::Android::Input::AndroidInputProcessor* pInputProcessor;
	Eegeo::Android::Input::AndroidInputHandler pInputHandler;
	Eegeo::EegeoWorld* pWorld;
	AndroidNativeState* pState;
	int width;
	int height;
    EGLDisplay display;
    EGLSurface surface;
    EGLSurface shareSurface;
    EGLContext context;
    EGLContext resourceBuildShareContext;
    Eegeo::Android::AndroidWebRequestService* pAndroidWebRequestService;
    Eegeo::Android::AndroidFileIO* pFileIO;
    Eegeo::Android::AndroidHttpCache* pHttpCache;
    Eegeo::Android::AndroidTextureFileLoader* pTextureLoader;
    Eegeo::Camera::GlobeCamera::GlobeCameraInterestPointProvider* m_pInterestPointProvider;
	Eegeo::Android::AndroidTaskQueue* pTaskQueue;
	Eegeo::Rendering::RenderContext* pRenderContext;
	Eegeo::Lighting::GlobalLighting *pLighting;
	Eegeo::Lighting::GlobalFogging *pFogging;
	Eegeo::Lighting::GlobalShadowing *pShadowing;
	Eegeo::Blitter* pBlitter;
	Eegeo::Android::AndroidWebLoadRequestFactory* pAndroidWebLoadRequestFactory;
	Eegeo::Android::AndroidLocationService* pAndroidLocationService;
	Eegeo::Android::AndroidUrlEncoder* pAndroidUrlEncoder;
    Eegeo::Resources::Terrain::Heights::TerrainHeightRepository m_terrainHeightRepository;
    Eegeo::Resources::Terrain::Heights::TerrainHeightProvider m_terrainHeightProvider;
    Eegeo::Rendering::EnvironmentFlatteningService* m_pEnvironmentFlatteningService;

	Eegeo::UI::NativeInput::Android::AndroidInputBoxFactory m_androidInputBoxFactory;
	Eegeo::UI::NativeInput::Android::AndroidKeyboardInputFactory m_androidKeyboardInputFactory;
	Eegeo::UI::NativeAlerts::Android::AndroidAlertBoxFactory m_androidAlertBoxFactory;
	Eegeo::UI::NativeUIFactories m_androidNativeUIFactories;

	Examples::AndroidRouteMatchingExampleViewFactory* m_pAndroidRouteMatchingExampleViewFactory;
	Examples::AndroidRouteSimulationExampleViewFactory* m_pAndroidRouteSimulationExampleViewFactory;
	ExampleCameraJumpController* m_pExampleCameraJumpController;

	bool appRunning;
	bool displayAvailable;
	int surfaceChanged;
	bool worldInitialised;
	bool initialStart;
	bool updatedForFirstTime;
	bool displayBound;

	pthread_t m_mainNativeThread;
	pthread_mutex_t m_mutex;

	double m_timeOfLastUpdateCall;

	std::deque<double> m_frameDeltas;

	double CalcSmoothedDelta() const;

	bool InitDisplay();
	void TerminateDisplay(bool teardownEGL);
	void TerminateWorld();
	void InitWorld();
	void UpdateWorld();
	double GetTimeNowSeconds();

	pthread_mutex_t m_inputMutex;
	std::vector<Eegeo::Android::Input::TouchInputEvent> touchBuffer;

	static void* Run(void* self);

	void RegisterAndroidSpecificExamples();
	void DestroyAndroidSpecificExamples();

public:

	AppWindow(AndroidNativeState* pState, PersistentAppState* pPersistentState, bool initialStart);
	~AppWindow();

	void Pause(PersistentAppState* pPersistentState);
	void Resume();
	void ActivateSurface();

	void EnqueuePointerEvent(Eegeo::Android::Input::TouchInputEvent& e);

	Eegeo::EegeoWorld& GetWorld() { return *pWorld; }
	ExampleApp& GetExampleApp() { return *m_pExampleApp; }

	void Event_TouchRotate 			(const AppInterface::RotateData& data);
	void Event_TouchRotate_Start	(const AppInterface::RotateData& data);
	void Event_TouchRotate_End 		(const AppInterface::RotateData& data);

	void Event_TouchPinch 			(const AppInterface::PinchData& data);
	void Event_TouchPinch_Start 	(const AppInterface::PinchData& data);
	void Event_TouchPinch_End 		(const AppInterface::PinchData& data);

	void Event_TouchPan				(const AppInterface::PanData& data);
	void Event_TouchPan_Start		(const AppInterface::PanData& data);
	void Event_TouchPan_End 		(const AppInterface::PanData& data);

	void Event_TouchTap 			(const AppInterface::TapData& data);
	void Event_TouchDoubleTap		(const AppInterface::TapData& data);

	void Event_TouchDown 			(const AppInterface::TouchData& data);
	void Event_TouchMove 			(const AppInterface::TouchData& data);
	void Event_TouchUp 				(const AppInterface::TouchData& data);

	bool Event_KeyPress(const AppInterface::KeyboardData& data) { return false; }
	void AddKeyPressListener(Eegeo::UI::NativeInput::IKeyboardInputKeyPressedHandler* handler) { }
	bool RemoveKeyPressListener(Eegeo::UI::NativeInput::IKeyboardInputKeyPressedHandler* handler) { return false; }
};
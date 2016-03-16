// Copyright eeGeo Ltd (2012-2014), All Rights Reserved

#include "AppRunner.h"
#include "Graphics.h"
#include "AndroidThreadHelper.h"

AppRunner::AppRunner
(
    const std::string& apiKey,
    AndroidNativeState* pNativeState
)
	: m_apiKey(apiKey)
	, m_pNativeState(pNativeState)
	, m_pAppHost(NULL)
	, m_isPaused(false)
{
	Eegeo::Helpers::ThreadHelpers::SetThisThreadAsMainThread();
}

AppRunner::~AppRunner()
{
	bool destroyEGL = true;
	m_displayService.ReleaseDisplay(destroyEGL);

	if(m_pAppHost != NULL)
	{
		Eegeo_DELETE(m_pAppHost);
	}
}

void AppRunner::CreateAppHost()
{
	if(m_pAppHost == NULL && m_displayService.IsDisplayAvailable())
	{
		m_pAppHost = Eegeo_NEW(AppHost)
		             (
		                 m_apiKey,
		                 *m_pNativeState,
		                 m_displayService.GetDisplayWidth(),
		                 m_displayService.GetDisplayHeight(),
		                 m_displayService.GetDisplay(),
		                 m_displayService.GetSharedSurface(),
		                 m_displayService.GetResourceBuildSharedContext()
		             );
	}
}

void AppRunner::Pause()
{
	if(m_pAppHost != NULL && !m_isPaused)
	{
		m_pAppHost->OnPause();
		m_isPaused = true;
	}

	ReleaseDisplay();
}

void AppRunner::Resume()
{
	if(m_pAppHost != NULL && m_isPaused)
	{
		m_pAppHost->OnResume();
	}

	m_isPaused = false;
}

void AppRunner::ActivateSurface()
{
	Pause();
	bool displayBound = TryBindDisplay();
	Eegeo_ASSERT(displayBound);
	CreateAppHost();
	Resume();
}


void AppRunner::HandleTouchEvent(const Eegeo::Android::Input::TouchInputEvent& event)
{
    if(m_pAppHost != NULL)
    {
        m_pAppHost->HandleTouchInputEvent(event);
    }
}

void AppRunner::MagnetTriggered()
{
    if(m_pAppHost != NULL)
    {
        m_pAppHost->MagnetTriggered();
    }
}

void AppRunner::ReleaseDisplay()
{
	if(m_displayService.IsDisplayAvailable())
	{
		const bool teardownEGL = false;
		m_displayService.ReleaseDisplay(teardownEGL);
	}
}

bool AppRunner::TryBindDisplay()
{
	if(m_displayService.TryBindDisplay(*(m_pNativeState->window)))
	{
        
        JNIEnv* env = m_pNativeState->mainThreadEnv;
        jobject activity = m_pNativeState->activity;
        jclass fpl_class = env->GetObjectClass(activity);
        jmethodID undistort = env->GetMethodID(fpl_class, "SetHeadMountedDisplayResolution", "(II)V");
        env->CallVoidMethod(activity, undistort, (jint) (m_displayService.GetDisplayWidth()*2.f), (jint) m_displayService.GetDisplayHeight());
        env->DeleteLocalRef(fpl_class);
        
		if(m_pAppHost != NULL)
		{
			m_pAppHost->SetSharedSurface(m_displayService.GetSharedSurface());
			const Eegeo::Rendering::ScreenProperties& screenProperties = Eegeo::Rendering::ScreenProperties::Make(
					m_displayService.GetDisplayWidth(),
					m_displayService.GetDisplayHeight(),
					1.f,
					m_pNativeState->deviceDpi);
			m_pAppHost->NotifyScreenPropertiesChanged(screenProperties);
			m_pAppHost->SetViewportOffset(0, 0);
		}

		return true;
	}

	return false;
}

void AppRunner::Update(float deltaSeconds, float headTansform[])
{
	if(m_pAppHost != NULL && m_displayService.IsDisplayAvailable())
	{
		m_pAppHost->Update(deltaSeconds, headTansform);

        
		Eegeo_GL(eglSwapBuffers(m_displayService.GetDisplay(), m_displayService.GetSurface()));
        
        Eegeo::Helpers::GLHelpers::ClearBuffers();
        
//        m_displayService.BeginUndistortFramebuffer();
        
        GLuint textureId = m_displayService.FinishUndistortFramebuffer();
		m_pAppHost->Draw(deltaSeconds, headTansform, textureId);
        
        
        JNIEnv* env = m_pNativeState->mainThreadEnv;
        jobject activity = m_pNativeState->activity;
        jclass fpl_class = env->GetObjectClass(activity);
        jmethodID undistort = env->GetMethodID(fpl_class, "UndistortTexture", "(I)V");
        env->CallVoidMethod(activity, undistort, (jint)textureId);
        env->DeleteLocalRef(fpl_class);
	}
}



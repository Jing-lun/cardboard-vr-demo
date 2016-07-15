// Copyright eeGeo Ltd (2012-2014), All Rights Reserved

#pragma once

#include "Types.h"
#include "IJumpPointObserver.h"
#include "IJumpPointObservable.h"
#include "JumpPointView.h"
#include "IJumpPointViewFactory.h"
#include <map>
#include <algorithm>
#include "IUIInteractionObservable.h"
#include "../UI/Animations/AnimationsController.h"

namespace Eegeo
{
    namespace UI
    {
        namespace JumpPoints
        {
            /*!
             * \brief Controller for a set of Jumppoints.
             *
             */
            class JumpPointController : protected Eegeo::NonCopyable, public IJumpPointObserver
            {
            public:
                JumpPointController(IJumpPointObservable& jumpPointObservable
                                    , IJumpPointViewFactory& viewFactory
                                    , IUIInteractionObservable& uiInteractionObservable
                                    , Animations::AnimationsController& animationsController);
                
                ~JumpPointController();
                
                void Update(float deltaTime);
                
                virtual void OnJumpPointAdded(JumpPoint& jumpPoint);
                
                virtual void OnJumpPointRemoved(JumpPoint& jumpPoint);
                
            protected:
                virtual void UpdateViews(float deltaTime);
                
            private:
                IJumpPointObservable& m_jumppointRepository;
                IJumpPointViewFactory& m_viewFactory;
                IUIInteractionObservable& m_uiInteractionObservable;
                Animations::AnimationsController& m_animationsController;
                
                typedef std::map<JumpPoint*, JumpPointView*> TViewsByModel;
                
                TViewsByModel m_viewsByModel;
                
                JumpPointView* GetViewForModel(const JumpPoint& jp) const;
                
                bool HasViewForModel(const JumpPoint& jp) const;
                
            };
        }
    }
}

// Copyright eeGeo Ltd (2012-2014), All Rights Reserved

#include "DeadZoneMenuController.h"
#include "DeadZoneMenuItem.h"
#include "Logger.h"

namespace Eegeo
{
    namespace UI
    {
        namespace DeadZoneMenu
        {
            
            DeadZoneMenuController::DeadZoneMenuController(IDeadZoneMenuItemObservable& deadZoneMenuItemObservable, IDeadZoneMenuItemViewFactory& viewFactory, IUIInteractionObservable& uiInteractionObservable , IUICameraProvider& uiCameraProvider)
            : m_deadZoneMenuItemRepository(deadZoneMenuItemObservable)
            , m_viewFactory(viewFactory)
            , m_pIUIInteractionObservable(uiInteractionObservable)
            , m_uiCameraProvider(uiCameraProvider)
            {
                
                m_isMenuShown = false;
                m_deadZoneMenuItemRepository.AddDeadZoneMenuObserver(this);
            }
            
            DeadZoneMenuController::~DeadZoneMenuController()
            {
                m_deadZoneMenuItemRepository.RemoveDeadZoneMenuObserver(this);
                
                for(TViewsByModel::iterator it = m_viewsByModel.begin(); it != m_viewsByModel.end(); ++it)
                {
                    DeadZoneMenuItemView* pView = it->second;
                    m_pIUIInteractionObservable.UnRegisterInteractableItem(pView);
                    Eegeo_DELETE pView;
                }
                m_viewsByModel.clear();
            }
            
            void DeadZoneMenuController::Update(float deltaTime)
            {
                PositionItems();
                for(TViewsByModel::iterator it = m_viewsByModel.begin(); it != m_viewsByModel.end(); ++it)
                {
                    DeadZoneMenuItemView* pView = it->second;
                    pView->Update(deltaTime);
                }
            }
            
            void DeadZoneMenuController::OnDeadZoneMenuItemAdded(DeadZoneMenuItem& deadZoneMenuItem)
            {
                Eegeo_ASSERT(!HasViewForModel(deadZoneMenuItem), "Attempt to add duplicate model to DeadZoneMenuController.");
                
                DeadZoneMenuItemView* pView = m_viewFactory.CreateViewForDeadZoneMenuItem(deadZoneMenuItem);
                m_viewsByModel[&deadZoneMenuItem] = pView;
                
                m_pIUIInteractionObservable.RegisterInteractableItem(pView);
                m_isMenuShown = false;
            }
            
            void DeadZoneMenuController::OnDeadZoneMenuItemRemoved(DeadZoneMenuItem& deadZoneMenuItem)
            {
                Eegeo_ASSERT(HasViewForModel(deadZoneMenuItem), "Attempt to remove unknown model from DeadZoneMenuController.");
                DeadZoneMenuItemView* pView = GetViewForModel(deadZoneMenuItem);
                
                m_viewsByModel.erase(&deadZoneMenuItem);
                m_pIUIInteractionObservable.UnRegisterInteractableItem(pView);
                
                Eegeo_DELETE(pView);
            }
            
            
            void DeadZoneMenuController::PositionItems()
            {
                m33 headTrackedOrientation;
                m33::Mul(headTrackedOrientation, m_uiCameraProvider.GetBaseOrientation(), m_cachedHeadTracker);
                
                float halfCount = m_viewsByModel.size()/2;
                if(m_viewsByModel.size()%2==0)
                    halfCount-=0.5f;
                
                Eegeo::dv3 center = m_uiCameraProvider.GetRenderCameraForUI().GetEcefLocation();
                
                Eegeo::v3 top(headTrackedOrientation.GetRow(1));
                Eegeo::v3 forward(headTrackedOrientation.GetRow(2));
                
                Eegeo::v3 vA = center.ToSingle();
                Eegeo::v3 vB = m_uiCameraProvider.GetOrientation().GetRow(2);
                float angle = Eegeo::Math::Rad2Deg(Eegeo::Math::ACos(Eegeo::v3::Dot(vA, vB)/(vA.Length()*vB.Length())));
                
                float marginAngle = 150;
                
                int positionMultiplier = -400;
                int radius = 210;
                int degreeDelta = 20;
                
                bool shouldUpdatePosition = false;
                
                if(!m_isMenuShown && angle>marginAngle)
                {
                    m_isMenuShown = true;
                    shouldUpdatePosition = true;
                    m_cachedHeadTracker = m_uiCameraProvider.GetHeadTrackerOrientation();
                    m_cachedCenter = center;
                }
                else if(m_isMenuShown && angle<=marginAngle)
                {
                    m_isMenuShown = false;
                    shouldUpdatePosition = true;
                }
                
                if ((m_cachedCenter - center).LengthSq() > 1) {
                    m_cachedCenter = center;
                    shouldUpdatePosition = true;
                }
                
                if(shouldUpdatePosition || m_isMenuShown)
                {
                    for(TViewsByModel::iterator it = m_viewsByModel.begin(); it != m_viewsByModel.end(); ++it)
                    {
                        DeadZoneMenuItemView* pView = it->second;
                        
                        if(!m_isMenuShown)
                        {
                            pView->SetItemShouldRender(false);
                            continue;
                        }
                        
                        pView->SetItemShouldRender(true);
                        float theta = Math::Deg2Rad(180 + (degreeDelta*halfCount));
                        Eegeo::dv3 k = forward;
                        Eegeo::dv3 v = ((forward*positionMultiplier + top*radius)).Norm()-forward;
                        
                        Eegeo::dv3 vRot = v*Eegeo::Math::Cos(theta) + dv3::Cross(k, v)*Eegeo::Math::Sin(theta) + k*Eegeo::dv3::Dot(k, v)*(1-Eegeo::Math::Cos(theta));
                        
                        Eegeo::dv3 position(center + (vRot.ToSingle()*positionMultiplier) - (top*(35+radius)));
                        pView->SetEcefPosition(position);
                        halfCount-=1;
                        
                    }
                    
                    m_lastCameraPosition = center;
                }
            }
            
            
            DeadZoneMenuItemView* DeadZoneMenuController::GetViewForModel(const DeadZoneMenuItem& jp) const
            {
                TViewsByModel::const_iterator foundJP = m_viewsByModel.find(const_cast<DeadZoneMenuItem*>(&jp));
                
                if(foundJP != m_viewsByModel.end())
                {
                    return foundJP->second;
                }
                
                return NULL;
            }
            
            bool DeadZoneMenuController::HasViewForModel(const DeadZoneMenuItem& jp) const
            {
                return (GetViewForModel(jp) != NULL);
            }
            
            
        }
    }
}
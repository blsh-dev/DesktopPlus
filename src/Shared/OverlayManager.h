#pragma once

#include "ConfigManager.h"

#ifndef DPLUS_UI
    #include "Overlays.h"   //UI app only deals with overlay config data
#endif

static const unsigned int k_ulOverlayID_Dashboard = 0;

class OverlayManager
{
    private:
        #ifndef DPLUS_UI
            std::vector<Overlay> m_Overlays;
        #endif
        std::vector<OverlayConfigData> m_OverlayConfigData;
        unsigned int m_CurrentOverlayID;

    public:
        static OverlayManager& Get();

        OverlayManager();
        unsigned int AddOverlay(const OverlayConfigData& data);
        #ifndef DPLUS_UI
            Overlay& GetOverlay(unsigned int id);
            Overlay& GetCurrentOverlay();
        #endif
        OverlayConfigData& GetConfigData(unsigned int id);
        OverlayConfigData& GetCurrentConfigData();

        unsigned int GetCurrentOverlayID() const;
        void SetCurrentOverlayID(unsigned int id);

        vr::VROverlayHandle_t FindOverlayHandle(unsigned int id); //For UI app since it doesn't keep track of existing overlay handles
        unsigned int GetOverlayCount() const;
        void SwapOverlays(unsigned int id, unsigned int id2);
        void RemoveOverlay(unsigned int id);
};
#pragma once

#include <atomic>
#include "cheat_main/HookManager.h"

namespace cheat {
    namespace feature {
        class Feature;

        inline std::vector<Feature *> m_Features;

        class Feature {
        public:
            Feature(Feature const &) = delete;

            void operator=(Feature const &) = delete;

            virtual void OnGameUpdate(CGameManager *) = 0;

            virtual void BeginHook() = 0;

            virtual void SetActive(bool active) = 0;

            virtual bool IsActive() = 0;

        protected:
            Feature() {
                events::GameUpdateEvent += MY_METHOD_HANDLER(Feature::OnGameUpdate);
                m_Features.push_back(this);
            };
        };
    }
}
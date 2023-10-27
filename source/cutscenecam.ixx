module;

#include <common.hxx>

export module cutscenecam;

import common;

class CutsceneCam
{
public:
    CutsceneCam()
    {
        FusionFix::onInitEvent() += []()
        {
            // By Sergeanur

            auto pattern = find_pattern("74 20 83 FF 03 74 1B 83", "74 24 8B 44 24 2C");
            injector::WriteMemory<uint8_t>(pattern.get_first(), 0xEB, true);

            pattern = find_pattern("E8 ? ? ? ? 8B 4C 24 2C 5F 5E 33 CC B0 01", "E8 ? ? ? ? 8B 4C 24 2C 5F 5E 5B");

            static void* patchOffset = pattern.get_first();

            static void* originalHookster = (void*)(*(ptrdiff_t*)((intptr_t)patchOffset + 1) + 5 + (intptr_t)patchOffset);
            static void* originalHooksterBytePatch = (void*)((intptr_t)originalHookster + 74);
            static double incrementalTimeStep = 0.0;

            pattern = find_pattern("F3 0F 10 05 ? ? ? ? F3 0F 59 05 ? ? ? ? 8B 43 20 53", "F3 0F 10 05 ? ? ? ? F3 0F 59 44 24 ? 83 C4 04 83 7C 24");
            static float& fTimeStep = **pattern.get_first<float*>(4);

            struct CutsceneCamJitterWorkaround
            {
                float data[320];

                bool OriginalHookster(float a2)
                {
                    return ((bool(__thiscall*)(CutsceneCamJitterWorkaround*, float))originalHookster)(this, a2);
                }

                bool Hookster(float a2)
                {
#if 1
                    incrementalTimeStep += fTimeStep;

                    CutsceneCamJitterWorkaround temp = *this;

                    injector::WriteMemory<uint8_t>(originalHooksterBytePatch, 1, true);
                    bool result = OriginalHookster(a2) != 0.0;

                    CutsceneCamJitterWorkaround temp2 = *this;

                    if (incrementalTimeStep < 0.3333)
                        return result;

                    *this = temp;

                    injector::WriteMemory<uint8_t>(originalHooksterBytePatch, 0, true);
                    bool result2 = OriginalHookster(a2) != 0.0;

                    temp = *this;

                    if (fabs(temp.data[8] - temp2.data[8]) > 0.05f
                        || fabs(temp.data[9] - temp2.data[9]) > 0.05f
                        || fabs(temp.data[10] - temp2.data[10]) > 0.05f
                        || fabs(temp.data[16] - temp2.data[16]) > 0.5f
                        || fabs(temp.data[17] - temp2.data[17]) > 0.5f
                        || fabs(temp.data[18] - temp2.data[18]) > 0.5f)
                    {
                        incrementalTimeStep = 0.0;
                        *this = temp2;
                        return result;
                    }
                    return result2;
#else
                    return OriginalHookster(a2) != 0.0;
#endif
                }
            };

            auto dest = &CutsceneCamJitterWorkaround::Hookster;
            injector::MakeCALL(patchOffset, *(void**)&dest, true);

            pattern = find_pattern("E8 ? ? ? ? 8B CD 88 44 24 0F", "E8 ? ? ? ? 8B CF 88 44 24 0F");
            injector::MakeCALL(pattern.get_first(), *(void**)&dest, true);
        };
    }
} CutsceneCam;
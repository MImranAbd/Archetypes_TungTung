#pragma once

#include <raylib.h>
#include "enemies.h"

namespace Audio
{
    enum class Effect
    {
        AccuracyDown,
        AreaFireAttack,
        BasicPhysical,
        BuffAttack,
        BuffDefense,
        DefenseDown,
        DarkAttack,
        EnemyBuffing,
        EvasionUp,
        Heal,
        HeavyPhysical,
        IceAttack,
        LightAttack,
        Miss,
        SelectInBattle,
        Select,
        SingleFireAttack,
        Victory,
        Count
    };

    struct State
    {
        Sound sounds[static_cast<int>(Effect::Count)] = {};
    };

    inline State& GetState()
    {
        static State state;
        return state;
    }

    inline bool IsLoaded(const Sound& sound)
    {
        return sound.stream.buffer != nullptr;
    }

    inline const char* AssetPath(Effect effect)
    {
        switch (effect)
        {
            case Effect::AccuracyDown: return "res/ost/ACU Down.mp3";
            case Effect::AreaFireAttack: return "res/ost/AOE Fire Attack.mp3";
            case Effect::BasicPhysical: return "res/ost/Basic Physical.mp3";
            case Effect::BuffAttack: return "res/ost/Buff ATK.mp3";
            case Effect::BuffDefense: return "res/ost/Buff DEF.mp3";
            case Effect::DefenseDown: return "res/ost/DEF Down.mp3";
            case Effect::DarkAttack: return "res/ost/Dark Attack.mp3";
            case Effect::EnemyBuffing: return "res/ost/Enemy Buffing.mp3";
            case Effect::EvasionUp: return "res/ost/EVA UP.mp3";
            case Effect::Heal: return "res/ost/Heal Sound.mp3";
            case Effect::HeavyPhysical: return "res/ost/Heavy Physical Strike.mp3";
            case Effect::IceAttack: return "res/ost/Ice Attack.mp3";
            case Effect::LightAttack: return "res/ost/Light Attack.mp3";
            case Effect::Miss: return "res/ost/MISS.mp3";
            case Effect::SelectInBattle: return "res/ost/Select In Battle.mp3";
            case Effect::Select: return "res/ost/Select.mp3";
            case Effect::SingleFireAttack: return "res/ost/Single Fire Attack.mp3";
            case Effect::Victory: return "res/ost/Victory.mp3";
            case Effect::Count: break;
        }

        return "";
    }

    inline void Load()
    {
        if (!IsAudioDeviceReady()) return;

        State& state = GetState();

        for (int i = 0; i < static_cast<int>(Effect::Count); i++)
        {
            const Effect effect = static_cast<Effect>(i);
            state.sounds[i] = LoadSound(AssetPath(effect));

            if (!IsLoaded(state.sounds[i]))
                TraceLog(LOG_WARNING, "Audio: unable to load sound: %s", AssetPath(effect));
        }
    }

    inline void Play(Effect effect)
    {
        if (!IsAudioDeviceReady()) return;

        Sound sound = GetState().sounds[static_cast<int>(effect)];
        if (IsLoaded(sound)) PlaySound(sound);
    }

    inline void PlayMiss()
    {
        if (!IsAudioDeviceReady()) return;

        for (Sound& sound : GetState().sounds)
        {
            if (IsLoaded(sound))
                StopSound(sound);
        }

        Play(Effect::Miss);
    }

    inline void PlayAttack(Element element, int power, bool allTargets)
    {
        switch (element)
        {
            case Element::Physical:
                Play(power >= 2 ? Effect::HeavyPhysical : Effect::BasicPhysical);
                break;
            case Element::Ice:
                Play(Effect::IceAttack);
                break;
            case Element::Fire:
                Play(allTargets ? Effect::AreaFireAttack : Effect::SingleFireAttack);
                break;
            case Element::Light:
                Play(Effect::LightAttack);
                break;
            case Element::Dark:
                Play(Effect::DarkAttack);
                break;
        }
    }

    inline void Unload()
    {
        if (!IsAudioDeviceReady()) return;

        State& state = GetState();

        for (Sound& sound : state.sounds)
        {
            if (IsLoaded(sound))
                UnloadSound(sound);

            sound = {};
        }
    }
}

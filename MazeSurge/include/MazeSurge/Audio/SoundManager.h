#pragma once
#include "MazeSurge/Core/Core.h"
#include <xaudio2.h>
#include <memory>
#include <cstdint>

class SoundManager
{
public:
    // 初期化
    bool Init();

    // 破棄
    void Cleanup();

    // GameBGMを再生
    void PlayGameBGM();

    // TitleBGMを再生
    void PlayTitleBGM();

    // 音楽の再生を終了
    void Stop();

private:
    IXAudio2 *m_xaudio;
    IXAudio2MasteringVoice *m_masteringVoice;
    IXAudio2SourceVoice *m_sourceVoice;

    // 各wavファイルを記憶しておくポインタ
    std::unique_ptr<uint8_t[]> m_waveGameBGM;
    std::unique_ptr<uint8_t[]> m_waveTitleBGM;
};

extern SoundManager g_soundManager;
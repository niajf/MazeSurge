#pragma once
#include "MazeSurge/Core/Core.h"
#include <xaudio2.h>
#include <memory>
#include <cstdint>

class SoundManager
{
public:
    // 唯一のインスタンスを返す関数
    static SoundManager &GetInstance();

    // 初期化
    bool Init();

    // 破棄
    void Cleanup();

    // GameBGMを再生
    void PlayGameBGM();

    // TitleBGMを再生
    void PlayTitleBGM();

    // 弾が敵に衝突したときのSEを再生
    void PlayHitEnemySE();

    // BGMの再生を終了
    void StopBGM();

    // SEの再生を終了
    void StopSE();

private:
    // コンストラクタへの外部アクセス禁止
    SoundManager() = default;
    ~SoundManager() = default;

    // コピーコンストラクタと代入演算子を禁止
    SoundManager(const SoundManager &) = default;
    SoundManager &operator=(const SoundManager &) = default;

    IXAudio2 *m_xaudio;
    IXAudio2MasteringVoice *m_masteringVoice;
    IXAudio2SourceVoice *m_sourceVoiceBGM;
    IXAudio2SourceVoice *m_sourceVoiceSE;

    // 各wavファイルを記憶しておくポインタ
    std::unique_ptr<uint8_t[]> m_waveGameBGM;
    std::unique_ptr<uint8_t[]> m_waveTitleBGM;
    std::unique_ptr<uint8_t[]> m_waveHitEnemySE;
};
#pragma once
#include "MazeSurge/Core/Core.h"
#include <xaudio2.h>
#include <memory>
#include <cstdint>
#include <optional>

struct SoundData
{
    std::unique_ptr<uint8_t[]> data; // バイト列を所有
    IXAudio2SourceVoice *sourceVoice = nullptr;
    const uint8_t *start = nullptr; // data 内の再生開始位置
    uint32_t bytes = 0;
    uint32_t loopBegin = 0;
    uint32_t loopLength = 0;

    void Release()
    {
        if (sourceVoice)
        {
            sourceVoice->Stop();
            sourceVoice->DestroyVoice();
            sourceVoice = nullptr;
        }
        data.reset();
        start = nullptr;
        bytes = 0;
        loopBegin = 0;
        loopLength = 0;
    }
};

class SoundManager
{
public:
    // 唯一のインスタンスを返す関数
    static SoundManager &GetInstance();

    // 初期化
    bool Init();

    // 破棄
    void Cleanup();

    // SEのサウンドデータを作成
    bool CreateSoundData(const wchar_t *filePath, SoundData &soundData);

    // SEをまとめてセットする
    bool SetSE();

    // GameBGMを再生
    void PlayGameBGM();

    // TitleBGMを再生
    void PlayTitleBGM();

    // 弾が敵に衝突したときのSEを再生
    void PlayHitEnemySE();

    // 敵がプレイヤーに衝突したときのSEを再生
    void PlayHitPlayerSE();

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

    IXAudio2 *m_xaudio = nullptr;
    IXAudio2MasteringVoice *m_masteringVoice = nullptr;

    // 音声の再生に使う情報を保持
    SoundData m_BGM;
    SoundData m_hitEnemySE;
    SoundData m_hitPlayerSE;
};
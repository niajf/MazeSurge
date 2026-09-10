#include "MazeSurge/Audio/SoundManager.h"
#include "MazeSurge/Audio/WAVFileReader.h"
#include <vector>
#include <string>

// グローバルSoundManagerインスタンスの定義
SoundManager g_soundManager;

bool SoundManager::Init()
{
    //
    //  Initialize XAudio2
    //
    HRESULT hr;
    if (FAILED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
        return false;

    UINT32 flags = 0;
#ifdef _DEBUG
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif
    if (FAILED(hr = XAudio2Create(&m_xaudio, flags)))
        return false;

    //
    //  Create a mastering voice
    //
    if (FAILED(hr = m_xaudio->CreateMasteringVoice(&m_masteringVoice)))
        return false;

    return true;
}

void SoundManager::Cleanup()
{
    //
    // Cleanup XAudio2
    //
    if (m_masteringVoice != 0)
    {
        m_masteringVoice->DestroyVoice();
        m_masteringVoice = 0;
    }
    if (m_xaudio != 0)
    {
        m_xaudio->Release();
        m_xaudio = 0;
    }
    CoUninitialize();
}

void SoundManager::PlayGameBGM()
{
    //
    //  WAVファイルを開く
    //
    DirectX::WAVData waveData{0};

    HRESULT hr = DirectX::LoadWAVAudioFromFileEx(Audio::GAME_BGM_PATH, m_waveGameBGM, waveData);
    if (FAILED(hr))
        throw "LoadWAVAudioFromFileEx";

    //
    //  WAVファイルのWAVEFORMATEXを使ってSourceVoiceを作成
    //
    if (FAILED(m_xaudio->CreateSourceVoice(&m_sourceVoiceBGM, waveData.wfx)))
        throw "CreateSourceVoice";

    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = waveData.startAudio;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = waveData.audioBytes;
    m_sourceVoiceBGM->SubmitSourceBuffer(&buffer);

    m_sourceVoiceBGM->Start(0);
}

void SoundManager::PlayTitleBGM()
{
    //
    //  WAVファイルを開く
    //
    DirectX::WAVData waveData{0};

    HRESULT hr = DirectX::LoadWAVAudioFromFileEx(Audio::TITLE_BGM_PATH, m_waveGameBGM, waveData);
    if (FAILED(hr))
        throw "LoadWAVAudioFromFileEx";

    //
    //  WAVファイルのWAVEFORMATEXを使ってSourceVoiceを作成
    //
    if (FAILED(m_xaudio->CreateSourceVoice(&m_sourceVoiceBGM, waveData.wfx)))
        throw "CreateSourceVoice";

    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = waveData.startAudio;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = waveData.audioBytes;
    m_sourceVoiceBGM->SubmitSourceBuffer(&buffer);

    m_sourceVoiceBGM->Start(0);
}

void SoundManager::PlayHitEnemySE()
{
    //
    //  WAVファイルを開く
    //
    DirectX::WAVData waveData{0};

    HRESULT hr = DirectX::LoadWAVAudioFromFileEx(Audio::HIT_ENEMY_SE_PATH, m_waveHitEnemySE, waveData);
    if (FAILED(hr))
        throw "LoadWAVAudioFromFileEx";

    //
    //  WAVファイルのWAVEFORMATEXを使ってSourceVoiceを作成
    //
    if (FAILED(m_xaudio->CreateSourceVoice(&m_sourceVoiceSE, waveData.wfx)))
        throw "CreateSourceVoice";

    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = waveData.startAudio;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = waveData.audioBytes;
    m_sourceVoiceSE->SubmitSourceBuffer(&buffer);

    m_sourceVoiceSE->Start(0);
}

void SoundManager::StopBGM()
{
    //
    // SourceVoiceの破棄
    //
    m_sourceVoiceBGM->Stop();
    m_sourceVoiceBGM->DestroyVoice();
}

void SoundManager::StopSE()
{
    //
    // SourceVoiceの破棄
    //
    m_sourceVoiceSE->Stop();
    m_sourceVoiceSE->DestroyVoice();
}
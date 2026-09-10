#include "MazeSurge/Audio/SoundManager.h"
#include "MazeSurge/Audio/WAVFileReader.h"
#include <vector>
#include <string>

SoundManager &SoundManager::GetInstance()
{
    static SoundManager instance;
    return instance;
}

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

    // SEの生成
    if (!SetSE())
        return false;

    return true;
}

void SoundManager::Cleanup()
{
    //
    // Cleanup XAudio2
    //
    m_BGM.Release();
    m_hitEnemySE.Release();
    m_hitPlayerSE.Release();

    if (m_masteringVoice)
    {
        m_masteringVoice->DestroyVoice();
        m_masteringVoice = nullptr;
    }

    if (m_xaudio)
    {
        m_xaudio->Release();
        m_xaudio = nullptr;
    }
}

void SoundManager::PlayGameBGM()
{

    m_BGM.Release();

    //
    //  WAVファイルを開く
    //
    DirectX::WAVData waveData{0};

    HRESULT hr = DirectX::LoadWAVAudioFromFileEx(Audio::GAME_BGM_PATH, m_BGM.data, waveData);
    if (FAILED(hr))
        throw "LoadWAVAudioFromFileEx";

    //
    //  WAVファイルのWAVEFORMATEXを使ってSourceVoiceを作成
    //
    if (FAILED(m_xaudio->CreateSourceVoice(&m_BGM.sourceVoice, waveData.wfx)))
        throw "CreateSourceVoice";

    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = waveData.startAudio;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = waveData.audioBytes;
    m_BGM.sourceVoice->SubmitSourceBuffer(&buffer);

    // 再生
    m_BGM.sourceVoice->Start(0);
}

void SoundManager::PlayTitleBGM()
{

    m_BGM.Release();

    //
    //  WAVファイルを開く
    //
    DirectX::WAVData waveData{0};

    HRESULT hr = DirectX::LoadWAVAudioFromFileEx(Audio::TITLE_BGM_PATH, m_BGM.data, waveData);
    if (FAILED(hr))
        throw "LoadWAVAudioFromFileEx";

    //
    //  WAVファイルのWAVEFORMATEXを使ってSourceVoiceを作成
    //
    if (FAILED(m_xaudio->CreateSourceVoice(&m_BGM.sourceVoice, waveData.wfx)))
        throw "CreateSourceVoice";

    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = waveData.startAudio;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = waveData.audioBytes;
    m_BGM.sourceVoice->SubmitSourceBuffer(&buffer);

    m_BGM.sourceVoice->Start(0);
}

bool SoundManager::CreateSoundData(const wchar_t *filePath, SoundData &soundData)
{
    //
    //  WAVファイルを開く
    //
    DirectX::WAVData waveData{0};

    HRESULT hr = DirectX::LoadWAVAudioFromFileEx(filePath, soundData.data, waveData);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"wavファイルの読み込みに失敗", L"エラー", MB_OK);
        return false;
    }

    //
    //  WAVファイルのWAVEFORMATEXを使ってSourceVoiceを作成
    //
    if (FAILED(m_xaudio->CreateSourceVoice(&soundData.sourceVoice, waveData.wfx)))
    {
        MessageBox(nullptr, L"sourceVoiceの作成に失敗", L"エラー", MB_OK);
        return false;
    }

    soundData.start = waveData.startAudio;
    soundData.bytes = waveData.audioBytes;

    return true;
}

bool SoundManager::SetSE()
{
    // サウンドデータを作成し、保存
    if (!CreateSoundData(Audio::HIT_ENEMY_SE_PATH, m_hitEnemySE))
        return false;

    if (!CreateSoundData(Audio::HIT_PLAYER_SE_PATH, m_hitPlayerSE))
        return false;

    if (!CreateSoundData(Audio::GET_CP_SE_PATH, m_getCpSE))
        return false;

    return true;
}

void SoundManager::PlayHitEnemySE()
{
    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = m_hitEnemySE.start;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = m_hitEnemySE.bytes;
    m_hitEnemySE.sourceVoice->SubmitSourceBuffer(&buffer);

    // 再生
    m_hitEnemySE.sourceVoice->Start(0);
}

void SoundManager::PlayHitPlayerSE()
{
    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = m_hitPlayerSE.start;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = m_hitPlayerSE.bytes;
    m_hitPlayerSE.sourceVoice->SubmitSourceBuffer(&buffer);

    // 再生
    m_hitPlayerSE.sourceVoice->Start(0);
}

void SoundManager::PlayGetCpSE()
{
    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = m_getCpSE.start;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = m_getCpSE.bytes;
    m_getCpSE.sourceVoice->SubmitSourceBuffer(&buffer);

    // 再生
    m_getCpSE.sourceVoice->Start(0);
}
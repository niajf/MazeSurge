#include "MazeSurge/Audio/SoundManager.h"
#include "MazeSurge/External/dr_mp3.h"
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
    if (!LoadMp3(Audio::GAME_BGM_PATH, m_BGM))
        throw "LoadMp3";

    PlayBGM();
}

void SoundManager::PlayTitleBGM()
{

    m_BGM.Release();
    if (!LoadMp3(Audio::TITLE_BGM_PATH, m_BGM))
        throw "LoadMp3";

    PlayBGM();
}

void SoundManager::PlayClearBGM()
{

    m_BGM.Release();
    if (!LoadMp3(Audio::CLEAR_BGM_PATH, m_BGM))
        throw "LoadMp3";

    PlayBGM();
}

void SoundManager::PlayOverBGM()
{

    m_BGM.Release();
    if (!LoadMp3(Audio::OVER_BGM_PATH, m_BGM))
        throw "LoadMp3";

    PlayBGM();
}

// AI生成
bool SoundManager::LoadMp3(const wchar_t *filePath, SoundData &soundData)
{
    drmp3 mp3;
    if (!drmp3_init_file_w(&mp3, filePath, nullptr))
    {
        MessageBox(nullptr, L"mp3ファイルの読み込みに失敗", L"エラー", MB_OK);
        return false;
    }

    drmp3_uint64 totalFrames = drmp3_get_pcm_frame_count(&mp3);

    if (totalFrames == 0)
    {
        drmp3_uninit(&mp3);
        return false;
    }

    // s16 で全展開（f32 の半分のメモリ。1回だけの変換コストは無視できる）
    uint32_t channels = mp3.channels;
    uint32_t sampleRate = mp3.sampleRate;
    size_t sampleCount = static_cast<size_t>(totalFrames) * channels;
    auto pcm = std::make_unique<uint8_t[]>(sampleCount * sizeof(int16_t));

    drmp3_seek_to_pcm_frame(&mp3, 0);
    const drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(
        &mp3, totalFrames, reinterpret_cast<drmp3_int16 *>(pcm.get()));
    drmp3_uninit(&mp3);

    if (framesRead == 0)
        return false;

    WAVEFORMATEX wfx{};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = static_cast<WORD>(channels);
    wfx.nSamplesPerSec = sampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = static_cast<WORD>(channels * sizeof(int16_t));
    wfx.nAvgBytesPerSec = sampleRate * wfx.nBlockAlign;

    IXAudio2SourceVoice *voice = nullptr;
    if (FAILED(m_xaudio->CreateSourceVoice(&voice, &wfx)))
    {
        MessageBox(nullptr, L"sourceVoiceの作成に失敗", L"エラー", MB_OK);
        return false;
    }

    soundData.data = std::move(pcm);
    soundData.start = soundData.data.get();
    soundData.bytes = static_cast<uint32_t>(framesRead * channels * sizeof(int16_t));
    soundData.sourceVoice = voice;

    return true;
}

bool SoundManager::SetSE()
{
    // サウンドデータを作成し、保存
    if (!LoadMp3(Audio::HIT_ENEMY_SE_PATH, m_hitEnemySE))
        return false;

    if (!LoadMp3(Audio::HIT_PLAYER_SE_PATH, m_hitPlayerSE))
        return false;

    if (!LoadMp3(Audio::GET_CP_SE_PATH, m_getCpSE))
        return false;

    if (!LoadMp3(Audio::RANK_UP_SE_PATH, m_rankUpSE))
        return false;

    if (!LoadMp3(Audio::SELECT_SE_PATH, m_selectSE))
        return false;

    return true;
}

void SoundManager::PlaySE(SoundData &soundData)
{
    // キューにつまれたSEを削除（呼んだタイミングで必ず再生させるため）
    soundData.sourceVoice->Stop();
    soundData.sourceVoice->FlushSourceBuffers();

    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = soundData.start;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = soundData.bytes;
    soundData.sourceVoice->SubmitSourceBuffer(&buffer);

    // 再生
    soundData.sourceVoice->Start(0);
}

void SoundManager::PlayBGM()
{
    //
    //  SourceVoiceにデータを送信
    //
    XAUDIO2_BUFFER buffer{0};
    buffer.pAudioData = m_BGM.start;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = m_BGM.bytes;
    buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    m_BGM.sourceVoice->SubmitSourceBuffer(&buffer);

    // 再生
    m_BGM.sourceVoice->Start(0);
}
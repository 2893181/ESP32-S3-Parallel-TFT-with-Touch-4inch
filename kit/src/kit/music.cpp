#include "music.h"

#define MUSIC_LIMIT 30

int mute_flag = 0;

// Thermal alert variables
int alert_playing = 0;
int audio_initialized = 0;

String music_list[MUSIC_LIMIT];
int music_num = 0;
int music_index = 0;


// ======================================================
// NORMAL MUSIC INITIALIZATION
// ======================================================

void music_init()
{
    // Read SD
    music_num = get_music_list(SD, "/", music_list, MUSIC_LIMIT);

    Serial.print("Music file count:");
    Serial.println(music_num);

    Serial.println("All music:");

    for (int i = 0; i < music_num; i++)
    {
        Serial.println(music_list[i]);
    }


    // Init Audio
    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT);
    audio.setVolume(21);


    // Only start music if files exist
    if (music_num > 0)
    {
        open_new_song(music_list[music_index]);
    }


    // Only create the audio task once
    if (audio_initialized == 0)
    {
        xTaskCreatePinnedToCore(
            Task_Audio,
            "Task_Audio",
            10240,
            NULL,
            2,
            NULL,
            1
        );

        audio_initialized = 1;
    }
}


// ======================================================
// THERMAL ALERT AUDIO INITIALIZATION
// ======================================================

void alert_audio_init()
{
    Serial.println("Initializing thermal alert audio...");

    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT);
    audio.setVolume(21);


    // Only create audio task once
    if (audio_initialized == 0)
    {
        xTaskCreatePinnedToCore(
            Task_Audio,
            "Task_Audio",
            10240,
            NULL,
            2,
            NULL,
            1
        );

        audio_initialized = 1;
    }


    Serial.println("Thermal alert audio ready.");
}


// ======================================================
// 30 C TEMPERATURE WARNING
// ======================================================

void play_temp_warning()
{
    if (mute_flag == 0 && alert_playing == 0)
    {
        Serial.println("WARNING: Temperature >= 30 C");

        alert_playing = 1;

        audio.connecttoFS(
            SD,
            "/temp_warning.mp3"
        );
    }
}


// ======================================================
// 36 C POSSIBLE HUMAN HEAT ALERT
// ======================================================

void play_human_alert()
{
    if (mute_flag == 0)
    {
        Serial.println("ALERT: Temperature >= 36 C");
        Serial.println("Possible human heat signature detected");

        alert_playing = 1;

        audio.connecttoFS(
            SD,
            "/human_alert.mp3"
        );
    }
}


// ======================================================
// MUTE / UNMUTE
// ======================================================

void music_mute()
{
    if (mute_flag == 0)
    {
        mute_flag = 1;
        audio.setVolume(0);
    }
    else
    {
        mute_flag = 0;
        audio.setVolume(21);
    }
}


// ======================================================
// AUDIO TASK
// ======================================================

void Task_Audio(void *pvParameters)
{
    while (1)
    {
        audio.loop();

        vTaskDelay(1);
    }
}


// ======================================================
// READ AUDIO FILES FROM SD CARD
// ======================================================

int get_music_list(
    fs::FS &fs,
    const char *dirname,
    String *list,
    int length
)
{
    Serial.printf("Listing directory: %s\n", dirname);

    int i = 0;

    File root = fs.open(dirname);

    if (!root)
    {
        Serial.println("Failed to open directory");
        return i;
    }

    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return i;
    }


    File file = root.openNextFile();


    while (file)
    {
        if (file.isDirectory())
        {
        }
        else
        {
            String temp = file.name();


            if (temp.endsWith(".wav"))
            {
                list[i] = temp;
                i++;
            }

            else if (temp.endsWith(".mp3"))
            {
                list[i] = temp;
                i++;
            }


            if (i >= MUSIC_LIMIT)
            {
                return i;
            }
        }


        file = root.openNextFile();
    }


    return i;
}


// ======================================================
// START NEW AUDIO FILE
// ======================================================

void open_new_song(String filename)
{
    audio.connecttoFS(
        SD,
        filename.c_str()
    );

    Serial.println(
        "**********start a new sound************"
    );
}


// ======================================================
// MP3 FINISHED
// ======================================================

void audio_eof_mp3(const char *info)
{
    Serial.print("eof_mp3: ");
    Serial.println(info);


    // If thermal alert finished, stop.
    // Do not automatically start normal music.
    if (alert_playing == 1)
    {
        alert_playing = 0;

        Serial.println("Thermal alert finished.");

        return;
    }


    // No music available
    if (music_num <= 0)
    {
        return;
    }


    // Normal music behaviour
    music_index++;


    if (music_index >= music_num)
    {
        music_index = 0;
    }


    open_new_song(
        music_list[music_index]
    );
}


// ======================================================
// NEXT SONG
// ======================================================

void next_song()
{
    audio.pauseResume();

    vTaskDelay(500);


    if (music_num <= 0)
    {
        return;
    }


    if (music_index < music_num - 1)
    {
        music_index++;
    }

    else
    {
        music_index = 0;
    }


    open_new_song(
        music_list[music_index]
    );


    if (mute_flag == 1)
    {
        mute_flag = 0;

        audio.setVolume(21);
    }
}

#include "page.h"

int touch_x_temp = 0;
int touch_y_temp = 0;

// Pages
void page_main()
{
    Serial.println(F("Main page"));

    gfx->fillScreen(COLOR_BACKGROUND);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(3);

    gfx->setCursor(20, 20);
    gfx->println("Makerfabs ESP32-S3");
    gfx->setCursor(20, 50);
    gfx->println("4inch Development Kit");

    Button b[B_NUM_MAIN];
    String b_name[] = {"A", "B", "C", "D"};

    b[0].set(20, 200, 200, 80, "Music Frame", ENABLE);
    b[1].set(240, 200, 200, 80, "Global Weather", ENABLE);
    b[2].set(20, 320, 200, 80, "TVOC Monitor", ENABLE);
    b[3].set(240, 320, 200, 80, "Thermal Camera", ENABLE);

    drawButton(b[0]);
    drawButton(b[1]);
    drawButton(b[2]);
    drawButton(b[3]);

    while (1)
    {

        if (touch_flag == 1)
        {
            // 防止按键检测期间被修改
            touch_x_temp = touch_x;
            touch_y_temp = touch_y;

            if (b[0].checkTouch(touch_x_temp, touch_y_temp) != UNABLE)
            {
                touch_flag = 0;
                page_1();
            }
            if (b[1].checkTouch(touch_x_temp, touch_y_temp) != UNABLE)
            {
                touch_flag = 0;
                page_2();
            }
            if (b[2].checkTouch(touch_x_temp, touch_y_temp) != UNABLE)
            {
                touch_flag = 0;
                page_3();
            }
            if (b[3].checkTouch(touch_x_temp, touch_y_temp) != UNABLE)
            {
                touch_flag = 0;
                page_4();
            }
            touch_flag = 0;
        }
        vTaskDelay(10);
    }
}

// 电子相册
void page_1()
{
    Serial.println(F("1 page"));

    Button b_mute;
    Button b_next;

    b_mute.set(20, 410, 120, 60, "Mute", ENABLE);
    b_next.set(160, 410, 120, 60, "Next", ENABLE);

    gfx->fillScreen(BLACK);

    img_init();
    drawButton(b_mute);
    drawButton(b_next);

    music_init();

    vTaskDelay(500);

    long runtime_1 = 0;

    while (1)
    {
        if ((millis() - runtime_1) > 3000)
        {
            img_next();
            drawButton(b_mute);
            drawButton(b_next);
            runtime_1 = millis();
        }

        if (touch_flag == 1)
        {
            touch_x_temp = touch_x;
            touch_y_temp = touch_y;

            if (b_mute.checkTouch(touch_x_temp, touch_y_temp) != UNABLE)
            {
                vTaskDelay(200);
                Serial.println(F("Mute"));
                music_mute();
            }

            if (b_next.checkTouch(touch_x_temp, touch_y_temp) != UNABLE)
            {
                vTaskDelay(200);
                Serial.println(F("Next"));
                next_song();
            }
            touch_flag = 0;
        }
        vTaskDelay(100);
    }
}

// 时钟+天气
void page_2()
{
    Serial.println(F("2 page"));

    Button b_countrys[B_COUNTY_NUM];
    int country_index = 0;

    for (int i = 0; i < B_COUNTY_NUM; i++)
    {
        b_countrys[i].set(20 + 110 * (i % (B_COUNTY_NUM / 2)), 330 + 70 * (i / (B_COUNTY_NUM / 2)), 100, 60, flag_list[i], ENABLE);
    }

    gfx->fillScreen(COLOR_BACKGROUND);

    for (int i = 0; i < B_COUNTY_NUM; i++)
        drawFlag(b_countrys[i]);

    if (wifi_init() != 1)
    {
        gfx->setTextColor(COLOR_TEXT);
        gfx->setTextSize(2);

        gfx->setCursor(0, 0);
        gfx->println("Can't connect your wifi.");
        gfx->println("Please check wifi.txt in SD card.");
        gfx->println("Your ssid:");
        gfx->println(ssid);
        gfx->println("Your password:");
        gfx->println(pwd);

        while (1)
            vTaskDelay(100);
    }

    time_zone_set(country_index);

    weather_display(20, 120, country_index);

    long runtime_1 = 0;
    touch_flag = 0;

    while (1)
    {
        if ((millis() - runtime_1) > 1000)
        {
            time_display(20, 20, country_index);
            runtime_1 = millis();
        }

        if (touch_flag == 1)
        {
            touch_x_temp = touch_x;
            touch_y_temp = touch_y;

            for (int i = 0; i < B_COUNTY_NUM; i++)
                if (b_countrys[i].checkTouch(touch_x_temp, touch_y_temp) != UNABLE)
                {
                    vTaskDelay(200);
                    Serial.println(i);
                    country_index = i;
                    time_zone_set(i);

                    time_display(20, 20, country_index);
                    weather_display(20, 120, i);
                }

            touch_flag = 0;
        }
        vTaskDelay(100);
    }
}

// 传感器x2
void page_3()
{
    Serial.println(F("3 page"));

    gfx->fillScreen(COLOR_BACKGROUND);

    // Set the device as a Station and Soft Access Point simultaneously
    WiFi.mode(WIFI_AP_STA);

    if (wifi_init() != 1)
    {
        gfx->setTextColor(COLOR_TEXT);
        gfx->setTextSize(2);

        gfx->setCursor(0, 0);
        gfx->println("Can't connect your wifi.");
        gfx->println("Please check wifi.txt in SD card.");
        gfx->println("Your ssid:");
        gfx->println(ssid);
        gfx->println("Your password:");
        gfx->println(pwd);

        while (1)
            vTaskDelay(100);
    }

    tvoc_task();
}

// 热成像
void page_4()
{
    float max_temp = 0.0;
    String temp = "";

    // Alert states:
    // 0 = below 30 C
    // 1 = 30 C to below 36 C
    // 2 = 36 C or higher
    int alert_state = 0;

    // 关闭触屏抗干扰
    touch_flag = -1;
    vTaskDelay(1000);

    Serial.println(F("4 page"));

    // Initialize thermal camera
    mlx_init();
    mlx_frame_malloc();

    // Initialize speaker for thermal alerts
    alert_audio_init();

    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(4);


    while (1)
    {
        // Get maximum temperature
        max_temp = mlx_img();


        // ==========================================
        // TEMPERATURE ALERT LOGIC
        // ==========================================

        // 36 C or higher
        if (max_temp >= 36.0)
        {
            // Only play when entering this state
            if (alert_state != 2)
            {
                alert_state = 2;

                Serial.println(
                    "ALERT: Possible human heat signature detected"
                );

                play_human_alert();
            }
        }

        // 30 C to below 36 C
        else if (max_temp >= 30.0)
        {
            // Only play when entering this state
            if (alert_state != 1)
            {
                alert_state = 1;

                Serial.println(
                    "WARNING: Elevated temperature detected"
                );

                play_temp_warning();
            }
        }

        // Below 30 C
        else
        {
            alert_state = 0;
        }


        // ==========================================
        // DISPLAY TEMPERATURE
        // ==========================================

        temp = "";
        temp = temp + "Max temp:" + max_temp;

        gfx->fillRect(
            0,
            361,
            480,
            120,
            BLACK
        );

        gfx->setCursor(20, 365);
        gfx->println(temp);


        vTaskDelay(100);
    }
}

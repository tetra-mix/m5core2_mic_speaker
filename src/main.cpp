#include <M5Unified.h>
#include <cmath>

static constexpr uint32_t SAMPLE_RATE = 16000;
static constexpr size_t    N          = 1024;   // 一度に取るサンプル数
const float SOFT_GAIN = 8.0f;

int16_t samples[N];

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // マイク初期化
  M5.Mic.begin();
  M5.Mic.setSampleRate(SAMPLE_RATE);

  Serial.begin(115200);
  delay(200);
  Serial.println("time_ms,rms,peak");  // CSV ヘッダ（後でプロッタ用）

  // 画面初期化
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 8);
  M5.Display.println("Mic level (RMS)");
}

void loop() {
  // N サンプル録音（true が返れば N 取れてる）
  if (M5.Mic.record(samples, N, SAMPLE_RATE)) {
    // 音量計算（RMS とピーク）
    long   peak = 0;
    double sum2 = 0.0;
    for (size_t i = 0; i < N; ++i) {
      int32_t v = int32_t(samples[i] * SOFT_GAIN); // ソフトゲイン
      peak = std::max<long>(peak, std::abs(v));
      sum2 += double(v) * double(v);
    }
    double rms = std::sqrt(sum2 / N); // 16bit の生値基準

    // シリアル出力（CSV）
    Serial.print(millis());
    Serial.print(",");
    Serial.print(rms, 2);
    Serial.print(",");
    Serial.println(peak);

    // 画面にバー表示（0〜32767 を 0〜幅へ）
    int w  = M5.Display.width() - 20;
    int h  = 30;
    int x0 = 10, y0 = 60;
    int bar = std::min(w, int(rms / 32767.0 * w));

    // バー描画
    M5.Display.fillRect(x0, y0, w, h, TFT_DARKGREY);
    M5.Display.fillRect(x0, y0, bar, h, TFT_GREEN);
    M5.Display.setCursor(10, y0 + h + 10);
    M5.Display.printf("RMS: %.0f  Peak: %ld     ", rms, peak);
  }

  M5.update();
}

/**
 *
 * Przykład pokazujący prosty program odtwarzający muzykę
 *
 * Zobacz: https://wiki.libsdl.org/SDL_AudioSpec
 *
 * UWAGA: Przykład nie sprawdza błędów!!!
 *
 * (przygotował na zajęcia z SGD na PJATK Tadeusz Puźniakowski)
 *
 *
 * Przykład pokazuje podstawową obsługę dźwięku. Jest to niskopoziomowe
 * rozwiązanie które pozwala na zapoznanie się z metodą geneorwania dźwięku
 * przez niektóre gry i sterowniki kart dźwiękowych. Takie rozwiązanie pozwala
 * na tworzenie zaawansowanych efektów oraz innych cudów, ale wymaga większej
 * ilości pisania kodu.
 *
 * Jeśli preferujesz prostsze rozwiązanie, to poelcam bibliotekę
 * SDL2_mixer. Pozwala ona na miksowanie i zapamiętywanie dźwięków z plików. Nie
 * obsługuje tak zaawansowafnych funkcji jak na przykład DirectSound, ale
 * wystarcza dla gier arkadowych.
 *
 * */
#include <SDL.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>
#include <algorithm>

using namespace std;

class custom_mixer {
public:
  //  mutex i bufor do odtwarzania muzyki
  std::mutex audiobufmutex;
  std::list<std::pair<float, float>> audiobuf;

  void playSound(std::list<std::pair<float, float>> buf_to_play) {
    // miksujemy z początkiem bufora
    std::lock_guard<std::mutex> guard(audiobufmutex);
    for (auto &p : audiobuf) {
      if (buf_to_play.size() <= 0)
        break;
      p.first = p.first + buf_to_play.front().first;
      p.second = p.second + buf_to_play.front().second;
      buf_to_play.pop_front();
    }
    // a resztę dodajemy na końcu
    for (auto &rest_part : buf_to_play) {
      audiobuf.push_back(rest_part);
    }
  }
  void playSound(std::list<float> snd0) {
    std::list<std::pair<float, float>> snd;
    std::transform(snd0.begin() , snd0.end() , std::back_inserter(snd) , [](const float & val)->std::pair<float,float>{ return {val,val};});
    playSound(snd);
  }

  std::list<float> loadSound(const string &fname) {
    std::list<float> result;
    SDL_AudioSpec wav_spec;
    Uint32 wav_length;
    Uint8 *wav_buffer;
    short int *wav_buffer_p;

    if (SDL_LoadWAV(fname.c_str(), &wav_spec, &wav_buffer, &wav_length) ==
        NULL) {
      cout << "Blad otwarcia pliku wav: " << SDL_GetError() << endl;
    } else {
      wav_buffer_p = (short int *)wav_buffer;
      wav_length /= sizeof(short int);

      // zaladowalem! Zakładam że jest to dźwięk mono z 32bit/próbkę. --
      // AUDIO_S16 Jako ćwiczenie, zachęcam do rozwinięcia tego o inne formaty
      while (wav_length > 0) {
        result.push_back(((float)*wav_buffer_p) / 33000.0);
        wav_buffer_p++;
        wav_length--;
      }
      SDL_FreeWAV(wav_buffer);
    }
    return result;
  }

  custom_mixer() {
    SDL_AudioSpec wav_spec;
    SDL_memset(&wav_spec, 0, sizeof(wav_spec));
    wav_spec.callback = [](void *userdata, Uint8 *stream, int len) -> void {
      custom_mixer *pThis = (custom_mixer *)userdata;
      vector<float> fragment(len, 0);
      {
        std::lock_guard<std::mutex> guard(pThis->audiobufmutex);
        for (int i = 0; i < (len / sizeof(float) - 1); i += 2) {
          if (!pThis->audiobuf.empty()) {
            fragment[i] = pThis->audiobuf.front().first;
            fragment[i + 1] = pThis->audiobuf.front().second;
            pThis->audiobuf.pop_front();
          } else
            break;
        }
      }
      SDL_memcpy(stream, fragment.data(), len);
    };
    wav_spec.freq = 44100;
    wav_spec.format = AUDIO_F32;
    wav_spec.channels = 2;
    wav_spec.samples = 8192;
    wav_spec.userdata = this; // przekazuje this do callbacka
    int dev;
    if ((dev = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL,
                                   SDL_AUDIO_ALLOW_FORMAT_CHANGE)) == 0) {
      throw std::invalid_argument(
          std::string("Blad uruchamiania karty dzwiekowej: ") + SDL_GetError());
    }
    SDL_PauseAudioDevice(dev, 0);
  }

};

int main(int argc, char *args[]) {
  SDL_Init(SDL_INIT_AUDIO);
  custom_mixer audio;
  std::list<std::pair<float, float>> effect;
  for (int i = 0; i < 65536; i++) {
    effect.push_back({sin((float)i / 10) * (sin((float)i / 10000) + 1.0) * 0.5,
                      sin(((float)i / (cos((float)i / 500) * 10 + 11))) *
                          (cos((float)i / 10000) + 1.0) * 0.5});
  }
  // dzwiek z https://freewavesamples.com/e-mu-proteus-fx-wacky-snare
  auto snd = audio.loadSound("sample.wav");
 
  audio.playSound(effect);

  int i = 0;
  while (true) {
    SDL_Delay(100);
    i++;
    if (i == 4)
      audio.playSound(snd);
    if (i == 6)
      audio.playSound(snd);
    if (i == 15)
      audio.playSound(snd);
    if (i == 10)
      audio.playSound(effect);
    std::lock_guard<std::mutex> guard(audio.audiobufmutex);
    if (audio.audiobuf.empty())
      break;
  }

  SDL_Quit();
  return 0;
}

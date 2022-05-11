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
 * */
#include <SDL.h>
#include <vector>
#include <list>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

class AudioEngine {
public:

	//  mutex i bufor do odtwarzania muzyki
	std::mutex audiobufmutex;
	std::list < float > audiobuf;

	static void callback(void *userdata, Uint8 *stream, int len) {
		AudioEngine *pThis = (AudioEngine *)userdata;
		vector<float> fragment(len);
		{
			std::lock_guard<std::mutex> guard(pThis->audiobufmutex); // pamiętajcie o mutexach!
			for (int i = 0; i < len/sizeof(float); i++) {
				if (pThis->audiobuf.empty()) {
					fragment[i] = 0;
				} else {
					fragment[i] = pThis->audiobuf.front();
					pThis->audiobuf.pop_front();
				}
			}
		}
		SDL_memcpy (stream, fragment.data(), len);
	}
	
	int init() {
		SDL_AudioSpec wav_spec;
		SDL_memset(&wav_spec, 0, sizeof(wav_spec));
		wav_spec.callback = AudioEngine::callback;
		wav_spec.freq = 44100;
		wav_spec.format = AUDIO_F32;
		wav_spec.channels = 2;
		wav_spec.samples = 8192;
		wav_spec.userdata = this; // przekazuje this do callbacka
		int dev;
		if (( dev = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, SDL_AUDIO_ALLOW_FORMAT_CHANGE)) == 0) {
		  cout << "Blad uruchamiania karty dzwiekowej: " <<  SDL_GetError() << endl;
		  return -1;
		}
		SDL_PauseAudioDevice(dev, 0);
		return 0;
	}
	
	void loadSound(const string &fname) {
		SDL_AudioSpec wav_spec;
		Uint32 wav_length;
		Uint8 *wav_buffer;
		short int *wav_buffer_p;
	
		if (SDL_LoadWAV(fname.c_str(), &wav_spec, &wav_buffer, &wav_length) == NULL) {
			cout << "Blad otwarcia pliku wav: " << SDL_GetError() << endl;
		} else {
			wav_buffer_p = (short int *)wav_buffer;
			wav_length /= sizeof(short int); 
			
			// zaladowalem! Zakładam że jest to dźwięk mono z 32bit/próbkę. -- AUDIO_S16
			// Jako ćwiczenie, zachęcam do rozwinięcia tego o inne formaty
			{
				std::lock_guard<std::mutex> guard(audiobufmutex); // pamiętaj o muteksie!
				// miksujemy z początkiem bufora
				int x = 0;
				for (auto &p : audiobuf) {
					p = p + ((float)*wav_buffer_p)/33000.0; // miksujemy
					wav_length-=(x % 2); // zgrywamy z mono, wiec trzeba podwoic probki!!
					wav_buffer_p+=(x % 2);
					x++;
					if (wav_length <= 0) break;
				}
			}
			// a resztę dodajemy na końcu
			while (wav_length > 0) {
				std::lock_guard<std::mutex> guard(audiobufmutex); // pamiętaj o muteksie!
				audiobuf.push_back(((float)*wav_buffer_p)/33000.0); // lewy
				audiobuf.push_back(((float)*wav_buffer_p)/33000.0); // prawy
				wav_buffer_p++;
				wav_length--;
			}
			SDL_FreeWAV(wav_buffer);
		}
	}
};


int main( int argc, char* args[] ) {
	SDL_Init(SDL_INIT_AUDIO);
	AudioEngine audio;
	if (audio.init() == 0) {
		std::lock_guard<std::mutex> guard(audio.audiobufmutex);
		for (int i = 0; i < 65536; i++) {
			audio.audiobuf.push_back(sin((float)i/10)*(sin((float)i/10000)+1.0)*0.5); // lewy
			audio.audiobuf.push_back(sin(((float)i/(cos((float)i/500)*10+11)))*(cos((float)i/10000)+1.0)*0.5); // prawy
		}
	} else {
		return -1;
	}
	
	while (true) {
		SDL_Delay(100); 
		std::lock_guard<std::mutex> guard(audio.audiobufmutex);
		if (audio.audiobuf.empty()) break;
	}
	
	SDL_Quit();
	return 0;
}


#ifndef EXPORTED_TD0_HPP
#define EXPORTED_TD0_HPP

#include "TD0.hpp"

#ifdef WEBSITE
#include <chrono>
#include <sstream>
#include <thread>
#include <emscripten/fetch.h>


TD0 TD0::best_model = TD0(0, 0.0f);
bool TD0::best_model_loaded = false;
void TD0::load_best() {
    if (TD0::best_model_loaded) return;  // avoid repeat downloads
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = [](emscripten_fetch_t* fetch) {
        std::cerr << "fetch succeeded!" << std::endl;
        std::istringstream is(std::string(fetch->data, fetch->numBytes));
        emscripten_fetch_close(fetch);

        TD0::best_model = TD0(0.0f, is);
        TD0::best_model_loaded = true;
    };
    attr.onerror = [](emscripten_fetch_t* fetch) {
        std::cerr << "failed with status code " << fetch->status << std::endl;
        emscripten_fetch_close(fetch);
    };
    emscripten_fetch(&attr, "../model.bmp");  // model file has .bmp extension to force GitHub Pages to compress it
}

class ExportedTD0: public Strategy {
    public:
    ExportedTD0() { TD0::load_best(); }
    const int pick_move(const board_t board) override {
        if (TD0::best_model_loaded == false) {
            std::cerr << "Waiting for model to load!" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        return TD0::best_model.pick_move(board);
    }
    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<ExportedTD0>();
    }
};

#elif defined TESTING

std::ifstream fin("machine_learning/model_8-6_16_0.000150/model_8-6_16_0.000150_1000.dat", std::ios::binary);
TD0 TD0::best_model = TD0(0.00015f, fin);
bool TD0::best_model_loaded = true;
void TD0::load_best() { /* do nothing, model already loaded */ }

#endif  // WEBSITE

#endif


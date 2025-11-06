#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL.h>

#include "AppData.h"
#include "CityDataPanelBuilder.h"
#include "Panel.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <memory>

class BuilderApplication {
public:
    bool initialize();
    void run();
    void shutdown();

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    AppData app_data_;
    bool done_ = false;
    bool show_demo_window_ = false;
    float dpi_scale_ = 1.0f;
    ImGuiStyle base_style_{};

    void initialize_app_data();
    std::unique_ptr<Panel> build_city_panel();
    void render_menu_bar();
    void handle_keyboard_shortcuts();
    void handle_window_event(const SDL_Event& event);
    void toggle_dpi_scale();
    void apply_dpi_scale(float scale);
};

bool BuilderApplication::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window_ = SDL_CreateWindow("ImGui Builder Demo",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               1400,
                               900,
                               window_flags);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    base_style_ = ImGui::GetStyle();

    if (!ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_)) {
        std::cerr << "ImGui_ImplSDL2_InitForSDLRenderer failed" << std::endl;
        return false;
    }
    if (!ImGui_ImplSDLRenderer2_Init(renderer_)) {
        std::cerr << "ImGui_ImplSDLRenderer2_Init failed" << std::endl;
        return false;
    }

    initialize_app_data();

    auto city_panel = build_city_panel();
    if (city_panel) {
        PanelManager::instance().add_panel("city_data", std::move(city_panel));
    }
    apply_dpi_scale(dpi_scale_);

    return true;
}

void BuilderApplication::run() {
    while (!done_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                done_ = true;
            }
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window_)) {
                done_ = true;
            } else if (event.type == SDL_WINDOWEVENT) {
                handle_window_event(event);
            }
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        render_menu_bar();
        handle_keyboard_shortcuts();

        PanelManager::instance().render_all();

        if (show_demo_window_) {
            ImGui::ShowDemoWindow(&show_demo_window_);
        }

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer_);
        SDL_RenderPresent(renderer_);
    }
}

void BuilderApplication::shutdown() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}

void BuilderApplication::initialize_app_data() {
    static const std::array<CityData, 6> defaults = {{
        {"New York", 40.7128f, -74.0060f, 10, 12.5f, 8400000, 3},
        {"Los Angeles", 34.0522f, -118.2437f, 71, 18.2f, 3900000, 2},
        {"Chicago", 41.8781f, -87.6298f, 181, 9.8f, 2700000, 3},
        {"Houston", 29.7604f, -95.3698f, 13, 20.7f, 2300000, 1},
        {"Phoenix", 33.4484f, -112.0740f, 331, 22.9f, 1700000, 2},
        {"Philadelphia", 39.9526f, -75.1652f, 12, 13.1f, 1600000, 3},
    }};

    app_data_.cities.resize(defaults.size());
    for (std::size_t i = 0; i < defaults.size(); ++i) {
        app_data_.cities[i] = defaults[i];
    }
}

std::unique_ptr<Panel> BuilderApplication::build_city_panel() {
CityDataPanelBuilder builder(app_data_);
    builder.with_title("City Data Grid")
        .with_size(1100.0f, 680.0f)
        .with_max_rows(6)
        .on_save([this]() {
            std::cout << "City data saved:" << std::endl;
            for (std::size_t i = 0; i < std::min<std::size_t>(app_data_.cities.size(), 6); ++i) {
                const auto& city = app_data_.cities[i];
                std::cout << "  " << city.name << ": "
                          << city.latitude << ", "
                          << city.longitude << " @ "
                          << city.elevation << "m, "
                          << city.avg_temp << "°C"
                          << std::endl;
            }
        })
        .on_reset([this]() {
            std::cout << "Resetting city data to defaults" << std::endl;
            initialize_app_data();
            PanelManager::instance().update_all_layouts();
        })
        .on_toggle_dpi([this]() {
            toggle_dpi_scale();
        });

    return builder.build();
}

void BuilderApplication::render_menu_bar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Panels")) {
            if (ImGui::MenuItem("Show City Data Panel")) {
                PanelManager::instance().show_panel("city_data");
            }
            if (ImGui::MenuItem("Toggle Demo Window")) {
                show_demo_window_ = !show_demo_window_;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                done_ = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void BuilderApplication::handle_keyboard_shortcuts() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q, false)) {
        done_ = true;
    }
}

void BuilderApplication::handle_window_event(const SDL_Event& event) {
    if (event.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED) {
        float ddpi = 0.0f;
        float hdpi = 0.0f;
        float vdpi = 0.0f;
        if (SDL_GetDisplayDPI(event.window.data1, &ddpi, &hdpi, &vdpi) == 0 && hdpi > 0.0f) {
            float suggested_scale = hdpi / 96.0f;
            apply_dpi_scale(std::max(0.5f, suggested_scale));
            std::cout << "Detected DPI change: applying scale " << dpi_scale_ << std::endl;
        }
    }
}

void BuilderApplication::toggle_dpi_scale() {
    const std::array<float, 3> scales = {1.0f, 1.5f, 2.0f};
    auto it = std::find(scales.begin(), scales.end(), dpi_scale_);
    if (it == scales.end() || ++it == scales.end()) {
        apply_dpi_scale(scales.front());
    } else {
        apply_dpi_scale(*it);
    }
    std::cout << "Manual DPI toggle -> scale " << dpi_scale_ << std::endl;
}

void BuilderApplication::apply_dpi_scale(float scale) {
    if (scale <= 0.0f) {
        return;
    }
    dpi_scale_ = scale;

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = dpi_scale_;

    ImGuiStyle& style = ImGui::GetStyle();
    style = base_style_;
    style.ScaleAllSizes(dpi_scale_);

    PanelManager::instance().set_all_dpi_scale(dpi_scale_);
    PanelManager::instance().update_all_layouts();
}

int main() {
    BuilderApplication app;
    if (!app.initialize()) {
        return 1;
    }
    app.run();
    app.shutdown();
    return 0;
}

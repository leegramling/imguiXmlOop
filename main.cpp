#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL.h>
#include "Widget.h"
#include "Panel.h"
#include "XmlParser.h"
#include <iostream>
#include <memory>

/**
 * @brief Application class implementing the Facade pattern
 * 
 * This class provides a simplified interface to the complex subsystem
 * of ImGui, SDL, XML parsing, and panel management.
 */
class Application : public XmlFileObserver {
public:
    Application() : parser_(std::make_unique<XmlParser>()) {}
    
    bool initialize();
    void run();
    void shutdown();
    
    // XmlFileObserver implementation
    void on_file_changed(const std::string& file_path) override;
    
private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    std::unique_ptr<XmlParser> parser_;
    std::unique_ptr<XmlFileWatcher> contact_watcher_;
    std::unique_ptr<XmlFileWatcher> city_watcher_;
    AppData app_data_;
    bool done_ = false;
    bool show_demo_window_ = false;
    
    void initialize_app_data();
    void setup_button_callbacks();
    void setup_file_watchers();
    void render_menu_bar();
    void handle_keyboard_shortcuts();
};

bool Application::initialize() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window_ = SDL_CreateWindow("ImGui XML OOP Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                              1000, 700, window_flags);
    if (window_ == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer_ == nullptr) {
        printf("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return false;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer2_Init(renderer_);
    
    // Initialize application data and setup
    initialize_app_data();
    setup_button_callbacks();
    
    // Load panels
    parser_->set_app_data(&app_data_);
    
    auto contact_panel = parser_->parse_panel_from_file("contact_panel.xml");
    if (contact_panel) {
        contact_panel->set_open(false); // Start closed
        PanelManager::instance().add_panel("contact", std::move(contact_panel));
    }
    
    auto city_panel = parser_->parse_panel_from_file("city_data_panel.xml");
    if (city_panel) {
        city_panel->set_open(true); // Start open
        PanelManager::instance().add_panel("city_data", std::move(city_panel));
    }
    
    setup_file_watchers();
    
    return true;
}

void Application::run() {
    while (!done_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done_ = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && 
                event.window.windowID == SDL_GetWindowID(window_))
                done_ = true;
        }

        // Check for file changes
        if (contact_watcher_ && contact_watcher_->has_changed()) {
            std::cout << "Contact XML file changed, reloading..." << std::endl;
            auto new_panel = parser_->parse_panel_from_file("contact_panel.xml");
            if (new_panel) {
                bool was_open = PanelManager::instance().get_panel("contact")->is_open();
                new_panel->set_open(was_open);
                PanelManager::instance().add_panel("contact", std::move(new_panel));
                std::cout << "Contact panel reloaded successfully!" << std::endl;
            }
        }
        
        if (city_watcher_ && city_watcher_->has_changed()) {
            std::cout << "City XML file changed, reloading..." << std::endl;
            auto new_panel = parser_->parse_panel_from_file("city_data_panel.xml");
            if (new_panel) {
                bool was_open = PanelManager::instance().get_panel("city_data")->is_open();
                new_panel->set_open(was_open);
                PanelManager::instance().add_panel("city_data", std::move(new_panel));
                std::cout << "City panel reloaded successfully!" << std::endl;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        render_menu_bar();
        handle_keyboard_shortcuts();

        // Render all panels
        PanelManager::instance().render_all();

        // Optional demo window
        if (show_demo_window_) {
            ImGui::ShowDemoWindow(&show_demo_window_);
        }

        // Rendering
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer_);
        SDL_RenderPresent(renderer_);
    }
}

void Application::shutdown() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Application::on_file_changed(const std::string& file_path) {
    std::cout << "File changed: " << file_path << std::endl;
}

void Application::initialize_app_data() {
    // Initialize city data (first 6 cities for the demo)
    app_data_.cities = {
        {"New York", 40.7128f, -74.0060f, 10, 12.5f, 8400000, 3},
        {"Los Angeles", 34.0522f, -118.2437f, 71, 18.2f, 3900000, 2},
        {"Chicago", 41.8781f, -87.6298f, 181, 9.8f, 2700000, 3},
        {"Houston", 29.7604f, -95.3698f, 13, 20.7f, 2300000, 1},
        {"Phoenix", 33.4484f, -112.0740f, 331, 22.9f, 1700000, 2},
        {"Philadelphia", 39.9526f, -75.1652f, 12, 13.1f, 1600000, 3}
    };
}

void Application::setup_button_callbacks() {
    // Contact form callbacks
    parser_->add_button_callback("ok_button", [this]() {
        std::cout << "Contact saved:" << std::endl;
        std::cout << "Name: " << app_data_.name << std::endl;
        std::cout << "Email: " << app_data_.email << std::endl;
        std::cout << "Python: " << (app_data_.python_selected ? "Yes" : "No") << std::endl;
        std::cout << "Swift: " << (app_data_.swift_selected ? "Yes" : "No") << std::endl;
        std::cout << "C++: " << (app_data_.cpp_selected ? "Yes" : "No") << std::endl;
    });
    
    parser_->add_button_callback("cancel_button", []() {
        std::cout << "Cancel pressed" << std::endl;
    });
    
    // City data callbacks
    parser_->add_button_callback("save_cities", [this]() {
        std::cout << "City data saved:" << std::endl;
        for (size_t i = 0; i < std::min(app_data_.cities.size(), size_t(6)); i++) {
            const auto& city = app_data_.cities[i];
            std::cout << "  " << city.name << ": " << city.latitude << ", " << city.longitude 
                      << " @ " << city.elevation << "m, " << city.avg_temp << "°C" << std::endl;
        }
    });
    
    parser_->add_button_callback("reset_cities", [this]() {
        std::cout << "Resetting city data to defaults" << std::endl;
        initialize_app_data(); // Reset to initial values
    });
}

void Application::setup_file_watchers() {
    contact_watcher_ = std::make_unique<XmlFileWatcher>("contact_panel.xml");
    city_watcher_ = std::make_unique<XmlFileWatcher>("city_data_panel.xml");
    
    contact_watcher_->add_observer(this);
    city_watcher_->add_observer(this);
}

void Application::render_menu_bar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Panels")) {
            if (ImGui::MenuItem("Show Contact Panel")) {
                PanelManager::instance().show_panel("contact");
            }
            if (ImGui::MenuItem("Show City Data Panel")) {
                PanelManager::instance().show_panel("city_data");
            }
            if (ImGui::MenuItem("Show Demo Window")) {
                show_demo_window_ = !show_demo_window_;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                done_ = true;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Reload")) {
            if (ImGui::MenuItem("Reload Contact Panel", "Ctrl+R")) {
                auto new_panel = parser_->parse_panel_from_file("contact_panel.xml");
                if (new_panel) {
                    bool was_open = PanelManager::instance().get_panel("contact")->is_open();
                    new_panel->set_open(was_open);
                    PanelManager::instance().add_panel("contact", std::move(new_panel));
                    std::cout << "Contact panel manually reloaded!" << std::endl;
                }
            }
            if (ImGui::MenuItem("Reload City Panel", "Ctrl+Shift+R")) {
                auto new_panel = parser_->parse_panel_from_file("city_data_panel.xml");
                if (new_panel) {
                    bool was_open = PanelManager::instance().get_panel("city_data")->is_open();
                    new_panel->set_open(was_open);
                    PanelManager::instance().add_panel("city_data", std::move(new_panel));
                    std::cout << "City panel manually reloaded!" << std::endl;
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reset File Watchers")) {
                contact_watcher_->reset();
                city_watcher_->reset();
                std::cout << "File watchers reset" << std::endl;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Application::handle_keyboard_shortcuts() {
    if (ImGui::IsKeyPressed(ImGuiKey_R) && ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyShift) {
        auto new_panel = parser_->parse_panel_from_file("contact_panel.xml");
        if (new_panel) {
            bool was_open = PanelManager::instance().get_panel("contact")->is_open();
            new_panel->set_open(was_open);
            PanelManager::instance().add_panel("contact", std::move(new_panel));
            std::cout << "Contact panel reloaded via keyboard shortcut!" << std::endl;
        }
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_R) && ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeyShift) {
        auto new_panel = parser_->parse_panel_from_file("city_data_panel.xml");
        if (new_panel) {
            bool was_open = PanelManager::instance().get_panel("city_data")->is_open();
            new_panel->set_open(was_open);
            PanelManager::instance().add_panel("city_data", std::move(new_panel));
            std::cout << "City panel reloaded via keyboard shortcut!" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    Application app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.run();
    app.shutdown();
    
    return 0;
}
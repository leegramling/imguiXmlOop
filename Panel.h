#pragma once
#include "Widget.h"
#include <string>
#include <memory>
#include <map>

/**
 * @brief Represents a complete UI panel that can be rendered as a window
 * 
 * The Panel class encapsulates a complete user interface panel that can be
 * displayed as an ImGui window. It follows the Command pattern for handling
 * user interactions and the Observer pattern for responding to data changes.
 */
class Panel {
public:
    /**
     * @brief Constructor
     * @param title The title of the panel window
     * @param width Initial width of the panel
     * @param height Initial height of the panel
     */
    explicit Panel(const std::string& title = "Panel", float width = 400.0f, float height = 300.0f);
    
    virtual ~Panel() = default;
    
    // Core functionality
    void render();
    void update_layout();
    
    // Property accessors
    const std::string& get_title() const { return title_; }
    void set_title(const std::string& title) { title_ = title; }
    
    float get_width() const { return width_; }
    void set_width(float width);
    
    float get_height() const { return height_; }
    void set_height(float height);
    
    bool is_open() const { return is_open_; }
    void set_open(bool open) { is_open_ = open; }
    void show() { is_open_ = true; }
    void hide() { is_open_ = false; }
    void toggle() { is_open_ = !is_open_; }

    float get_dpi_scale() const { return dpi_scale_; }
    void set_dpi_scale(float scale);
    
    // Widget management
    void set_root_widget(std::unique_ptr<Widget> root);
    Widget* get_root_widget() const { return root_widget_.get(); }
    
    // Utility functions
    Widget* find_widget(const std::string& id);
    
    template<typename T>
    T* find_widget_as(const std::string& id) {
        return dynamic_cast<T*>(find_widget(id));
    }
    
protected:
    virtual void on_before_render() {}
    virtual void on_after_render() {}
    
private:
    std::string title_;
    float width_;
    float height_;
    float base_width_;
    float base_height_;
    float dpi_scale_ = 1.0f;
    bool is_open_ = true;
    std::unique_ptr<Widget> root_widget_;
    
    // Helper function for recursive widget search
    Widget* find_widget_recursive(Widget* widget, const std::string& id);
};

/**
 * @brief Panel manager for handling multiple panels
 * 
 * This class implements the Mediator pattern to coordinate between multiple
 * panels and provides centralized panel management functionality.
 */
class PanelManager {
public:
    static PanelManager& instance() {
        static PanelManager instance;
        return instance;
    }
    
    // Panel management
    void add_panel(const std::string& name, std::unique_ptr<Panel> panel);
    void remove_panel(const std::string& name);
    Panel* get_panel(const std::string& name);
    
    // Rendering
    void render_all();
    void update_all_layouts();
    
    // Utility
    void show_panel(const std::string& name);
    void hide_panel(const std::string& name);
    void toggle_panel(const std::string& name);
    void set_all_dpi_scale(float scale);
    
    const std::map<std::string, std::unique_ptr<Panel>>& get_panels() const { return panels_; }
    
private:
    PanelManager() = default;
    std::map<std::string, std::unique_ptr<Panel>> panels_;
};

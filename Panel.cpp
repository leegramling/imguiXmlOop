#include "Panel.h"
#include "imgui.h"
#include <iostream>

// ============================================================================
// Panel Implementation
// ============================================================================

Panel::Panel(const std::string& title, float width, float height)
    : title_(title), width_(width), height_(height) {}

void Panel::render() {
    if (!is_open_) return;
    
    on_before_render();
    
    ImGui::SetNextWindowSize(ImVec2(width_, height_), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin(title_.c_str(), &is_open_)) {
        ImVec2 content_size = ImGui::GetContentRegionAvail();
        
        if (root_widget_) {
            // Update layout based on available content size
            root_widget_->update_layout(content_size.x, content_size.y);
            
            // Render the widget tree
            root_widget_->render();
        }
    }
    ImGui::End();
    
    on_after_render();
}

void Panel::update_layout() {
    if (root_widget_) {
        root_widget_->update_layout(width_, height_);
    }
}

void Panel::set_root_widget(std::unique_ptr<Widget> root) {
    root_widget_ = std::move(root);
    update_layout();
}

Widget* Panel::find_widget(const std::string& id) {
    if (!root_widget_) return nullptr;
    
    if (root_widget_->get_id() == id) {
        return root_widget_.get();
    }
    
    return find_widget_recursive(root_widget_.get(), id);
}

Widget* Panel::find_widget_recursive(Widget* widget, const std::string& id) {
    if (!widget) return nullptr;
    
    // Check if this widget is a container
    ContainerWidget* container = dynamic_cast<ContainerWidget*>(widget);
    if (container) {
        // Search through children
        for (const auto& child : container->get_children()) {
            if (child->get_id() == id) {
                return child.get();
            }
            
            // Recursively search in child containers
            Widget* found = find_widget_recursive(child.get(), id);
            if (found) {
                return found;
            }
        }
    }
    
    return nullptr;
}

// ============================================================================
// Panel Manager Implementation
// ============================================================================

void PanelManager::add_panel(const std::string& name, std::unique_ptr<Panel> panel) {
    if (panel) {
        panels_[name] = std::move(panel);
    }
}

void PanelManager::remove_panel(const std::string& name) {
    panels_.erase(name);
}

Panel* PanelManager::get_panel(const std::string& name) {
    auto it = panels_.find(name);
    return (it != panels_.end()) ? it->second.get() : nullptr;
}

void PanelManager::render_all() {
    for (auto& [name, panel] : panels_) {
        if (panel) {
            panel->render();
        }
    }
}

void PanelManager::update_all_layouts() {
    for (auto& [name, panel] : panels_) {
        if (panel) {
            panel->update_layout();
        }
    }
}

void PanelManager::show_panel(const std::string& name) {
    Panel* panel = get_panel(name);
    if (panel) {
        panel->show();
    }
}

void PanelManager::hide_panel(const std::string& name) {
    Panel* panel = get_panel(name);
    if (panel) {
        panel->hide();
    }
}

void PanelManager::toggle_panel(const std::string& name) {
    Panel* panel = get_panel(name);
    if (panel) {
        panel->toggle();
    }
}
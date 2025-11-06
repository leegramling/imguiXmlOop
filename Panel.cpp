#include "Panel.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>

// ============================================================================
// Panel Implementation
// ============================================================================

Panel::Panel(const std::string& title, float width, float height)
    : title_(title), width_(width), height_(height), base_width_(width), base_height_(height) {}

void Panel::render() {
    if (!is_open_) return;
    
    on_before_render();
    
    ImGuiCond size_condition = size_dirty_ ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    ImGui::SetNextWindowSize(ImVec2(width_, height_), size_condition);
    if (size_dirty_) {
        size_dirty_ = false;
    }
    
    if (ImGui::Begin(title_.c_str(), &is_open_)) {
        ImVec2 content_size = ImGui::GetContentRegionAvail();
        
        if (root_widget_) {
            // Update layout only if the available size changed
            constexpr float kEpsilon = 0.5f;
            if (std::abs(last_layout_width_ - content_size.x) > kEpsilon ||
                std::abs(last_layout_height_ - content_size.y) > kEpsilon) {
                last_layout_width_ = content_size.x;
                last_layout_height_ = content_size.y;
                auto start = std::chrono::high_resolution_clock::now();
                root_widget_->update_layout(content_size.x, content_size.y);
                auto end = std::chrono::high_resolution_clock::now();
                last_layout_duration_ms_ = std::chrono::duration<float, std::milli>(end - start).count();
            }
            
            // Render the widget tree
            root_widget_->render();
        }
    }
    ImGui::End();
    
    on_after_render();
}

void Panel::update_layout() {
    if (root_widget_) {
        auto start = std::chrono::high_resolution_clock::now();
        root_widget_->update_layout(width_, height_);
        auto end = std::chrono::high_resolution_clock::now();
        last_layout_duration_ms_ = std::chrono::duration<float, std::milli>(end - start).count();
        last_layout_width_ = width_;
        last_layout_height_ = height_;
    }
}

void Panel::fit_to_content() {
    if (!root_widget_ || !root_widget_->get_yoga_node()) {
        return;
    }

    // Let Yoga compute the natural size for the layout.
    root_widget_->update_layout(YGUndefined, YGUndefined);

    float content_width = YGNodeLayoutGetWidth(root_widget_->get_yoga_node());
    float content_height = YGNodeLayoutGetHeight(root_widget_->get_yoga_node());

    if (content_width <= 0.0f) {
        content_width = width_;
    }
    if (content_height <= 0.0f) {
        content_height = height_;
    }

    const ImGuiStyle& style = ImGui::GetStyle();
    float window_width = content_width + style.WindowPadding.x * 2.0f;
    float window_height = content_height + style.WindowPadding.y * 2.0f;

    set_width(window_width);
    set_height(window_height);
    update_layout();
}

void Panel::set_width(float width) {
    width_ = width;
    if (dpi_scale_ > 0.0f) {
        base_width_ = width / dpi_scale_;
    } else {
        base_width_ = width;
    }
    size_dirty_ = true;
}

void Panel::set_height(float height) {
    height_ = height;
    if (dpi_scale_ > 0.0f) {
        base_height_ = height / dpi_scale_;
    } else {
        base_height_ = height;
    }
    size_dirty_ = true;
}

void Panel::set_dpi_scale(float scale) {
    if (scale <= 0.0f) {
        return;
    }
    dpi_scale_ = scale;
    width_ = base_width_ * dpi_scale_;
    height_ = base_height_ * dpi_scale_;
    last_layout_width_ = -1.0f;
    last_layout_height_ = -1.0f;
    size_dirty_ = true;
    update_layout();
}

void Panel::set_root_widget(std::unique_ptr<Widget> root) {
    root_widget_ = std::move(root);
    last_layout_width_ = -1.0f;
    last_layout_height_ = -1.0f;
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

void PanelManager::set_all_dpi_scale(float scale) {
    if (scale <= 0.0f) {
        return;
    }
    for (auto& [name, panel] : panels_) {
        if (panel) {
            panel->set_dpi_scale(scale);
        }
    }
}

void PanelManager::fit_all_to_content() {
    for (auto& [name, panel] : panels_) {
        if (panel) {
            panel->fit_to_content();
        }
    }
}

std::pair<float, float> PanelManager::get_layout_durations() {
    float current_max = 0.0f;
    for (const auto& [name, panel] : panels_) {
        if (panel) {
            current_max = std::max(current_max, panel->get_last_layout_duration_ms());
        }
    }
    peak_layout_duration_ms_ = std::max(peak_layout_duration_ms_, current_max);
    return {current_max, peak_layout_duration_ms_};
}

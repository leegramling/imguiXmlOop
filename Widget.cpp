#include "Widget.h"
#include <iostream>
#include <algorithm>
#include <cstring>

// ============================================================================
// Base Widget Implementation
// ============================================================================

Widget::Widget(const std::string& id) : id_(id) {
    yoga_node_ = YGNodeNew();
}

Widget::~Widget() {
    if (yoga_node_) {
        YGNodeFree(yoga_node_);
    }
}

void Widget::update_layout(float available_width, float available_height) {
    if (yoga_node_) {
        YGNodeCalculateLayout(yoga_node_, available_width, available_height, YGDirectionLTR);
    }
}

void Widget::set_width(float width) {
    width_ = width;
    if (yoga_node_ && width != YGUndefined) {
        YGNodeStyleSetWidth(yoga_node_, width);
    }
}

void Widget::set_height(float height) {
    height_ = height;
    if (yoga_node_ && height != YGUndefined) {
        YGNodeStyleSetHeight(yoga_node_, height);
    }
}

void Widget::set_flex(float flex) {
    flex_ = flex;
    if (yoga_node_ && flex != YGUndefined) {
        YGNodeStyleSetFlex(yoga_node_, flex);
    }
}

void Widget::apply_styles() {
    if (!yoga_node_) return;
    
    // Apply spacing
    if (style_.margin > 0) {
        YGNodeStyleSetMargin(yoga_node_, YGEdgeAll, style_.margin);
    }
    
    if (style_.padding > 0) {
        YGNodeStyleSetPadding(yoga_node_, YGEdgeAll, style_.padding);
    }
    
    // Apply align-self for individual items
    if (style_.align_self != "auto") {
        if (style_.align_self == "center") {
            YGNodeStyleSetAlignSelf(yoga_node_, YGAlignCenter);
        } else if (style_.align_self == "flex-start") {
            YGNodeStyleSetAlignSelf(yoga_node_, YGAlignFlexStart);
        } else if (style_.align_self == "flex-end") {
            YGNodeStyleSetAlignSelf(yoga_node_, YGAlignFlexEnd);
        } else if (style_.align_self == "stretch") {
            YGNodeStyleSetAlignSelf(yoga_node_, YGAlignStretch);
        }
    }
}

void Widget::setup_yoga_layout() {
    // Base implementation - override in derived classes
    apply_styles();
}

// ============================================================================
// Container Widget Implementation
// ============================================================================

ContainerWidget::ContainerWidget(const std::string& id) : Widget(id) {}

void ContainerWidget::add_child(std::unique_ptr<Widget> child) {
    if (!child) return;
    
    if (yoga_node_ && child->get_yoga_node()) {
        YGNodeInsertChild(yoga_node_, child->get_yoga_node(), children_.size());
    }
    
    children_.push_back(std::move(child));
}

void ContainerWidget::remove_child(const std::string& id) {
    auto it = std::find_if(children_.begin(), children_.end(),
        [&id](const std::unique_ptr<Widget>& widget) {
            return widget->get_id() == id;
        });
    
    if (it != children_.end()) {
        if (yoga_node_ && (*it)->get_yoga_node()) {
            YGNodeRemoveChild(yoga_node_, (*it)->get_yoga_node());
        }
        children_.erase(it);
    }
}

Widget* ContainerWidget::find_child(const std::string& id) {
    auto it = std::find_if(children_.begin(), children_.end(),
        [&id](const std::unique_ptr<Widget>& widget) {
            return widget->get_id() == id;
        });
    
    return (it != children_.end()) ? it->get() : nullptr;
}

void ContainerWidget::update_layout(float available_width, float available_height) {
    Widget::update_layout(available_width, available_height);
    
    // Update children layouts
    for (auto& child : children_) {
        float child_width = YGNodeLayoutGetWidth(child->get_yoga_node());
        float child_height = YGNodeLayoutGetHeight(child->get_yoga_node());
        child->update_layout(child_width, child_height);
    }
}

// ============================================================================
// Layout Widget Implementations
// ============================================================================

HLayoutWidget::HLayoutWidget(const std::string& id) : ContainerWidget(id) {
    setup_yoga_layout();
}

void HLayoutWidget::render() {
    // Render children horizontally
    for (size_t i = 0; i < children_.size(); i++) {
        if (i > 0) ImGui::SameLine();
        children_[i]->render();
    }
}

void HLayoutWidget::setup_yoga_layout() {
    if (yoga_node_) {
        YGNodeStyleSetFlexDirection(yoga_node_, YGFlexDirectionRow);
        YGNodeStyleSetAlignItems(yoga_node_, YGAlignCenter);
        YGNodeStyleSetGap(yoga_node_, YGGutterAll, style_.gap);
        
        // Apply container-specific alignment
        if (style_.justify == "center") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifyCenter);
        } else if (style_.justify == "flex-end") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifyFlexEnd);
        } else if (style_.justify == "space-between") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifySpaceBetween);
        } else if (style_.justify == "space-around") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifySpaceAround);
        } else if (style_.justify == "space-evenly") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifySpaceEvenly);
        } else {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifyFlexStart);
        }
        
        if (style_.align == "center") {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignCenter);
        } else if (style_.align == "flex-start") {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignFlexStart);
        } else if (style_.align == "flex-end") {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignFlexEnd);
        } else if (style_.align == "baseline") {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignBaseline);
        } else {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignStretch);
        }
    }
    
    ContainerWidget::apply_styles();
}

VLayoutWidget::VLayoutWidget(const std::string& id) : ContainerWidget(id) {
    setup_yoga_layout();
}

void VLayoutWidget::render() {
    // Render children vertically (natural ImGui flow)
    for (auto& child : children_) {
        child->render();
    }
}

void VLayoutWidget::setup_yoga_layout() {
    if (yoga_node_) {
        YGNodeStyleSetFlexDirection(yoga_node_, YGFlexDirectionColumn);
        YGNodeStyleSetGap(yoga_node_, YGGutterAll, style_.gap);
        
        // Apply same alignment logic as HLayout but for column direction
        if (style_.justify == "center") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifyCenter);
        } else if (style_.justify == "flex-end") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifyFlexEnd);
        } else if (style_.justify == "space-between") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifySpaceBetween);
        } else if (style_.justify == "space-around") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifySpaceAround);
        } else if (style_.justify == "space-evenly") {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifySpaceEvenly);
        } else {
            YGNodeStyleSetJustifyContent(yoga_node_, YGJustifyFlexStart);
        }
        
        if (style_.align == "center") {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignCenter);
        } else if (style_.align == "flex-start") {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignFlexStart);
        } else if (style_.align == "flex-end") {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignFlexEnd);
        } else if (style_.align == "baseline") {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignBaseline);
        } else {
            YGNodeStyleSetAlignItems(yoga_node_, YGAlignStretch);
        }
    }
    
    ContainerWidget::apply_styles();
}

// ============================================================================
// Leaf Widget Implementations
// ============================================================================

LabelWidget::LabelWidget(const std::string& id, const std::string& text) 
    : Widget(id), text_(text) {
    setup_yoga_layout();
}

void LabelWidget::render() {
    apply_styles();
    
    // Apply ImGui font scaling based on style
    if (style_.font_size == "small") {
        ImGui::SetWindowFontScale(0.8f);
    } else if (style_.font_size == "large") {
        ImGui::SetWindowFontScale(1.2f);
    } else {
        ImGui::SetWindowFontScale(1.0f);
    }
    
    // Apply text color
    ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // default white
    if (style_.text_color == "red") {
        text_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    } else if (style_.text_color == "green") {
        text_color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    } else if (style_.text_color == "blue") {
        text_color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    } else if (style_.text_color == "yellow") {
        text_color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    } else if (style_.text_color == "gray") {
        text_color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    
    if (style_.bold) {
        // Bold text approximation
        ImGui::Text("%s", text_.c_str());
        ImGui::SameLine(0, 0);
    }
    ImGui::Text("%s", text_.c_str());
    
    ImGui::PopStyleColor();
    
    // Reset font scale
    ImGui::SetWindowFontScale(1.0f);
}

InputTextWidget::InputTextWidget(const std::string& id, std::string* value) 
    : Widget(id), value_(value) {
    setup_yoga_layout();
}

void InputTextWidget::render() {
    apply_styles();
    
    if (value_) {
        strncpy(buffer_, value_->c_str(), sizeof(buffer_) - 1);
        buffer_[sizeof(buffer_) - 1] = '\0';
        
        float w = YGNodeLayoutGetWidth(yoga_node_);
        if (w > 0) {
            ImGui::SetNextItemWidth(w);
        }
        
        if (style_.disabled) {
            ImGui::BeginDisabled();
        }
        
        if (ImGui::InputText(("##" + id_).c_str(), buffer_, sizeof(buffer_))) {
            *value_ = buffer_;
        }
        
        if (style_.disabled) {
            ImGui::EndDisabled();
        }
    }
}

InputNumberWidget::InputNumberWidget(const std::string& id) : Widget(id) {
    setup_yoga_layout();
}

void InputNumberWidget::render() {
    apply_styles();
    
    float w = YGNodeLayoutGetWidth(yoga_node_);
    if (w > 0) {
        ImGui::SetNextItemWidth(w);
    }
    
    if (style_.disabled) {
        ImGui::BeginDisabled();
    }
    
    if (float_value_) {
        ImGui::InputFloat(("##" + id_).c_str(), float_value_);
    } else if (int_value_) {
        ImGui::InputInt(("##" + id_).c_str(), int_value_);
    }
    
    if (style_.disabled) {
        ImGui::EndDisabled();
    }
}

CheckboxWidget::CheckboxWidget(const std::string& id, const std::string& text, bool* value)
    : Widget(id), text_(text), value_(value) {
    setup_yoga_layout();
}

void CheckboxWidget::render() {
    apply_styles();
    
    if (style_.disabled) {
        ImGui::BeginDisabled();
    }
    
    if (value_) {
        ImGui::Checkbox(text_.c_str(), value_);
    }
    
    if (style_.disabled) {
        ImGui::EndDisabled();
    }
}

RadioButtonWidget::RadioButtonWidget(const std::string& id, const std::string& text, 
                                     const std::string& group, int value, int* selected)
    : Widget(id), text_(text), group_(group), value_(value), selected_(selected) {
    setup_yoga_layout();
}

void RadioButtonWidget::render() {
    apply_styles();
    
    if (style_.disabled) {
        ImGui::BeginDisabled();
    }
    
    if (selected_) {
        bool is_selected = (*selected_ == value_);
        if (ImGui::RadioButton(text_.c_str(), is_selected)) {
            *selected_ = value_;
        }
    }
    
    if (style_.disabled) {
        ImGui::EndDisabled();
    }
}

ButtonWidget::ButtonWidget(const std::string& id, const std::string& text)
    : Widget(id), text_(text) {
    setup_yoga_layout();
}

void ButtonWidget::render() {
    apply_styles();
    
    float w = YGNodeLayoutGetWidth(yoga_node_);
    float h = YGNodeLayoutGetHeight(yoga_node_);
    ImVec2 button_size(w > 0 ? w : 80, h > 0 ? h : 0);
    
    if (style_.disabled) {
        ImGui::BeginDisabled();
    }
    
    // Apply button variant styling
    int colors_pushed = 0;
    if (style_.variant == "primary") {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.6f, 1.0f, 1.0f));
        colors_pushed = 2;
    } else if (style_.variant == "danger") {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        colors_pushed = 2;
    }
    
    if (ImGui::Button(text_.c_str(), button_size)) {
        if (callback_) {
            callback_();
        }
    }
    
    if (colors_pushed > 0) {
        ImGui::PopStyleColor(colors_pushed);
    }
    
    if (style_.disabled) {
        ImGui::EndDisabled();
    }
}

// ============================================================================
// Widget Factory Implementation
// ============================================================================

std::unique_ptr<Widget> WidgetFactory::create_widget(const std::string& type, const std::string& id) {
    if (type == "label") {
        return std::make_unique<LabelWidget>(id);
    } else if (type == "input_text") {
        return std::make_unique<InputTextWidget>(id);
    } else if (type == "input_number") {
        return std::make_unique<InputNumberWidget>(id);
    } else if (type == "checkbox") {
        return std::make_unique<CheckboxWidget>(id);
    } else if (type == "radio") {
        return std::make_unique<RadioButtonWidget>(id);
    } else if (type == "button") {
        return std::make_unique<ButtonWidget>(id);
    } else if (type == "hlayout") {
        return std::make_unique<HLayoutWidget>(id);
    } else if (type == "vlayout") {
        return std::make_unique<VLayoutWidget>(id);
    }
    
    return nullptr;
}

std::unique_ptr<LabelWidget> WidgetFactory::create_label(const std::string& id, const std::string& text) {
    auto widget = std::make_unique<LabelWidget>(id, text);
    return widget;
}

std::unique_ptr<InputTextWidget> WidgetFactory::create_input_text(const std::string& id, std::string* value) {
    auto widget = std::make_unique<InputTextWidget>(id, value);
    return widget;
}

std::unique_ptr<InputNumberWidget> WidgetFactory::create_input_number(const std::string& id) {
    return std::make_unique<InputNumberWidget>(id);
}

std::unique_ptr<CheckboxWidget> WidgetFactory::create_checkbox(const std::string& id, const std::string& text, bool* value) {
    return std::make_unique<CheckboxWidget>(id, text, value);
}

std::unique_ptr<RadioButtonWidget> WidgetFactory::create_radio_button(const std::string& id, const std::string& text, 
                                                                       const std::string& group, int value, int* selected) {
    return std::make_unique<RadioButtonWidget>(id, text, group, value, selected);
}

std::unique_ptr<ButtonWidget> WidgetFactory::create_button(const std::string& id, const std::string& text) {
    return std::make_unique<ButtonWidget>(id, text);
}

std::unique_ptr<HLayoutWidget> WidgetFactory::create_hlayout(const std::string& id) {
    return std::make_unique<HLayoutWidget>(id);
}

std::unique_ptr<VLayoutWidget> WidgetFactory::create_vlayout(const std::string& id) {
    return std::make_unique<VLayoutWidget>(id);
}
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "imgui.h"
#include "yoga/Yoga.h"

// Forward declarations
class AppData;

/**
 * @brief Base class for all UI widgets
 * 
 * This abstract base class defines the interface for all widgets in the system.
 * It uses the Template Method pattern to define the rendering pipeline while
 * allowing concrete widgets to implement their specific rendering logic.
 */
class Widget {
public:
    virtual ~Widget();
    
    // Core interface
    virtual void render() = 0;
    virtual void update_layout(float available_width, float available_height);
    virtual bool accepts_children() const { return false; }
    
    // Property accessors
    const std::string& get_id() const { return id_; }
    void set_id(const std::string& id) { id_ = id; }
    
    float get_width() const { return width_; }
    void set_width(float width);
    
    float get_height() const { return height_; }
    void set_height(float height);
    
    float get_flex() const { return flex_; }
    void set_flex(float flex);
    
    // Style properties
    struct Style {
        float margin = 0.0f;
        float padding = 0.0f;
        float gap = 8.0f;
        
        std::string justify = "flex-start";
        std::string align = "stretch";
        std::string align_self = "auto";
        
        bool disabled = false;
        std::string variant = "default";
        
        std::string font_size = "default";
        bool bold = false;
        
        std::string text_color = "default";
        std::string bg_color = "default";
        
        bool stretch = false;
        bool wrap = false;
    };
    
    Style& get_style() { return style_; }
    const Style& get_style() const { return style_; }
    
    // Layout management
    YGNodeRef get_yoga_node() const { return yoga_node_; }
    
    // Style and layout application
    virtual void apply_styles();
    virtual void setup_yoga_layout();
    
protected:
    Widget(const std::string& id = "");
    
    std::string id_;
    float width_ = YGUndefined;
    float height_ = YGUndefined;
    float flex_ = YGUndefined;
    Style style_;
    YGNodeRef yoga_node_ = nullptr;
};

/**
 * @brief Container widget that can hold child widgets
 * 
 * This class implements the Composite pattern, allowing widgets to be
 * organized in a tree structure. It provides common functionality for
 * managing child widgets.
 */
class ContainerWidget : public Widget {
public:
    virtual ~ContainerWidget() = default;
    
    // Child management
    void add_child(std::unique_ptr<Widget> child);
    void remove_child(const std::string& id);
    Widget* find_child(const std::string& id);
    const std::vector<std::unique_ptr<Widget>>& get_children() const { return children_; }
    
    bool accepts_children() const override { return true; }
    void update_layout(float available_width, float available_height) override;

protected:
    ContainerWidget(const std::string& id = "");
    
    std::vector<std::unique_ptr<Widget>> children_;
};

/**
 * @brief Horizontal layout container using Flexbox
 */
class HLayoutWidget : public ContainerWidget {
public:
    explicit HLayoutWidget(const std::string& id = "");
    void render() override;

protected:
    void setup_yoga_layout() override;
};

/**
 * @brief Vertical layout container using Flexbox
 */
class VLayoutWidget : public ContainerWidget {
public:
    explicit VLayoutWidget(const std::string& id = "");
    void render() override;

protected:
    void setup_yoga_layout() override;
};

/**
 * @brief Text label widget
 */
class LabelWidget : public Widget {
public:
    explicit LabelWidget(const std::string& id = "", const std::string& text = "");
    
    void render() override;
    
    const std::string& get_text() const { return text_; }
    void set_text(const std::string& text) { text_ = text; }

private:
    std::string text_;
};

/**
 * @brief Text input widget
 */
class InputTextWidget : public Widget {
public:
    explicit InputTextWidget(const std::string& id = "", std::string* value = nullptr);
    
    void render() override;
    
    void bind_value(std::string* value) { value_ = value; }
    std::string* get_value() const { return value_; }

private:
    std::string* value_ = nullptr;
    char buffer_[256] = {0};
};

/**
 * @brief Number input widget (supports both int and float)
 */
class InputNumberWidget : public Widget {
public:
    explicit InputNumberWidget(const std::string& id = "");
    
    void render() override;
    
    void bind_float_value(float* value) { float_value_ = value; int_value_ = nullptr; }
    void bind_int_value(int* value) { int_value_ = value; float_value_ = nullptr; }
    
    float* get_float_value() const { return float_value_; }
    int* get_int_value() const { return int_value_; }

private:
    float* float_value_ = nullptr;
    int* int_value_ = nullptr;
};

/**
 * @brief Checkbox widget
 */
class CheckboxWidget : public Widget {
public:
    explicit CheckboxWidget(const std::string& id = "", const std::string& text = "", bool* value = nullptr);
    
    void render() override;
    
    const std::string& get_text() const { return text_; }
    void set_text(const std::string& text) { text_ = text; }
    
    void bind_value(bool* value) { value_ = value; }
    bool* get_value() const { return value_; }

private:
    std::string text_;
    bool* value_ = nullptr;
};

/**
 * @brief Radio button widget
 */
class RadioButtonWidget : public Widget {
public:
    explicit RadioButtonWidget(const std::string& id = "", const std::string& text = "", 
                              const std::string& group = "", int value = 0, int* selected = nullptr);
    
    void render() override;
    
    const std::string& get_text() const { return text_; }
    void set_text(const std::string& text) { text_ = text; }
    
    const std::string& get_group() const { return group_; }
    void set_group(const std::string& group) { group_ = group; }
    
    int get_value() const { return value_; }
    void set_value(int value) { value_ = value; }
    
    void bind_selected(int* selected) { selected_ = selected; }
    int* get_selected() const { return selected_; }

private:
    std::string text_;
    std::string group_;
    int value_ = 0;
    int* selected_ = nullptr;
};

/**
 * @brief Button widget
 */
class ButtonWidget : public Widget {
public:
    explicit ButtonWidget(const std::string& id = "", const std::string& text = "");
    
    void render() override;
    
    const std::string& get_text() const { return text_; }
    void set_text(const std::string& text) { text_ = text; }
    
    void set_callback(std::function<void()> callback) { callback_ = callback; }

private:
    std::string text_;
    std::function<void()> callback_;
};

/**
 * @brief Widget factory for creating widgets from strings
 * 
 * This class implements the Factory pattern to create widgets dynamically
 * based on string type names. Used primarily by the XML parser.
 */
class WidgetFactory {
public:
    static std::unique_ptr<Widget> create_widget(const std::string& type, const std::string& id = "");
    
    // Specialized creation methods
    static std::unique_ptr<LabelWidget> create_label(const std::string& id, const std::string& text);
    static std::unique_ptr<InputTextWidget> create_input_text(const std::string& id, std::string* value);
    static std::unique_ptr<InputNumberWidget> create_input_number(const std::string& id);
    static std::unique_ptr<CheckboxWidget> create_checkbox(const std::string& id, const std::string& text, bool* value);
    static std::unique_ptr<RadioButtonWidget> create_radio_button(const std::string& id, const std::string& text, 
                                                                  const std::string& group, int value, int* selected);
    static std::unique_ptr<ButtonWidget> create_button(const std::string& id, const std::string& text);
    static std::unique_ptr<HLayoutWidget> create_hlayout(const std::string& id);
    static std::unique_ptr<VLayoutWidget> create_vlayout(const std::string& id);
};
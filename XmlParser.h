#pragma once
#include "Widget.h"
#include "Panel.h"
#include "AppData.h"
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <ctime>
#include <vector>
#include <algorithm>

/**
 * @brief Strategy interface for handling different XML element types
 * 
 * This implements the Strategy pattern to handle parsing of different
 * XML element types in a extensible way.
 */
class ElementParsingStrategy {
public:
    virtual ~ElementParsingStrategy() = default;
    virtual std::unique_ptr<Widget> parse(void* xml_element, AppData* app_data, 
                                         const std::map<std::string, std::function<void()>>& callbacks) = 0;
};

/**
 * @brief Concrete strategies for different element types
 */
class LabelParsingStrategy : public ElementParsingStrategy {
public:
    std::unique_ptr<Widget> parse(void* xml_element, AppData* app_data, 
                                 const std::map<std::string, std::function<void()>>& callbacks) override;
};

class InputParsingStrategy : public ElementParsingStrategy {
public:
    std::unique_ptr<Widget> parse(void* xml_element, AppData* app_data, 
                                 const std::map<std::string, std::function<void()>>& callbacks) override;
};

class CheckboxParsingStrategy : public ElementParsingStrategy {
public:
    std::unique_ptr<Widget> parse(void* xml_element, AppData* app_data, 
                                 const std::map<std::string, std::function<void()>>& callbacks) override;
};

class RadioParsingStrategy : public ElementParsingStrategy {
public:
    std::unique_ptr<Widget> parse(void* xml_element, AppData* app_data, 
                                 const std::map<std::string, std::function<void()>>& callbacks) override;
};

class ButtonParsingStrategy : public ElementParsingStrategy {
public:
    std::unique_ptr<Widget> parse(void* xml_element, AppData* app_data, 
                                 const std::map<std::string, std::function<void()>>& callbacks) override;
};

class LayoutParsingStrategy : public ElementParsingStrategy {
public:
    std::unique_ptr<Widget> parse(void* xml_element, AppData* app_data, 
                                 const std::map<std::string, std::function<void()>>& callbacks) override;
};

/**
 * @brief Main XML parser class
 * 
 * This class implements the Builder pattern to construct widget hierarchies
 * from XML descriptions. It uses the Strategy pattern internally to handle
 * different element types.
 */
class XmlParser {
public:
    XmlParser();
    ~XmlParser();
    
    // Core functionality
    std::unique_ptr<Panel> parse_panel_from_file(const std::string& xml_file);
    std::unique_ptr<Widget> parse_widget_from_string(const std::string& xml_string);
    
    // Data binding
    void set_app_data(AppData* data) { app_data_ = data; }
    AppData* get_app_data() const { return app_data_; }
    
    // Callback management
    void add_button_callback(const std::string& id, std::function<void()> callback);
    void remove_button_callback(const std::string& id);
    void clear_callbacks();
    
    // Hot reload support
    bool reload_panel(Panel& panel, const std::string& xml_file);
    
    // Validation
    bool validate_xml_file(const std::string& xml_file, std::string& error_message);
    
private:
    AppData* app_data_ = nullptr;
    std::map<std::string, std::function<void()>> button_callbacks_;
    std::map<std::string, std::unique_ptr<ElementParsingStrategy>> strategies_;
    
    // Helper methods
    std::unique_ptr<Widget> parse_element(void* xml_element);
    void apply_properties_to_widget(Widget& widget, void* xml_element);
    void apply_style_properties(Widget::Style& style, void* xml_element);
    std::string get_attribute(void* xml_element, const std::string& name, const std::string& default_value = "");
    
    // Data binding helpers
    std::string* bind_string_field(const std::string& bind_expression);
    bool* bind_bool_field(const std::string& bind_expression);
    float* bind_float_field(const std::string& bind_expression);
    int* bind_int_field(const std::string& bind_expression);
    
    // Validation helpers
    bool validate_layout_hierarchy(Widget* widget, std::string& error_message);
    bool can_add_child(Widget* parent, Widget* child, std::string& error_message);
};

/**
 * @brief Observer interface for XML file changes
 * 
 * This implements the Observer pattern to enable hot reloading of XML files.
 */
class XmlFileObserver {
public:
    virtual ~XmlFileObserver() = default;
    virtual void on_file_changed(const std::string& file_path) = 0;
};

/**
 * @brief File watcher for hot reload functionality
 * 
 * Monitors XML files for changes and notifies observers when files are modified.
 */
class XmlFileWatcher {
public:
    explicit XmlFileWatcher(const std::string& file_path);
    ~XmlFileWatcher() = default;
    
    bool has_changed();
    void reset();
    
    void add_observer(XmlFileObserver* observer);
    void remove_observer(XmlFileObserver* observer);
    
private:
    std::string file_path_;
    std::time_t last_modified_time_;
    std::vector<XmlFileObserver*> observers_;
    
    void notify_observers();
    std::time_t get_file_modified_time(const std::string& file_path);
};

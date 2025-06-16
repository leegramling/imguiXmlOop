#include "XmlParser.h"
#include <tinyxml2.h>
#include <iostream>
#include <filesystem>
#include <ctime>
#include <chrono>

using namespace tinyxml2;

// ============================================================================
// Element Parsing Strategies Implementation
// ============================================================================

std::unique_ptr<Widget> LabelParsingStrategy::parse(void* xml_element, AppData* app_data, 
                                                   const std::map<std::string, std::function<void()>>& callbacks) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    
    std::string id = element->Attribute("id") ? element->Attribute("id") : "";
    std::string text = element->Attribute("text") ? element->Attribute("text") : "";
    
    return WidgetFactory::create_label(id, text);
}

std::unique_ptr<Widget> InputParsingStrategy::parse(void* xml_element, AppData* app_data, 
                                                   const std::map<std::string, std::function<void()>>& callbacks) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    
    std::string id = element->Attribute("id") ? element->Attribute("id") : "";
    std::string type = element->Attribute("type") ? element->Attribute("type") : "text";
    std::string bind = element->Attribute("bind") ? element->Attribute("bind") : "";
    
    if (type == "text") {
        auto widget = WidgetFactory::create_input_text(id, nullptr);
        
        // Bind to appropriate field
        if (bind == "name") {
            widget->bind_value(&app_data->name);
        } else if (bind == "email") {
            widget->bind_value(&app_data->email);
        } else if (bind.find("city_name_") == 0) {
            try {
                int city_idx = std::stoi(bind.substr(10));
                if (city_idx < app_data->cities.size()) {
                    widget->bind_value(&app_data->cities[city_idx].name);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing city name index: " << e.what() << std::endl;
            }
        }
        
        return std::move(widget);
    } else if (type == "number") {
        auto widget = WidgetFactory::create_input_number(id);
        
        // Bind to appropriate numeric field
        if (bind.find("city_lat_") == 0) {
            try {
                int city_idx = std::stoi(bind.substr(9));
                if (city_idx < app_data->cities.size()) {
                    widget->bind_float_value(&app_data->cities[city_idx].latitude);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing city lat index: " << e.what() << std::endl;
            }
        } else if (bind.find("city_lon_") == 0) {
            try {
                int city_idx = std::stoi(bind.substr(9));
                if (city_idx < app_data->cities.size()) {
                    widget->bind_float_value(&app_data->cities[city_idx].longitude);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing city lon index: " << e.what() << std::endl;
            }
        } else if (bind.find("city_elev_") == 0) {
            try {
                int city_idx = std::stoi(bind.substr(10));
                if (city_idx < app_data->cities.size()) {
                    widget->bind_int_value(&app_data->cities[city_idx].elevation);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing city elev index: " << e.what() << std::endl;
            }
        } else if (bind.find("city_temp_") == 0) {
            try {
                int city_idx = std::stoi(bind.substr(10));
                if (city_idx < app_data->cities.size()) {
                    widget->bind_float_value(&app_data->cities[city_idx].avg_temp);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing city temp index: " << e.what() << std::endl;
            }
        } else if (bind.find("city_pop_") == 0) {
            try {
                int city_idx = std::stoi(bind.substr(9));
                if (city_idx < app_data->cities.size()) {
                    widget->bind_int_value(&app_data->cities[city_idx].population);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing city pop index: " << e.what() << std::endl;
            }
        }
        
        return std::move(widget);
    }
    
    return nullptr;
}

std::unique_ptr<Widget> CheckboxParsingStrategy::parse(void* xml_element, AppData* app_data, 
                                                      const std::map<std::string, std::function<void()>>& callbacks) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    
    std::string id = element->Attribute("id") ? element->Attribute("id") : "";
    std::string text = element->Attribute("text") ? element->Attribute("text") : "";
    std::string bind = element->Attribute("bind") ? element->Attribute("bind") : "";
    
    auto widget = WidgetFactory::create_checkbox(id, text, nullptr);
    
    // Bind to appropriate boolean field
    if (bind == "python") {
        widget->bind_value(&app_data->python_selected);
    } else if (bind == "go") {
        widget->bind_value(&app_data->go_selected);
    } else if (bind == "swift") {
        widget->bind_value(&app_data->swift_selected);
    } else if (bind == "rust") {
        widget->bind_value(&app_data->rust_selected);
    } else if (bind == "cpp") {
        widget->bind_value(&app_data->cpp_selected);
    }
    
    return std::move(widget);
}

std::unique_ptr<Widget> RadioParsingStrategy::parse(void* xml_element, AppData* app_data, 
                                                   const std::map<std::string, std::function<void()>>& callbacks) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    
    std::string id = element->Attribute("id") ? element->Attribute("id") : "";
    std::string text = element->Attribute("text") ? element->Attribute("text") : "";
    std::string group = element->Attribute("group") ? element->Attribute("group") : "";
    std::string value_str = element->Attribute("value") ? element->Attribute("value") : "0";
    std::string bind = element->Attribute("bind") ? element->Attribute("bind") : "";
    
    int value = std::stoi(value_str);
    auto widget = WidgetFactory::create_radio_button(id, text, group, value, nullptr);
    
    // Bind to appropriate integer field
    if (bind.find("city_climate_") == 0) {
        try {
            int city_idx = std::stoi(bind.substr(13));
            if (city_idx < app_data->cities.size()) {
                widget->bind_selected(&app_data->cities[city_idx].climate_zone);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing city climate index: " << e.what() << std::endl;
        }
    }
    
    return std::move(widget);
}

std::unique_ptr<Widget> ButtonParsingStrategy::parse(void* xml_element, AppData* app_data, 
                                                    const std::map<std::string, std::function<void()>>& callbacks) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    
    std::string id = element->Attribute("id") ? element->Attribute("id") : "";
    std::string text = element->Attribute("text") ? element->Attribute("text") : "";
    
    auto widget = WidgetFactory::create_button(id, text);
    
    // Set callback if available
    auto callback_it = callbacks.find(id);
    if (callback_it != callbacks.end()) {
        widget->set_callback(callback_it->second);
    }
    
    return std::move(widget);
}

std::unique_ptr<Widget> LayoutParsingStrategy::parse(void* xml_element, AppData* app_data, 
                                                    const std::map<std::string, std::function<void()>>& callbacks) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    
    std::string id = element->Attribute("id") ? element->Attribute("id") : "";
    std::string node_name = element->Name();
    
    if (node_name == "hlayout") {
        return WidgetFactory::create_hlayout(id);
    } else if (node_name == "vlayout") {
        return WidgetFactory::create_vlayout(id);
    }
    
    return nullptr;
}

// ============================================================================
// XML Parser Implementation
// ============================================================================

XmlParser::XmlParser() {
    // Initialize parsing strategies
    strategies_["label"] = std::make_unique<LabelParsingStrategy>();
    strategies_["input"] = std::make_unique<InputParsingStrategy>();
    strategies_["checkbox"] = std::make_unique<CheckboxParsingStrategy>();
    strategies_["radio"] = std::make_unique<RadioParsingStrategy>();
    strategies_["button"] = std::make_unique<ButtonParsingStrategy>();
    strategies_["hlayout"] = std::make_unique<LayoutParsingStrategy>();
    strategies_["vlayout"] = std::make_unique<LayoutParsingStrategy>();
}

XmlParser::~XmlParser() = default;

std::unique_ptr<Panel> XmlParser::parse_panel_from_file(const std::string& xml_file) {
    XMLDocument doc;
    if (doc.LoadFile(xml_file.c_str()) != XML_SUCCESS) {
        std::cerr << "Failed to load XML file: " << xml_file << std::endl;
        return nullptr;
    }
    
    XMLElement* panel_element = doc.FirstChildElement("panel");
    if (!panel_element) {
        std::cerr << "No panel element found in XML" << std::endl;
        return nullptr;
    }
    
    std::string title = get_attribute(panel_element, "title", "Panel");
    float width = 400.0f;
    float height = 300.0f;
    
    std::string width_str = get_attribute(panel_element, "width");
    std::string height_str = get_attribute(panel_element, "height");
    
    if (!width_str.empty()) width = std::stof(width_str);
    if (!height_str.empty()) height = std::stof(height_str);
    
    auto panel = std::make_unique<Panel>(title, width, height);
    
    // Parse root widget
    XMLElement* root_element = panel_element->FirstChildElement();
    if (root_element) {
        auto root_widget = parse_element(root_element);
        if (root_widget) {
            std::string validation_error;
            if (validate_layout_hierarchy(root_widget.get(), validation_error)) {
                panel->set_root_widget(std::move(root_widget));
            } else {
                std::cerr << "Layout validation error: " << validation_error << std::endl;
                // Set anyway but warn user
                panel->set_root_widget(std::move(root_widget));
            }
        }
    }
    
    return panel;
}

std::unique_ptr<Widget> XmlParser::parse_element(void* xml_element) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    std::string node_name = element->Name();
    std::string id = get_attribute(element, "id");
    
    std::unique_ptr<Widget> widget;
    
    // Use strategy pattern to parse different element types
    auto strategy_it = strategies_.find(node_name);
    if (strategy_it != strategies_.end()) {
        widget = strategy_it->second->parse(xml_element, app_data_, button_callbacks_);
    }
    
    if (!widget) {
        std::cerr << "Unknown element type: " << node_name << std::endl;
        return nullptr;
    }
    
    // Apply common properties
    apply_properties_to_widget(*widget, xml_element);
    
    // Parse children for container widgets
    ContainerWidget* container = dynamic_cast<ContainerWidget*>(widget.get());
    if (container) {
        for (XMLElement* child = element->FirstChildElement(); child; child = child->NextSiblingElement()) {
            auto child_widget = parse_element(child);
            if (child_widget) {
                container->add_child(std::move(child_widget));
            }
        }
    }
    
    return widget;
}

void XmlParser::apply_properties_to_widget(Widget& widget, void* xml_element) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    
    // Layout properties
    std::string width_str = get_attribute(element, "width");
    std::string height_str = get_attribute(element, "height");
    std::string flex_str = get_attribute(element, "flex");
    
    if (!width_str.empty()) {
        widget.set_width(std::stof(width_str));
    }
    if (!height_str.empty()) {
        widget.set_height(std::stof(height_str));
    }
    if (!flex_str.empty()) {
        widget.set_flex(std::stof(flex_str));
    }
    
    // Apply style properties
    apply_style_properties(widget.get_style(), xml_element);
    
    // Re-setup yoga layout with new properties
    widget.setup_yoga_layout();
}

void XmlParser::apply_style_properties(Widget::Style& style, void* xml_element) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    
    // Spacing
    std::string margin_str = get_attribute(element, "margin");
    if (!margin_str.empty()) {
        style.margin = std::stof(margin_str);
    }
    
    std::string padding_str = get_attribute(element, "padding");
    if (!padding_str.empty()) {
        style.padding = std::stof(padding_str);
    }
    
    std::string gap_str = get_attribute(element, "gap");
    if (!gap_str.empty()) {
        style.gap = std::stof(gap_str);
    }
    
    // Alignment
    style.justify = get_attribute(element, "justify", style.justify);
    style.align = get_attribute(element, "align", style.align);
    style.align_self = get_attribute(element, "align-self", style.align_self);
    
    // Appearance
    std::string disabled_str = get_attribute(element, "disabled");
    if (!disabled_str.empty()) {
        style.disabled = (disabled_str == "true" || disabled_str == "1");
    }
    
    style.variant = get_attribute(element, "variant", style.variant);
    
    // Text
    style.font_size = get_attribute(element, "font-size", style.font_size);
    
    std::string bold_str = get_attribute(element, "bold");
    if (!bold_str.empty()) {
        style.bold = (bold_str == "true" || bold_str == "1");
    }
    
    // Colors
    style.text_color = get_attribute(element, "text-color", style.text_color);
    style.bg_color = get_attribute(element, "bg-color", style.bg_color);
    
    // Behavior
    std::string stretch_str = get_attribute(element, "stretch");
    if (!stretch_str.empty()) {
        style.stretch = (stretch_str == "true" || stretch_str == "1");
    }
    
    std::string wrap_str = get_attribute(element, "wrap");
    if (!wrap_str.empty()) {
        style.wrap = (wrap_str == "true" || wrap_str == "1");
    }
}

std::string XmlParser::get_attribute(void* xml_element, const std::string& name, const std::string& default_value) {
    XMLElement* element = static_cast<XMLElement*>(xml_element);
    const char* attr = element->Attribute(name.c_str());
    return attr ? std::string(attr) : default_value;
}

void XmlParser::add_button_callback(const std::string& id, std::function<void()> callback) {
    button_callbacks_[id] = callback;
}

void XmlParser::remove_button_callback(const std::string& id) {
    button_callbacks_.erase(id);
}

void XmlParser::clear_callbacks() {
    button_callbacks_.clear();
}

bool XmlParser::reload_panel(Panel& panel, const std::string& xml_file) {
    auto new_panel = parse_panel_from_file(xml_file);
    if (new_panel) {
        panel.set_title(new_panel->get_title());
        panel.set_width(new_panel->get_width());
        panel.set_height(new_panel->get_height());
        
        // Transfer ownership of the root widget
        // We need to extract the widget from the new panel
        // This is a bit tricky because we need to move ownership
        // For now, let's rebuild the panel entirely in the calling code
        return true;
    }
    return false;
}

bool XmlParser::validate_layout_hierarchy(Widget* widget, std::string& error_message) {
    ContainerWidget* container = dynamic_cast<ContainerWidget*>(widget);
    if (container) {
        for (const auto& child : container->get_children()) {
            ContainerWidget* child_container = dynamic_cast<ContainerWidget*>(child.get());
            if (child_container) {
                // Check for same-type nesting
                HLayoutWidget* parent_hlayout = dynamic_cast<HLayoutWidget*>(container);
                VLayoutWidget* parent_vlayout = dynamic_cast<VLayoutWidget*>(container);
                HLayoutWidget* child_hlayout = dynamic_cast<HLayoutWidget*>(child.get());
                VLayoutWidget* child_vlayout = dynamic_cast<VLayoutWidget*>(child.get());
                
                if ((parent_hlayout && child_hlayout) || (parent_vlayout && child_vlayout)) {
                    error_message = "Layout container '" + container->get_id() + 
                                   "' contains child layout '" + child->get_id() + 
                                   "' of the same type. Consider using different layout types.";
                    return false;
                }
                
                // Recursively validate children
                if (!validate_layout_hierarchy(child.get(), error_message)) {
                    return false;
                }
            }
        }
    }
    return true;
}

// ============================================================================
// File Watcher Implementation
// ============================================================================

XmlFileWatcher::XmlFileWatcher(const std::string& file_path) : file_path_(file_path) {
    last_modified_time_ = get_file_modified_time(file_path_);
}

bool XmlFileWatcher::has_changed() {
    std::time_t current_time = get_file_modified_time(file_path_);
    if (current_time != last_modified_time_ && current_time != 0) {
        last_modified_time_ = current_time;
        notify_observers();
        return true;
    }
    return false;
}

void XmlFileWatcher::reset() {
    last_modified_time_ = get_file_modified_time(file_path_);
}

void XmlFileWatcher::add_observer(XmlFileObserver* observer) {
    observers_.push_back(observer);
}

void XmlFileWatcher::remove_observer(XmlFileObserver* observer) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void XmlFileWatcher::notify_observers() {
    for (auto* observer : observers_) {
        observer->on_file_changed(file_path_);
    }
}

std::time_t XmlFileWatcher::get_file_modified_time(const std::string& file_path) {
    try {
        if (std::filesystem::exists(file_path)) {
            auto ftime = std::filesystem::last_write_time(file_path);
            return std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
        }
    } catch (const std::exception&) {
        // File doesn't exist or other error
    }
    return 0;
}
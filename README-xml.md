# ImGui XML OOP - Object-Oriented UI Framework

This project demonstrates a comprehensive object-oriented approach to building XML-driven user interfaces with ImGui. It serves as a counterpart to the data-oriented `panelWidgets` project, showcasing how the same functionality can be implemented using different programming paradigms.

## 🎯 Project Overview

This framework implements a complete UI system using **Object-Oriented Programming (OOP)** principles with ImGui as the rendering backend. The system allows developers to define user interfaces in XML and have them automatically rendered with proper data binding, styling, and interaction handling.

## 🏗️ Architecture & Design Patterns

### Core Design Patterns Used

#### 1. **Template Method Pattern**
The base `Widget` class defines the widget rendering pipeline while allowing concrete implementations to customize specific steps:

```cpp
class Widget {
public:
    virtual void render() = 0;           // Pure virtual - must implement
    virtual void update_layout(...);     // Template method
protected:
    virtual void apply_styles();         // Hook for customization
    virtual void setup_yoga_layout();    // Hook for layout setup
};
```

#### 2. **Composite Pattern**
The `ContainerWidget` class implements the Composite pattern, allowing widgets to be organized in tree structures:

```cpp
class ContainerWidget : public Widget {
    std::vector<std::unique_ptr<Widget>> children_;
public:
    void add_child(std::unique_ptr<Widget> child);
    void remove_child(const std::string& id);
    Widget* find_child(const std::string& id);
};
```

#### 3. **Strategy Pattern**
The XML parser uses different strategies for parsing different element types:

```cpp
class ElementParsingStrategy {
public:
    virtual std::unique_ptr<Widget> parse(void* xml_element, ...) = 0;
};

class LabelParsingStrategy : public ElementParsingStrategy { ... };
class InputParsingStrategy : public ElementParsingStrategy { ... };
class ButtonParsingStrategy : public ElementParsingStrategy { ... };
```

#### 4. **Factory Pattern**
The `WidgetFactory` creates widgets dynamically based on string type names:

```cpp
class WidgetFactory {
public:
    static std::unique_ptr<Widget> create_widget(const std::string& type, ...);
    static std::unique_ptr<LabelWidget> create_label(...);
    static std::unique_ptr<ButtonWidget> create_button(...);
};
```

#### 5. **Builder Pattern**
The `XmlParser` builds complex widget hierarchies step by step:

```cpp
class XmlParser {
    std::unique_ptr<Panel> parse_panel_from_file(const std::string& xml_file);
    std::unique_ptr<Widget> parse_element(void* xml_element);
    void apply_properties_to_widget(Widget& widget, void* xml_element);
};
```

#### 6. **Observer Pattern**
The file watching system uses observers for hot reload functionality:

```cpp
class XmlFileObserver {
public:
    virtual void on_file_changed(const std::string& file_path) = 0;
};

class XmlFileWatcher {
    std::vector<XmlFileObserver*> observers_;
public:
    void add_observer(XmlFileObserver* observer);
    void notify_observers();
};
```

#### 7. **Mediator Pattern**
The `PanelManager` coordinates between multiple panels:

```cpp
class PanelManager {
    std::map<std::string, std::unique_ptr<Panel>> panels_;
public:
    void add_panel(const std::string& name, std::unique_ptr<Panel> panel);
    void render_all();
    void show_panel(const std::string& name);
};
```

#### 8. **Facade Pattern**
The `Application` class provides a simplified interface to the complex subsystem:

```cpp
class Application : public XmlFileObserver {
public:
    bool initialize();
    void run();
    void shutdown();
private:
    // Encapsulates SDL, ImGui, XML parsing, panel management
};
```

## 🏛️ Class Hierarchy

### Widget Hierarchy
```
Widget (abstract base)
├── ContainerWidget (abstract)
│   ├── HLayoutWidget (horizontal layout)
│   └── VLayoutWidget (vertical layout)
├── LabelWidget (text display)
├── InputTextWidget (text input)
├── InputNumberWidget (numeric input)
├── CheckboxWidget (boolean input)
├── RadioButtonWidget (radio selection)
└── ButtonWidget (clickable button)
```

### Core Components
```
Panel (UI window container)
├── title, width, height properties
├── root_widget (Widget hierarchy)
└── rendering and layout management

PanelManager (Singleton)
├── panel collection management
├── coordinated rendering
└── panel lifecycle management

XmlParser (Builder)
├── strategy-based element parsing
├── data binding system
└── validation and error handling
```

## 🔧 Key Features

### 1. **Type Safety & Encapsulation**
- Strong typing with virtual functions
- Private member variables with accessor methods
- Clear public interfaces with hidden implementation details

### 2. **Polymorphism**
- Virtual functions for widget-specific behavior
- Runtime type identification with `dynamic_cast`
- Strategy pattern for extensible parsing

### 3. **RAII & Memory Management**
- `std::unique_ptr` for automatic memory management
- RAII for resource cleanup (Yoga nodes, SDL resources)
- Exception-safe resource handling

### 4. **Extensibility**
- Easy to add new widget types by inheriting from `Widget`
- New parsing strategies can be added without modifying existing code
- Template-based type-safe widget lookup

### 5. **Data Binding System**
```cpp
// Type-safe binding methods
void bind_value(std::string* value);        // For text inputs
void bind_value(bool* value);               // For checkboxes
void bind_float_value(float* value);        // For numeric inputs
void bind_selected(int* selected);          // For radio buttons
```

### 6. **Hot Reload Support**
- Observer pattern for file change notifications
- Automatic panel reconstruction on XML changes
- State preservation during reloads

## 🆚 OOP vs Data-Oriented Comparison

| Aspect | OOP Approach (This Project) | Data-Oriented Approach (panelWidgets) |
|--------|---------------------------|--------------------------------------|
| **Structure** | Class hierarchies with inheritance | Structs and free functions |
| **Memory Layout** | Objects scattered in memory | Potentially better cache locality |
| **Extensibility** | Inheritance and virtual functions | Function pointers and composition |
| **Type Safety** | Strong compile-time checking | Manual type management |
| **Performance** | Virtual function call overhead | Direct function calls |
| **Maintainability** | Clear abstractions and interfaces | Simpler, more direct code |
| **Complexity** | Higher cognitive overhead | Lower conceptual complexity |

## 🚀 Getting Started

### Prerequisites
- C++20 compatible compiler
- SDL2 development libraries
- CMake 3.20 or higher

### Building the Project

1. **Clone dependencies:**
   ```bash
   ./setup.sh
   ```

2. **Build:**
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

3. **Run:**
   ```bash
   ./imgui_oop_app
   ```

### Project Structure
```
imguiXmlOop/
├── Widget.h/cpp           # Base widget classes and hierarchy
├── Panel.h/cpp            # Panel management and rendering
├── XmlParser.h/cpp        # XML parsing with strategy pattern
├── main.cpp               # Application facade and entry point
├── contact_panel.xml      # Contact form definition
├── city_data_panel.xml    # Data grid definition
├── CMakeLists.txt         # Build configuration
├── setup.sh               # Dependency setup script
└── thirdparty/            # External dependencies
    ├── imgui/
    ├── yoga/
    └── tinyxml2/
```

## 🎨 Usage Examples

### Creating Widgets Programmatically
```cpp
// Using factory pattern
auto label = WidgetFactory::create_label("header", "Welcome");
auto input = WidgetFactory::create_input_text("name_input", &app_data.name);

// Creating layouts
auto layout = WidgetFactory::create_vlayout("main_layout");
layout->add_child(std::move(label));
layout->add_child(std::move(input));
```

### Panel Management
```cpp
// Create and register panels
auto panel = std::make_unique<Panel>("My Panel", 400, 300);
panel->set_root_widget(std::move(layout));
PanelManager::instance().add_panel("my_panel", std::move(panel));

// Show/hide panels
PanelManager::instance().show_panel("my_panel");
PanelManager::instance().render_all();
```

### XML-Driven UI
```xml
<panel title="Contact Form" width="450" height="400">
    <vlayout id="main_layout" padding="16" gap="12">
        <label id="header" text="Contact Information" font-size="large"/>
        <input id="name_input" bind="name" flex="1"/>
        <button id="save_btn" text="Save" variant="primary"/>
    </vlayout>
</panel>
```

### Data Binding
```cpp
// Automatic binding via XML attributes
<input id="name" bind="name"/>           <!-- binds to app_data.name -->
<checkbox id="python" bind="python"/>    <!-- binds to app_data.python_selected -->
<input id="lat" type="number" bind="city_lat_0"/>  <!-- binds to app_data.cities[0].latitude -->
```

## 🔍 Advanced Features

### Type-Safe Widget Lookup
```cpp
// Find widgets with type safety
auto* input = panel.find_widget_as<InputTextWidget>("name_input");
if (input) {
    input->set_text("Default Value");
}
```

### Custom Widget Creation
```cpp
class CustomWidget : public Widget {
public:
    CustomWidget(const std::string& id) : Widget(id) {}
    
    void render() override {
        // Custom rendering logic
        ImGui::Text("Custom Widget: %s", get_id().c_str());
    }
};

// Register with factory
// (Would need factory extension for full integration)
```

### Event Handling
```cpp
// Button callbacks with lambda capture
parser->add_button_callback("save_button", [&app_data]() {
    std::cout << "Saving: " << app_data.name << std::endl;
    // Custom save logic here
});
```

## 🎯 Benefits of OOP Approach

### 1. **Clear Abstractions**
- Each widget type has a well-defined interface
- Behavior is encapsulated within appropriate classes
- Complex systems are broken down into manageable components

### 2. **Code Reusability**
- Base classes provide common functionality
- Derived classes can override specific behaviors
- Polymorphism enables generic algorithms

### 3. **Maintainability**
- Changes to widget behavior are localized to specific classes
- Interface contracts ensure consistent behavior
- Testing can be done at the class level

### 4. **Extensibility**
- New widget types can be added by inheritance
- Existing code doesn't need modification
- Plugin-like architecture is possible

### 5. **Type Safety**
- Compile-time checking prevents many runtime errors
- Template-based helpers provide additional safety
- RAII ensures proper resource management

## ⚠️ Trade-offs & Considerations

### Performance Implications
- **Virtual function overhead**: Each widget method call goes through vtable
- **Memory fragmentation**: Objects may be scattered in memory
- **Cache efficiency**: Potentially worse cache locality compared to DoP

### Complexity Considerations
- **Learning curve**: Requires understanding of multiple design patterns
- **Cognitive overhead**: More abstractions to understand
- **Debugging complexity**: Call stacks may be deeper

### When to Use OOP Approach
- ✅ Complex UI with many different widget types
- ✅ Need for extensive customization and extension
- ✅ Large development teams requiring clear interfaces
- ✅ Long-term maintainability is priority
- ✅ Type safety is critical

### When to Consider Alternatives
- ❌ Performance is the absolute priority
- ❌ Simple UIs with few widget types
- ❌ Memory usage must be minimized
- ❌ Development team prefers simpler approaches

## 🔄 Comparison with Data-Oriented Design

The sister project `panelWidgets` implements the same functionality using Data-Oriented Programming principles. Key differences:

### OOP Strengths
- Better abstraction and encapsulation
- Easier to extend with new widget types
- Type-safe interfaces and error checking
- Clear ownership and lifecycle management

### DoP Strengths (from sister project)
- Better cache efficiency and performance
- Simpler mental model and debugging
- More direct control over memory layout
- Potentially faster compilation times

Both approaches have their merits, and the choice depends on your specific requirements, team preferences, and project constraints.

## 📝 License

This project is provided as an educational example demonstrating object-oriented programming principles in GUI development.

## 🤝 Contributing

This is an educational project, but contributions that demonstrate additional OOP patterns or improve the existing implementation are welcome!

---

*This project demonstrates the power and flexibility of object-oriented design in creating maintainable, extensible UI frameworks while acknowledging the trade-offs involved in different programming paradigms.*
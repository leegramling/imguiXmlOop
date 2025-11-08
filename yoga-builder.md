# Yoga Builder Guide

This project wraps Dear ImGui and Facebook Yoga with a small builder DSL so you can describe complex layouts in code or XML. The contact panel (`contact_panel.xml`) is a good example to study: a single vertical layout (`<vlayout>`) owns several child layouts (horizontal rows, nested vertical stacks, and buttons) and wires inputs to `AppData` plus button callbacks.

This document explains:

1. How Yoga thinks about layouts (rows, columns, grids, nesting).
2. How the builder pattern implemented in `UiBuilder.h` lets you author UI trees fluently.
3. How to write your own builder class that constructs a `Panel`, binds data, and registers callbacks.
4. How to hook callbacks / bindings so ImGui widgets talk to your application.

---

## 1. Yoga Basics in This Project

Each `Widget` owns a Yoga node (`YGNodeRef`). Containers (`HLayoutWidget`, `VLayoutWidget`) configure the underlying node:

| Widget          | Yoga property                                        |
| --------------- | ---------------------------------------------------- |
| `HLayoutWidget` | `YGFlexDirectionRow` (children laid out horizontally)|
| `VLayoutWidget` | `YGFlexDirectionColumn` (children stacked vertically)|

Useful attributes (available in XML and via builders):

- `flex`: Distributes remaining space within the parent (think CSS `flex-grow`).
- `width` / `height`: Explicit size in pixels (scaled by DPI later).
- `margin`, `padding`, `gap`: Spacing on the Yoga node (gaps apply between children).
- `justify`, `align`, `align-self`: Map directly to Yoga’s justify-content / align-items / align-self.

### Building a grid

Yoga doesn’t have an explicit grid primitive, but you get the same result by combining vertical and horizontal layouts:

```
VLayout
 ├── HLayout (row 1)
 │    ├── city field (flex 2)
 │    ├── lat field (flex 1)
 │    └── …
 ├── HLayout (row 2)
 │    └── …
 └── …
```

This is exactly what `city_data_panel.xml` does, and you can follow the same pattern for any table-like UI.

---

## 2. Understanding the Builder Pattern (`UiBuilder.h`)

Key pieces:

- `WidgetBuilderBase<Derived, WidgetType>` implements a fluent API: every setter (`width`, `padding`, `text_color`, etc.) returns `*this` so you can chain calls.
- `ContainerBuilderBase` extends `WidgetBuilderBase` with `add_child`, allowing nested layouts.
- Concrete builders like `VLayoutBuilder`, `HLayoutBuilder`, `LabelBuilder`, `ButtonBuilder`, etc. simply inherit the fluent API and expose type-specific helpers (`ButtonBuilder::on_click`, `LabelBuilder::text`).

Because builders return `std::unique_ptr<Widget>` from `build()`, you can compose them without worrying about raw pointers. Example:

```cpp
auto buttons = HLayoutBuilder("buttons_row")
    .justify("space-evenly")
    .gap(12.0f)
    .add_child(ButtonBuilder("ok_button", "Save").variant("primary").on_click(on_ok))
    .add_child(ButtonBuilder("cancel_button", "Cancel").variant("danger").on_click(on_cancel))
    .build();
```

### Writing your own builder class

To bundle a full panel, create a small façade (see `CityDataPanelBuilder`):

```cpp
class ContactPanelBuilder {
public:
    explicit ContactPanelBuilder(AppData& data) : data_(data) {}

    ContactPanelBuilder& with_title(std::string title) { title_ = std::move(title); return *this; }
    ContactPanelBuilder& with_size(float w, float h) { width_ = w; height_ = h; return *this; }
    ContactPanelBuilder& on_save(std::function<void()> cb) { on_save_ = std::move(cb); return *this; }
    // … add other fluent setters …

    std::unique_ptr<Panel> build();

private:
    AppData& data_;
    std::string title_ = "Contact Form";
    float width_ = 450.0f;
    float height_ = 400.0f;
    std::function<void()> on_save_;
    std::function<void()> on_cancel_;
};
```

Inside `build()`:

1. Instantiate a `Panel title width height`.
2. Create a root layout (`VLayoutBuilder`) and configure padding/gap.
3. Compose rows/sections using more builders.
4. Call `panel->set_root_widget(root.build())`.
5. Register callbacks (e.g., `ButtonBuilder::on_click` or by storing widget IDs and calling `Panel::find_widget` later).

---

## 3. Example: Contact Panel Layout

`contact_panel.xml` describes the UI declaratively. Translating it to builders looks like this (you could use it as the body of `ContactPanelBuilder::build()`):

```cpp
std::unique_ptr<Panel> ContactPanelBuilder::build() {
    auto panel = std::make_unique<Panel>(title_, width_, height_);

    auto main_layout = VLayoutBuilder("main_layout")
        .padding(16.0f)
        .gap(12.0f)
        .align("stretch");

    main_layout.add_child(LabelBuilder("header", "📝 Contact Information")
        .font_size("large")
        .text_color("blue")
        .align_self("center")
        .margin(8.0f));

    auto make_row = [&](std::string id, const char* label_text, std::string* binding) {
        return HLayoutBuilder(std::move(id))
            .align("center").gap(10.0f).margin(4.0f)
            .add_child(LabelBuilder(label_text + std::string("_label"), label_text).width(80.0f))
            .add_child(InputTextBuilder(label_text + std::string("_input"), binding).flex(1.0f))
            .build();
    };

    main_layout.add_child(make_row("name_row", "Name:", &data_.name));
    main_layout.add_child(make_row("email_row", "Email:", &data_.email));

    auto languages = VLayoutBuilder("languages_section")
        .gap(6.0f).padding(12.0f).margin(8.0f)
        .add_child(LabelBuilder("languages_label", "Programming Languages:")
            .text_color("green").bold(true))
        .add_child(CheckboxBuilder("python_check", "🐍 Python").bind(&data_.python_selected))
        .add_child(CheckboxBuilder("swift_check", "🦄 Swift").bind(&data_.swift_selected))
        .add_child(CheckboxBuilder("cpp_check", "⚡ C++").bind(&data_.cpp_selected))
        .build();
    main_layout.add_child(std::move(languages));

    auto buttons = HLayoutBuilder("buttons_row")
        .justify("space-evenly").gap(12.0f).margin(8.0f)
        .add_child(ButtonBuilder("ok_button", "✅ Save Contact")
            .flex(1.0f).variant("primary").on_click(on_save_))
        .add_child(ButtonBuilder("cancel_button", "❌ Cancel")
            .flex(1.0f).variant("danger").on_click(on_cancel_))
        .build();
    main_layout.add_child(std::move(buttons));

    main_layout.add_child(LabelBuilder("footer", "All fields are optional")
        .font_size("small").text_color("gray").align_self("center").margin(4.0f));

    panel->set_root_widget(main_layout.build());
    return panel;
}
```

Notice how we nest layouts:

- The root `VLayout` stacks everything vertically.
- Name/email rows are `HLayout`s to align label + input horizontally.
- The languages section is a nested `VLayout`.
- Buttons are another `HLayout`.

This mirrors the XML structure exactly, which makes it trivial to flip between declarative XML and builder-based C++.

---

## 4. Callbacks & Data Binding

There are two complementary mechanisms:

1. **Data binding:** XML `bind="name"` or `InputTextBuilder("...", &data_.name)` connects a widget to fields inside `AppData`. The builders simply store pointers into `AppData`. Yoga doesn’t care about bindings; it only handles layout.

2. **Callbacks:** Use `ButtonBuilder::on_click`, `CheckboxBuilder::bind_value`, etc. For XML-driven panels, `XmlParser::add_button_callback("ok_button", callback)` registers handlers (see `Application::setup_button_callbacks` in `main.cpp`). When you construct panels via C++ builders, call `.on_click` directly.

Because callbacks capture `this`, you can update app state, toggle DPI, or call `PanelManager::update_all_layouts()` as needed.

---

## 5. Tips for Custom Builders

- **Keep layout intent clear.** Favor small helper functions (`build_city_row`, `build_button_row`) so it’s obvious where Yoga rows/columns begin and end.
- **Leverage `flex`.** Set `flex(1.0f)` (or higher) on controls that should grow, keep labels at fixed widths.
- **Spacing is inherited.** `gap`, `padding`, and `margin` are Yoga properties and scale during DPI changes because `Widget::apply_styles` reapplies them before each layout pass.
- **Nested layouts are cheap.** Don’t hesitate to create a `VLayout` inside an `HLayout` if it clarifies structure; Yoga resolves the tree recursively.
- **Callbacks live near construction.** Pass std::function captures into your builder so that `build()` returns a panel that is immediately interactive.

With these patterns you can design any combination of vertical, horizontal, or pseudo-grid layouts, wire them to data/callbacks, and let Yoga handle the math. When in doubt, inspect `CityDataPanelBuilder.cpp` and `contact_panel.xml` side by side—they demonstrate every concept described above.

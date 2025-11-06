# ImGui Yoga Builder Guide

## Overview
- `imgui_builder` showcases a functor-driven builder pattern that creates the city data panel entirely in C++ without XML. Widgets are composed with fluent helpers in `UiBuilder.h`, enabling callbacks and Yoga settings to be wired inline.
- `imgui_oop_app` keeps the original XML-driven workflow. The XML is parsed into widgets, callbacks are matched by id, and Yoga is still responsible for flex-style sizing.
- Shared data, callbacks, and Yoga layout rules live in reusable components so both approaches behave consistently during resize, DPI changes, and hot reloads.

## Yoga Layout Overview
- Yoga represents every widget as a node with flex properties (width, height, flex, gap, align, justify). We attach a Yoga node to each `Widget` and call `YGNodeCalculateLayout`.
- The results feed ImGui rendering: after `Panel::render` queries the content region, the root widget recomputes layout so all inputs and labels pick up their measured size.
- Because Yoga is declarative, swapping from XML to the builder pattern does not change how layout constraints are evaluated—the same Yoga nodes are reused.

## Using the Builder Pattern
1. Pick a layout container (`HLayoutBuilder` or `VLayoutBuilder`) and chain style setters such as `.gap(10.0f)` or `.justify("space-between")`.
2. Add children by nesting other builders. Example:
   ```cpp
   HLayoutBuilder row("row_0");
   row.justify("space-between")
      .add_child(InputTextBuilder("city_0", &city.name).flex(2.0f))
      .add_child(InputNumberBuilder("lat_0").bind_float(&city.latitude).flex(1.0f));
   ```
3. Build the widget tree (`row.build()`) and assign it as the panel root (`Panel::set_root_widget`). The base `WidgetBuilderBase` handles Yoga recalculation on build.
4. Use `.on_click(std::function<...>)` to attach callbacks or `.bind_*` helpers to connect fields from `AppData`.

## Yoga Reflow During Resize
- `Panel::render` captures `ImGui::GetContentRegionAvail()` on every frame and re-runs `update_layout`. This feeds Yoga the latest available width and height so rows stretch or wrap when the window size changes.
- Input widgets ask Yoga for their computed width (`YGNodeLayoutGetWidth`) when rendering, which is why table columns stay proportional even if you drag-resize the window.
- `PanelManager::update_all_layouts()` can be invoked to force a recalculation after you mutate bindings (for example when resetting data).

## DPI Scaling Workflows
- The builder demo adds a “Toggle DPI” button that cycles through 100%, 150%, and 200% scales. The callback updates `ImGuiIO::FontGlobalScale`, reapplies the base ImGui style with `ScaleAllSizes`, and calls `PanelManager::set_all_dpi_scale`.
- Every `Panel` tracks its baseline width/height and recomputes Yoga layouts when `set_dpi_scale` runs. This mirrors the way operating systems increase logical pixels on a high-DPI monitor.
- SDL emits `SDL_WINDOWEVENT_DISPLAY_CHANGED` when you drag the window between monitors. We sample `SDL_GetDisplayDPI`, derive a scale from the reported DPI, and coerce Yoga to recalculate—so the same pipeline handles both simulated and real DPI transitions.

## XML-Driven Panels and Callback Lookups
- The XML variant still lives in `city_data_panel.xml`. A trimmed excerpt:
  ```xml
  <hlayout id="row_0" justify="space-between" gap="10">
      <input id="city_0" type="text" bind="city_name_0" flex="2"/>
      <input id="lat_0" type="number" bind="city_lat_0" flex="1"/>
      <vlayout id="climate_0" flex="1" gap="2">
          <radio id="radio_0_0" text="Temperate" group="climate_0" value="3" bind="city_climate_0"/>
      </vlayout>
  </hlayout>
  ```
- `XmlParser::parse_element` dispatches by tag name using strategy objects. Each element becomes the corresponding widget from `WidgetFactory`, and shared Yoga attributes (`flex`, `gap`, `justify`) are copied into the `Widget::Style` struct before `setup_yoga_layout()` is called.
- Button callbacks are resolved by id: when the XML parser hits a `<button id="save_cities"/>`, it looks up `button_callbacks_["save_cities"]` and installs the functor. That mirrors how the builder version wires callbacks inline.
- Yoga behaves identically because the XML parser and the builder both populate the same widget types. Whether that node came from XML or C++ code, Yoga receives the same flex settings and recalculates when the panel updates.

## Building and Running
```bash
cmake -S . -B build
cmake --build build --target imgui_builder
cmake --build build --target imgui_oop_app
```
- Run `./build/imgui_builder` to explore the builder workflow, toggle DPI, and confirm Yoga reflow.
- Run `./build/imgui_oop_app` to validate the XML pipeline, hot reload, and shared Yoga behavior.

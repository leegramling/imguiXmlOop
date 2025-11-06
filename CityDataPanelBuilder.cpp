#include "CityDataPanelBuilder.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

CityDataPanelBuilder::CityDataPanelBuilder(AppData& data)
    : data_(data) {}

CityDataPanelBuilder& CityDataPanelBuilder::with_title(const std::string& title) {
    title_ = title;
    return *this;
}

CityDataPanelBuilder& CityDataPanelBuilder::with_size(float width, float height) {
    width_ = width;
    height_ = height;
    return *this;
}

CityDataPanelBuilder& CityDataPanelBuilder::with_max_rows(std::size_t rows) {
    max_rows_ = rows;
    return *this;
}

CityDataPanelBuilder& CityDataPanelBuilder::on_save(std::function<void()> callback) {
    on_save_ = std::move(callback);
    return *this;
}

CityDataPanelBuilder& CityDataPanelBuilder::on_reset(std::function<void()> callback) {
    on_reset_ = std::move(callback);
    return *this;
}

CityDataPanelBuilder& CityDataPanelBuilder::on_toggle_dpi(std::function<void()> callback) {
    on_toggle_dpi_ = std::move(callback);
    return *this;
}

std::unique_ptr<Panel> CityDataPanelBuilder::build() {
    ensure_minimum_city_entries(max_rows_);

    auto panel = std::make_unique<Panel>(title_, width_, height_);

    VLayoutBuilder root("main_layout");
    root.padding(10.0f).gap(15.0f);

    root.add_child(build_header_row());

    std::size_t row_count = std::min(max_rows_, data_.cities.size());
    for (std::size_t i = 0; i < row_count; ++i) {
        root.add_child(build_city_row(i, data_.cities[i]));
    }

    root.add_child(build_button_row());

    panel->set_root_widget(root.build());
    return panel;
}

void CityDataPanelBuilder::ensure_minimum_city_entries(std::size_t count) {
    if (data_.cities.size() >= count) {
        return;
    }

    data_.cities.reserve(count);
    while (data_.cities.size() < count) {
        data_.cities.emplace_back();
    }
}

std::unique_ptr<Widget> CityDataPanelBuilder::build_header_row() {
    HLayoutBuilder header("header_row");
    header.justify("space-between").align("center").gap(10.0f);

    header.add_child(LabelBuilder("header_city", "City").flex(2.0f).font_size("large").bold(true));
    header.add_child(LabelBuilder("header_lat", "Latitude").flex(1.0f).font_size("large").bold(true));
    header.add_child(LabelBuilder("header_lon", "Longitude").flex(1.0f).font_size("large").bold(true));
    header.add_child(LabelBuilder("header_elev", "Elevation (m)").flex(1.0f).font_size("large").bold(true));
    header.add_child(LabelBuilder("header_temp", "Avg Temp (°C)").flex(1.0f).font_size("large").bold(true));
    header.add_child(LabelBuilder("header_climate", "Climate").flex(1.0f).font_size("large").bold(true));

    return header.build();
}

std::unique_ptr<Widget> CityDataPanelBuilder::build_city_row(std::size_t index, CityData& city) {
    const std::string idx = std::to_string(index);

    HLayoutBuilder row("row_" + idx);
    row.justify("space-between").align("center").gap(10.0f);

    row.add_child(InputTextBuilder("city_" + idx, &city.name).flex(2.0f));
    row.add_child(InputNumberBuilder("lat_" + idx).bind_float(&city.latitude).flex(1.0f));
    row.add_child(InputNumberBuilder("lon_" + idx).bind_float(&city.longitude).flex(1.0f));
    row.add_child(InputNumberBuilder("elev_" + idx).bind_int(&city.elevation).flex(1.0f));
    row.add_child(InputNumberBuilder("temp_" + idx).bind_float(&city.avg_temp).flex(1.0f));
    row.add_child(build_climate_column("climate_" + idx, &city.climate_zone));

    return row.build();
}

std::unique_ptr<Widget> CityDataPanelBuilder::build_climate_column(const std::string& base_id, int* binding) {
    VLayoutBuilder climate(base_id);
    climate.flex(1.0f).gap(2.0f);

    const std::array<std::pair<const char*, int>, 3> options = {{
        {"Temperate", 3},
        {"Tropical", 1},
        {"Arid", 2},
    }};

    int option_index = 0;
    for (const auto& option : options) {
        const std::string radio_id = base_id + "_" + std::to_string(option_index++);
        climate.add_child(RadioButtonBuilder(radio_id, option.first, base_id, option.second, binding));
    }

    return climate.build();
}

std::unique_ptr<Widget> CityDataPanelBuilder::build_button_row() {
    HLayoutBuilder row("button_row");
    row.justify("center").gap(15.0f).margin(10.0f);

    row.add_child(ButtonBuilder("toggle_dpi", "Toggle DPI").on_click(on_toggle_dpi_));
    row.add_child(ButtonBuilder("save_cities", "Save City Data").variant("primary").on_click(on_save_));
    row.add_child(ButtonBuilder("reset_cities", "Reset Data").variant("danger").on_click(on_reset_));

    return row.build();
}

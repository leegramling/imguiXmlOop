#pragma once

#include "AppData.h"
#include "Panel.h"
#include "UiBuilder.h"
#include <cstddef>
#include <functional>
#include <memory>
#include <string>

/**
 * @brief Builder responsible for constructing the City Data panel UI tree
 */
class CityDataPanelBuilder {
public:
    explicit CityDataPanelBuilder(AppData& data);

    CityDataPanelBuilder& with_title(const std::string& title);
    CityDataPanelBuilder& with_size(float width, float height);
    CityDataPanelBuilder& with_max_rows(std::size_t rows);
    CityDataPanelBuilder& on_save(std::function<void()> callback);
    CityDataPanelBuilder& on_reset(std::function<void()> callback);
    CityDataPanelBuilder& on_toggle_dpi(std::function<void()> callback);

    std::unique_ptr<Panel> build();

private:
    AppData& data_;
    std::string title_ = "City Data Grid";
    float width_ = 900.0f;
    float height_ = 600.0f;
    std::size_t max_rows_ = 6;
    std::function<void()> on_save_;
    std::function<void()> on_reset_;
    std::function<void()> on_toggle_dpi_;

    void ensure_minimum_city_entries(std::size_t count);
    std::unique_ptr<Widget> build_title_section();
    std::unique_ptr<Widget> build_city_row(std::size_t index, CityData& city);
    std::unique_ptr<Widget> build_climate_column(const std::string& base_id, int* binding);
    std::unique_ptr<Widget> build_button_row();
};

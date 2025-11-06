#pragma once

#include "Widget.h"
#include <functional>
#include <memory>
#include <string>
#include <utility>

/**
 * @brief CRTP base providing fluent setters for widget properties and style
 */
template <typename Derived, typename WidgetType>
class WidgetBuilderBase {
public:
    using widget_type = WidgetType;

    explicit WidgetBuilderBase(std::unique_ptr<WidgetType> widget)
        : widget_(std::move(widget)) {}

    WidgetType* get() { return widget_.get(); }
    const WidgetType* get() const { return widget_.get(); }

    Derived& width(float value) {
        if (widget_) widget_->set_width(value);
        return self();
    }

    Derived& height(float value) {
        if (widget_) widget_->set_height(value);
        return self();
    }

    Derived& flex(float value) {
        if (widget_) widget_->set_flex(value);
        return self();
    }

    Derived& margin(float value) {
        if (widget_) widget_->get_style().margin = value;
        return self();
    }

    Derived& padding(float value) {
        if (widget_) widget_->get_style().padding = value;
        return self();
    }

    Derived& gap(float value) {
        if (widget_) widget_->get_style().gap = value;
        return self();
    }

    Derived& justify(const std::string& value) {
        if (widget_) widget_->get_style().justify = value;
        return self();
    }

    Derived& align(const std::string& value) {
        if (widget_) widget_->get_style().align = value;
        return self();
    }

    Derived& align_self(const std::string& value) {
        if (widget_) widget_->get_style().align_self = value;
        return self();
    }

    Derived& font_size(const std::string& value) {
        if (widget_) widget_->get_style().font_size = value;
        return self();
    }

    Derived& bold(bool value) {
        if (widget_) widget_->get_style().bold = value;
        return self();
    }

    Derived& text_color(const std::string& value) {
        if (widget_) widget_->get_style().text_color = value;
        return self();
    }

    Derived& background_color(const std::string& value) {
        if (widget_) widget_->get_style().bg_color = value;
        return self();
    }

    Derived& variant(const std::string& value) {
        if (widget_) widget_->get_style().variant = value;
        return self();
    }

    Derived& disabled(bool value) {
        if (widget_) widget_->get_style().disabled = value;
        return self();
    }

    Derived& stretch(bool value) {
        if (widget_) widget_->get_style().stretch = value;
        return self();
    }

    Derived& wrap(bool value) {
        if (widget_) widget_->get_style().wrap = value;
        return self();
    }

    std::unique_ptr<Widget> build() {
        if (widget_) {
            Widget* base_widget = static_cast<Widget*>(widget_.get());
            base_widget->setup_yoga_layout();
        }
        return std::move(widget_);
    }

protected:
    Derived& self() { return static_cast<Derived&>(*this); }
    WidgetType* widget() { return widget_.get(); }
    const WidgetType* widget() const { return widget_.get(); }

    std::unique_ptr<WidgetType> widget_;
};

/**
 * @brief Container-specific builder base adding child management helpers
 */
template <typename Derived, typename WidgetType>
class ContainerBuilderBase : public WidgetBuilderBase<Derived, WidgetType> {
public:
    explicit ContainerBuilderBase(std::unique_ptr<WidgetType> widget)
        : WidgetBuilderBase<Derived, WidgetType>(std::move(widget)) {}

    Derived& add_child(std::unique_ptr<Widget> child) {
        if (this->widget() && child) {
            this->widget()->add_child(std::move(child));
        }
        return this->self();
    }

    template <typename Builder>
    Derived& add_child(Builder&& builder) {
        return add_child(std::forward<Builder>(builder).build());
    }
};

// -----------------------------------------------------------------------------
// Concrete builders
// -----------------------------------------------------------------------------

class HLayoutBuilder : public ContainerBuilderBase<HLayoutBuilder, HLayoutWidget> {
public:
    explicit HLayoutBuilder(const std::string& id)
        : ContainerBuilderBase<HLayoutBuilder, HLayoutWidget>(WidgetFactory::create_hlayout(id)) {}
};

class VLayoutBuilder : public ContainerBuilderBase<VLayoutBuilder, VLayoutWidget> {
public:
    explicit VLayoutBuilder(const std::string& id)
        : ContainerBuilderBase<VLayoutBuilder, VLayoutWidget>(WidgetFactory::create_vlayout(id)) {}
};

class LabelBuilder : public WidgetBuilderBase<LabelBuilder, LabelWidget> {
public:
    LabelBuilder(const std::string& id, const std::string& text)
        : WidgetBuilderBase<LabelBuilder, LabelWidget>(WidgetFactory::create_label(id, text)) {}

    LabelBuilder& text(const std::string& value) {
        if (widget()) widget()->set_text(value);
        return self();
    }
};

class InputTextBuilder : public WidgetBuilderBase<InputTextBuilder, InputTextWidget> {
public:
    InputTextBuilder(const std::string& id, std::string* value = nullptr)
        : WidgetBuilderBase<InputTextBuilder, InputTextWidget>(WidgetFactory::create_input_text(id, value)) {}

    InputTextBuilder& bind(std::string* value) {
        if (widget()) widget()->bind_value(value);
        return self();
    }
};

class InputNumberBuilder : public WidgetBuilderBase<InputNumberBuilder, InputNumberWidget> {
public:
    explicit InputNumberBuilder(const std::string& id)
        : WidgetBuilderBase<InputNumberBuilder, InputNumberWidget>(WidgetFactory::create_input_number(id)) {}

    InputNumberBuilder& bind_float(float* value) {
        if (widget()) widget()->bind_float_value(value);
        return self();
    }

    InputNumberBuilder& bind_int(int* value) {
        if (widget()) widget()->bind_int_value(value);
        return self();
    }
};

class RadioButtonBuilder : public WidgetBuilderBase<RadioButtonBuilder, RadioButtonWidget> {
public:
    RadioButtonBuilder(const std::string& id,
                       const std::string& text,
                       const std::string& group,
                       int value,
                       int* selected = nullptr)
        : WidgetBuilderBase<RadioButtonBuilder, RadioButtonWidget>(
              WidgetFactory::create_radio_button(id, text, group, value, selected)) {}

    RadioButtonBuilder& bind_selected(int* selected) {
        if (widget()) widget()->bind_selected(selected);
        return self();
    }
};

class ButtonBuilder : public WidgetBuilderBase<ButtonBuilder, ButtonWidget> {
public:
    ButtonBuilder(const std::string& id, const std::string& text)
        : WidgetBuilderBase<ButtonBuilder, ButtonWidget>(WidgetFactory::create_button(id, text)) {}

    ButtonBuilder& on_click(std::function<void()> callback) {
        if (widget()) widget()->set_callback(std::move(callback));
        return self();
    }

    ButtonBuilder& text(const std::string& value) {
        if (widget()) widget()->set_text(value);
        return self();
    }
};

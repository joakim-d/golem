#include <interfaces/gui/views/instruments/PulseConfigurationView.h>

#include <interfaces/graphics/GraphicsFactory.h>
#include <interfaces/graphics/Widget.h>

#include <interfaces/gui/style/Style.h>

#include <golem/domain/Song.h>

namespace gui::views
{

PulseConfigurationView::PulseConfigurationView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& view_widget,
    size_t instrument_index,
    std::shared_ptr<domain::Song> song_model)
    : m_view_widget(view_widget)
    , m_duty_cycle(graphics_factory.createWidget("duty_cycle")
                       .fill(view_widget.entity, 32)
                       .addZPosition(view_widget.z()))
{
    m_duty_cycle.onDraw(
        [this, instrument_index, song_model](graphics::Painter& painter)
        {
            // const auto height = m_duty_cycle.height();
            // const auto width = m_duty_cycle.width();
            // auto duty_cycle =
            // song_model->getInstrument(instrument_index).pulse().dutyCycle();
            // SDL_Rect rectangle;
            // rectangle.x = 0;
            // rectangle.w = width;

            // switch (duty_cycle) {
            // case domain::PulseInstrument::DutyCycle::Percent_12_5:
            //     rectangle.h = height / 8;
            //     break;
            // case domain::PulseInstrument::DutyCycle::Percent_25:
            //     rectangle.h = height / 4;
            //     break;
            // case domain::PulseInstrument::DutyCycle::Percent_50:
            //     rectangle.h = height / 2;
            //     break;
            // case domain::PulseInstrument::DutyCycle::Percent_75:
            //     rectangle.h = (height * 3) / 4;
            //     break;
            // }
            // rectangle.y = height - rectangle.h;

            // painter.drawRectangle(rectangle, Style::defaultColor());
        });

    m_duty_cycle.onPressed(
        ecs::Button::Left,
        [song_model, instrument_index](ecs::Button, int, int)
        {
            // const auto duty_cycle
            //     = song_model->getInstrument(instrument_index)
            //           .pulse()
            //           .dutyCycle();

            // song_model->getInstrument(instrument_index)
            //     .pulse()
            //     .setDutyCycle(static_cast<domain::PulseInstrument::DutyCycle>(
            //         (int(duty_cycle) + 1) % 4));
        });
}

PulseConfigurationView::~PulseConfigurationView()
{
    m_view_widget.die();
    m_duty_cycle.die();
}

void PulseConfigurationView::setZPosition(int z_position)
{
    m_view_widget.addZPosition(z_position);
    m_duty_cycle.addZPosition(z_position + 1);
}

void PulseConfigurationView::fill(ecs::Entity entity, int margin)
{
    m_view_widget.fill(entity, margin);
}

}

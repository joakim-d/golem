#include <gui/views/PianoKeyboardView.h>
#include <gui/views/PianoRollView.h>

#include <gui/views/PianoKeyboardView.h>

#include <graphics/GraphicsFactory.h>

#include <ecs/Entity.h>

namespace gui::views {

PianoRollView::PianoRollView(
    graphics::GraphicsFactory& graphics_factory,
    const graphics::Widget& view_widget)
{
    auto first_keyboard
        = graphics_factory.createWidget("keyboard")
              .addSize(64, 168)
              .anchorTop(view_widget.entity, ecs::Top)
              .anchorLeft(view_widget.entity, ecs::Left);

    PianoKeyboardView piano_view { graphics_factory, first_keyboard, 7 };

    ecs::Entity previous_entity = first_keyboard.entity;

    std::shared_ptr<int> y_offset = std::make_shared<int>(0);

    for (unsigned int octave = 6; octave >= 2; --octave) {
        auto keyboard
            = graphics_factory.createWidget("keyboard")
                  .addSize(64, 168)
                  .anchorTop(previous_entity, ecs::Bottom)
                  .anchorLeft(view_widget.entity, ecs::Left);

        PianoKeyboardView piano_view { graphics_factory, keyboard, octave };

        previous_entity = keyboard.entity;
    }
}

}

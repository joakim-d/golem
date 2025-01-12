#include <gui/views/SequencerView.h>

#include <gui/views/PhraseView.h>
#include <gui/views/PianoKeyboardView.h>

#include <graphics/GraphicsFactory.h>

namespace gui::views {

SequencerView::SequencerView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& sequencer_view_widget)
{
    auto first_keyboard
        = graphics_factory.createWidget("keyboard")
              .addSize(64, 168)
              .anchorTop(sequencer_view_widget.entity, ecs::Top)
              .anchorLeft(sequencer_view_widget.entity, ecs::Left);

    PianoKeyboardView piano_view { graphics_factory, first_keyboard, 7 };

    ecs::Entity previous_entity = first_keyboard.entity;

    for (unsigned int octave = 6; octave >= 2; --octave) {
        auto keyboard
            = graphics_factory.createWidget("keyboard")
                  .addSize(64, 168)
                  .anchorTop(previous_entity, ecs::Bottom)
                  .anchorLeft(sequencer_view_widget.entity, ecs::Left);

        PianoKeyboardView piano_view { graphics_factory, keyboard, octave };

        previous_entity = keyboard.entity;
    }

    std::vector<graphics::Widget> phrase_views;
    for (int phrase_index = 0; phrase_index < 16; ++phrase_index) {
        auto phrase_widget
            = graphics_factory.createWidget("phrase_view")
                  .addSize(256, 0)
                  .anchorTop(sequencer_view_widget.entity, ecs::Top)
                  .anchorBottom(sequencer_view_widget.entity, ecs::Bottom)
                  .addPosition(65 + (phrase_index * (256 + 2)), 0);

        PhraseView phrase_view { graphics_factory, phrase_widget };

        phrase_views.push_back(phrase_widget);
    }
}

}

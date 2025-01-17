#include <gui/views/PatternView.h>

#include <graphics/Widget.h>

#include <model/Song.h>

namespace gui::views {

PatternView::PatternView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& view_widget,
    std::shared_ptr<model::Song> song_model,
    size_t track_index,
    size_t phrase_index)
{
    const int width = view_widget.width();
    const int height = view_widget.height();

    view_widget.onDraw(
        [width, height, song_model, track_index, phrase_index](
            graphics::Painter& painter) {
            SDL_Rect rectangle;
            auto model_phrase_index = song_model->getTrack(track_index).phraseIndex(phrase_index);
            if (!model_phrase_index.has_value()) {
                return;
            }
            auto& phrase = song_model->getPhrase(*model_phrase_index);
            std::optional<model::Note> lowest_note;
            std::optional<model::Note> highest_note;

            for (int i = 0; i < 16; ++i) {
                auto current_note = phrase.note(i);
                if (!current_note.has_value())
                    continue;
                if (!lowest_note.has_value() || *current_note < *lowest_note) {
                    lowest_note = *current_note;
                }
                if (!highest_note.has_value() || *current_note > *highest_note) {
                    highest_note = *current_note;
                }
            }
            int scale;
            if (!lowest_note.has_value() && !highest_note.has_value()) {
                return;
            }
            if (!lowest_note.has_value()
                || !highest_note.has_value()
                || int(*highest_note) - int(*lowest_note) < 7) {
                scale = 8;
            } else {
                scale = int(*highest_note) - int(*lowest_note) + 1;
            }

            const int note_height = height / scale;
            const int note_width = width / 16;

            for (int i = 0; i < 16; ++i) {
                auto current_note = phrase.note(i);
                if (!current_note.has_value()) {
                    continue;
                }
                rectangle.x = note_width * i;
                rectangle.y = height - ((int(*current_note) - int(*lowest_note) + 1) * note_height);
                rectangle.h = note_height;
                rectangle.w = note_width;

                painter.fillRectangle(
                    rectangle,
                    graphics::core::Color::fromName("red"));
            }
        });
}
}

#include <interfaces/gui/views/PatternView.h>

#include <interfaces/graphics/Widget.h>

#include <golem/domain/Song.h>
#include <golem/use_cases/ProjectUseCases.h>

namespace gui::views
{

PatternView::PatternView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& view_widget,
    use_cases::ProjectUseCases& project_use_cases,
    size_t track_index,
    size_t phrase_index)
{
    const int width = view_widget.width();
    const int height = view_widget.height();

    view_widget.onDraw(
        [width, height, this, track_index, phrase_index,
         &project_use_cases](graphics::Painter& painter)
        {
            SDL_Rect rectangle;

            auto phrase = project_use_cases.get_phrase_view.execute(
                0, track_index, phrase_index);

            if (phrase.has_value())
            {
                return;
            }

            std::optional<domain::Note> lowest_note;
            std::optional<domain::Note> highest_note;

            for (int i = 0; i < phrase->size(); ++i)
            {
                auto current_note = phrase->note(i);
                if (!current_note.has_value())
                    continue;
                if (!lowest_note.has_value()
                    || current_note->frequency() < lowest_note->frequency())
                {
                    lowest_note = *current_note;
                }
                if (!highest_note.has_value()
                    || current_note->frequency() > highest_note->frequency())
                {
                    highest_note = *current_note;
                }
            }
            int scale;
            if (!lowest_note.has_value() && !highest_note.has_value())
            {
                return;
            }
            if (!lowest_note.has_value() || !highest_note.has_value()
                || int(highest_note->frequency())
                        - int(lowest_note->frequency())
                    < 7)
            {
                scale = 8;
            }
            else
            {
                scale = int(highest_note->frequency())
                    - int(lowest_note->frequency()) + 1;
            }

            const int note_height = height / scale;
            const int note_width = width / phrase->size();

            for (int i = 0; i < phrase->size(); ++i)
            {
                auto current_note = phrase->note(i);
                if (!current_note.has_value())
                {
                    continue;
                }
                rectangle.x = note_width * i;
                rectangle.y = height
                    - ((int(current_note->frequency())
                        - int(lowest_note->frequency()) + 1)
                       * note_height);
                rectangle.h = note_height;
                rectangle.w = note_width * current_note->duration();

                painter.fillRectangle(
                    rectangle, graphics::core::Color::fromName("red"));
            }
        });
}
}

#include <interfaces/GuiState.h>

namespace gui {

GuiState& GuiState::instance()
{
	static GuiState instance;
	return instance;
}

GuiState::GuiState()
	: selected_color(
		  ImColor {
			  0xb9,
			  0x00,
			  0xff,
			  64})
	, idle_color(
		  ImColor {
			  0xea,
			  0xb2,
			  0xff,
			  64})
	, hovered_color(
		  ImColor {
			  0xb9,
			  0x00,
			  0xff,
			  128})
	, song_index(0)
	, track_index(0)
	, instrument_index(0)
	, pattern_index(0)
{
}

}

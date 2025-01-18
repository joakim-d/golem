#pragma once

#include <model/Note.h>

unsigned noteToPulsePeriod(model::NoteFrequency note);
unsigned noteToWavePeriod(model::NoteFrequency note);

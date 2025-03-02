#pragma once

#include <golem/domain/Note.h>

unsigned noteToPulsePeriod(domain::NoteFrequency note);
unsigned noteToWavePeriod(domain::NoteFrequency note);

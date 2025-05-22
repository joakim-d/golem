#pragma once

#include <cstddef>
#include <functional>
#include <optional>

namespace use_cases {

class ProjectUseCases;

}

namespace gui {

class PatternView {
public:
    PatternView(
        use_cases::ProjectUseCases& project_use_cases,
		std::optional<size_t> pattern_index,
		const char* pattern_id,
		bool track_pattern = false,
		std::function<void()> on_drag = {});
};

}

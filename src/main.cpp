#include <gui/MainWindow.h>

#include <iostream>

int main(int argc, char* argv[])
{
    auto window = gui::MainWindow::create(
        800,
        600,
        "[GoleM]");

    if (!window.has_value()) {
        std::cerr << "Failed to create MainWindow" << std::endl;
        return -1;
    }

    window->execute();

    return 0;
}

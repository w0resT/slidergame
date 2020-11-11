#include "gui/gui.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!gui::setup_window(hInstance))
		return EXIT_FAILURE;

	gui::main();

	gui::release_window(hInstance);

	return EXIT_SUCCESS;
}

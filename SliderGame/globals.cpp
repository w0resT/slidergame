#include "globals.h"

namespace globals
{
	LPDIRECT3D9 d3d = NULL;
	LPDIRECT3DDEVICE9 d3d_device = NULL;
	D3DPRESENT_PARAMETERS d3d_params = {};

	bool exit = false;
}
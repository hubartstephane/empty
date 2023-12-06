#include "Ludum48PCH.h"
#include "Ludum48Game.h"

int main(int argc, char ** argv, char ** env)
{
	WindowCreateParams create_params;
	create_params.width = 500;
	create_params.height = 500;

	return RunGame<LudumGame>(argc, argv, env, create_params);
}
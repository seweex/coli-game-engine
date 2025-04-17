#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include "Example/Blocks.hxx"
#include "Example/Player.hxx"

int main() 
{
	try {
		Blocks::GameInstance game;

		game.load();
		game.run();
		game.save();
	}
	catch (std::exception const& exc) {
		std::cerr << "Error: " << exc.what() << std::endl;
	}
}
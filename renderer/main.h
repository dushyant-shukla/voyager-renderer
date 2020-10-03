#pragma once

#include <stdexcept>

#include "Application.h"

extern vr::Application* vr::CreateApplication();

int main(int argc, char** argv)
{
	try
	{
		vr::Application* application = vr::CreateApplication();
		application->Run();
		delete application;
		return EXIT_SUCCESS;
	}
	catch (std::runtime_error& error)
	{
		return EXIT_FAILURE;
	}
}

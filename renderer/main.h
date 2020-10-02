#pragma once

#include "Application.h"

extern vr::Application* vr::CreateApplication();

int main(int argc, char** argv)
{
	vr::Application* application = vr::CreateApplication();
	application->Run();
	delete application;
}

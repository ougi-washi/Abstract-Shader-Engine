#pragma once

#include <functional> // TODO: remove

namespace as
{
	enum key_type
	{
		// following GLFW numbers	
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		RIGHT = 262,
		LEFT = 263,
		DOWN = 264,
		UP = 265,

		NUMPAD0 = 320,
		NUMPAD1 = 321,
		NUMPAD2 = 322,
		NUMPAD3 = 323,
		NUMPAD4 = 324,
		NUMPAD5 = 325,
		NUMPAD6 = 326,
		NUMPAD7 = 327,
		NUMPAD8 = 328,
		NUMPAD9 = 329,

		NUM0 = 48,
		NUM1 = 49,
		NUM2 = 50,
		NUM3 = 51,
		NUM4 = 52,
		NUM5 = 53,
		NUM6 = 54,
		NUM7 = 55,
		NUM8 = 56,
		NUM9 = 57,

		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301
	};

	enum input_event_type
	{
		PRESSED,
		RELEASED,
		REPEATED
	};

	struct key_params
	{
		as::key_type type;
		as::input_event_type input_event;
	};

	struct input_handle
	{
		// input
		//void (*on_input)(const key_params&);
		std::function<void(const as::key_params&)> on_receive_input = std::function<void(const as::key_params&)>{};
	};
};
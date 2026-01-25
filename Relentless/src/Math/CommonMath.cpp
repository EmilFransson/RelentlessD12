#include "CommonMath.h"

namespace Relentless
{
	namespace Math
	{
		Color MakeFromColorTemperature(float aTemp) noexcept
		{
			constexpr float MAX_TEMPERATURE = 15000.0f;
			constexpr float MIN_TEMPERATURE = 1000.0f;
			aTemp = Clamp(aTemp, MIN_TEMPERATURE, MAX_TEMPERATURE);

			//[Krystek85] Algorithm works in the CIE 1960 (UCS) space,
			float u = (0.860117757f + 1.54118254e-4f * aTemp + 1.28641212e-7f * aTemp * aTemp) / (1.0f + 8.42420235e-4f * aTemp + 7.08145163e-7f * aTemp * aTemp);
			float v = (0.317398726f + 4.22806245e-5f * aTemp + 4.20481691e-8f * aTemp * aTemp) / (1.0f - 2.89741816e-5f * aTemp + 1.61456053e-7f * aTemp * aTemp);

			//UCS to xyY
			float x = 3.0f * u / (2.0f * u - 8.0f * v + 4.0f);
			float y = 2.0f * v / (2.0f * u - 8.0f * v + 4.0f);
			float z = 1.0f - x - y;

			//xyY to XYZ
			float Y = 1.0f;
			float X = Y / y * x;
			float Z = Y / y * z;

			// XYZ to RGB - BT.709
			float R = 3.2404542f * X + -1.5371385f * Y + -0.4985314f * Z;
			float G = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
			float B = 0.0556434f * X + -0.2040259f * Y + 1.0572252f * Z;

			return Color(R, G, B);
		}
	}
}



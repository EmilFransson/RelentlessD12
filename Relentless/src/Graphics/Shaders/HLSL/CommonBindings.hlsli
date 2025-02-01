#pragma once

#include "../Interop/ShaderInterop.h"

ConstantBuffer<ViewUniforms> cView : 						register(b2);

//Static samplers
SamplerState sLinearWrap :								  	register(s0, space1);
SamplerState sLinearClamp :								 	register(s1, space1);
SamplerState sLinearBorder :								register(s2, space1);
SamplerState sPointWrap :								   	register(s3, space1);
SamplerState sPointClamp :								  	register(s4, space1);
SamplerState sPointBorder :								 	register(s5, space1);
SamplerState sAnisoWrap :								   	register(s6, space1);
SamplerState sAnisoClamp :								  	register(s7, space1);
SamplerState sAnisoBorder :									register(s8, space1);
SamplerState sMaterialSampler :							 	register(s9, space1);
SamplerComparisonState sLinearClampComparisonGreater :		register(s10, space1);
SamplerComparisonState sLinearWrapComparisonGreater :		register(s11, space1);
#include "earth_renderer.h"
#include "dem.h"

SLIB_MAP_NAMESPACE_START

void MapEarthRenderer::_runThreadControl()
{
	m_timeLastThreadControl = Time::now();
	while (!Thread::isStoppingCurrent()) {
		Time now = Time::now();
		m_dtimeThreadControl = (sl_uint32)(Math::clamp((now - m_timeLastThreadControl).getMillisecondsCount(), _SI64(0), _SI64(1000)));
		m_timeCurrentThreadControl = now;
		if (m_flagStartedRendering) {
			_runThreadControlStep();
		}
		Time now2 = Time::now();
		sl_uint32 dt = (sl_uint32)(Math::clamp((now2 - now).getMillisecondsCount(), _SI64(0), _SI64(1000)));
		if (dt < 20u) {
			Thread::sleep(20u - dt);
		}
		m_timeLastThreadControl = now;
	}
}

void MapEarthRenderer::_runThreadControlStep()
{
	m_camera->stepMotions((sl_real)m_dtimeThreadControl);
}

SLIB_MAP_NAMESPACE_END

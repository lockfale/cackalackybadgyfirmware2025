#pragma once

namespace BADGE_STATUS
{
	void update( void );
    void taskCallback();

	bool isRunning( void );
	void doStop( void );
	void doStart( void );
};
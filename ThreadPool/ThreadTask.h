/**

	@file      ThreadTask.h
	@brief
	@details   ~
	@author    Yuhei kashima
	@date      28.02.2025

**/

#ifndef _THREADTASK_
#define _THREADTASK_

namespace myLib {
	class ThreadTask {
	public:
		ThreadTask();
		~ThreadTask();

		virtual void Execute() {}
	};
}

#endif // !_THREADTASK_
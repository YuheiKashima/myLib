/**

	@file      ThreadTask.h
	@brief
	@details   ~
	@author    Yuhei kashima
	@date      28.02.2025

**/

#ifndef _THREADTASK_
#define _THREADTASK_

#include <thread>

namespace myLib {

	class ThreadTask {
	public:
		ThreadTask();
		~ThreadTask();

		virtual void Execute(std::thread::id _workingId) = 0;
	protected:

	private:

	};
}

#endif // !_THREADTASK_
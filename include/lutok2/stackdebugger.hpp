#ifndef LUTOK2_STACKDEBUGGER_H
#define LUTOK2_STACKDEBUGGER_H

namespace lutok2 {
	class Stack;

	class StackDebugger {
	private:
		Stack * stack;
		int stackItems;
	public:
		StackDebugger(Stack * stack){
			this->stack = stack;
			stackItems = stack->getTop();
		}
		~StackDebugger(){
			if (stackItems != stack->getTop()){
				throw std::exception("Stack corruption detected!");
			}
		}
		void setReturnValues(int count){
			stackItems += count;
		}
	};
};

#endif
#include "common.hpp"

namespace lutok2 {


	class error : public std::runtime_error {
	public:
		explicit error(const std::string& message) : std::runtime_error(message){
		}

		virtual ~error(void) throw();
	};

}
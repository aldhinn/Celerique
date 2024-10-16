/*

File: ./include/celerique/abstracts.h
Author: Aldhinn Espinas
Description: This header file contains common abstract type declarations.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_ABSTRACTS_HEADER_FILE)
#define CELERIQUE_ABSTRACTS_HEADER_FILE

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <memory>

namespace celerique {
    /// @brief An interface to an object that contains the update data.
    class IUpdateData {
    public:
        /// @brief Pure virtual constructor.
        virtual ~IUpdateData() = 0;
    };

    /// @brief An interface to a stateful object.
    class IStateful {
    public:
        /// @brief Updates the state.
        /// @param ptrArg The shared pointer to the update data container.
        virtual void onUpdate(::std::shared_ptr<IUpdateData> ptrUpdateData = nullptr) = 0;

    public:
        /// @brief Pure virtual constructor.
        virtual ~IStateful() = 0;
    };
}
#endif
// End C++ Only Region

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.
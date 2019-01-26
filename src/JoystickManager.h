#ifndef JOYSTICK_MANAGER_H
#define JOYSTICK_MANAGER_H

#include <memory>
#include <filesystem>

#include "Joystick.h"

class JoystickManager {
    using JoystickID = int;
    public:
        void addJoystick(std::shared_ptr<Joystick> joystick);
        void update();

    private:
        JoystickID next_id_ = 0;
        std::map<JoystickID, int> glfw_ids_;
        std::map<JoystickID, std::shared_ptr<Joystick>> joysticks_;
};

#endif

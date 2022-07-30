#ifndef GAMEPAD_H
#define GAMEPAD_H

namespace gamepad {

    bool init();
    void stop();



    double get_axis(int axis);
    bool get_button(int button);

}

#endif
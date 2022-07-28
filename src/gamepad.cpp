#include <gamepad.h>

#include <fcntl.h>
#include <cstdint>
#include <thread>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <logger.h>

#include <drone.h>

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

typedef std::int8_t u8;
typedef std::int16_t s16;
typedef std::uint32_t u32;

static int fd;
static std::thread polling_thread;
static double axises[24];
// static void (*std::func) (double) axis_callbacks[24];
static bool buttons[24];
// static void (*std::func)() button_on_callbacks[24];
// static void (*std::func)() button_off_callbacks[24];
static bool running;

struct js_event {
    u32 time;
    s16 value;
    u8 type;
    u8 number;
};


static js_event event;
static size_t event_s = sizeof(event);

static void polling_thread_function(){
    
    while(running){

        ssize_t bytes = read(fd, &event, event_s);
        #ifdef GAMEPAD_DEBUG
            printf("| Time: %10d | Event Type: %#3d | Number: %2d | Value: %8d |\n", event.time, event.type, event.number, event.value);
        #endif
        // std::cout << "fuck yo\n";
        // if(bytes > 0){
        //     printf("| Time: %10d | Event Type: %#3d | Number: %2d | Value: %8d |\n", event.time, event.type, event.number, event.value);
        
        if(bytes < 0) {
            logger::info("Lost contact with joystick");
            // drone::set_ctrller_connected_flag(false);
            fd = -1;
            while(fd < 0){
                fd = open("/dev/input/js0", O_RDONLY);
                usleep(10000);
            }
            // drone::set_ctrller_connected_flag(true);
            logger:info("Restablished contact with joystick.");
        }

        if((event.type & JS_EVENT_AXIS) == JS_EVENT_AXIS){
            axises[event.number] = ((double)(event.value)) / 32767.0;
        }else if((event.type & JS_EVENT_BUTTON) ==JS_EVENT_BUTTON){
            buttons[event.number] = event.value;
        }
        // }

        usleep(10000);
    }
}
bool gamepad::init(){
    if(!running){
        fd = open("/dev/input/js0", O_RDONLY);
        if(fd >= 0){
            drone::set_ctrller_connected_flag(true);
            running = true;
            polling_thread = std::thread(polling_thread_function);
            return true;
        }else{
            return false;
        }
    }
    return false;
}

void gamepad::stop(){
    if(running){
        running = false;
        polling_thread.join();
        close(fd);
    }
}


double gamepad::get_axis(int axis){
    return axises[axis];
}

bool gamepad::get_button(int button){
    return buttons[button];
}

